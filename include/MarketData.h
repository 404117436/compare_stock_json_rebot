#pragma once

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <iomanip>
#include "JsonParser.h"

// 市场行情数据结构体
struct MarketData {
    // 常量定义
    static constexpr size_t ORDER_BOOK_LEVELS = 10;  // 买卖盘档位数量

    // 基本信息
    std::string local_time;      // 本地时间："20250818-11:21:25.409"
    int64_t recv_time;           // 接收时间
    std::string market;          // 市场代码："USHD"
    std::string code;            // 证券代码："110084"
    std::string codename;        // 证券名称："贵燃转债"
    std::string status;          // 状态："C"
    int32_t date;                // 日期：20250818
    int64_t time;                // 时间：91503020

    // 价格信息
    double pre_price;            // 前收价
    double open_price;           // 开盘价
    double high_price;           // 最高价
    double low_price;            // 最低价
    double new_price;            // 最新价
    double uplimit_price;        // 涨停价
    double downlimit_price;      // 跌停价
    double avgbid_price;         // 平均买价
    double avgask_price;         // 平均卖价

    // 成交信息
    int64_t volume;              // 成交量
    double turnover;             // 成交额
    int32_t trade_num;           // 成交笔数

    // 买卖盘统计
    int64_t totalbid_volume;     // 总买量
    int64_t totalask_volume;     // 总卖量
    int32_t totalbid_num;        // 总买笔数
    int32_t totalask_num;        // 总卖笔数
    int32_t bidorder_num;        // 买委托笔数
    int32_t askorder_num;        // 卖委托笔数

    // 10档买卖委托数组
    std::array<double, ORDER_BOOK_LEVELS> bidorder_price;   // 买委托价格
    std::array<int64_t, ORDER_BOOK_LEVELS> bidorder_volume; // 买委托量
    std::array<int32_t, ORDER_BOOK_LEVELS> bid_numorders;   // 买委托笔数
    std::array<double, ORDER_BOOK_LEVELS> askorder_price;   // 卖委托价格
    std::array<int64_t, ORDER_BOOK_LEVELS> askorder_volume; // 卖委托量
    std::array<int32_t, ORDER_BOOK_LEVELS> ask_numorders;   // 卖委托笔数

    // 撤单信息
    int32_t withdraw_buynum;     // 撤买笔数
    int64_t withdraw_buyvolume;  // 撤买量
    double withdraw_buyturnover; // 撤买金额
    int32_t withdraw_sellnum;    // 撤卖笔数
    int64_t withdraw_sellvol;    // 撤卖量
    double withdraw_sellturnover;// 撤卖金额

    // 内外盘信息
    int64_t inner_volume;        // 内盘量
    int64_t outer_volume;        // 外盘量

    // 盘后交易
    int64_t aftermarket_volume;  // 盘后成交量
    int64_t aftermarket_volmount;// 盘后成交额

    // 撮合信息
    int64_t matching_volume;     // 撮合成交量
    double matching_turnover;    // 撮合成交额
    double matching_price;       // 撮合成交价
    int32_t deal_mode;           // 成交模式
    std::string mode_state;      // 模式状态

    // 财务指标
    double pe;                   // 市盈率
    double iopv;                 // IOPV净值

    // ETF申购赎回
    int32_t etf_buy_num;         // ETF申购笔数
    int64_t etf_buy_amount;      // ETF申购金额
    int32_t etf_sell_num;        // ETF赎回笔数
    int64_t etf_sell_amount;     // ETF赎回金额

    // 构造函数
    MarketData();

    // 从JsonParser解析数据
    bool parseFromJson(const JsonParser& parser);

    // 从JSON字符串解析
    bool parseFromJsonString(const std::string& jsonStr);

    // 验证数据完整性
    bool isValid() const;

    // 输出方法
    std::string toString() const;
    std::string toCompactString() const;
    void print() const;
    void printOrderBook() const;  // 打印买卖盘信息

    // 辅助方法
    void clear();                 // 清空所有数据
    double getBidPrice(size_t level) const;  // 获取指定档位买价
    double getAskPrice(size_t level) const;  // 获取指定档位卖价
    int64_t getBidVolume(size_t level) const; // 获取指定档位买量
    int64_t getAskVolume(size_t level) const; // 获取指定档位卖量

    // 计算字段
    double getSpread() const;     // 获取买卖价差
    double getMidPrice() const;   // 获取中间价
    double getTurnoverRate() const; // 计算换手率（需要流通股本）

private:
    // 私有辅助方法
    template<typename T, size_t N>
    bool parseArray(const JsonValue& jsonArray, std::array<T, N>& arr);

    bool parseDoubleArray(const JsonValue& jsonArray, std::array<double, ORDER_BOOK_LEVELS>& arr);
    bool parseIntArray(const JsonValue& jsonArray, std::array<int64_t, ORDER_BOOK_LEVELS>& arr);
    bool parseInt32Array(const JsonValue& jsonArray, std::array<int32_t, ORDER_BOOK_LEVELS>& arr);
};

// MarketDataContainer - 基于std::vector的容器类
class MarketDataContainer {
private:
    std::vector<MarketData> data_;
    std::string dataSource_;     // 数据源标识

public:
    // 构造函数
    MarketDataContainer() = default;
    explicit MarketDataContainer(const std::string& source) : dataSource_(source) {}

    // 容器操作
    void addMarketData(const MarketData& marketData);
    void addMarketData(MarketData&& marketData);

    // 从JSON字符串批量添加
    bool addFromJsonString(const std::string& jsonStr);

    // 从JSON数组批量添加
    bool addFromJsonArray(const std::string& jsonArrayStr);

    // 访问操作
    size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }
    void clear() { data_.clear(); }
    void reserve(size_t capacity) { data_.reserve(capacity); }

    // 迭代器支持
    std::vector<MarketData>::iterator begin() { return data_.begin(); }
    std::vector<MarketData>::iterator end() { return data_.end(); }
    std::vector<MarketData>::const_iterator begin() const { return data_.begin(); }
    std::vector<MarketData>::const_iterator end() const { return data_.end(); }
    std::vector<MarketData>::const_iterator cbegin() const { return data_.cbegin(); }
    std::vector<MarketData>::const_iterator cend() const { return data_.cend(); }

    // 索引访问
    MarketData& operator[](size_t index) { return data_[index]; }
    const MarketData& operator[](size_t index) const { return data_[index]; }

    MarketData& at(size_t index) { return data_.at(index); }
    const MarketData& at(size_t index) const { return data_.at(index); }

    // 查找操作
    std::vector<MarketData>::iterator findByCode(const std::string& code);
    std::vector<MarketData>::const_iterator findByCode(const std::string& code) const;

    std::vector<MarketData*> findByMarket(const std::string& market);
    std::vector<const MarketData*> findByMarket(const std::string& market) const;

    // 统计操作
    size_t countByMarket(const std::string& market) const;
    double getTotalTurnover() const;
    int64_t getTotalVolume() const;

    // 排序操作
    void sortByCode();
    void sortByTurnover(bool descending = true);
    void sortByVolume(bool descending = true);
    void sortByTime();

    // 输出操作
    void print() const;
    void printSummary() const;
    std::string toString() const;

    // 数据源管理
    const std::string& getDataSource() const { return dataSource_; }
    void setDataSource(const std::string& source) { dataSource_ = source; }

    // 导出功能
    bool exportToCsv(const std::string& filename) const;
    bool exportToJson(const std::string& filename) const;
};