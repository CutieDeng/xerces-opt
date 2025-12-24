# OBJ_TYPE_REF 处理指南

## 基本结构

OBJ_TYPE_REF 是 GCC 中表示虚函数调用的 tree 节点，包含三个关键字段：

```c
OBJ_TYPE_REF_EXPR    // 虚表中的函数指针（通常是 SSA_NAME）
OBJ_TYPE_REF_OBJECT  // 调用对象
OBJ_TYPE_REF_TOKEN   // 虚表槽索引（INTEGER_CST）
```

## 核心要点

### 1. TOKEN 解析

**错误做法**：
```c
// ❌ 不要除以指针大小
long long byte_offset = TREE_INT_CST_LOW(token);
long long vtable_index = byte_offset / sizeof(void*);  // 错误！
```

**正确做法**：
```c
// ✓ 直接使用 TOKEN 值作为槽索引
long long vtable_index = (long long)TREE_INT_CST_LOW(token);
```

### 2. 虚函数名称解析

**最佳方法：使用 BINFO_VIRTUALS**

```c
tree binfo = TYPE_BINFO(object_type);
if (binfo) {
    tree virtuals = BINFO_VIRTUALS(binfo);
    int index = 0;

    for (tree virt = virtuals; virt; virt = TREE_CHAIN(virt), index++) {
        tree fn_decl = TREE_VALUE(virt);

        // 某些版本中可能在 TREE_PURPOSE
        if (!fn_decl || TREE_CODE(fn_decl) != FUNCTION_DECL) {
            fn_decl = TREE_PURPOSE(virt);
        }

        if (index == vtable_index && TREE_CODE(fn_decl) == FUNCTION_DECL) {
            const char* name = IDENTIFIER_POINTER(DECL_NAME(fn_decl));
            // 找到函数名
            break;
        }
    }
}
```

**为什么不用 TYPE_FIELDS？**

TYPE_FIELDS 的 method_index 顺序 ≠ vtable 槽索引顺序：
- 析构函数在 TYPE_FIELDS 中只有一个声明
- 但在 vtable 中占用 2 个槽位（complete + deleting）
- 导致索引错位

**示例**：MemoryManager 类
```cpp
class MemoryManager {
    virtual ~MemoryManager();     // TYPE_FIELDS method_index=0
    virtual void* allocate(...);  // TYPE_FIELDS method_index=1
    virtual void deallocate(...);  // TYPE_FIELDS method_index=2
};
```

实际 vtable 布局（Itanium C++ ABI）：
```
slot 0: __dt_comp  (complete destructor)
slot 1: __dt_del   (deleting destructor)
slot 2: allocate   ← TOKEN=2 指向这里
slot 3: deallocate
```

TYPE_FIELDS method_index=2 是 `deallocate`，但 vtable slot 2 是 `allocate`！

### 3. 后备方案

```c
// 优先级：BINFO_VIRTUALS > TYPE_FIELDS > TYPE_METHODS

// 1. BINFO_VIRTUALS（推荐）
tree virtuals = BINFO_VIRTUALS(TYPE_BINFO(type));

// 2. TYPE_FIELDS（可能不准确）
for (tree decl = TYPE_FIELDS(type); decl; decl = DECL_CHAIN(decl)) {
    if (TREE_CODE(decl) == FUNCTION_DECL && DECL_VIRTUAL_P(decl)) {
        // method_index 可能与 vtable_index 不一致
    }
}

// 3. TYPE_METHODS（GCC 12 及更早版本，条件编译）
#ifdef TYPE_METHODS
for (tree method = TYPE_METHODS(type); method; method = DECL_CHAIN(method)) {
    if (DECL_VIRTUAL_P(method)) { ... }
}
#endif
```

### 4. 获取对象类型

```c
tree obj_type_ref_object = OBJ_TYPE_REF_OBJECT(obj_type_ref);
tree object_type = TREE_TYPE(obj_type_ref_object);

// 去除 cv-qualifiers，获取主变体
object_type = TYPE_MAIN_VARIANT(object_type);

// 解引用指针类型
if (TREE_CODE(object_type) == POINTER_TYPE) {
    object_type = TREE_TYPE(object_type);
}
```

## 完整示例

```c
ArrayDetectErrorCode resolveVirtualCall(gimple* call_stmt, const char** out_name) {
    tree fn = gimple_call_fn(call_stmt);
    if (TREE_CODE(fn) != OBJ_TYPE_REF) {
        return ERROR_NOT_VIRTUAL_CALL;
    }

    // 1. 获取 vtable 索引
    tree token = OBJ_TYPE_REF_TOKEN(fn);
    if (TREE_CODE(token) != INTEGER_CST) {
        return ERROR_INVALID_TOKEN;
    }
    long long vtable_index = (long long)TREE_INT_CST_LOW(token);

    // 2. 获取对象类型
    tree obj = OBJ_TYPE_REF_OBJECT(fn);
    tree obj_type = TYPE_MAIN_VARIANT(TREE_TYPE(obj));
    if (TREE_CODE(obj_type) == POINTER_TYPE) {
        obj_type = TREE_TYPE(obj_type);
    }

    // 3. 使用 BINFO_VIRTUALS 解析
    tree binfo = TYPE_BINFO(obj_type);
    if (!binfo) {
        return ERROR_NO_BINFO;
    }

    tree virtuals = BINFO_VIRTUALS(binfo);
    if (!virtuals) {
        return ERROR_NO_VIRTUALS;
    }

    int index = 0;
    for (tree virt = virtuals; virt; virt = TREE_CHAIN(virt), index++) {
        if (index != vtable_index) continue;

        tree fn_decl = TREE_VALUE(virt);
        if (!fn_decl || TREE_CODE(fn_decl) != FUNCTION_DECL) {
            fn_decl = TREE_PURPOSE(virt);
        }

        if (fn_decl && TREE_CODE(fn_decl) == FUNCTION_DECL && DECL_NAME(fn_decl)) {
            *out_name = IDENTIFIER_POINTER(DECL_NAME(fn_decl));
            return OK;
        }
    }

    return ERROR_NOT_FOUND;
}
```

## GCC 版本差异

| GCC 版本 | BINFO_VIRTUALS | TYPE_METHODS | TYPE_FIELDS |
|---------|----------------|--------------|-------------|
| 15      | ✓ 推荐         | ✗ 已移除      | ✓ 后备      |
| 12      | ✓ 推荐         | ✓ 条件编译    | ✓ 后备      |
| 8-11    | ✓ 推荐         | ✗ 废弃        | ✓ 后备      |

## 常见陷阱

1. **不要假设 TOKEN 是字节偏移** - 直接当作索引用
2. **不要依赖 TYPE_FIELDS 的顺序** - 析构函数会导致索引错位
3. **不要忘记处理 TREE_PURPOSE** - 某些版本函数在这里
4. **不要跳过 TYPE_MAIN_VARIANT** - cv-qualifiers 会影响类型查找

## 调试技巧

```c
// 打印 vtable 布局
tree virtuals = BINFO_VIRTUALS(binfo);
int i = 0;
for (tree v = virtuals; v; v = TREE_CHAIN(v), i++) {
    tree fn = TREE_VALUE(v) ?: TREE_PURPOSE(v);
    if (fn && DECL_NAME(fn)) {
        fprintf(stderr, "slot %d: %s\n", i, IDENTIFIER_POINTER(DECL_NAME(fn)));
    }
}

// 打印 OBJ_TYPE_REF 详情
fprintf(stderr, "TOKEN=%lld, object type=%s\n",
        (long long)TREE_INT_CST_LOW(OBJ_TYPE_REF_TOKEN(obj_type_ref)),
        TYPE_NAME_STRING(object_type));
```
