#pragma once

#include "StockDataContainer.h"
#include "LineReader.h"
#include <deque>
#include <vector>
#include <string>
#include <memory>

/**
 * 股票数据批量读取器
 * 功能：按index_value_连续性批量读取股票数据，支持内存软上限控制
 */
class StockDataBatchReader {
private:
    std::deque<StockDataContainer> dataQueue_;       // 数据队列
    std::string filePath_;                           // 文件路径
    std::unique_ptr<LineReader> lineReader_;         // 行读取器
    size_t maxMemorySize_;                           // 内存软上限（字节）

    // 临界数据处理
    StockDataContainer pendingData_;                 // 缓存的临界数据
    bool hasPendingData_;                           // 是否有待处理数据

    // 内部辅助方法
    bool readSingleRecord(StockDataContainer& container, const std::string& indexKey); // 读取单条记录

public:
    // 构造函数
    StockDataBatchReader(const std::string& filePath, size_t maxMemorySize = 1024 * 1024); // 默认1MB

    // 禁用拷贝
    StockDataBatchReader(const StockDataBatchReader&) = delete;
    StockDataBatchReader& operator=(const StockDataBatchReader&) = delete;

    // 核心接口
    size_t readNextBatch(const std::string& indexKey);              // 读取下一批次
    std::vector<StockDataContainer> popBatch();                     // 取出当前批次
    std::vector<StockDataContainer> getBatch() const;               // 获取批次（拷贝）
    bool hasCompleteBatch() const;                                  // 检查是否有完整批次
    void clearBatch();                                              // 清空批次

    // 状态查询
    size_t getBatchSize() const { return dataQueue_.size(); }
    bool isEmpty() const { return dataQueue_.empty(); }
    std::string getFilePath() const { return filePath_; }
    size_t getCurrentMemoryUsage() const;                                  // 计算当前内存使用
};