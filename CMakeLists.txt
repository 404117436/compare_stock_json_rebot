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
target_link_libraries(json_example JsonParser StockDataComparator StockDataBatchReader)


# 为不同构建类型设置编译选项
target_compile_options(json_example PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)


# 如果是Debug模式，添加调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(json_example PRIVATE DEBUG)
endif()

# 安装规则（可选）
install(TARGETS JsonParser LineReader GenericJsonContainer StockDataContainer StockDataBatchReader StockDataComparator json_example
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

# 创建一个简单的测试程序（可选）
enable_testing()

# 添加基本测试
add_test(NAME json_parser_test COMMAND json_example)

# 打印配置信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")

# 如果想要创建包的话，可以添加CPack支持
include(CPack)