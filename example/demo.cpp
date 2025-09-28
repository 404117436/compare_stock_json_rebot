#include <iostream>
#include <iomanip>
#include <chrono>
#include "../include/LineReader.h"

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void testBasicReading(const std::string& filename) {
    printSeparator("基本读取测试: " + filename);

    try {
        LineReader reader(filename);
        int lineNumber = 1;

        while (reader.hasNextLine()) {
            std::string line = reader.output();
            std::cout << "行 " << std::setw(3) << lineNumber++
                      << ": [" << line << "]" << std::endl;
        }

        std::cout << "\n✓ 文件读取完成，共读取 " << (lineNumber - 1) << " 行" << std::endl;

    } catch (const LineReaderException& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
    }
}

void testWindowsLineEndings() {
    printSeparator("Windows换行符测试");

    try {
        LineReader reader("../example/test_windows.txt");
        int lineNumber = 1;

        while (reader.hasNextLine()) {
            std::string line = reader.output();
            std::cout << "行 " << lineNumber++ << ": [" << line << "]" << std::endl;
        }

    } catch (const LineReaderException& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
    }
}

void testBufferSize() {
    printSeparator("不同缓冲区大小测试");

    // 测试很小的缓冲区（1KB）
    try {
        std::cout << "使用 1KB 缓冲区：" << std::endl;
        LineReader reader("../example/test_large.txt", 1024);
        int count = 0;

        while (reader.hasNextLine()) {
            std::string line = reader.output();
            std::cout << "  行 " << ++count << ": "
                      << (line.length() > 50 ? line.substr(0, 50) + "..." : line)
                      << std::endl;
        }

        std::cout << "✓ 小缓冲区测试完成，共 " << count << " 行" << std::endl;

    } catch (const LineReaderException& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
    }
}

void testErrorHandling() {
    printSeparator("错误处理测试");

    // 测试不存在的文件
    try {
        std::cout << "测试打开不存在的文件..." << std::endl;
        LineReader reader("nonexistent_file.txt");
        std::cout << "❌ 应该抛出异常！" << std::endl;
    } catch (const LineReaderException& e) {
        std::cout << "✓ 正确捕获异常: " << e.what() << std::endl;
    }

    // 测试EOF后继续读取
    try {
        std::cout << "\n测试EOF后继续读取..." << std::endl;
        LineReader reader("../example/test_data.txt");

        // 读取所有行
        while (reader.hasNextLine()) {
            reader.output();
        }

        // 尝试继续读取
        std::cout << "文件已读完，尝试继续读取..." << std::endl;
        std::string line = reader.output();
        std::cout << "❌ 应该抛出异常！" << std::endl;

    } catch (const LineReaderException& e) {
        std::cout << "✓ 正确捕获EOF异常: " << e.what() << std::endl;
    }
}

void testMoveSemantics() {
    printSeparator("移动语义测试");

    try {
        // 创建LineReader
        LineReader reader1("../example/test_data.txt");
        std::cout << "✓ 创建第一个reader: " << reader1.getFilePath() << std::endl;

        // 读取第一行
        if (reader1.hasNextLine()) {
            std::string line = reader1.output();
            std::cout << "✓ 读取第一行: [" << line << "]" << std::endl;
        }

        // 移动构造
        LineReader reader2 = std::move(reader1);
        std::cout << "✓ 移动构造完成" << std::endl;

        // 从移动后的对象继续读取
        if (reader2.hasNextLine()) {
            std::string line = reader2.output();
            std::cout << "✓ 从移动后的对象读取: [" << line << "]" << std::endl;
        }

        // 检查原对象状态
        if (!reader1.isOpen()) {
            std::cout << "✓ 原对象已正确清空" << std::endl;
        }

    } catch (const LineReaderException& e) {
        std::cerr << "❌ 移动语义测试失败: " << e.what() << std::endl;
    }
}

void testReset() {
    printSeparator("重置功能测试");

    try {
        LineReader reader("../example/test_data.txt");

        // 读取前几行
        std::cout << "第一次读取：" << std::endl;
        for (int i = 0; i < 3 && reader.hasNextLine(); ++i) {
            std::string line = reader.output();
            std::cout << "  行 " << (i + 1) << ": [" << line << "]" << std::endl;
        }

        // 重置到文件开头
        reader.reset();
        std::cout << "\n重置后再次读取：" << std::endl;

        for (int i = 0; i < 3 && reader.hasNextLine(); ++i) {
            std::string line = reader.output();
            std::cout << "  行 " << (i + 1) << ": [" << line << "]" << std::endl;
        }

        std::cout << "✓ 重置功能正常" << std::endl;

    } catch (const LineReaderException& e) {
        std::cerr << "❌ 重置测试失败: " << e.what() << std::endl;
    }
}

void testPerformance() {
    printSeparator("性能测试");

    try {
        auto start = std::chrono::high_resolution_clock::now();

        LineReader reader("../example/test_large.txt");
        int lineCount = 0;
        size_t totalChars = 0;

        while (reader.hasNextLine()) {
            std::string line = reader.output();
            lineCount++;
            totalChars += line.length();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "性能统计：" << std::endl;
        std::cout << "  读取行数: " << lineCount << std::endl;
        std::cout << "  总字符数: " << totalChars << std::endl;
        std::cout << "  耗时: " << duration.count() << " 微秒" << std::endl;
        std::cout << "  平均每行: " << (lineCount > 0 ? duration.count() / lineCount : 0) << " 微秒" << std::endl;

    } catch (const LineReaderException& e) {
        std::cerr << "❌ 性能测试失败: " << e.what() << std::endl;
    }
}

int main() {
    printSeparator("基于fread的LineReader类演示");

    std::cout << "这个演示展示了LineReader类的各种功能：" << std::endl;
    std::cout << "- 使用C标准库fread进行文件读取" << std::endl;
    std::cout << "- 高效的缓冲区管理" << std::endl;
    std::cout << "- 支持不同换行符格式" << std::endl;
    std::cout << "- 完善的错误处理" << std::endl;
    std::cout << "- C++11移动语义支持" << std::endl;

    // 基本功能测试
    testBasicReading("../example/test_data.txt");

    // Windows换行符测试
    testWindowsLineEndings();

    // 缓冲区大小测试
    testBufferSize();

    // 错误处理测试
    testErrorHandling();

    // 移动语义测试
    testMoveSemantics();

    // 重置功能测试
    testReset();

    // 性能测试
    testPerformance();

    printSeparator("演示完成");
    std::cout << "✅ 所有测试完成！LineReader类功能正常。" << std::endl;

    return 0;
}