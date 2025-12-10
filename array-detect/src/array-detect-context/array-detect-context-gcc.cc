#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <execinfo.h>
#include <string.h>

#include "array-detect-context-gcc.hh"
#include "array-detect-context-gcc-interface.hh"

namespace array_detect_ns {

// Global GCC-specific context instance - 复杂对象，不是指针
ArrayDetectContextGcc gArrayDetectContextGcc;

// Initialize GCC context
void initGccContext(AD_FUNC_ARGS) {
  AD_ARGS_WARN_DENY;
  // 直接初始化 vec 容器
  gcc_ctx.stack_frames.create(0);
}

// Cleanup GCC context
void deinitGccContext(AD_FUNC_ARGS) {
  clearStackFrames(AD_ARGS);
}

namespace controlflow {

// Stack frame management functions
void pushStackFrame(AD_FUNC_ARGS, uint64_t frame_id) {
  AD_ARGS_WARN_DENY;
  gcc_ctx.stack_frames.safe_push(frame_id);
}

void popStackFrame(AD_FUNC_ARGS) {
  AD_ARGS_WARN_DENY;
  if (!gcc_ctx.stack_frames.is_empty()) {
    gcc_ctx.stack_frames.pop();
  }
}

} // namespace controlflow

void clearStackFrames(AD_FUNC_ARGS) {
  AD_ARGS_WARN_DENY;
  gcc_ctx.stack_frames.truncate(0);
}

size_t getStackDepth(AD_FUNC_ARGS) {
  AD_ARGS_WARN_DENY;
  return gcc_ctx.stack_frames.length();
}

void getCurrentFrame(AD_FUNC_ARGS, uint64_t &result, bool &is_exists) {
  AD_ARGS_WARN_DENY;
  if (gcc_ctx.stack_frames.is_empty()) {
    is_exists = false;
    return ;
  }
  result = gcc_ctx.stack_frames.last();
  is_exists = true;
}

bool isStackEmpty(AD_FUNC_ARGS) {
  AD_ARGS_WARN_DENY;
  return gcc_ctx.stack_frames.is_empty();
}

// Debug output functions
void printStackFrames(AD_FUNC_ARGS) {
  AD_DEBUG_PRINT("Stack frames (depth: %zu)", getStackDepth(AD_ARGS));
  for (size_t i = 0; i < gcc_ctx.stack_frames.length(); ++i) {
    AD_DEBUG_PRINT("\t[%zu]: 0x%lx", i, (unsigned long)gcc_ctx.stack_frames[i]);
  }
}

void printCurrentFrame(AD_FUNC_ARGS) {
  AD_DEBUG_PRINT("Stack frames (depth: %zu)", getStackDepth(AD_ARGS));
  if (!isStackEmpty(AD_ARGS)) {
    bool is_exists;
    uint64_t v;
    getCurrentFrame(AD_ARGS, v, is_exists);
    AD_DEBUG_PRINT("Current frame: 0x%lx", ((unsigned long) (is_exists ? v : 0)));
  } else {
    AD_DEBUG_PRINT("Current frame: <null>");
  }
}

// Enhanced debug with source code locations
void printStackFramesWithSource(AD_FUNC_ARGS) {
  AD_DEBUG_PRINT("Call stack with source locations (depth: %zu):", getStackDepth(AD_ARGS));
  for (size_t i = 0; i < gcc_ctx.stack_frames.length(); ++i) {
    uint64_t frame_addr = gcc_ctx.stack_frames[i];
    const char* source_info;
    bool is_valid;
    ArrayDetectErrorCode err = resolveFrameAddressToSource(AD_ARGS, frame_addr, source_info, is_valid);
    (void)err; // 忽略错误，仅用于调试输出

    if (is_valid) {
      AD_DEBUG_PRINT("  [%zu]: 0x%016lx -> %s", i, (unsigned long)frame_addr, source_info);
    } else {
      AD_DEBUG_PRINT("  [%zu]: 0x%016lx -> <unknown source>", i, (unsigned long)frame_addr);
    }
  }
}

void printStackFrameSource(AD_FUNC_ARGS, uint64_t frame_addr) {
  char source_buf[256];
  ArrayDetectErrorCode err = getFrameSourceLocation(AD_ARGS, frame_addr, source_buf, sizeof(source_buf));
  (void)err; // 忽略错误，仅用于调试输出
  if (source_buf[0] != '\0') {
    AD_DEBUG_PRINT("Frame 0x%016lx -> %s", (unsigned long)frame_addr, source_buf);
  } else {
    AD_DEBUG_PRINT("Frame 0x%016lx -> <unknown source>", (unsigned long)frame_addr);
  }
}

ArrayDetectErrorCode getFrameSourceLocation(AD_FUNC_ARGS, uint64_t frame_addr, char* buffer, size_t buffer_size) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;
  if (!buffer || buffer_size == 0) {
    AD_RETURNE(INVALID_ARGUMENT);
  }

  // Initialize buffer
  buffer[0] = '\0';

  // 保持原有简单实现
  snprintf(buffer, buffer_size, "return_addr:0x%lx", (unsigned long)frame_addr);
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 辅助函数：解析 C++ mangled 名称
// 返回解析后的名称，存储在 buffer 中
static const char* demangle_name(const char* mangled, char* buffer, size_t buffer_size) {
  if (!mangled || !buffer || buffer_size == 0) {
    return mangled;
  }
  
  int status = 0;
  size_t buf_size = buffer_size;
  char* demangled = abi::__cxa_demangle(mangled, buffer, &buf_size, &status);
  
  if (status == 0 && demangled) {
    // 成功解析
    // 如果 demangled != buffer，说明内部分配了内存，需要复制到 buffer
    if (demangled != buffer) {
      strncpy(buffer, demangled, buffer_size - 1);
      buffer[buffer_size - 1] = '\0';
      free(demangled);  // 释放内部分配的内存
    }
    return buffer;
  } else {
    // 解析失败，返回原始名称（复制到 buffer）
    strncpy(buffer, mangled, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return buffer;
  }
}

// 辅助函数：尝试获取源码位置（使用平台特定的工具）
static bool get_source_location_from_addr(uint64_t addr, char* file_buffer, size_t file_size, 
                                           unsigned int* line, char* func_buffer, size_t func_size) {
  // 获取当前可执行文件路径
  Dl_info info;
  if (!dladdr((void*)addr, &info) || !info.dli_fname) {
    return false;
  }
  
  char cmd[512];
  FILE* pipe = NULL;
  bool success = false;
  char line_buf[512];
  
#ifdef __APPLE__
  // macOS 使用 atos 工具
  // atos 需要绝对地址，输出格式：函数名 (文件名:行号)
  snprintf(cmd, sizeof(cmd), "atos -o %s 0x%lx 2>/dev/null", 
           info.dli_fname, (unsigned long)addr);
  
  pipe = popen(cmd, "r");
  if (pipe && fgets(line_buf, sizeof(line_buf), pipe)) {
    size_t len = strlen(line_buf);
    if (len > 0 && line_buf[len-1] == '\n') {
      line_buf[len-1] = '\0';
    }
    
    // atos 输出格式：函数名 (文件名:行号)
    // 或者：函数名 (文件名:行号:列号)
    char* paren_start = strchr(line_buf, '(');
    if (paren_start) {
      // 提取函数名（括号之前的部分，去除末尾空格）
      *paren_start = '\0';
      char* func_name = line_buf;
      while (*func_name == ' ') func_name++;  // 去除前导空格
      size_t func_len = strlen(func_name);
      while (func_len > 0 && func_name[func_len-1] == ' ') {
        func_name[func_len-1] = '\0';
        func_len--;
      }
      
      if (func_size > 0 && func_len > 0) {
        demangle_name(func_name, func_buffer, func_size);
      }
      
      // 提取文件名和行号（括号内的部分）
      char* location = paren_start + 1;
      char* paren_end = strchr(location, ')');
      if (paren_end) {
        *paren_end = '\0';
      }
      
      // 查找冒号（分隔文件名和行号）
      char* colon = strchr(location, ':');
      if (colon) {
        *colon = '\0';
        if (file_size > 0) {
          strncpy(file_buffer, location, file_size - 1);
          file_buffer[file_size - 1] = '\0';
        }
        
        // 解析行号（可能还有列号，用第二个冒号分隔）
        char* line_str = colon + 1;
        char* colon2 = strchr(line_str, ':');
        if (colon2) {
          *colon2 = '\0';  // 忽略列号
        }
        *line = (unsigned int)atoi(line_str);
        success = (*line > 0);  // 行号必须大于0才有效
      }
    }
  }
#else
  // Linux 使用 addr2line 工具
  // addr2line 需要相对于文件基址的偏移
  uint64_t offset = addr;
  if (info.dli_fbase) {
    offset = (uint64_t)((char*)addr - (char*)info.dli_fbase);
  }
  
  snprintf(cmd, sizeof(cmd), "addr2line -e %s -f -C -i -p 0x%lx 2>/dev/null", 
           info.dli_fname, (unsigned long)offset);
  
  pipe = popen(cmd, "r");
  if (pipe && fgets(line_buf, sizeof(line_buf), pipe)) {
    size_t len = strlen(line_buf);
    if (len > 0 && line_buf[len-1] == '\n') {
      line_buf[len-1] = '\0';
    }
    
    // addr2line 输出格式可能是：
    // 1. "函数名 at 文件名:行号"
    // 2. "函数名\n文件名:行号" (如果没有 -p 选项)
    char* at_pos = strstr(line_buf, " at ");
    if (at_pos) {
      // 格式1：函数名 at 文件名:行号
      *at_pos = '\0';
      if (func_size > 0) {
        demangle_name(line_buf, func_buffer, func_size);
      }
      
      char* location = at_pos + 4;  // 跳过 " at "
      char* colon = strchr(location, ':');
      if (colon) {
        *colon = '\0';
        if (file_size > 0) {
          strncpy(file_buffer, location, file_size - 1);
          file_buffer[file_size - 1] = '\0';
        }
        *line = (unsigned int)atoi(colon + 1);
        success = (*line > 0);
      }
    } else {
      // 可能是格式2，需要读取第二行
      if (func_size > 0) {
        demangle_name(line_buf, func_buffer, func_size);
      }
      
      // 尝试读取第二行（文件名:行号）
      if (fgets(line_buf, sizeof(line_buf), pipe)) {
        len = strlen(line_buf);
        if (len > 0 && line_buf[len-1] == '\n') {
          line_buf[len-1] = '\0';
        }
        
        char* colon = strchr(line_buf, ':');
        if (colon) {
          *colon = '\0';
          if (file_size > 0) {
            strncpy(file_buffer, line_buf, file_size - 1);
            file_buffer[file_size - 1] = '\0';
          }
          *line = (unsigned int)atoi(colon + 1);
          success = (*line > 0);
        }
      }
    }
  }
#endif
  
  if (pipe) {
    pclose(pipe);
  }
  return success;
}

// 使用新地址解析器的增强版本
ArrayDetectErrorCode resolveFrameAddressToSource(AD_FUNC_ARGS, uint64_t frame_addr, const char* &result, bool &out_is_valid) AD_FUNCTION_BEGIN {
  AD_ARGS_WARN_DENY;

  // 使用 Context 中的 source_location_buffer
  if (!ctx.source_location_buffer || ctx.source_location_buffer_size == 0) {
    result = "<no buffer>";
    out_is_valid = false;
    AD_RETURNE(OK);
  }

  // 直接使用 dladdr 进行地址解析
  if (frame_addr == 0) {
    snprintf(ctx.source_location_buffer, ctx.source_location_buffer_size, "addr:0x%lx", (unsigned long)frame_addr);
    result = ctx.source_location_buffer;
    out_is_valid = false;
    AD_RETURNE(OK);
  }

  Dl_info info;
  if (dladdr((void*)frame_addr, &info) && info.dli_sname) {
    // 尝试解析 mangled 名称
    char demangle_buffer[512];
    const char* demangled = demangle_name(info.dli_sname, demangle_buffer, sizeof(demangle_buffer));
    
    size_t offset = (size_t)((char*)frame_addr - (char*)info.dli_saddr);
    
    // 尝试获取源码位置
    char file_buffer[512];
    char func_buffer[512];
    unsigned int line = 0;
    bool has_source = get_source_location_from_addr(frame_addr, file_buffer, sizeof(file_buffer), 
                                                     &line, func_buffer, sizeof(func_buffer));
    
    if (has_source && line > 0) {
      // 有源码位置信息
      snprintf(ctx.source_location_buffer, ctx.source_location_buffer_size,
               "%s (%s:%u) +0x%lx", demangled, file_buffer, line, (unsigned long)offset);
    } else {
      // 只有函数名和偏移
      snprintf(ctx.source_location_buffer, ctx.source_location_buffer_size,
               "%s+0x%lx", demangled, (unsigned long)offset);
    }
    result = ctx.source_location_buffer;
    out_is_valid = true;
    AD_RETURNE(OK);
  }

  snprintf(ctx.source_location_buffer, ctx.source_location_buffer_size, "addr:0x%lx", (unsigned long)frame_addr);
  result = ctx.source_location_buffer;
  out_is_valid = false;
  AD_RETURNE(OK);
} AD_FUNCTION_END

// 获取当前栈帧信息
void getCurrentFrameInfo(AD_FUNC_ARGS, CurrentFrameInfo& info) {
  AD_ARGS_WARN_DENY;
  
  // 初始化结构
  info.frame_address = 0;
  info.is_valid = false;
  info.source_location = nullptr;
  info.demangled_name = nullptr;
  info.source_file = nullptr;
  info.source_line = 0;
  info.stack_depth = getStackDepth(AD_ARGS);
  
  // 获取当前栈帧地址
  bool is_exists;
  getCurrentFrame(AD_ARGS, info.frame_address, is_exists);
  
  if (!is_exists) {
    info.is_valid = false;
    return;
  }
  
  // 使用辅助缓冲区进行解析
  if (!ctx.source_location_buffer || ctx.source_location_buffer_size == 0) {
    info.is_valid = false;
    return;
  }
  
  // 使用静态缓冲区存储解析结果（避免在结构体中存储临时数据）
  static char demangle_buffer[512];
  static char file_buffer[512];
  static char func_buffer[512];
  
  // 获取符号信息
  Dl_info dl_info;
  if (dladdr((void*)info.frame_address, &dl_info) && dl_info.dli_sname) {
    // 解析函数名
    info.demangled_name = demangle_name(dl_info.dli_sname, demangle_buffer, sizeof(demangle_buffer));
    
    // 尝试获取源码位置
    unsigned int line = 0;
    bool has_source = get_source_location_from_addr(info.frame_address, file_buffer, sizeof(file_buffer),
                                                      &line, func_buffer, sizeof(func_buffer));
    
    if (has_source && line > 0) {
      info.source_file = file_buffer;
      info.source_line = line;
    } else {
      info.source_file = nullptr;
      info.source_line = 0;
    }
    
    // 格式化完整的位置信息
    size_t offset = (size_t)((char*)info.frame_address - (char*)dl_info.dli_saddr);
    if (has_source && line > 0) {
      snprintf(ctx.source_location_buffer, ctx.source_location_buffer_size,
               "%s (%s:%u) +0x%lx", info.demangled_name, file_buffer, line, (unsigned long)offset);
    } else {
      snprintf(ctx.source_location_buffer, ctx.source_location_buffer_size,
               "%s+0x%lx", info.demangled_name, (unsigned long)offset);
    }
    info.source_location = ctx.source_location_buffer;
    info.is_valid = true;
  } else {
    snprintf(ctx.source_location_buffer, ctx.source_location_buffer_size, 
             "addr:0x%lx", (unsigned long)info.frame_address);
    info.source_location = ctx.source_location_buffer;
    info.demangled_name = nullptr;
    info.source_file = nullptr;
    info.source_line = 0;
    info.is_valid = false;
  }
}

// 输出当前栈帧信息（保留以兼容）
void printCurrentFrameInfo(AD_FUNC_ARGS) {
  CurrentFrameInfo info;
  getCurrentFrameInfo(AD_ARGS, info);
  
  if (info.is_valid) {
    AD_DEBUG_PRINT("Current frame: 0x%016lx (depth: %zu) -> %s", 
                   (unsigned long)info.frame_address, 
                   info.stack_depth,
                   info.source_location);
  } else if (info.stack_depth > 0) {
    AD_DEBUG_PRINT("Current frame: 0x%016lx (depth: %zu) -> <unknown source>", 
                   (unsigned long)info.frame_address, 
                   info.stack_depth);
  } else {
    AD_DEBUG_PRINT("Current frame: <empty stack>");
  }
}

// 输出所有栈帧信息
void printAllStackFramesInfo(AD_FUNC_ARGS) {
  AD_ARGS_WARN_DENY;
  
  size_t depth = getStackDepth(AD_ARGS);
  
  if (depth == 0) {
    AD_DEBUG_PRINT("Stack frames: <empty stack>");
    return;
  }
  
  AD_DEBUG_PRINT("Stack frames (depth: %zu):", depth);
  
  // 从栈底到栈顶输出所有帧
  for (size_t i = 0; i < gcc_ctx.stack_frames.length(); ++i) {
    uint64_t frame_addr = gcc_ctx.stack_frames[i];
    CurrentFrameInfo info;
    
    // 临时设置当前帧地址以获取信息
    info.frame_address = frame_addr;
    info.is_valid = false;
    info.source_location = nullptr;
    info.demangled_name = nullptr;
    info.source_file = nullptr;
    info.source_line = 0;
    info.stack_depth = i + 1;
    
    // 使用辅助缓冲区进行解析
    if (ctx.source_location_buffer && ctx.source_location_buffer_size > 0) {
      // 使用静态缓冲区存储解析结果
      static char demangle_buffer[512];
      static char file_buffer[512];
      static char func_buffer[512];
      
      // 获取符号信息
      Dl_info dl_info;
      if (dladdr((void*)frame_addr, &dl_info) && dl_info.dli_sname) {
        // 解析函数名
        info.demangled_name = demangle_name(dl_info.dli_sname, demangle_buffer, sizeof(demangle_buffer));
        
        // 尝试获取源码位置
        unsigned int line = 0;
        bool has_source = get_source_location_from_addr(frame_addr, file_buffer, sizeof(file_buffer),
                                                          &line, func_buffer, sizeof(func_buffer));
        
        if (has_source && line > 0) {
          info.source_file = file_buffer;
          info.source_line = line;
        } else {
          info.source_file = nullptr;
          info.source_line = 0;
        }
        
        // 格式化完整的位置信息
        size_t offset = (size_t)((char*)frame_addr - (char*)dl_info.dli_saddr);
        if (has_source && line > 0) {
          snprintf(ctx.source_location_buffer, ctx.source_location_buffer_size,
                   "%s (%s:%u) +0x%lx", info.demangled_name, file_buffer, line, (unsigned long)offset);
        } else {
          snprintf(ctx.source_location_buffer, ctx.source_location_buffer_size,
                   "%s+0x%lx", info.demangled_name, (unsigned long)offset);
        }
        info.source_location = ctx.source_location_buffer;
        info.is_valid = true;
      } else {
        snprintf(ctx.source_location_buffer, ctx.source_location_buffer_size, 
                 "addr:0x%lx", (unsigned long)frame_addr);
        info.source_location = ctx.source_location_buffer;
        info.is_valid = false;
      }
    }
    
    // 输出栈帧信息
    if (info.is_valid) {
      AD_DEBUG_PRINT("  [%zu] 0x%016lx -> %s", i, (unsigned long)frame_addr, info.source_location);
    } else {
      AD_DEBUG_PRINT("  [%zu] 0x%016lx -> <unknown source>", i, (unsigned long)frame_addr);
    }
  }
}

} // namespace array_detect_ns
