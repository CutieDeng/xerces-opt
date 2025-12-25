# Array Detect GCC Plugin

GCC 插件，用于静态分析 C++ 代码中的数组字段使用模式。

## 编译

### 1. 生成 Makefile

```bash
racket build-makefile.rkt
```

### 2. 编译插件

```bash
make
```

生成的插件文件：`out/plugin-array-detect.dylib` (macOS) 或 `out/plugin-array-detect.so` (Linux)

## 测试

### 核心测试套（集成测试）

```bash
make test
```

### 模块级/功能级测试

```bash
make test-simple-ptr-field
make test-simple-virtual-call
make test-simple-ptr-copy-escape
```

## 使用

### 基本用法

```bash
g++ -fplugin=./out/plugin-array-detect.dylib your-code.cc -o your-program
```

### 配置调试输出

```bash
# 输出到文件
AD_DEBUG_FILE=/tmp/debug.log g++ -fplugin=./out/plugin-array-detect.dylib code.cc

# 输出到 stderr
AD_DEBUG_FILE=stderr g++ -fplugin=./out/plugin-array-detect.dylib code.cc
```

## 配置

编译器和库路径配置：
- `exe-config.rkt` - GCC 编译器路径
- `lib-config.rkt` - GMP/MPC/MPFR 库路径

## 清理

```bash
make clean
```
