#include "FastCodeExtractor.h"
#include "LineReader.h"
#include "rapidjson/document.h"
#include <unordered_set>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sstream>

// 从 JSON 行文件中提取所有 code 字段值
std::vector<std::string> FastCodeExtractor::extractCodes(
    const std::string& filepath,
    size_t reserveSize
) {
    std::vector<std::string> codes;
    extractCodesStreaming(filepath, codes, reserveSize);
    return codes;
}

// 流式提取 code 字段值
void FastCodeExtractor::extractCodesStreaming(
    const std::string& filepath,
    std::vector<std::string>& codes,
    size_t reserveSize
) {
    // 预分配 vector 容量，避免频繁扩容
    codes.reserve(codes.size() + reserveSize);

    // 使用 LineReader 高效读取文件（64KB 缓冲）
    LineReader reader(filepath, BUFFER_SIZE);

    // 用于 insitu 解析的缓冲区
    std::string lineBuffer;
    lineBuffer.reserve(4096);  // 预分配行缓冲，避免小内存分配

    size_t lineCount = 0;
    size_t successCount = 0;
    size_t failCount = 0;

    // 逐行读取并解析
    while (reader.hasNextLine()) {
        lineCount++;

        try {
            // 读取一行 JSON
            std::string line = reader.output();

            // 跳过空行
            if (line.empty()) {
                continue;
            }

            // rapidjson insitu 解析需要可修改的字符串
            // 为了性能，我们直接修改 line（避免额外拷贝）
            rapidjson::Document doc;

            // ParseInsitu 会修改原字符串，但性能更好（零拷贝）
            // 注意：line 必须保持有效直到 Document 使用完毕
            doc.ParseInsitu(&line[0]);

            // 检查解析是否成功
            if (doc.HasParseError()) {
                failCount++;
                continue;  // 跳过解析失败的行
            }

            // 检查是否为对象类型
            if (!doc.IsObject()) {
                failCount++;
                continue;
            }

            // 快速检查并提取 "code" 字段
            if (doc.HasMember("code") && doc["code"].IsString()) {
                codes.emplace_back(doc["code"].GetString());
                successCount++;
            } else {
                failCount++;
            }

        } catch (const std::exception& e) {
            // 捕获任何异常，跳过当前行继续处理
            failCount++;
        }
    }

    // 可选：输出统计信息（调试用）
    // std::cerr << "Total lines: " << lineCount
    //           << ", Success: " << successCount
    //           << ", Failed: " << failCount << std::endl;
}

// 提取唯一的 code 值（去重）
std::vector<std::string> FastCodeExtractor::extractUniqueCodes(
    const std::string& filepath,
    size_t reserveSize
) {
    std::vector<std::string> codes;
    codes.reserve(reserveSize);

    // 使用 unordered_set 去重（O(1) 查找）
    std::unordered_set<std::string> seenCodes;
    seenCodes.reserve(reserveSize);

    // 使用 LineReader 高效读取文件
    LineReader reader(filepath, BUFFER_SIZE);

    size_t lineCount = 0;
    size_t successCount = 0;

    // 逐行读取并解析
    while (reader.hasNextLine()) {
        lineCount++;

        try {
            std::string line = reader.output();

            if (line.empty()) {
                continue;
            }

            rapidjson::Document doc;
            doc.ParseInsitu(&line[0]);

            if (doc.HasParseError() || !doc.IsObject()) {
                continue;
            }

            // 提取 code 字段
            if (doc.HasMember("code") && doc["code"].IsString()) {
                std::string code = doc["code"].GetString();

                // 只添加未见过的 code（保持第一次出现的顺序）
                if (seenCodes.find(code) == seenCodes.end()) {
                    seenCodes.insert(code);
                    codes.push_back(std::move(code));
                    successCount++;
                }
            }

        } catch (const std::exception& e) {
            // 跳过异常行
        }
    }

    return codes;
}

// 按 code 字段分组，将 JSON 行写入不同文件
FastCodeExtractor::SplitStats FastCodeExtractor::splitByCode(
    const std::string& inputFilepath,
    const std::string& outputDir
) {
    SplitStats stats = {0, 0, 0, 0, {}};

    // 创建输出目录（如果不存在）
    struct stat st;
    if (stat(outputDir.c_str(), &st) != 0) {
        // 目录不存在，创建它
        if (mkdir(outputDir.c_str(), 0755) != 0) {
            throw std::runtime_error("Failed to create output directory: " + outputDir);
        }
    }

    // 文件句柄映射：code -> FILE*
    std::unordered_map<std::string, FILE*> fileHandles;

    try {
        // 使用 LineReader 高效读取文件
        LineReader reader(inputFilepath, BUFFER_SIZE);

        // 逐行读取并分组写入
        while (reader.hasNextLine()) {
            stats.totalLines++;

            try {
                // 读取原始行（保持原样，不修改）
                std::string originalLine = reader.output();

                // 跳过空行
                if (originalLine.empty()) {
                    continue;
                }

                // 为了提取 code，需要解析 JSON
                // 创建一个副本用于 insitu 解析
                std::string lineCopy = originalLine;
                rapidjson::Document doc;
                doc.ParseInsitu(&lineCopy[0]);

                // 检查解析是否成功
                if (doc.HasParseError() || !doc.IsObject()) {
                    stats.failedLines++;
                    continue;
                }

                // 提取 code 字段
                if (!doc.HasMember("code") || !doc["code"].IsString()) {
                    stats.failedLines++;
                    continue;
                }

                std::string code = doc["code"].GetString();

                // 如果这个 code 还没有打开文件，则创建文件
                if (fileHandles.find(code) == fileHandles.end()) {
                    std::string outputFilepath = outputDir + "/" + code + ".txt";
                    FILE* fp = fopen(outputFilepath.c_str(), "w");
                    if (!fp) {
                        throw std::runtime_error("Failed to create file: " + outputFilepath);
                    }
                    fileHandles[code] = fp;
                    stats.uniqueCodes++;
                }

                // 写入原始行到对应的文件（保持原始 JSON 格式）
                FILE* fp = fileHandles[code];
                fprintf(fp, "%s\n", originalLine.c_str());

                // 更新统计信息
                stats.successLines++;
                stats.codeCount[code]++;

            } catch (const std::exception& e) {
                stats.failedLines++;
            }
        }

        // 关闭所有文件句柄
        for (auto& pair : fileHandles) {
            if (pair.second) {
                fclose(pair.second);
            }
        }

    } catch (...) {
        // 确保所有文件句柄都被关闭（异常安全）
        for (auto& pair : fileHandles) {
            if (pair.second) {
                fclose(pair.second);
            }
        }
        throw;  // 重新抛出异常
    }

    return stats;
}
