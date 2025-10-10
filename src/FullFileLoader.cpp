#include "FullFileLoader.h"
#include "LineReader.h"
#include <iostream>
#include <sys/stat.h>

// 快速估算文件行数
size_t FullFileLoader::estimateLineCount(const std::string& filepath) {
    struct stat st;
    if (stat(filepath.c_str(), &st) != 0) {
        return 0;
    }

    // 估算：假设平均每行 500 字节（股票JSON通常200-800字节）
    return st.st_size / 500;
}

// 将忽略字段转换为逗号分隔字符串
std::string FullFileLoader::joinIgnoreFields(const std::vector<std::string>& fields) {
    if (fields.empty()) return "";

    std::string result;
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) result += ",";
        result += fields[i];
    }
    return result;
}

// 加载整个文件的所有记录
std::vector<StockDataContainer> FullFileLoader::loadAllRecords(
    const std::string& filepath,
    const std::string& indexKey,
    int64_t indexDecimal,
    const std::vector<std::string>& ignoreFields,
    const std::string& compareKey,
    size_t maxMemoryMB
) {
    std::vector<StockDataContainer> records;

    // 1. 检查文件大小（防止OOM）
    struct stat st;
    if (stat(filepath.c_str(), &st) != 0) {
        throw std::runtime_error("无法访问文件: " + filepath);
    }

    size_t fileSizeMB = st.st_size / (1024 * 1024);
    if (fileSizeMB > maxMemoryMB) {
        throw std::runtime_error(
            "文件过大（" + std::to_string(fileSizeMB) +
            "MB），超过内存限制（" + std::to_string(maxMemoryMB) +
            "MB）。建议使用流式模式。"
        );
    }

    std::cout << "📂 文件大小: " << fileSizeMB << " MB" << std::endl;

    // 2. 估算行数，预分配vector容量
    size_t estimatedLines = estimateLineCount(filepath);
    records.reserve(estimatedLines);
    std::cout << "📊 估算记录数: " << estimatedLines << std::endl;

    // 3. 预计算忽略字段字符串（性能优化）
    std::string ignoreString = joinIgnoreFields(ignoreFields);

    // 4. 使用 LineReader 读取所有行
    LineReader reader(filepath);

    size_t lineNum = 0;
    size_t successCount = 0;
    size_t failCount = 0;

    while (reader.hasNextLine()) {
        lineNum++;

        try {
            std::string jsonLine = reader.output();

            // 跳过空行
            if (jsonLine.empty() || jsonLine.find_first_not_of(" \t\r\n") == std::string::npos) {
                continue;
            }

            // 创建容器
            StockDataContainer container("FullLoad", indexKey, indexDecimal);

            // 设置比较键（如果有）
            if (!compareKey.empty()) {
                container.setCompareKey(compareKey);
            }

            // 解析JSON
            bool parseSuccess = false;
            if (ignoreFields.empty()) {
                parseSuccess = container.parseFromJsonString(jsonLine);
            } else {
                parseSuccess = container.parseFromJsonString(jsonLine, ignoreString);
            }

            if (parseSuccess) {
                records.push_back(std::move(container));
                successCount++;
            } else {
                failCount++;
            }

            // 进度显示（每10000行）
            if (lineNum % 10000 == 0) {
                std::cout << "\r⏳ 已读取 " << lineNum << " 行，成功 "
                         << successCount << " 条" << std::flush;
            }

        } catch (const std::exception& e) {
            failCount++;
        }
    }

    std::cout << "\r✅ 加载完成: " << successCount << " 条成功，"
             << failCount << " 条失败    " << std::endl;

    return records;
}
