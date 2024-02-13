#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <cmath>

using namespace std;

struct StockData {
    std::string date;
    std::string direction;  // Buy, Sell, Hold
    int quantity;
    double price;
};

double calculateAverageGain(const std::vector<double>& prices, int n) {
    double sumGain = 0.0;
    for (int i = 1; i <= n; ++i) {
        double gain = std::max(prices[i] - prices[i - 1], 0.0);
        sumGain += gain;
    }
    return sumGain / n;
}

double calculateAverageLoss(const std::vector<double>& prices, int n) {
    double sumLoss = 0.0;
    for (int i = 1; i <= n; ++i) {
        double loss = std::max(prices[i - 1] - prices[i], 0.0);
        sumLoss += loss;
    }
    return sumLoss / n;
}

double calculateRS(double avgGain, double avgLoss) {
    if (avgLoss == 0.0) return 1.0; // To avoid division by zero
    return avgGain / avgLoss;
}

double calculateRSI(double RS) {
    return 100.0 - (100.0 / (1.0 + RS));
}

int main(int argc, char* argv[]) {
    // if (argc != 8) {
    //     std::cerr << "Usage: " << argv[0] << " strategy symbol n x oversold_threshold overbought_threshold additional_threshold start_date end_date" << std::endl;
    //     return 1;
    // }

    std::string strategy = argv[1];
    std::string symbol = argv[2];
    int n = std::stoi(argv[3]); // Number of days for RSI calculation
    int x = std::stoi(argv[4]); // Placeholder for configurable parameter x
    int oversold_threshold = std::stoi(argv[5]); // Threshold for oversold condition
    int overbought_threshold = std::stoi(argv[6]); // Threshold for overbought condition
    //int additional_threshold = std::stoi(argv[7]); // Additional threshold
    std::string start_date = argv[7];
    std::string end_date = argv[8];

    std::ifstream input_file("archive/" + symbol + ".txt");
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open input file" << std::endl;
        return 1;
    }

    std::string line;
    std::vector<StockData> stock_data;
    StockData data;
    bool date_set = false;

    while (std::getline(input_file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        iss >> key >> value;

        if (key == "DATE:") {
            // Set the date for the current StockData object
            data.date = value;
            date_set = true;
        }
        else if (key == "CLOSE:") {
            // Set the price for the current StockData object
            data.price = std::stod(value);

            // If both date and price are set, push the StockData object into the vector
            if (date_set) {
                stock_data.push_back(data);
                data = StockData(); // Create a new StockData object for the next pair of DATE and CLOSE lines
                date_set = false; // Reset the flag
            }
        }
    }

    // Handle non-trading days
    std::vector<std::string> trading_days;
    for (const auto& data : stock_data) {
        trading_days.push_back(data.date);
    }

    // Find the last trading day
    std::string last_trading_day = end_date;
    if (std::find(trading_days.begin(), trading_days.end(), last_trading_day) == trading_days.end()) {
        last_trading_day = trading_days.back();
    }

    // Implement RSI strategy
    std::vector<int> signals;
    for (size_t i = n; i < stock_data.size(); ++i) {
        if (stock_data[i].date >= start_date && stock_data[i].date <= last_trading_day) {
            // Calculate average gain and average loss over the last n days
            std::vector<double> prices;
            for (int j = i; j > i - n; --j) {
                prices.push_back(stock_data[j].price);
            }
            double avgGain = calculateAverageGain(prices, prices.size()-1);
            double avgLoss = calculateAverageLoss(prices, prices.size()-1);

            // Calculate RS (Relative Strength)
            double RS = calculateRS(avgGain, avgLoss);

            // Calculate RSI (Relative Strength Index)
            double RSI = calculateRSI(RS);

            // Generate buy/sell signals based on RSI thresholds
            if (RSI <= oversold_threshold) {
                signals.push_back(1); // Buy
            } else if (RSI >= overbought_threshold) {
                signals.push_back(-1); // Sell
            } else {
                signals.push_back(0); // Hold
            }
        }
    }

    // Filter data for start_date to end_date
    std::vector<StockData> result_data;
    for (const auto& data : stock_data) {
        if (data.date >= start_date && data.date <= last_trading_day) {
            result_data.push_back(data);
        }
    }
    cout<<result_data.size()<<endl;
    // Calculate daily cashflow
    std::ofstream cashflow_file("daily_cashflow.csv");
    cashflow_file << "Date,Cashflow\n";
    double cashflow = 0.0;
    int a = signals.size();
    int i = a - 1;
    while (i >= 0) {
        cashflow += signals[i] * (result_data[i].price);
        cashflow_file << result_data[i].date << "," << -1 * cashflow << "\n";
        i--;
    }
    cashflow_file.close();

    // Write to order statistics.csv
    std::ofstream order_file("order_statistics.csv");
    order_file << "Date,Direction,Quantity,Price\n";

    for (size_t i = 0; i < signals.size(); ++i) {
        if (signals[i] != 0) {
            order_file << result_data[i].date << "," << (signals[i] == 1 ? "BUY" : "SELL") << ",1," << result_data[i+1].price << "\n";
        }
    }
    order_file.close();

    return 0;
}
