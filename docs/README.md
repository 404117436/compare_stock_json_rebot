# compare_stock_json_rebot 技术文档

## 项目概述

这是一个专门用于对比两份股票行情 JSON 数据文件的命令行工具。它的核心价值在于：对于每一支股票（每个 code）、每一个时间戳，找出两个文件之间数据不一致的地方，并将差异按股票代码分类写入报告文件。项目面向的场景是行情数据系统的回归测试或双轨验证——即同一套数据由两个来源生成，需要逐字段校验它们是否完全吻合。

## 整体架构

数据从文件读入，经过解析、分组、比对，最终将差异写出到磁盘。整个流程如下：

```
输入文件 A / 输入文件 B
         │
         ▼
  ┌─────────────────────────────────────────────┐
  │             三种读取模式（在 main.cpp 中选择）        │
  │                                             │
  │  流式模式         分组模式           全量模式         │
  │  (BatchReader)   (FastCode+         (FullFile       │
  │                   FullFileLoader)    Loader)        │
  └─────────────────────────────────────────────┘
         │
         ▼
  ┌────────────────────┐
  │  StockDataContainer │  ← 每条 JSON 行解析为一个容器对象
  │  (继承 GenericJson  │    保存 code、时间戳、所有字段值
  │   Container)       │    以及原始 JSON 字符串
  └────────────────────┘
         │
         ▼
  ┌──────────────────────┐
  │  StockDataComparator  │  ← 核心对比引擎
  │                       │    按 code + 时间戳 匹配记录
  │                       │    逐字段比较，收集 FieldDifference
  └──────────────────────┘
         │
         ▼
  按股票代码分组 → 写入 <outputDir>/<code>.txt
```

底层依赖关系（从最底层到最顶层）：

```
rapidjson (第三方库)
    └── JsonParser        ← 封装 rapidjson，提供类型安全的访问接口
         └── GenericJsonContainer  ← 通用 JSON 键值容器（哈希表存储）
              └── StockDataContainer  ← 股票专用容器（缓存 code 和时间戳）
                   ├── StockDataBatchReader   ← 流式批量读取
                   ├── FullFileLoader         ← 全量一次性加载
                   └── StockDataComparator    ← 字段级差异对比

LineReader  ← 高性能逐行文件读取（64KB 缓冲区）
    ├── StockDataBatchReader（间接使用）
    ├── FullFileLoader（间接使用）
    └── FastCodeExtractor  ← 专用于按 code 拆分文件
```

## 模块列表

| 模块 | 主要文件 | 职责 |
|------|----------|------|
| LineReader | `include/LineReader.h`, `src/LineReader.cpp` | 带缓冲区的高性能逐行文件读取器 |
| JsonParser | `include/JsonParser.h`, `src/JsonParser.cpp` | 封装 rapidjson，提供面向对象的 JSON 解析接口 |
| GenericJsonContainer | `include/GenericJsonContainer.h`, `src/GenericJsonContainer.cpp` | 通用 JSON 键值容器，支持任意类型的值存储与读取 |
| StockDataContainer | `include/StockDataContainer.h`, `src/StockDataContainer.cpp` | 股票行情专用容器，自动提取 code 和时间戳字段 |
| StockDataBatchReader | `include/StockDataBatchReader.h`, `src/StockDataBatchReader.cpp` | 流式批量读取器，按时间戳分批加载数据 |
| StockDataComparator | `include/StockDataComparator.h`, `src/StockDataComparator.cpp` | 核心比对引擎，执行字段级差异分析 |
| FastCodeExtractor | `include/FastCodeExtractor.h`, `src/FastCodeExtractor.cpp` | 高性能 code 字段提取与按 code 拆分文件 |
| FullFileLoader | `include/FullFileLoader.h`, `src/FullFileLoader.cpp` | 全量文件加载器，适合中小型文件一次性读入内存 |
| 主程序入口 | `example/main.cpp` | 命令行参数解析、三种对比模式的调度与报告输出 |

## 核心数据结构

- **`StockDataContainer`**：贯穿整个项目最核心的类。每一条 JSON 行数据被解析成一个 `StockDataContainer` 对象，它不仅保存所有字段（存在 `GenericJsonContainer` 的哈希表中），还额外缓存了 `code`（股票代码）、`index_value_`（经过精度处理后的时间戳整数）以及原始 JSON 字符串，是整个比对流程的"数据载体"。

- **`CustomValue`**：项目自己实现的 JSON 值类型，能表示 null、bool、int、double、string、array、object 这七种类型。使用 C 语言的 union 联合体节省内存，同时手动管理指针生命周期（堆上分配 string、array、object）。它是所有字段值在内存中的统一表示。

- **`ComparisonResult` / `RecordComparisonDetail` / `FieldDifference`**：三层嵌套的对比结果结构。`FieldDifference` 描述单个字段的差异（字段名、A侧值、B侧值、差异类型），`RecordComparisonDetail` 汇总一条记录的所有字段差异，`ComparisonResult` 是整次对比的全局汇总（包括统计数量、相似度和所有记录差异列表）。

## 阅读建议

建议按以下顺序阅读各模块文档，由底向上，从简单到复杂：

1. **[LineReader](line-reader.md)**：了解文件读取的基础设施
2. **[JsonParser](json-parser.md)**：了解 JSON 解析层
3. **[GenericJsonContainer](generic-json-container.md)**：了解通用容器结构
4. **[StockDataContainer](stock-data-container.md)**：了解股票数据如何被表示
5. **[StockDataBatchReader](stock-data-batch-reader.md)** 和 **[FullFileLoader](full-file-loader.md)**：了解两种数据读取策略
6. **[FastCodeExtractor](fast-code-extractor.md)**：了解分组拆分工具
7. **[StockDataComparator](stock-data-comparator.md)**：了解核心比对逻辑
