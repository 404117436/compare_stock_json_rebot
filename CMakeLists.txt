cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(FileReaderDemo VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译选项
if(MSVC)
    add_compile_options(/W4)
    # 对于MSVC，可能需要添加编码选项
    add_compile_options(/utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 设置构建类型（如果没有指定）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 添加头文件目录
set(RAPIDJSON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# 查找所有源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# 创建JsonParser静态库
add_library(JsonParser STATIC
    src/JsonParser.cpp
    include/JsonParser.h
)

# 设置JsonParser库的包含目录
target_include_directories(JsonParser PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 设置JsonParser库的C++标准
target_compile_features(JsonParser PUBLIC cxx_std_11)

# 创建LineReader静态库
add_library(LineReader STATIC
    src/LineReader.cpp
    include/LineReader.h
)

# 设置LineReader库的包含目录
target_include_directories(LineReader PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 设置LineReader库的C++标准
target_compile_features(LineReader PUBLIC cxx_std_11)

# MarketData library removed - not needed for current demos

# 创建GenericJsonContainer静态库
add_library(GenericJsonContainer STATIC
    src/GenericJsonContainer.cpp
    include/GenericJsonContainer.h
)

# 设置GenericJsonContainer库的包含目录
target_include_directories(GenericJsonContainer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 设置GenericJsonContainer库的C++标准
target_compile_features(GenericJsonContainer PUBLIC cxx_std_11)

# GenericJsonContainer库依赖JsonParser库
target_link_libraries(GenericJsonContainer JsonParser)

# 创建StockDataContainer静态库
add_library(StockDataContainer STATIC
    src/StockDataContainer.cpp
    include/StockDataContainer.h
)

# 设置StockDataContainer库的包含目录
target_include_directories(StockDataContainer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 设置StockDataContainer库的C++标准
target_compile_features(StockDataContainer PUBLIC cxx_std_11)

# StockDataContainer库依赖GenericJsonContainer库
target_link_libraries(StockDataContainer GenericJsonContainer)

# 创建StockDataBatchReader静态库
add_library(StockDataBatchReader STATIC
    src/StockDataBatchReader.cpp
    include/StockDataBatchReader.h
)

# 设置StockDataBatchReader库的包含目录
target_include_directories(StockDataBatchReader PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 设置StockDataBatchReader库的C++标准
target_compile_features(StockDataBatchReader PUBLIC cxx_std_11)

# StockDataBatchReader库依赖StockDataContainer和LineReader库
target_link_libraries(StockDataBatchReader StockDataContainer LineReader)

# 创建StockDataComparator静态库
add_library(StockDataComparator STATIC
    src/StockDataComparator.cpp
    include/StockDataComparator.h
)

# 设置StockDataComparator库的包含目录
target_include_directories(StockDataComparator PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 设置StockDataComparator库的C++标准
target_compile_features(StockDataComparator PUBLIC cxx_std_11)

# StockDataComparator库依赖StockDataContainer库
target_link_libraries(StockDataComparator StockDataContainer)

# 创建JsonParser示例可执行文件
add_executable(json_example example/main.cpp)

# 链接JsonParser库到示例
target_link_libraries(json_example JsonParser)

# 创建LineReader示例可执行文件
add_executable(line_reader_demo example/demo.cpp)

# 链接LineReader库到示例
target_link_libraries(line_reader_demo LineReader)

# MarketData demo removed - MarketData library not available

# 创建GenericJsonContainer示例可执行文件
add_executable(generic_demo example/generic_demo.cpp)

# 链接GenericJsonContainer库到示例
target_link_libraries(generic_demo GenericJsonContainer)

# 创建LineReader + GenericJsonContainer + deque示例可执行文件
add_executable(line_json_demo example/line_json_demo.cpp)

# 链接需要的库到示例
target_link_libraries(line_json_demo LineReader GenericJsonContainer)

# 创建忽略字段功能演示可执行文件
add_executable(ignore_fields_demo example/ignore_fields_demo.cpp)

# 链接GenericJsonContainer库到忽略字段演示
target_link_libraries(ignore_fields_demo GenericJsonContainer)

# 创建股票数据容器演示可执行文件
add_executable(stock_data_demo example/stock_data_demo.cpp)

# 链接StockDataContainer库到股票数据演示
target_link_libraries(stock_data_demo StockDataContainer)

# 创建批次读取器演示可执行文件
add_executable(batch_reader_demo example/batch_reader_demo.cpp)

# 链接StockDataBatchReader库到批次读取器演示
target_link_libraries(batch_reader_demo StockDataBatchReader)

# 创建数值索引功能演示可执行文件
add_executable(numeric_index_demo example/numeric_index_demo.cpp)

# 链接StockDataBatchReader库到数值索引演示
target_link_libraries(numeric_index_demo StockDataBatchReader)

# 创建比较一致性验证可执行文件
add_executable(comparison_consistency_test example/comparison_consistency_test.cpp)

# 链接StockDataBatchReader库到比较一致性验证
target_link_libraries(comparison_consistency_test StockDataBatchReader)

# 创建股票数据比较器演示可执行文件
add_executable(stock_comparator_demo example/stock_comparator_demo.cpp)

# 链接StockDataComparator库到股票数据比较器演示
target_link_libraries(stock_comparator_demo StockDataComparator)

# 创建字段边界条件测试可执行文件
add_executable(field_boundary_test example/field_boundary_test.cpp)

# 链接StockDataComparator库到字段边界条件测试
target_link_libraries(field_boundary_test StockDataComparator)

# 创建详细差异功能测试可执行文件
add_executable(detailed_diff_test example/detailed_diff_test.cpp)

# 链接StockDataComparator库到详细差异功能测试
target_link_libraries(detailed_diff_test StockDataComparator)

# 创建简单测试可执行文件
add_executable(simple_test example/simple_test.cpp)

# 链接StockDataComparator库到简单测试
target_link_libraries(simple_test StockDataComparator)

# 创建JSON文件比较演示可执行文件
add_executable(json_file_comparison_demo example/json_file_comparison_demo.cpp)

# 链接所需库到JSON文件比较演示
target_link_libraries(json_file_comparison_demo StockDataComparator StockDataBatchReader)

# 为不同构建类型设置编译选项
target_compile_options(json_example PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(line_reader_demo PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# market_demo compile options removed

target_compile_options(generic_demo PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(line_json_demo PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(ignore_fields_demo PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(stock_data_demo PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(batch_reader_demo PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(numeric_index_demo PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(comparison_consistency_test PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(stock_comparator_demo PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(field_boundary_test PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(detailed_diff_test PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(simple_test PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(json_file_comparison_demo PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# 如果是Debug模式，添加调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(json_example PRIVATE DEBUG)
    target_compile_definitions(line_reader_demo PRIVATE DEBUG)
    # market_demo debug definition removed
    target_compile_definitions(generic_demo PRIVATE DEBUG)
    target_compile_definitions(line_json_demo PRIVATE DEBUG)
    target_compile_definitions(ignore_fields_demo PRIVATE DEBUG)
    target_compile_definitions(stock_data_demo PRIVATE DEBUG)
    target_compile_definitions(batch_reader_demo PRIVATE DEBUG)
    target_compile_definitions(numeric_index_demo PRIVATE DEBUG)
    target_compile_definitions(comparison_consistency_test PRIVATE DEBUG)
    target_compile_definitions(stock_comparator_demo PRIVATE DEBUG)
    target_compile_definitions(field_boundary_test PRIVATE DEBUG)
    target_compile_definitions(detailed_diff_test PRIVATE DEBUG)
    target_compile_definitions(simple_test PRIVATE DEBUG)
    target_compile_definitions(json_file_comparison_demo PRIVATE DEBUG)
endif()

# 安装规则（可选）
install(TARGETS JsonParser LineReader GenericJsonContainer StockDataContainer StockDataBatchReader StockDataComparator json_example line_reader_demo generic_demo line_json_demo ignore_fields_demo stock_data_demo batch_reader_demo numeric_index_demo comparison_consistency_test stock_comparator_demo field_boundary_test detailed_diff_test simple_test json_file_comparison_demo
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

# 创建一个简单的测试程序（可选）
enable_testing()

# 添加基本测试
add_test(NAME json_parser_test COMMAND json_example)
add_test(NAME line_reader_test COMMAND line_reader_demo)
# market_data_test removed
add_test(NAME generic_container_test COMMAND generic_demo)
add_test(NAME line_json_demo_test COMMAND line_json_demo)
add_test(NAME ignore_fields_demo_test COMMAND ignore_fields_demo)
add_test(NAME stock_data_demo_test COMMAND stock_data_demo)
add_test(NAME batch_reader_demo_test COMMAND batch_reader_demo)
add_test(NAME numeric_index_demo_test COMMAND numeric_index_demo)
add_test(NAME comparison_consistency_test COMMAND comparison_consistency_test)
add_test(NAME stock_comparator_demo_test COMMAND stock_comparator_demo)
add_test(NAME field_boundary_test COMMAND field_boundary_test)
add_test(NAME detailed_diff_test COMMAND detailed_diff_test)
add_test(NAME simple_test COMMAND simple_test)
add_test(NAME json_file_comparison_demo_test COMMAND json_file_comparison_demo)

# 打印配置信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")

# 如果想要创建包的话，可以添加CPack支持
include(CPack)