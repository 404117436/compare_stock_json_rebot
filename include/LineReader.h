#pragma once

#include <string>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <cstring>

class LineReaderException : public std::runtime_error {
public:
    explicit LineReaderException(const std::string& message)
        : std::runtime_error("LineReader Error: " + message) {}
};

class LineReader {
private:
    static const size_t DEFAULT_BUFFER_SIZE = 8192;  // 8KB缓冲区

    FILE* file_;                    // C标准库文件指针
    char* buffer_;                  // 读取缓冲区
    size_t bufferSize_;            // 缓冲区大小
    size_t bufferPos_;             // 当前在缓冲区中的位置
    size_t validBytes_;            // 缓冲区中有效字节数
    std::string currentLine_;      // 当前构建的行字符串
    bool fileEof_;                 // 文件结束标志
    bool hasUnfinishedLine_;       // 是否有未完成的行
    std::string filepath_;         // 文件路径（用于错误信息）

    // 私有辅助方法
    bool fillBuffer();             // 填充缓冲区
    size_t findLineEnd(size_t startPos, char& lineEndChar); // 查找行结束位置
    std::string extractLine(size_t endPos, char lineEndChar); // 提取完整行
    void cleanup();                // 清理资源

public:
    // 构造函数和析构函数
    explicit LineReader(const std::string& filepath, size_t bufferSize = DEFAULT_BUFFER_SIZE);
    ~LineReader();

    // 禁用拷贝构造和赋值操作符（文件指针不适合拷贝）
    LineReader(const LineReader&) = delete;
    LineReader& operator=(const LineReader&) = delete;

    // 移动构造和移动赋值
    LineReader(LineReader&& other) noexcept;
    LineReader& operator=(LineReader&& other) noexcept;

    // 核心接口
    std::string output();          // 返回下一行数据
    bool hasNextLine() const;      // 检查是否还有更多行
    void close();                  // 手动关闭文件
    bool isOpen() const;           // 检查文件是否打开

    // 状态查询接口
    const std::string& getFilePath() const { return filepath_; }
    size_t getBufferSize() const { return bufferSize_; }
    bool isEof() const { return fileEof_ && bufferPos_ >= validBytes_ && !hasUnfinishedLine_; }

    // 重置到文件开始（重新定位到文件开头）
    void reset();
};