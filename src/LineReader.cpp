#include "LineReader.h"
#include <iostream>

// 构造函数
LineReader::LineReader(const std::string& filepath, size_t bufferSize)
    : file_(nullptr)
    , buffer_(nullptr)
    , bufferSize_(bufferSize)
    , bufferPos_(0)
    , validBytes_(0)
    , fileEof_(false)
    , hasUnfinishedLine_(false)
    , filepath_(filepath)
{
    // 参数验证
    if (filepath.empty()) {
        throw LineReaderException("File path cannot be empty");
    }

    if (bufferSize < 1024) {
        throw LineReaderException("Buffer size must be at least 1024 bytes");
    }

    // 以二进制模式打开文件，避免自动换行符转换
    file_ = fopen(filepath.c_str(), "rb");
    if (!file_) {
        throw LineReaderException("Cannot open file: " + filepath);
    }

    // 分配缓冲区
    buffer_ = static_cast<char*>(malloc(bufferSize_));
    if (!buffer_) {
        fclose(file_);
        file_ = nullptr;
        throw LineReaderException("Failed to allocate buffer memory");
    }

    // 预留行缓冲区空间
    currentLine_.reserve(256);
}

// 析构函数
LineReader::~LineReader() {
    cleanup();
}

// 移动构造函数
LineReader::LineReader(LineReader&& other) noexcept
    : file_(other.file_)
    , buffer_(other.buffer_)
    , bufferSize_(other.bufferSize_)
    , bufferPos_(other.bufferPos_)
    , validBytes_(other.validBytes_)
    , currentLine_(std::move(other.currentLine_))
    , fileEof_(other.fileEof_)
    , hasUnfinishedLine_(other.hasUnfinishedLine_)
    , filepath_(std::move(other.filepath_))
{
    // 清空源对象
    other.file_ = nullptr;
    other.buffer_ = nullptr;
    other.bufferSize_ = 0;
    other.bufferPos_ = 0;
    other.validBytes_ = 0;
    other.fileEof_ = false;
    other.hasUnfinishedLine_ = false;
}

// 移动赋值操作符
LineReader& LineReader::operator=(LineReader&& other) noexcept {
    if (this != &other) {
        // 清理当前资源
        cleanup();

        // 移动资源
        file_ = other.file_;
        buffer_ = other.buffer_;
        bufferSize_ = other.bufferSize_;
        bufferPos_ = other.bufferPos_;
        validBytes_ = other.validBytes_;
        currentLine_ = std::move(other.currentLine_);
        fileEof_ = other.fileEof_;
        hasUnfinishedLine_ = other.hasUnfinishedLine_;
        filepath_ = std::move(other.filepath_);

        // 清空源对象
        other.file_ = nullptr;
        other.buffer_ = nullptr;
        other.bufferSize_ = 0;
        other.bufferPos_ = 0;
        other.validBytes_ = 0;
        other.fileEof_ = false;
        other.hasUnfinishedLine_ = false;
    }
    return *this;
}

// 清理资源
void LineReader::cleanup() {
    if (buffer_) {
        free(buffer_);
        buffer_ = nullptr;
    }

    if (file_) {
        fclose(file_);
        file_ = nullptr;
    }
}

// 填充缓冲区
bool LineReader::fillBuffer() {
    if (!file_ || fileEof_) {
        return false;
    }

    // 如果缓冲区还有未处理的数据，将其移到开头
    if (bufferPos_ < validBytes_) {
        size_t remainingBytes = validBytes_ - bufferPos_;
        memmove(buffer_, buffer_ + bufferPos_, remainingBytes);
        validBytes_ = remainingBytes;
    } else {
        validBytes_ = 0;
    }

    bufferPos_ = 0;

    // 读取新数据
    size_t bytesToRead = bufferSize_ - validBytes_;
    size_t bytesRead = fread(buffer_ + validBytes_, 1, bytesToRead, file_);

    validBytes_ += bytesRead;

    // 检查是否到达文件结尾
    if (bytesRead < bytesToRead) {
        if (feof(file_)) {
            fileEof_ = true;
        } else if (ferror(file_)) {
            throw LineReaderException("Error reading file: " + filepath_);
        }
    }

    return validBytes_ > 0;
}

// 查找行结束位置
size_t LineReader::findLineEnd(size_t startPos, char& lineEndChar) {
    for (size_t i = startPos; i < validBytes_; ++i) {
        char c = buffer_[i];
        if (c == '\n') {
            lineEndChar = '\n';
            return i;
        } else if (c == '\r') {
            // 检查是否是\r\n
            if (i + 1 < validBytes_ && buffer_[i + 1] == '\n') {
                lineEndChar = '\n';  // 标记为\n，但会跳过\r\n
                return i + 1;  // 返回\n的位置
            } else {
                lineEndChar = '\r';
                return i;
            }
        }
    }
    lineEndChar = '\0';  // 未找到行结束符
    return validBytes_;
}

// 提取完整行
std::string LineReader::extractLine(size_t endPos, char lineEndChar) {
    std::string result;

    // 如果有未完成的行，先添加它
    if (hasUnfinishedLine_) {
        result = currentLine_;
        currentLine_.clear();
        hasUnfinishedLine_ = false;
    }

    // 添加当前缓冲区的内容
    if (lineEndChar == '\n' && endPos > 0 && buffer_[endPos - 1] == '\r') {
        // 处理\r\n的情况，不包含\r
        result.append(buffer_ + bufferPos_, endPos - bufferPos_ - 1);
        bufferPos_ = endPos + 1;
    } else if (lineEndChar != '\0') {
        // 找到了行结束符
        result.append(buffer_ + bufferPos_, endPos - bufferPos_);
        bufferPos_ = endPos + 1;
    } else {
        // 没找到行结束符，整个缓冲区都是当前行的一部分
        result.append(buffer_ + bufferPos_, endPos - bufferPos_);
        bufferPos_ = endPos;
    }

    return result;
}

// 核心接口：返回下一行
std::string LineReader::output() {
    if (!isOpen()) {
        throw LineReaderException("File is not open");
    }

    while (true) {
        // 如果缓冲区为空或已读完，尝试填充
        if (bufferPos_ >= validBytes_) {
            if (!fillBuffer()) {
                // 无法读取更多数据
                if (hasUnfinishedLine_) {
                    // 返回最后一行（可能没有换行符结尾）
                    std::string result = currentLine_;
                    currentLine_.clear();
                    hasUnfinishedLine_ = false;
                    return result;
                } else {
                    // 没有更多数据
                    throw LineReaderException("No more lines to read (EOF reached)");
                }
            }
        }

        // 在当前缓冲区中查找行结束符
        char lineEndChar;
        size_t lineEndPos = findLineEnd(bufferPos_, lineEndChar);

        if (lineEndChar != '\0') {
            // 找到了行结束符，提取完整行
            return extractLine(lineEndPos, lineEndChar);
        } else {
            // 没找到行结束符，当前缓冲区的内容是行的一部分
            currentLine_.append(buffer_ + bufferPos_, validBytes_ - bufferPos_);
            hasUnfinishedLine_ = true;
            bufferPos_ = validBytes_;

            // 如果文件已结束，返回这一行
            if (fileEof_) {
                std::string result = currentLine_;
                currentLine_.clear();
                hasUnfinishedLine_ = false;
                return result;
            }

            // 继续读取下一个缓冲区
        }
    }
}

// 检查是否还有更多行
bool LineReader::hasNextLine() const {
    if (!isOpen()) {
        return false;
    }

    // 如果有未完成的行，肯定还有数据
    if (hasUnfinishedLine_) {
        return true;
    }

    // 如果缓冲区还有未处理的数据
    if (bufferPos_ < validBytes_) {
        return true;
    }

    // 如果文件还没到结尾
    if (!fileEof_) {
        return true;
    }

    return false;
}

// 手动关闭文件
void LineReader::close() {
    cleanup();
    bufferPos_ = 0;
    validBytes_ = 0;
    fileEof_ = true;
    hasUnfinishedLine_ = false;
    currentLine_.clear();
}

// 检查文件是否打开
bool LineReader::isOpen() const {
    return file_ != nullptr;
}

// 重置到文件开始
void LineReader::reset() {
    if (!file_) {
        throw LineReaderException("File is not open");
    }

    if (fseek(file_, 0, SEEK_SET) != 0) {
        throw LineReaderException("Failed to reset file position");
    }

    bufferPos_ = 0;
    validBytes_ = 0;
    fileEof_ = false;
    hasUnfinishedLine_ = false;
    currentLine_.clear();
}