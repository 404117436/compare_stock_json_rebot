# StockDataComparator

## 职责

StockDataComparator 是整个项目的核心比对引擎，负责接收两个股票数据集合（A 和 B），找出它们之间所有的差异，并详细记录每一条记录、每一个字段的差异情况。它不关心数据是如何读取的（流式还是全量），只专注于"给我两组数据，告诉我它们哪里不同"这一件事。

## 文件组成

| 文件 | 说明 |
|------|------|
| `include/StockDataComparator.h` | 类声明，包含 `FieldDifference`、`RecordComparisonDetail`、`ComparisonResult` 三个结构体定义 |
| `src/StockDataComparator.cpp` | 完整实现，包含记录匹配、字段逐一比对、差异收集逻辑 |

## 核心数据结构

### FieldDifference

描述单个字段的差异，包含：字段名、该字段在 A/B 中是否存在、A/B 中各自的值、差异类型（`missing_in_A`、`missing_in_B`、`value_different`、`type_mismatch`）。

### RecordComparisonDetail

一条记录的完整比对结果：是否完全相同（`identical`）、所有字段差异的列表、记录的唯一键（`recordKey`，格式为 `code_时间戳`）、A/B 侧的原始 JSON 字符串。

### ComparisonResult

整次比对的全局汇总：A/B 各自的记录数、仅在 A/B 中存在的记录数、完全匹配的记录数、相似度（0.0-1.0）、所有有差异记录的详细列表。

## 核心类

### StockDataComparator

**职责**：接收两个 `vector<StockDataContainer>`，执行两阶段比对——先按记录键（code + 时间戳）分组匹配，再对匹配到的记录进行逐字段比对。

**关键成员变量**：
- `std::vector<StockDataContainer> a_`：数据集 A
- `std::vector<StockDataContainer> b_`：数据集 B
- `double tolerance_`：浮点数比较容差，默认 1e-9（即两个浮点数之差小于这个值就认为相等）

**核心方法**：

#### `std::string generateRecordKey(const StockDataContainer& container) const`

为一条记录生成唯一键，格式为 `code_时间戳`（如果启用了 compare_key，则为 `code_时间戳_compareValue`）。这个键用于在 A 和 B 之间匹配"应该是同一条记录"的数据。

#### `ComparisonResult compareDetailed() const`

整个比对流程的主入口，分两个阶段：

**第一阶段——按键分组**：
1. 遍历 A 中所有记录，为每条记录生成键，构建 `groupA`（键 → A 中该键对应的记录下标列表）
2. 同样处理 B，构建 `groupB`
3. 合并两个分组的所有键为 `allKeys`

**第二阶段——逐组比对**：
遍历 `allKeys` 中的每个键，有三种情况：
- 键只在 A 中有：这些记录在 B 中缺失（`MISSING_IN_B`）
- 键只在 B 中有：这些记录在 A 中缺失（`MISSING_IN_A`）
- 两边都有：调用 `compareRecordFieldsDetailed()` 逐字段比对。如果比对发现记录完全相同，计入 `exactMatches`；如果有差异，把差异详情加入结果

对于两边都有且有多条记录的情况（同一时间戳下同一股票出现多条记录），使用二重循环 + 标记数组做最优匹配：先找完全相同的对，剩下未匹配的记录记录为有差异。

相似度的计算公式：`exactMatches / totalRecords`（完全匹配的记录数除以总记录数）。

#### `RecordComparisonDetail compareRecordFieldsDetailed(const StockDataContainer& a, const StockDataContainer& b) const`

对两条"已经按键匹配好"的记录进行逐字段比对：

1. 先验证基础字段（code、index_value）是否一致，不一致直接标记为不同返回
2. 合并 A 和 B 的所有字段名（去重），但排除索引字段本身（时间戳字段用于匹配，不参与字段内容比对）
3. 对每个字段：
   - 两边都有：调用 `compareCustomValues()` 比较值，不同则创建 `FieldDifference` 记录
   - 只有一边有：直接创建"字段缺失"类型的 `FieldDifference`

#### `bool compareCustomValues(const CustomValue& a, const CustomValue& b) const`

值比较的核心逻辑：
- 首先类型必须相同，类型不同直接返回 false（即使值看起来相同，如整数 1 和浮点数 1.0 也视为不同）
- string：直接用 `==` 比较（利用标准库的 SSE/AVX 优化）
- int：直接用 `==` 比较
- double：用 `abs(a - b) < tolerance_` 比较，避免浮点数精度问题
- array：先比较长度，再逐元素递归比较
- null：两个 null 视为相等

#### `RecordComparisonDetail createMissRecord(const StockDataContainer& record, bool missingInA) const`

为一条仅在某一侧存在的记录创建差异详情。生成一个特殊的 `FieldDifference`，字段名为 `"RECORD_STATUS"`，差异类型为 `"MISSING_IN_A"` 或 `"MISSING_IN_B"`，说明这整条记录在另一侧不存在。

## 与其他模块的交互

- **依赖**：`StockDataContainer`（数据载体，读取所有字段进行比对）
- **被依赖**：`main.cpp` 中三种模式都会调用本类进行字段级比对，并把结果写入差异文件

## 数据流

```
vector<StockDataContainer> A   vector<StockDataContainer> B
              │                              │
              └──────────────┬──────────────┘
                             │  setDataA() / setDataB()
                             ▼
                      StockDataComparator
                             │  compareDetailed()
                             │
              ┌──────────────┴──────────────┐
              │   按 code_时间戳 分组匹配   │
              │   逐字段比对                │
              └──────────────┬──────────────┘
                             ▼
                     ComparisonResult
                    （含所有 RecordComparisonDetail
                      每个含所有 FieldDifference）
                             │
                             ▼
                  main.cpp 按 code 分组写入文件
```
