# JsonParser

## 职责

JsonParser 是对 rapidjson 库的封装层，把 rapidjson 偏底层的 C 风格 API 包装成更安全、更易用的面向对象接口。它负责将一个 JSON 字符串或文件解析成内存中的文档对象，并提供类型安全的字段访问方式。它不负责任何业务逻辑，只是一个纯粹的 JSON 读取工具。

## 文件组成

| 文件 | 说明 |
|------|------|
| `include/JsonParser.h` | 主要实现都在头文件中（模板方法 + inline），包含 `JsonValue` 和 `JsonParser` 两个类 |
| `src/JsonParser.cpp` | 少量辅助方法的实现，主要是错误信息格式化和结构验证 |

## 核心类

### JsonValue

**职责**：对 rapidjson 的 `rapidjson::Value*` 指针做了一层轻量包装，提供类型检查和类型安全的取值方法，避免调用方直接操作 rapidjson 的原始指针。

**关键成员变量**：
- `const rapidjson::Value* value_`：指向 rapidjson 文档树中某个节点的指针，不拥有所有权（生命周期由 `JsonParser` 管理）

**核心方法**：

#### 类型检查方法（`isString()`, `isInt()`, `isDouble()` 等）

直接转发到 rapidjson 的对应方法。值得注意的是 `isNull()` 的实现：当指针本身为 `nullptr` 时也返回 true，这样访问不存在的字段时不会崩溃，而是得到一个"null 值"。

#### `operator[](const std::string& key)`

访问 JSON 对象中的某个字段。如果字段不存在，返回一个包含 `nullptr` 的 `JsonValue`（即 null 值），而不是抛出异常。这个设计让调用方可以先访问再用 `isNull()` 检查，而不必先检查再访问。

#### `asString()`, `asInt()`, `asDouble()`, `asBool()`

类型不匹配时抛出 `JsonParseException`，而不是返回默认值或静默失败。这是一种"快速失败"的设计，让错误在第一时间暴露。

---

### JsonParser

**职责**：持有一个完整的 rapidjson `Document` 对象（即整棵 JSON 解析树），提供从字符串或文件解析 JSON 的入口，以及获取根节点的方法。

**关键成员变量**：
- `std::unique_ptr<rapidjson::Document> doc_`：用智能指针管理 rapidjson 文档的生命周期，确保析构时自动释放内存

**核心方法**：

#### `bool parseFromString(const std::string& jsonString)`

将一个 JSON 字符串解析到内部的 `doc_` 中。解析失败时返回 false（不抛异常），调用方可以通过 `isValid()` 或 `getDetailedParseError()` 进一步了解失败原因。

#### `bool parseFromFile(const std::string& filepath)`

打开文件，用 rapidjson 的 `IStreamWrapper` 流式解析，避免先把整个文件读入字符串再解析的额外内存开销。

#### `JsonValue getRoot() const`

返回文档根节点的 `JsonValue` 包装。整个 JSON 树的访问都从这里开始。

**注意事项**：
- `JsonValue` 中的指针指向 `doc_` 内部的内存，因此 `JsonParser` 对象的生命周期必须长于所有从它获取的 `JsonValue` 对象。在 `GenericJsonContainer` 中，解析完成后会把所有值复制到自己的数据结构中，所以不存在悬空指针问题。

## 与其他模块的交互

- **依赖**：rapidjson（第三方库，header-only）
- **被依赖**：`GenericJsonContainer` 使用 `JsonParser` 解析 JSON 字符串，然后把解析结果转换成自己的 `CustomValue` 表示

## 数据流

```
JSON 字符串 / 文件
      │  parseFromString() / parseFromFile()
      ▼
rapidjson::Document（内存中的 JSON 树）
      │  getRoot() → operator[]
      ▼
JsonValue（对树节点的轻量包装）
      │  asString() / asInt() / asDouble() 等
      ▼
调用方（具体的字段值）
```
