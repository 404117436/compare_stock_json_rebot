#ifndef FULLFILELOADER_H
#define FULLFILELOADER_H

#include <string>
#include <vector>
#include "StockDataContainer.h"

/**
 * 全量文件加载器
 * 功能：一次性读取整个JSON行文件到内存
 * 适用场景：中小型文件（<500MB），追求最快速度
 */
class FullFileLoader {
public:
    /**
     * 加载整个文件的所有记录
     * @param filepath 文件路径
     * @param indexKey 索引字段名（默认"time"）
     * @param indexDecimal 索引精度（默认1）
     * @param ignoreFields 忽略字段列表
     * @param compareKey 比较键字段名（默认空）
     * @param maxMemoryMB 内存上限（MB），超过则抛出异常（默认1024MB）
     * @return 所有记录的容器列表
     */
    static std::vector<StockDataContainer> loadAllRecords(
        const std::string& filepath,
        const std::string& indexKey = "time",
        int64_t indexDecimal = 1,
        const std::vector<std::string>& ignoreFields = {},
        const std::string& compareKey = "",
        size_t maxMemoryMB = 1024
    );

    /**
     * 估算文件行数（快速扫描）
     */
    static size_t estimateLineCount(const std::string& filepath);

private:
    /**
     * 将忽略字段转换为逗号分隔字符串
     */
    static std::string joinIgnoreFields(const std::vector<std::string>& fields);
};

#endif // FULLFILELOADER_H
