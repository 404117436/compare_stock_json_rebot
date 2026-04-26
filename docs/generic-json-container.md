# GenericJsonContainer

## 职责

GenericJsonContainer 是一个通用的 JSON 数据容器，负责把一个 JSON 对象（一行 JSON 字符串）解析后存储在内存中，并提供按字段名读取值的能力。它是整个项目数据层的核心基类，解决了"如何在内存中表示任意结构的 JSON 对象"这个问题。它不关心数据的业务含义（那是 `StockDataContainer` 的事），只负责通用的存储和访问。

## 文件组成

| 文件 | 说明 |
|------|------|
| `include/GenericJsonContainer.h` | 接口声明，包含 `CustomValue`、`GenericJsonContainer` 两个核心类以及 `JsonValueType` 枚举 |
| `src/GenericJsonContainer.cpp` | 完整实现，包含 `CustomValue` 的内存管理和 `GenericJsonContainer` 的解析逻辑 |

## 核心类

### CustomValue

**职责**：项目自己实现的 JSON 值类型，能表示 JSON 中所有七种数据类型（null、bool、int、double、string、array、object）。之所以不直接用 rapidjson 的 `Value`，是因为 rapidjson 的 `Value` 生命周期绑定在 `Document` 上，一旦 `Document` 销毁，所有 `Value` 指针就失效了。`CustomValue` 是独立的、可以自由复制和移动的值对象。

**关键成员变量**：
- `JsonValueType type_`：记录当前存储的是哪种类型
- `union ValueData data_`：C 语言联合体，同一块内存根据 `type_` 的不同解释为不同类型。基本类型（bool、int64、double）直接存在联合体里；复杂类型（string、array、object）在堆上分配，联合体中只存指针

**核心方法**：

#### 构造函数族

针对每种类型都有对应的构造函数（`CustomValue(bool)`, `CustomValue(int64_t)`, `CustomValue(double)`, `CustomValue(const std::string&)` 等）。string、array、object 类型会在堆上 `new` 出对应的对象，并把指针存入联合体。

#### `void cleanup()`

析构时的清理逻辑。根据 `type_` 判断联合体中存的是什么，如果是 string/array/object 就 `delete` 对应的堆内存。这是手动内存管理的核心，必须在析构函数、拷贝赋值和移动赋值中都正确调用。

#### `void copyFrom(const CustomValue& other)`

深拷贝逻辑。对于 string/array/object，会 `new` 一个新对象并复制内容，而不是复制指针（否则两个 `CustomValue` 会共享同一块堆内存，导致双重释放）。

#### 移动构造 / 移动赋值

直接把联合体的数据（包括指针）复制过来，然后把源对象的 `type_` 设为 `Null`、指针清零。这样转移所有权后，源对象析构时不会释放已经转移出去的内存。

---

### GenericJsonContainer

**职责**：持有一个 JSON 对象的所有字段，以有序键值对列表（`JsonObjectData`，即 `vector<pair<string, CustomValue>>`）的形式存储，并提供按字段名查找、遍历所有字段等操作。

**关键成员变量**：
- `JsonObjectData data_`：存储所有字段的有序列表，每个元素是 `(字段名, CustomValue)` 的 pair。选择 vector 而非 unordered_map 是为了保持字段的原始顺序
- `std::string source_`：标记这条数据来自哪个数据源（如 "FileA" 或 "FileB"），用于调试

**核心方法**：

#### `bool parseFromJsonString(const std::string& jsonStr)`

解析一行 JSON 字符串的主入口：
1. 用 `JsonParser` 把字符串解析成 rapidjson 的文档树
2. 获取根节点，调用 `parseJsonObject()` 遍历所有字段
3. 对每个字段，调用 `parseJsonValue()` 把 rapidjson 的值递归转换成 `CustomValue`
4. 把所有 `(字段名, CustomValue)` 对存入 `data_`

#### `bool parseFromJsonString(const std::string& jsonStr, const std::string& ignore_fields)`

带字段过滤的版本。先把逗号分隔的 `ignore_fields` 字符串解析成 `unordered_set`，然后在遍历字段时跳过在集合中的字段名。用 `unordered_set` 是为了让每次字段名查找的时间复杂度是 O(1)。

#### `CustomValue parseJsonValue(const JsonValue& jsonValue)`

递归转换方法。根据 rapidjson 值的类型，创建对应的 `CustomValue`：
- 基本类型（bool、int、double、string）直接构造
- array 类型：遍历每个元素，递归调用自身，结果收集到 `JsonArrayData` 后构造 `CustomValue`
- object 类型：遍历每个字段，递归调用自身，结果收集到 `JsonObjectData` 后构造 `CustomValue`

#### `const CustomValue& getValue(const std::string& key) const`

在 `data_` 中线性查找指定字段名。由于 `data_` 是 vector，查找是 O(n)。对于股票 JSON 通常只有几十个字段，这个开销可以接受。

## 与其他模块的交互

- **依赖**：`JsonParser`（用于解析 JSON 字符串）
- **被依赖**：`StockDataContainer` 继承自本类，在其基础上增加股票业务字段的提取

## 数据流

```
JSON 字符串（一行）
      │  parseFromJsonString()
      ▼
JsonParser → rapidjson::Document（临时）
      │  parseJsonObject() 递归遍历
      ▼
vector<pair<string, CustomValue>>（data_）
      │  getValue() / getAllKeys() / hasKey()
      ▼
调用方（按字段名读取 CustomValue）
```
