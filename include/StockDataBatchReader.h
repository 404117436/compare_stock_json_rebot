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
    int64_t container_decimal_;                      // 创建容器时使用的精度参数
    std::string indexKey_;                           // 索引字段名称
    std::vector<std::string> ignore_fields_;         // 需要忽略的字段列表

    // 临界数据处理
    StockDataContainer pendingData_;                 // 缓存的临界数据
    bool hasPendingData_;                           // 是否有待处理数据

    // 内部辅助方法
    bool readSingleRecord(StockDataContainer& container); // 读取单条记录
    std::string joinIgnoreFields() const;            // 将忽略字段转换为逗号分隔字符串

public:
    // 构造函数
    StockDataBatchReader(const std::string& filePath,
                        const std::string& indexKey = "time",   // 默认使用time作为索引字段
                        size_t maxMemorySize = 1024 * 1024 * 100,
                        int64_t indexDecimal = 1,               // 默认1MB内存，精度为1
                        const std::vector<std::string>& ignoreFields = {}); // 默认不忽略任何字段

    // 禁用拷贝
    StockDataBatchReader(const StockDataBatchReader&) = delete;
    StockDataBatchReader& operator=(const StockDataBatchReader&) = delete;

    // 核心接口
    size_t readNextBatch();                                         // 读取下一批次
    bool popBatch(std::vector<StockDataContainer>& result);         // 取出当前批次
    std::vector<StockDataContainer> getBatch() const;               // 获取批次（拷贝）
    bool hasCompleteBatch() const;                                  // 检查是否有完整批次
    void clearBatch();                                              // 清空批次

    // 状态查询
    size_t getBatchSize() const { return dataQueue_.size(); }
    bool isEmpty() const { return dataQueue_.empty(); }
    std::string getFilePath() const { return filePath_; }
    size_t getCurrentMemoryUsage() const;                                  // 计算当前内存使用


    // 索引字段控制
    const std::string& getIndexKey() const { return indexKey_; }          // 获取索引字段名
    void setIndexKey(const std::string& indexKey) { indexKey_ = indexKey; } // 设置索引字段名

    // 字段过滤控制
    void setIgnoreFields(const std::vector<std::string>& fields);          // 设置忽略字段列表
    const std::vector<std::string>& getIgnoreFields() const { return ignore_fields_; } // 获取忽略字段列表
    void addIgnoreField(const std::string& field);                        // 添加忽略字段
    void removeIgnoreField(const std::string& field);                     // 移除忽略字段
    void clearIgnoreFields() { ignore_fields_.clear(); }                  // 清空忽略字段列表
};