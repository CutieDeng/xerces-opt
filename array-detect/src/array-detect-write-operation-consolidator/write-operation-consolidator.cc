#include "write-operation-consolidator.hh"
#include "function-call-equivalence.hh"
#include "gcc-ext-util.hh"

namespace array_detector {

using namespace ::array_detect_ns;

// ============================================================================
// Fingerprint 哈希和比较函数
// ============================================================================

size_t hashWriteOperationFingerprint (WriteOperationFingerprint const *fp) {
  if (!fp) {
    return 0;
  }
  
  size_t h = (size_t)fp->source_type;
  
  switch (fp->source_type) {
    case SOURCE_FUNCTION_CALL: {
      h ^= (size_t)fp->data.function_call.call_type;
      if (fp->data.function_call.function_name) {
        // 使用字符串指针的哈希（简单但有效）
        char const *str = fp->data.function_call.function_name;
        size_t str_hash = 0;
        for (; *str; ++str) {
          str_hash = str_hash * 31 + (unsigned char)*str;
        }
        h ^= str_hash;
      }
      break;
    }
    case SOURCE_VARIABLE: {
      if (fp->data.variable.var_name) {
        char const *str = fp->data.variable.var_name;
        size_t str_hash = 0;
        for (; *str; ++str) {
          str_hash = str_hash * 31 + (unsigned char)*str;
        }
        h ^= str_hash;
      }
      break;
    }
    case SOURCE_CONSTANT: {
      if (fp->data.constant.constant_str) {
        char const *str = fp->data.constant.constant_str;
        size_t str_hash = 0;
        for (; *str; ++str) {
          str_hash = str_hash * 31 + (unsigned char)*str;
        }
        h ^= str_hash;
      }
      break;
    }
    case SOURCE_COMPUTATION: {
      if (fp->data.computation.description) {
        char const *str = fp->data.computation.description;
        size_t str_hash = 0;
        for (; *str; ++str) {
          str_hash = str_hash * 31 + (unsigned char)*str;
        }
        h ^= str_hash;
      }
      break;
    }
    case SOURCE_PHI: {
      if (fp->data.phi.var_name) {
        char const *str = fp->data.phi.var_name;
        size_t str_hash = 0;
        for (; *str; ++str) {
          str_hash = str_hash * 31 + (unsigned char)*str;
        }
        h ^= str_hash;
      }
      break;
    }
    case SOURCE_UNKNOWN:
      break;
  }
  
  return h;
}

bool equalWriteOperationFingerprint (WriteOperationFingerprint const *fp1,
                                      WriteOperationFingerprint const *fp2) {
  if (!fp1 || !fp2) {
    return fp1 == fp2;
  }
  
  if (fp1->source_type != fp2->source_type) {
    return false;
  }
  
  switch (fp1->source_type) {
    case SOURCE_FUNCTION_CALL: {
      // 对于函数调用，需要更精确的等价性判断
      // 但 fingerprint 只包含简化信息，这里先做基本比较
      // 更精确的比较应该在生成 fingerprint 时使用 function-call-equivalence 模块
      if (fp1->data.function_call.call_type != fp2->data.function_call.call_type) {
        return false;
      }
      char const *name1 = fp1->data.function_call.function_name;
      char const *name2 = fp2->data.function_call.function_name;
      if (name1 && name2) {
        return strcmp (name1, name2) == 0;
      }
      return name1 == name2;
    }
    case SOURCE_VARIABLE: {
      char const *name1 = fp1->data.variable.var_name;
      char const *name2 = fp2->data.variable.var_name;
      if (name1 && name2) {
        return strcmp (name1, name2) == 0;
      }
      return name1 == name2;
    }
    case SOURCE_CONSTANT: {
      char const *str1 = fp1->data.constant.constant_str;
      char const *str2 = fp2->data.constant.constant_str;
      if (str1 && str2) {
        return strcmp (str1, str2) == 0;
      }
      return str1 == str2;
    }
    case SOURCE_COMPUTATION: {
      char const *desc1 = fp1->data.computation.description;
      char const *desc2 = fp2->data.computation.description;
      if (desc1 && desc2) {
        return strcmp (desc1, desc2) == 0;
      }
      return desc1 == desc2;
    }
    case SOURCE_PHI: {
      char const *name1 = fp1->data.phi.var_name;
      char const *name2 = fp2->data.phi.var_name;
      if (name1 && name2) {
        return strcmp (name1, name2) == 0;
      }
      return name1 == name2;
    }
    case SOURCE_UNKNOWN:
      return true;
  }
  
  return false;
}

// ============================================================================
// Fingerprint 生成函数
// ============================================================================

// 从 FieldSourceInfo 生成 Fingerprint
static ArrayDetectErrorCode generateFingerprint (
  AD_FUNC_ARGS,
  FieldSourceInfo const *source_info,
  WriteOperationFingerprint &fp
) AD_FUNCTION_BEGIN {
  memset (&fp, 0, sizeof (WriteOperationFingerprint));
  
  if (!source_info) {
    fp.source_type = SOURCE_UNKNOWN;
    AD_RETURNE (OK);
  }
  
  fp.source_type = source_info->source_type;
  
  switch (source_info->source_type) {
    case SOURCE_FUNCTION_CALL: {
      FunctionCallSource const &call = source_info->data.function_call;
      fp.data.function_call.call_type = call.call_type;
      // 对于函数调用，使用 function_name 作为 fingerprint
      // 更精确的等价性判断应该在生成 fingerprint 时使用 function-call-equivalence 模块
      // 但为了简化，这里先使用 function_name
      fp.data.function_call.function_name = call.function_name;  // 直接引用，不复制
      break;
    }
    case SOURCE_VARIABLE: {
      VariableSource const &var = source_info->data.variable;
      fp.data.variable.var_name = var.var_name;  // 直接引用，不复制
      break;
    }
    case SOURCE_CONSTANT: {
      ConstantSource const &cst = source_info->data.constant;
      fp.data.constant.constant_str = cst.constant_str;  // 直接引用，不复制
      break;
    }
    case SOURCE_COMPUTATION: {
      ComputationSource const &comp = source_info->data.computation;
      fp.data.computation.description = comp.description;  // 直接引用，不复制
      break;
    }
    case SOURCE_PHI: {
      PhiSource const &phi = source_info->data.phi;
      fp.data.phi.var_name = phi.var_name;  // 直接引用，不复制
      break;
    }
    case SOURCE_UNKNOWN:
      break;
  }
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

// ============================================================================
// 综合函数
// ============================================================================

ArrayDetectErrorCode consolidateWriteOperations (
  AD_FUNC_ARGS,
  ArrayDetector &detector,
  hash_map<TypeFieldKey,
           hash_map<WriteOperationFingerprint, WriteOperationDetail*>*,
           TypeFieldHashMapTraits> &consolidated
) AD_FUNCTION_BEGIN {
  typedef hash_map<TypeFieldKey, TypeFieldWriteOps*, TypeFieldHashMapTraits> TypeFieldHashMap;
  
  if (!detector.m_type_field_writes) {
    AD_RETURNE (OK);
  }
  
  // 遍历所有 (type, field) 对
  for (TypeFieldHashMap::iterator iter = detector.m_type_field_writes->begin ();
       iter != detector.m_type_field_writes->end ();
       ++iter) {
    TypeFieldKey key = (*iter).first;
    TypeFieldWriteOps *tfwo = (*iter).second;
    
    if (!tfwo || !tfwo->write_ops) {
      continue;
    }
    
    // 为当前 (type, field) 创建内层 hash_map
    hash_map<WriteOperationFingerprint, WriteOperationDetail*> *inner_map =
      ggc_alloc<hash_map<WriteOperationFingerprint, WriteOperationDetail*>> ();
    if (!inner_map) {
      AD_RETURNE (MEMORY_ERROR);
    }
    new (inner_map) hash_map<WriteOperationFingerprint, WriteOperationDetail*> ();
    
    // 遍历该 (type, field) 的所有 write operations
    for (unsigned int i = 0; i < tfwo->write_ops->length (); ++i) {
      FieldWriteCapture *capture = (*tfwo->write_ops)[i];
      if (!capture) {
        continue;
      }
      
      // 获取 source 信息
      FieldSourceInfo *source_info = (FieldSourceInfo*)capture->aux;
      if (!source_info) {
        continue;
      }
      
      // 生成 fingerprint
      WriteOperationFingerprint fp;
      AD_TRY (generateFingerprint (AD_ARGS, source_info, fp));
      
      // 查找或创建 detail
      WriteOperationDetail **detail_ptr = inner_map->get (fp);
      if (!detail_ptr) {
        // 创建新的 detail
        WriteOperationDetail *detail = ggc_alloc<WriteOperationDetail> ();
        if (!detail) {
          AD_RETURNE (MEMORY_ERROR);
        }
        memset (detail, 0, sizeof (WriteOperationDetail));
        detail->write_operations = ggc_alloc<vec<FieldWriteCapture*> > ();
        if (!detail->write_operations) {
          AD_RETURNE (MEMORY_ERROR);
        }
        detail->write_operations->create (0);
        
        inner_map->put (fp, detail);
        detail_ptr = inner_map->get (fp);
      }
      
      // 将当前 capture 添加到 detail 的 vec 中
      (*detail_ptr)->write_operations->safe_push (capture);
    }
    
    // 将内层 hash_map 添加到 consolidated
    consolidated.put (key, inner_map);
  }
  
  AD_RETURNE (OK);
} AD_FUNCTION_END

} // namespace array_detector

