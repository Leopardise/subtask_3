#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cmath>

struct StockData {
    std::string date;
    double high;
    double low;
    double close;
};

struct ADXData {
    double true_range;
    double dm_plus;
    double dm_minus;
    double atr;
    double di_plus;
    double di_minus;
    double dx;
    double adx;
};

int main(int argc, char* argv[]) {

  std::string strategy = argv[1];
  std::string symbol = argv[2];
  int n = std::stoi(argv[3]);
  int x = std::stoi(argv[4]);
  double adx_threshold = std::stod(argv[5]);
  std::string start_date = argv[6];
  std::string end_date = argv[7];


    // Parse the start date
 std::istringstream start_stream(start_date);
 int start_day, start_month, start_year;
 char discard;
 start_stream >> start_day >> discard >> start_month >> discard >> start_year;

 // Parse the end date
 std::istringstream end_stream(end_date);
 int end_day, end_month, end_year;
 end_stream >> end_day >> discard >> end_month >> discard >> end_year;

 // Convert to yyyy-mm-dd format
 std::ostringstream start_formatted;
 start_formatted << std::setw(4) << std::setfill('0') << start_year << "-"
                << std::setw(2) << std::setfill('0') << start_month << "-"
                << std::setw(2) << std::setfill('0') << start_day;
 start_date = start_formatted.str();

 std::ostringstream end_formatted;
 end_formatted << std::setw(4) << std::setfill('0') << end_year << "-"
              << std::setw(2) << std::setfill('0') << end_month << "-"
              << std::setw(2) << std::setfill('0') << end_day;
 end_date = end_formatted.str();

  

    std::ifstream input_file("archive/" + symbol + ".txt");
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open input file" << std::endl;
        return 1;
    }

    std::string line;
    std::vector<StockData> stock_data;
    StockData data;

    while (std::getline(input_file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        iss >> key >> value;

        if (key == "DATE:") {
            data.date = value;
        } else if (key == "HIGH:") {
            data.high = std::stod(value);
        } else if (key == "LOW:") {
            data.low = std::stod(value);
        } else if (key == "CLOSE:") {
            data.close = std::stod(value);
            stock_data.push_back(data);
        }
    }

    std::vector<ADXData> adx_data;
    ADXData adx_item;

    for (size_t i = 1; i < stock_data.size(); ++i) {
        adx_item.true_range = std::max(stock_data[i].high - stock_data[i].low, std::max(stock_data[i].high - stock_data[i - 1].close, stock_data[i - 1].close - stock_data[i].low));

        adx_item.dm_plus = std::max(0.0, stock_data[i].high - stock_data[i - 1].high);
        adx_item.dm_minus = std::max(0.0, stock_data[i - 1].low - stock_data[i].low);

        if (i < n) {
            adx_item.atr += adx_item.true_range / n;
            adx_item.di_plus += adx_item.dm_plus / n;
            adx_item.di_minus += adx_item.dm_minus / n;
        } else {
            adx_item.atr = (adx_item.atr * (n - 1) + adx_item.true_range) / n;
            adx_item.di_plus = (adx_item.di_plus * (n - 1) + adx_item.dm_plus) / n;
            adx_item.di_minus = (adx_item.di_minus * (n - 1) + adx_item.dm_minus) / n;
        }

        adx_item.dx = 100.0 * std::abs(adx_item.di_plus - adx_item.di_minus) / (adx_item.di_plus + adx_item.di_minus);

        if (i < n * 2 - 1) {
            adx_item.adx += adx_item.dx / n;
        } else {
            adx_item.adx = (adx_item.adx * (n - 1) + adx_item.dx) / n;
        }

        adx_data.push_back(adx_item);
    }

    // Generate buy or sell signals based on ADX threshold
    std::vector<int> signals;
    for (size_t i = 0; i < adx_data.size(); ++i) {
        if (stock_data[i].date >= start_date && stock_data[i].date <= end_date) {
            if (adx_data[i].adx > adx_threshold) {
                signals.push_back(1);  // Buy
            } else if (adx_data[i].adx < adx_threshold){
                signals.push_back(-1);  // Sell
            }else{
                signals.push_back(0);
            }
        }
    }

    // Write to daily cashflow.csv
    std::ofstream cashflow_file("daily_cashflow.csv");
    cashflow_file << "Date,Cashflow\n";
    double cashflow = 0.0;
    int a = signals.size();
    int i = a - 1;
    while (i >= 0) {
        cashflow += signals[i] * (stock_data[i + 1].close);
        cashflow_file << stock_data[i].date << "," << -1 * cashflow << "\n";
        i--;
    }
    cashflow_file.close();

    // Write to order statistics.csv
    std::ofstream order_file("order_statistics.csv");
    order_file << "Date,Direction,Quantity,Price\n";

    for (size_t i = 0; i < signals.size(); ++i) {
        order_file << stock_data[i].date << "," << (signals[i+1] == 1 ? "BUY" : "SELL") << ",1," << stock_data[i + 1].close << "\n";
    }
    order_file.close();

    return 0;
}