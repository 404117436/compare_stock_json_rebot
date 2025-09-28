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

# 创建MarketData静态库
add_library(MarketData STATIC
    src/MarketData.cpp
    include/MarketData.h
)

# 设置MarketData库的包含目录
target_include_directories(MarketData PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 设置MarketData库的C++标准
target_compile_features(MarketData PUBLIC cxx_std_11)

# MarketData库依赖JsonParser库
target_link_libraries(MarketData JsonParser)

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

# 创建JsonParser示例可执行文件
add_executable(json_example example/main.cpp)

# 链接JsonParser库到示例
target_link_libraries(json_example JsonParser)

# 创建LineReader示例可执行文件
add_executable(line_reader_demo example/demo.cpp)

# 链接LineReader库到示例
target_link_libraries(line_reader_demo LineReader)

# 创建MarketData示例可执行文件
add_executable(market_demo example/market_demo.cpp)

# 链接MarketData库到示例
target_link_libraries(market_demo MarketData)

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

# 为不同构建类型设置编译选项
target_compile_options(json_example PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(line_reader_demo PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(market_demo PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

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

# 如果是Debug模式，添加调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(json_example PRIVATE DEBUG)
    target_compile_definitions(line_reader_demo PRIVATE DEBUG)
    target_compile_definitions(market_demo PRIVATE DEBUG)
    target_compile_definitions(generic_demo PRIVATE DEBUG)
    target_compile_definitions(line_json_demo PRIVATE DEBUG)
    target_compile_definitions(ignore_fields_demo PRIVATE DEBUG)
endif()

# 安装规则（可选）
install(TARGETS JsonParser LineReader MarketData GenericJsonContainer json_example line_reader_demo market_demo generic_demo line_json_demo ignore_fields_demo
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
add_test(NAME market_data_test COMMAND market_demo)
add_test(NAME generic_container_test COMMAND generic_demo)
add_test(NAME line_json_demo_test COMMAND line_json_demo)
add_test(NAME ignore_fields_demo_test COMMAND ignore_fields_demo)

# 打印配置信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")

# 如果想要创建包的话，可以添加CPack支持
include(CPack)