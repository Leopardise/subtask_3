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

int main(int argc, char* argv[]) {
    if (argc != 8) {
        std::cerr << "Usage: " << argv[0] << " strategy symbol n x p start_date end_date" << std::endl;
        return 1;
    }

    std::string strategy = argv[1];
    std::string symbol = argv[2];
    int n = std::stoi(argv[3]);
    int x = std::stoi(argv[4]);
    float p = std::stof(argv[5]);
    std::string start_date = argv[6];
    std::string end_date = argv[7];

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
        if(data.date>= start_date && data.date <= end_date){
            trading_days.push_back(data.date);
        }
    }

    // Find the last trading day
    std::string last_trading_day = end_date;
    if (std::find(trading_days.begin(), trading_days.end(), last_trading_day) == trading_days.end()) {
        last_trading_day = trading_days.back();
    }

    // Implement DMA strategy
    std::vector<int> signals;
    for (size_t i = 0; i < stock_data.size(); ++i) {
        bool increasing = true;
        bool decreasing = true;
        int sum = 0;
        if (stock_data[i].date >= start_date && stock_data[i].date <= last_trading_day) {
            

            
            for (int j = 0; j < n; ++j) {
                sum += stock_data[i+j+1].price;
            }
            sum /= n; // Mean found and stored in sum

            double diff, net = 0;
            for (int j = 0; j < n; ++j) {
                diff = stock_data[i+j+1].price - sum;
                net += diff * diff;
            }
            net /= n;

            double std_dev = std::sqrt(net) * p;
            double std_dev_neg = -1 * std_dev * p;

            for (int j = 0; j < n; ++j) {
                diff = stock_data[i+j].price - sum;
                cout<<diff<<"    ";
                cout<<std_dev<<endl;
                increasing = increasing && (diff >= std_dev);
                decreasing = decreasing && (diff <= std_dev_neg);
            }

            if (increasing) {
                signals.push_back(1);  // Buy
            } else if (decreasing) {
                signals.push_back(-1);  // Sell
            } else {
                signals.push_back(0);  // Hold
            }
        }
    }


    std::tm start_date_tm = {};
    std::istringstream start_date_stream(start_date);
    start_date_stream >> std::get_time(&start_date_tm, "%Y-%m-%d");

    // Add one day
    start_date_tm.tm_mday -= 1;
    std::mktime(&start_date_tm);

    // Convert the updated date back to a string
    std::ostringstream start_date_updated_stream;
    start_date_updated_stream << std::put_time(&start_date_tm, "%Y-%m-%d");
    start_date = start_date_updated_stream.str();

    

    // Filter data for start_date to end_date
    std::vector<StockData> result_data;
    for (const auto& data : stock_data) {
        if (data.date >= start_date && data.date <= last_trading_day) {
            result_data.push_back(data);
        }
    }
    for(auto& a: signals){cout<<a<<" ";}
    // Calculate daily cashflow
    std::ofstream cashflow_file("daily_cashflow.csv");
    cashflow_file << "Date,Cashflow\n";
    double cashflow = 0.0;
    int a = signals.size();
    int i = a-1;
    while(i >= 0) {
        cashflow += signals[i] * (result_data[i].price);
        cashflow_file << result_data[i].date << "," << -1 * cashflow << "\n";
        i--;
    }
    cashflow_file.close();

    // Write to order statistics.csv
    std::ofstream order_file("order_statistics.csv");
    order_file << "Date,Direction,Quantity,Price\n";
    //cout<<signals.size()<<endl;
    for (size_t i = 0; i < signals.size(); ++i) {
        if (signals[i] != 0) {
            order_file << result_data[i].date << "," << (signals[i] == 1 ? "BUY" : "SELL") << ",1," << result_data[i+1].price << "\n";
        }
    }
    order_file.close();

    return 0;
}