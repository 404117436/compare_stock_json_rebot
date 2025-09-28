#include "MarketData.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <cmath>

// MarketData 构造函数
MarketData::MarketData() {
    clear();
}

// 清空所有数据
void MarketData::clear() {
    // 基本信息
    local_time.clear();
    recv_time = 0;
    market.clear();
    code.clear();
    codename.clear();
    status.clear();
    date = 0;
    time = 0;

    // 价格信息
    pre_price = 0.0;
    open_price = 0.0;
    high_price = 0.0;
    low_price = 0.0;
    new_price = 0.0;
    uplimit_price = 0.0;
    downlimit_price = 0.0;
    avgbid_price = 0.0;
    avgask_price = 0.0;

    // 成交信息
    volume = 0;
    turnover = 0.0;
    trade_num = 0;

    // 买卖盘统计
    totalbid_volume = 0;
    totalask_volume = 0;
    totalbid_num = 0;
    totalask_num = 0;
    bidorder_num = 0;
    askorder_num = 0;

    // 数组初始化
    bidorder_price.fill(0.0);
    bidorder_volume.fill(0);
    bid_numorders.fill(0);
    askorder_price.fill(0.0);
    askorder_volume.fill(0);
    ask_numorders.fill(0);

    // 撤单信息
    withdraw_buynum = 0;
    withdraw_buyvolume = 0;
    withdraw_buyturnover = 0.0;
    withdraw_sellnum = 0;
    withdraw_sellvol = 0;
    withdraw_sellturnover = 0.0;

    // 内外盘信息
    inner_volume = 0;
    outer_volume = 0;

    // 盘后交易
    aftermarket_volume = 0;
    aftermarket_volmount = 0;

    // 撮合信息
    matching_volume = 0;
    matching_turnover = 0.0;
    matching_price = 0.0;
    deal_mode = -1;
    mode_state.clear();

    // 财务指标
    pe = 0.0;
    iopv = 0.0;

    // ETF申购赎回
    etf_buy_num = 0;
    etf_buy_amount = 0;
    etf_sell_num = 0;
    etf_sell_amount = 0;
}

// 从JsonParser解析数据
bool MarketData::parseFromJson(const JsonParser& parser) {
    try {
        // 基本信息
        if (parser.hasKey("local_time")) {
            local_time = parser.getString("local_time");
        }
        if (parser.hasKey("recv_time")) {
            recv_time = parser.getInt("recv_time");
        }
        if (parser.hasKey("market")) {
            market = parser.getString("market");
        }
        if (parser.hasKey("code")) {
            code = parser.getString("code");
        }
        if (parser.hasKey("codename")) {
            codename = parser.getString("codename");
        }
        if (parser.hasKey("status")) {
            status = parser.getString("status");
        }
        if (parser.hasKey("date")) {
            date = static_cast<int32_t>(parser.getInt("date"));
        }
        if (parser.hasKey("time")) {
            time = parser.getInt("time");
        }

        // 价格信息
        if (parser.hasKey("pre_price")) {
            pre_price = parser.getDouble("pre_price");
        }
        if (parser.hasKey("open_price")) {
            open_price = parser.getDouble("open_price");
        }
        if (parser.hasKey("high_price")) {
            high_price = parser.getDouble("high_price");
        }
        if (parser.hasKey("low_price")) {
            low_price = parser.getDouble("low_price");
        }
        if (parser.hasKey("new_price")) {
            new_price = parser.getDouble("new_price");
        }
        if (parser.hasKey("uplimit_price")) {
            uplimit_price = parser.getDouble("uplimit_price");
        }
        if (parser.hasKey("downlimit_price")) {
            downlimit_price = parser.getDouble("downlimit_price");
        }
        if (parser.hasKey("avgbid_price")) {
            avgbid_price = parser.getDouble("avgbid_price");
        }
        if (parser.hasKey("avgask_price")) {
            avgask_price = parser.getDouble("avgask_price");
        }

        // 成交信息
        if (parser.hasKey("volume")) {
            volume = parser.getInt("volume");
        }
        if (parser.hasKey("turnover")) {
            turnover = parser.getDouble("turnover");
        }
        if (parser.hasKey("trade_num")) {
            trade_num = static_cast<int32_t>(parser.getInt("trade_num"));
        }

        // 买卖盘统计
        if (parser.hasKey("totalbid_volume")) {
            totalbid_volume = parser.getInt("totalbid_volume");
        }
        if (parser.hasKey("totalask_volume")) {
            totalask_volume = parser.getInt("totalask_volume");
        }
        if (parser.hasKey("totalbid_num")) {
            totalbid_num = static_cast<int32_t>(parser.getInt("totalbid_num"));
        }
        if (parser.hasKey("totalask_num")) {
            totalask_num = static_cast<int32_t>(parser.getInt("totalask_num"));
        }
        if (parser.hasKey("bidorder_num")) {
            bidorder_num = static_cast<int32_t>(parser.getInt("bidorder_num"));
        }
        if (parser.hasKey("askorder_num")) {
            askorder_num = static_cast<int32_t>(parser.getInt("askorder_num"));
        }

        // 数组字段
        if (parser.hasKey("bidorder_price")) {
            parseDoubleArray(parser.getArray("bidorder_price"), bidorder_price);
        }
        if (parser.hasKey("bidorder_volume")) {
            parseIntArray(parser.getArray("bidorder_volume"), bidorder_volume);
        }
        if (parser.hasKey("bid_numorders")) {
            parseInt32Array(parser.getArray("bid_numorders"), bid_numorders);
        }
        if (parser.hasKey("askorder_price")) {
            parseDoubleArray(parser.getArray("askorder_price"), askorder_price);
        }
        if (parser.hasKey("askorder_volume")) {
            parseIntArray(parser.getArray("askorder_volume"), askorder_volume);
        }
        if (parser.hasKey("ask_numorders")) {
            parseInt32Array(parser.getArray("ask_numorders"), ask_numorders);
        }

        // 撤单信息
        if (parser.hasKey("withdraw_buynum")) {
            withdraw_buynum = static_cast<int32_t>(parser.getInt("withdraw_buynum"));
        }
        if (parser.hasKey("withdraw_buyvolume")) {
            withdraw_buyvolume = parser.getInt("withdraw_buyvolume");
        }
        if (parser.hasKey("withdraw_buyturnover")) {
            withdraw_buyturnover = parser.getDouble("withdraw_buyturnover");
        }
        if (parser.hasKey("withdraw_sellnum")) {
            withdraw_sellnum = static_cast<int32_t>(parser.getInt("withdraw_sellnum"));
        }
        if (parser.hasKey("withdraw_sellvol")) {
            withdraw_sellvol = parser.getInt("withdraw_sellvol");
        }
        if (parser.hasKey("withdraw_sellturnover")) {
            withdraw_sellturnover = parser.getDouble("withdraw_sellturnover");
        }

        // 内外盘信息
        if (parser.hasKey("inner_volume")) {
            inner_volume = parser.getInt("inner_volume");
        }
        if (parser.hasKey("outer_volume")) {
            outer_volume = parser.getInt("outer_volume");
        }

        // 盘后交易
        if (parser.hasKey("aftermarket_volume")) {
            aftermarket_volume = parser.getInt("aftermarket_volume");
        }
        if (parser.hasKey("aftermarket_volmount")) {
            aftermarket_volmount = parser.getInt("aftermarket_volmount");
        }

        // 撮合信息
        if (parser.hasKey("matching_volume")) {
            matching_volume = parser.getInt("matching_volume");
        }
        if (parser.hasKey("matching_turnover")) {
            matching_turnover = parser.getDouble("matching_turnover");
        }
        if (parser.hasKey("matching_price")) {
            matching_price = parser.getDouble("matching_price");
        }
        if (parser.hasKey("deal_mode")) {
            deal_mode = static_cast<int32_t>(parser.getInt("deal_mode"));
        }
        if (parser.hasKey("mode_state")) {
            mode_state = parser.getString("mode_state");
        }

        // 财务指标
        if (parser.hasKey("pe")) {
            pe = parser.getDouble("pe");
        }
        if (parser.hasKey("iopv")) {
            iopv = parser.getDouble("iopv");
        }

        // ETF申购赎回
        if (parser.hasKey("etf_buy_num")) {
            etf_buy_num = static_cast<int32_t>(parser.getInt("etf_buy_num"));
        }
        if (parser.hasKey("etf_buy_amount")) {
            etf_buy_amount = parser.getInt("etf_buy_amount");
        }
        if (parser.hasKey("etf_sell_num")) {
            etf_sell_num = static_cast<int32_t>(parser.getInt("etf_sell_num"));
        }
        if (parser.hasKey("etf_sell_amount")) {
            etf_sell_amount = parser.getInt("etf_sell_amount");
        }

        return true;

    } catch (const JsonParseException& e) {
        std::cerr << "JSON解析错误: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "数据解析错误: " << e.what() << std::endl;
        return false;
    }
}

// 从JSON字符串解析
bool MarketData::parseFromJsonString(const std::string& jsonStr) {
    try {
        JsonParser parser(jsonStr);
        return parseFromJson(parser);
    } catch (const JsonParseException& e) {
        std::cerr << "JSON字符串解析错误: " << e.what() << std::endl;
        return false;
    }
}

// 解析double数组
bool MarketData::parseDoubleArray(const JsonValue& jsonArray, std::array<double, ORDER_BOOK_LEVELS>& arr) {
    if (!jsonArray.isArray()) {
        return false;
    }

    size_t arraySize = jsonArray.arraySize();
    size_t parseCount = std::min(arraySize, ORDER_BOOK_LEVELS);

    for (size_t i = 0; i < parseCount; ++i) {
        auto element = jsonArray[i];
        if (element.isDouble() || element.isInt()) {
            arr[i] = element.asDouble();
        } else {
            arr[i] = 0.0;
        }
    }

    // 剩余位置填0
    for (size_t i = parseCount; i < ORDER_BOOK_LEVELS; ++i) {
        arr[i] = 0.0;
    }

    return true;
}

// 解析int64_t数组
bool MarketData::parseIntArray(const JsonValue& jsonArray, std::array<int64_t, ORDER_BOOK_LEVELS>& arr) {
    if (!jsonArray.isArray()) {
        return false;
    }

    size_t arraySize = jsonArray.arraySize();
    size_t parseCount = std::min(arraySize, ORDER_BOOK_LEVELS);

    for (size_t i = 0; i < parseCount; ++i) {
        auto element = jsonArray[i];
        if (element.isInt()) {
            arr[i] = element.asInt();
        } else if (element.isDouble()) {
            arr[i] = static_cast<int64_t>(element.asDouble());
        } else {
            arr[i] = 0;
        }
    }

    // 剩余位置填0
    for (size_t i = parseCount; i < ORDER_BOOK_LEVELS; ++i) {
        arr[i] = 0;
    }

    return true;
}

// 解析int32_t数组
bool MarketData::parseInt32Array(const JsonValue& jsonArray, std::array<int32_t, ORDER_BOOK_LEVELS>& arr) {
    if (!jsonArray.isArray()) {
        return false;
    }

    size_t arraySize = jsonArray.arraySize();
    size_t parseCount = std::min(arraySize, ORDER_BOOK_LEVELS);

    for (size_t i = 0; i < parseCount; ++i) {
        auto element = jsonArray[i];
        if (element.isInt()) {
            arr[i] = static_cast<int32_t>(element.asInt());
        } else if (element.isDouble()) {
            arr[i] = static_cast<int32_t>(element.asDouble());
        } else {
            arr[i] = 0;
        }
    }

    // 剩余位置填0
    for (size_t i = parseCount; i < ORDER_BOOK_LEVELS; ++i) {
        arr[i] = 0;
    }

    return true;
}

// 验证数据完整性
bool MarketData::isValid() const {
    // 基本字段检查
    if (code.empty() || market.empty()) {
        return false;
    }

    // 价格合理性检查
    if (new_price < 0 || pre_price < 0) {
        return false;
    }

    // 涨跌停价检查
    if (uplimit_price > 0 && downlimit_price > 0 && uplimit_price <= downlimit_price) {
        return false;
    }

    return true;
}

// 辅助方法实现
double MarketData::getBidPrice(size_t level) const {
    return (level < ORDER_BOOK_LEVELS) ? bidorder_price[level] : 0.0;
}

double MarketData::getAskPrice(size_t level) const {
    return (level < ORDER_BOOK_LEVELS) ? askorder_price[level] : 0.0;
}

int64_t MarketData::getBidVolume(size_t level) const {
    return (level < ORDER_BOOK_LEVELS) ? bidorder_volume[level] : 0;
}

int64_t MarketData::getAskVolume(size_t level) const {
    return (level < ORDER_BOOK_LEVELS) ? askorder_volume[level] : 0;
}

double MarketData::getSpread() const {
    double bid1 = getBidPrice(0);
    double ask1 = getAskPrice(0);
    return (bid1 > 0 && ask1 > 0) ? (ask1 - bid1) : 0.0;
}

double MarketData::getMidPrice() const {
    double bid1 = getBidPrice(0);
    double ask1 = getAskPrice(0);
    return (bid1 > 0 && ask1 > 0) ? (bid1 + ask1) / 2.0 : 0.0;
}

double MarketData::getTurnoverRate() const {
    // 简单的换手率计算，实际应用中需要流通股本数据
    return (volume > 0 && pre_price > 0) ? static_cast<double>(volume) / 100000000.0 : 0.0;
}

// 输出方法
std::string MarketData::toString() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4);

    ss << "=== 市场行情数据 ===" << std::endl;
    ss << "代码: " << code << " (" << codename << ")" << std::endl;
    ss << "市场: " << market << ", 状态: " << status << std::endl;
    ss << "时间: " << local_time << std::endl;
    ss << "日期: " << date << ", 时间戳: " << time << std::endl;

    ss << "\n--- 价格信息 ---" << std::endl;
    ss << "最新价: " << new_price << ", 前收价: " << pre_price << std::endl;
    ss << "开盘: " << open_price << ", 最高: " << high_price << ", 最低: " << low_price << std::endl;
    ss << "涨停: " << uplimit_price << ", 跌停: " << downlimit_price << std::endl;

    ss << "\n--- 成交信息 ---" << std::endl;
    ss << "成交量: " << volume << ", 成交额: " << turnover << std::endl;
    ss << "成交笔数: " << trade_num << std::endl;

    ss << "\n--- 买卖盘信息 ---" << std::endl;
    ss << "总买量: " << totalbid_volume << ", 总卖量: " << totalask_volume << std::endl;
    ss << "买一价: " << getBidPrice(0) << ", 卖一价: " << getAskPrice(0) << std::endl;
    ss << "价差: " << getSpread() << ", 中间价: " << getMidPrice() << std::endl;

    return ss.str();
}

std::string MarketData::toCompactString() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << code << "|" << codename << "|" << new_price << "|"
       << volume << "|" << turnover << "|" << getBidPrice(0) << "|" << getAskPrice(0);
    return ss.str();
}

void MarketData::print() const {
    std::cout << toString() << std::endl;
}

void MarketData::printOrderBook() const {
    std::cout << "\n=== 十档买卖盘 ===" << std::endl;
    std::cout << std::left << std::setw(8) << "档位"
              << std::setw(12) << "买价" << std::setw(12) << "买量"
              << std::setw(12) << "卖价" << std::setw(12) << "卖量" << std::endl;
    std::cout << std::string(56, '-') << std::endl;

    for (size_t i = 0; i < ORDER_BOOK_LEVELS; ++i) {
        std::cout << std::left << std::setw(8) << (i + 1)
                  << std::setw(12) << std::fixed << std::setprecision(4) << bidorder_price[i]
                  << std::setw(12) << bidorder_volume[i]
                  << std::setw(12) << std::fixed << std::setprecision(4) << askorder_price[i]
                  << std::setw(12) << askorder_volume[i] << std::endl;
    }
}

// MarketDataContainer 实现
void MarketDataContainer::addMarketData(const MarketData& marketData) {
    data_.push_back(marketData);
}

void MarketDataContainer::addMarketData(MarketData&& marketData) {
    data_.push_back(std::move(marketData));
}

bool MarketDataContainer::addFromJsonString(const std::string& jsonStr) {
    MarketData marketData;
    if (marketData.parseFromJsonString(jsonStr)) {
        addMarketData(std::move(marketData));
        return true;
    }
    return false;
}

bool MarketDataContainer::addFromJsonArray(const std::string& jsonArrayStr) {
    // 简化实现：暂时只支持单个JSON对象，不支持数组
    // 实际项目中应该逐个解析数组中的每个JSON对象
    MarketData marketData;
    if (marketData.parseFromJsonString(jsonArrayStr)) {
        addMarketData(std::move(marketData));
        std::cout << "成功添加 1 条行情数据" << std::endl;
        return true;
    } else {
        std::cerr << "JSON解析失败" << std::endl;
        return false;
    }
}

// 查找操作
std::vector<MarketData>::iterator MarketDataContainer::findByCode(const std::string& code) {
    return std::find_if(data_.begin(), data_.end(),
        [&code](const MarketData& data) { return data.code == code; });
}

std::vector<MarketData>::const_iterator MarketDataContainer::findByCode(const std::string& code) const {
    return std::find_if(data_.begin(), data_.end(),
        [&code](const MarketData& data) { return data.code == code; });
}

std::vector<MarketData*> MarketDataContainer::findByMarket(const std::string& market) {
    std::vector<MarketData*> result;
    for (auto& data : data_) {
        if (data.market == market) {
            result.push_back(&data);
        }
    }
    return result;
}

std::vector<const MarketData*> MarketDataContainer::findByMarket(const std::string& market) const {
    std::vector<const MarketData*> result;
    for (const auto& data : data_) {
        if (data.market == market) {
            result.push_back(&data);
        }
    }
    return result;
}

// 统计操作
size_t MarketDataContainer::countByMarket(const std::string& market) const {
    return std::count_if(data_.begin(), data_.end(),
        [&market](const MarketData& data) { return data.market == market; });
}

double MarketDataContainer::getTotalTurnover() const {
    double total = 0.0;
    for (const auto& data : data_) {
        total += data.turnover;
    }
    return total;
}

int64_t MarketDataContainer::getTotalVolume() const {
    int64_t total = 0;
    for (const auto& data : data_) {
        total += data.volume;
    }
    return total;
}

// 排序操作
void MarketDataContainer::sortByCode() {
    std::sort(data_.begin(), data_.end(),
        [](const MarketData& a, const MarketData& b) { return a.code < b.code; });
}

void MarketDataContainer::sortByTurnover(bool descending) {
    if (descending) {
        std::sort(data_.begin(), data_.end(),
            [](const MarketData& a, const MarketData& b) { return a.turnover > b.turnover; });
    } else {
        std::sort(data_.begin(), data_.end(),
            [](const MarketData& a, const MarketData& b) { return a.turnover < b.turnover; });
    }
}

void MarketDataContainer::sortByVolume(bool descending) {
    if (descending) {
        std::sort(data_.begin(), data_.end(),
            [](const MarketData& a, const MarketData& b) { return a.volume > b.volume; });
    } else {
        std::sort(data_.begin(), data_.end(),
            [](const MarketData& a, const MarketData& b) { return a.volume < b.volume; });
    }
}

void MarketDataContainer::sortByTime() {
    std::sort(data_.begin(), data_.end(),
        [](const MarketData& a, const MarketData& b) {
            if (a.date != b.date) return a.date < b.date;
            return a.time < b.time;
        });
}

// 输出操作
void MarketDataContainer::print() const {
    std::cout << "=== MarketDataContainer (" << dataSource_ << ") ===" << std::endl;
    std::cout << "总记录数: " << data_.size() << std::endl;

    for (const auto& data : data_) {
        std::cout << data.toCompactString() << std::endl;
    }
}

void MarketDataContainer::printSummary() const {
    std::cout << "=== 数据摘要 (" << dataSource_ << ") ===" << std::endl;
    std::cout << "总记录数: " << data_.size() << std::endl;
    std::cout << "总成交额: " << std::fixed << std::setprecision(2) << getTotalTurnover() << std::endl;
    std::cout << "总成交量: " << getTotalVolume() << std::endl;

    if (!data_.empty()) {
        std::cout << "第一条: " << data_.front().code << " (" << data_.front().codename << ")" << std::endl;
        std::cout << "最后条: " << data_.back().code << " (" << data_.back().codename << ")" << std::endl;
    }
}

std::string MarketDataContainer::toString() const {
    std::stringstream ss;
    ss << "MarketDataContainer: " << data_.size() << " records from " << dataSource_ << std::endl;
    for (const auto& data : data_) {
        ss << data.toCompactString() << std::endl;
    }
    return ss.str();
}

// 导出功能
bool MarketDataContainer::exportToCsv(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法创建CSV文件: " << filename << std::endl;
        return false;
    }

    // CSV头部
    file << "code,codename,market,new_price,pre_price,volume,turnover,bid1,ask1" << std::endl;

    // 数据行
    for (const auto& data : data_) {
        file << data.code << "," << data.codename << "," << data.market << ","
             << std::fixed << std::setprecision(4)
             << data.new_price << "," << data.pre_price << ","
             << data.volume << "," << data.turnover << ","
             << data.getBidPrice(0) << "," << data.getAskPrice(0) << std::endl;
    }

    file.close();
    std::cout << "成功导出 " << data_.size() << " 条记录到 " << filename << std::endl;
    return true;
}

bool MarketDataContainer::exportToJson(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法创建JSON文件: " << filename << std::endl;
        return false;
    }

    file << "[" << std::endl;
    for (size_t i = 0; i < data_.size(); ++i) {
        if (i > 0) file << "," << std::endl;
        file << "  {" << std::endl;
        file << "    \"code\": \"" << data_[i].code << "\"," << std::endl;
        file << "    \"codename\": \"" << data_[i].codename << "\"," << std::endl;
        file << "    \"market\": \"" << data_[i].market << "\"," << std::endl;
        file << "    \"new_price\": " << data_[i].new_price << "," << std::endl;
        file << "    \"volume\": " << data_[i].volume << "," << std::endl;
        file << "    \"turnover\": " << data_[i].turnover << std::endl;
        file << "  }";
    }
    file << std::endl << "]" << std::endl;

    file.close();
    std::cout << "成功导出 " << data_.size() << " 条记录到 " << filename << std::endl;
    return true;
}