# StockDataContainer

## 职责

StockDataContainer 是 `GenericJsonContainer` 的子类，在通用 JSON 容器的基础上增加了股票行情数据特有的业务逻辑。它的核心贡献是：解析完一条 JSON 记录后，自动把最关键的两个字段——股票代码（`code`）和时间戳（由用户指定的索引字段）——提取出来缓存为成员变量，方便后续的分组和比对操作。它还保存了原始 JSON 字符串，以便在输出差异报告时能还原原始数据。

## 文件组成

| 文件 | 说明 |
|------|------|
| `include/StockDataContainer.h` | 类声明，定义了所有成员变量和方法签名 |
| `src/StockDataContainer.cpp` | 实现，重点是 `extractKeyFields()` 的字段提取逻辑 |

## 核心类

### StockDataContainer

**职责**：作为整个项目中数据流转的基本单元，每一条 JSON 行数据对应一个 `StockDataContainer` 实例。它既是数据的载体（通过继承的 `data_` 存储所有字段），也是比对的索引（通过 `code_` 和 `index_value_` 快速定位记录）。

**关键成员变量**：
- `std::string code_`：股票代码，固定从 JSON 的 `"code"` 字段提取，是记录分组的第一维度
- `std::string index_key_`：用户指定的索引字段名，默认为 `"time"`，决定用哪个字段作为时间戳
- `int64_t index_value_`：索引字段的值经过精度处理后的整数形式，是记录匹配的第二维度
- `int64_t index_decimal_`：精度控制参数，默认为 1。当时间戳是浮点数时（如 `1234567890.123`），乘以这个倍数转成整数（如设为 1000 则保留毫秒精度）
- `std::string compare_key_`：可选的第三维度比较键，用于在同一 code + 时间戳下进一步区分记录
- `std::string raw_json_`：原始 JSON 字符串，在解析前保存，用于差异报告中展示原始数据

**核心方法**：

#### `bool parseFromJsonString(const std::string& jsonStr)`

重写父类的解析方法，在调用父类解析之前先把原始字符串保存到 `raw_json_`，解析成功后调用 `extractKeyFields()` 提取关键字段。

#### `void extractKeyFields()`

解析完成后自动运行的字段提取逻辑：

1. 提取 `code` 字段：在已解析的 `data_` 中查找 `"code"` 键，支持字符串和整数两种类型（有些股票代码存为数字）
2. 提取索引字段：查找 `index_key_` 指定的字段（默认 `"time"`），把它的值转换成字符串，再调用 `convertIndexToComparableValue()` 转成可比较的整数
3. 提取比较键字段（如果配置了 `compare_key_`）：类似索引字段的提取，但不做精度转换，直接存为整数

#### `int64_t convertIndexToComparableValue(const std::string& indexValue) const`

把索引字段的字符串值转换成可以直接用整数比较的形式：
1. 如果字符串中包含小数点，说明是浮点数时间戳，把小数点去掉后乘以 `index_decimal_` 得到整数
2. 如果是纯整数字符串，直接乘以 `index_decimal_`

这个设计的意图是：两个文件中同一时刻的时间戳可能一个是 `"1234567890"` 另一个是 `"1234567890.000"`，通过统一转换成整数后就能正确匹配。

**注意事项**：
- `extractKeyFields()` 中所有操作都用 try-catch 包裹，提取失败时静默置空，不会导致整条记录解析失败。这是一种容错设计，允许部分字段缺失的记录仍然被加载进来。

## 与其他模块的交互

- **依赖**：继承自 `GenericJsonContainer`，所有字段存储和访问能力来自父类
- **被依赖**：
  - `StockDataBatchReader` 创建并填充 `StockDataContainer` 对象
  - `FullFileLoader` 批量创建 `StockDataContainer` 对象
  - `StockDataComparator` 消费 `StockDataContainer` 对象，读取 `code_`、`index_value_` 进行匹配，读取所有字段进行比对

## 数据流

```
一行 JSON 字符串
      │  parseFromJsonString()
      ▼
父类 GenericJsonContainer 解析 → data_（所有字段）
      │  extractKeyFields()
      ▼
code_（股票代码）
index_value_（时间戳整数）
compare_value_（可选第三键）
raw_json_（原始字符串备份）
      │
      ▼
StockDataComparator 用 code_ + index_value_ 匹配记录
StockDataComparator 用 data_ 中的字段逐一比对
```
