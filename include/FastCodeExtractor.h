#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

/**
 * FastCodeExtractor - 高性能 JSON code 字段提取器
 *
 * 专门用于从 JSON 行文件中快速提取 "code" 字段值
 *
 * 性能优化：
 * - rapidjson insitu 解析（零拷贝）
 * - vector 预分配减少扩容
 * - LineReader 64KB 缓冲 I/O
 * - 跳过不必要的字段解析
 *
 * 使用示例：
 * ```cpp
 * // 方式1：一次性提取所有 code
 * auto codes = FastCodeExtractor::extractCodes("stock.log");
 *
 * // 方式2：流式提取（适合超大文件）
 * std::vector<std::string> codes;
 * FastCodeExtractor::extractCodesStreaming("stock.log", codes);
 * ```
 */
class FastCodeExtractor {
public:
    /**
     * 从 JSON 行文件中提取所有 code 字段值
     *
     * @param filepath JSON 文件路径（每行一个 JSON 对象）
     * @param reserveSize vector 预分配大小（默认10万）
     * @return 包含所有 code 值的 vector
     * @throws std::runtime_error 文件读取失败时抛出
     */
    static std::vector<std::string> extractCodes(
        const std::string& filepath,
        size_t reserveSize = 100000
    );

    /**
     * 流式提取 code 字段值（追加到已有 vector）
     *
     * 适合超大文件或需要多次追加的场景
     *
     * @param filepath JSON 文件路径
     * @param codes 输出 vector（结果追加到此 vector）
     * @param reserveSize 额外预分配大小
     * @throws std::runtime_error 文件读取失败时抛出
     */
    static void extractCodesStreaming(
        const std::string& filepath,
        std::vector<std::string>& codes,
        size_t reserveSize = 100000
    );

    /**
     * 从 JSON 行文件中提取所有唯一的 code 字段值（去重）
     *
     * @param filepath JSON 文件路径
     * @param reserveSize vector 预分配大小
     * @return 包含所有唯一 code 值的 vector（保持出现顺序）
     * @throws std::runtime_error 文件读取失败时抛出
     */
    static std::vector<std::string> extractUniqueCodes(
        const std::string& filepath,
        size_t reserveSize = 100000
    );

    /**
     * 统计信息结构
     */
    struct SplitStats {
        size_t totalLines;          // 总行数
        size_t successLines;        // 成功处理的行数
        size_t failedLines;         // 失败的行数
        size_t uniqueCodes;         // 唯一 code 数量
        std::unordered_map<std::string, size_t> codeCount;  // 每个 code 的记录数
    };

    /**
     * 按 code 字段分组，将 JSON 行写入不同文件
     *
     * 每个 code 生成一个独立的输出文件：<outputDir>/<code>.txt
     * 流式处理，内存占用小，适合处理超大文件
     *
     * @param inputFilepath 输入 JSON 文件路径
     * @param outputDir 输出目录路径（自动创建）
     * @return 统计信息
     * @throws std::runtime_error 文件操作失败时抛出
     *
     * 示例：
     * ```cpp
     * auto stats = FastCodeExtractor::splitByCode("stock.log", "./output");
     * // 生成文件：./output/688001.txt, ./output/688002.txt, ...
     * ```
     */
    static SplitStats splitByCode(
        const std::string& inputFilepath,
        const std::string& outputDir
    );

private:
    static const size_t DEFAULT_RESERVE_SIZE = 100000;  // 默认预分配10万条
    static const size_t BUFFER_SIZE = 65536;            // LineReader 缓冲区大小

    // 禁止实例化（工具类）
    FastCodeExtractor() = delete;
    ~FastCodeExtractor() = delete;
    FastCodeExtractor(const FastCodeExtractor&) = delete;
    FastCodeExtractor& operator=(const FastCodeExtractor&) = delete;
};
