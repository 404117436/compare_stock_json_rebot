# StockDataBatchReader

## 职责

StockDataBatchReader 是流式读取模式的核心组件，负责从一个 JSON 行文件中按批次读取股票数据。"批次"的定义是：索引字段值（通常是时间戳）相同的一组记录。它的设计目标是在内存受限的情况下处理超大文件——通过 10MB 的内存软上限控制，确保程序不会因为一次性加载太多数据而耗尽内存。

## 文件组成

| 文件 | 说明 |
|------|------|
| `include/StockDataBatchReader.h` | 类声明，定义内存上限常量和所有接口 |
| `src/StockDataBatchReader.cpp` | 实现，包含批次读取的核心状态机逻辑 |

## 核心类

### StockDataBatchReader

**职责**：维护一个文件读取游标和一个数据队列，每次调用 `readNextBatch()` 时从文件中读取若干条记录，直到遇到时间戳变化且内存超限为止，然后把这批数据交给调用方处理。

**关键成员变量**：
- `std::deque<StockDataContainer> dataQueue_`：当前批次的数据队列，用 deque 方便从头部弹出
- `std::unique_ptr<LineReader> lineReader_`：底层文件读取器，持有文件的读取游标
- `size_t maxMemorySize_`：内存软上限，默认 10MB（`MEM_SIZE` 宏定义）
- `StockDataContainer pendingData_`：临界数据缓存——当读到一条时间戳不同的记录时，这条记录不属于当前批次，需要暂存起来留给下一批次使用
- `bool hasPendingData_`：标记 `pendingData_` 中是否有待处理的临界数据
- `std::string cached_ignore_string_`：忽略字段列表的逗号分隔字符串缓存，避免每条记录都重新拼接

**核心方法**：

#### `size_t readNextBatch()`

批次读取的核心状态机，返回本次读取的记录数：

1. 如果上一批次结束时有临界数据（`hasPendingData_` 为 true），先把它放入队列作为新批次的第一条，并以它的时间戳作为当前批次的基准值
2. 循环调用 `readSingleRecord()` 读取下一条记录
3. 每读到一条，检查它的时间戳是否与当前批次基准值相同：
   - 相同：直接加入队列，继续读取
   - 不同：说明进入了新的时间戳。此时检查当前内存使用量是否超过 10MB 软上限
     - 超限：把这条新记录存入 `pendingData_`，设置 `hasPendingData_` 为 true，停止读取，返回
     - 未超限：更新基准值，继续把新记录加入队列（允许一个批次跨越多个时间戳，直到内存超限）
4. 文件读完后自然退出循环

这个设计的关键在于"临界数据"的处理：读取是贪婪的（尽量多读），但遇到内存超限时会在时间戳边界处干净地切断，不会把同一时间戳的记录拆分到两个批次中。

#### `bool readSingleRecord(StockDataContainer& container)`

读取文件中的一行，跳过空行，用 `StockDataContainer` 解析 JSON，配置好索引字段、精度和忽略字段后返回。解析失败的行会被跳过（记录错误日志但不中断读取）。

#### `bool popBatch(std::vector<StockDataContainer>& result)`

把当前队列中的所有数据移动到 `result` 中，并清空队列。使用移动语义避免数据拷贝。

#### `size_t getCurrentMemoryUsage() const`

估算当前队列的内存占用：遍历队列中每条记录，累加其原始 JSON 字符串的长度。这是一个近似值，但对于控制内存上限已经足够准确。

**注意事项**：
- 内存上限是"软上限"：当内存超限时，程序会在当前时间戳的最后一条记录处停止，而不是立即停止。这意味着实际内存使用可能略超过 10MB，但不会超过"10MB + 一个时间戳的所有记录大小"。
- 忽略字段列表在构造时预计算成逗号分隔字符串并缓存，每次修改忽略字段列表时会触发缓存重建，避免在热路径上重复拼接字符串。

## 与其他模块的交互

- **依赖**：
  - `LineReader`：底层文件读取
  - `StockDataContainer`：每行数据的解析和存储
- **被依赖**：`main.cpp` 中的流式对比模式和分组对比模式使用本类读取数据

## 数据流

```
磁盘文件
   │  LineReader（逐行）
   ▼
一行 JSON 字符串
   │  StockDataContainer::parseFromJsonString()
   ▼
StockDataContainer（含 code、index_value、所有字段）
   │  按时间戳分批，内存超限时切断
   ▼
deque<StockDataContainer>（当前批次）
   │  popBatch()
   ▼
调用方（vector<StockDataContainer>，交给 StockDataComparator）
```
