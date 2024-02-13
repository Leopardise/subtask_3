#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <ctime>
#include <algorithm>
using namespace std;

struct StockData {
    std::string date;
    std::string direction;  // Buy, Sell, Hold
    int quantity;
    double price;
    double high;
    double low;
    double close;
    double price1; // Price of stock 1
    double price2; // Price of stock 2
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

double calculateER(const std::vector<double>& prices, int n) {
    double sumAbsChange = 0.0;
    double priceChange = prices[n] - prices[0];
    for (int i = 1; i < n; ++i) {
        sumAbsChange += std::abs(prices[i] - prices[i - 1]);
    }
    if (sumAbsChange == 0) return 0.0;
    return std::abs(priceChange) / sumAbsChange;
}

double calculateSF(double SF_prev, double ER, double c1, double c2) {
    double numerator = 2 * ER;
    double denominator = 1 + c2;
    double SF = SF_prev + c1 * (numerator / denominator - 1);
    return SF;
}

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

    std::string strategy = argv[1];


 if(strategy == "BASIC"){

   std::string symbol = argv[2];
   int n = std::stoi(argv[3]);
   int x = std::stoi(argv[4]);
   std::string start_date = argv[5];
   std::string end_date = argv[6];

   // Build the command to execute the Python script
    std::string python_command = "python script.py " + symbol + " " + start_date + " " + end_date;

    // Execute the Python script
    int python_script_result = std::system(python_command.c_str());



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

           data = StockData();
           date_set = false; // Reset the flag
       }
   }
}



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



   // Implement basic strategy
   //cout<<"stock_data.size() ::::::"<<stock_data.size()<<endl;
   std::vector<int> signals;
   for (size_t i = 0; i < stock_data.size(); ++i) {
       bool increasing = true;
       bool decreasing = true;
     if (stock_data[i].date >= start_date && stock_data[i].date <= last_trading_day) {

       //cout<<"I am in the if condition"<<endl;
       for (int j = 0; j < n; ++j) {
           //cout<<"I am in the if for condition"<<endl;
           // std:: cout <<(stock_data[i+j].price) - (stock_data[i + j + 1].price) << "\n";
           increasing = increasing && (stock_data[i+j].price >= stock_data[i+j+1].price);
           // std:: cout << increasing;
           // cout<<"increasing is::: " <<increasing << endl;
           decreasing = decreasing && (stock_data[i+j].price <= stock_data[i+j+1].price);
           // cout<<"decreasing is:::"<<decreasing <<endl;
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
   //cout<<"signals size::::"<<signals.size()<<endl;

   // Add one day to the last trading day
// Convert last_trading_day to a date object for manipulation
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


   // Calculate daily cashflow
   std::ofstream cashflow_file("daily_cashflow.csv");
   cashflow_file << "Date,Cashflow\n";
   double cashflow = 0.0;
   int a = signals.size();
   int i = a-1;
   //cout<<"result_data.size():::::::"<<result_data.size()<<endl;
   //std::cout<<" i am here";
   while(i>=0) {
     std::cout<<i<<"::";
       cashflow += signals[i] * (result_data[i].price );//- result_data[i].price);
       cashflow_file << result_data[i].date << "," << -1 * cashflow << "\n";
       i--;
   }
   cashflow_file.close();

   // Write to order statistics.csv
   std::ofstream order_file("order_statistics.csv");
   order_file << "Date,Direction,Quantity,Price\n";

   for (size_t i = 0; i < signals.size(); ++i) {
       // cout<<signals[i]<<":::";
       if (signals[i] != 0) {
           order_file << result_data[i].date << "," << (signals[i] == 1 ? "BUY" : "SELL") << ",1," << result_data[i+1].price << "\n";
       }
   }
   order_file.close();

 }

if(strategy == "DMA"){

  std::string symbol = argv[2];
  int n = std::stoi(argv[3]);
  int x = std::stoi(argv[4]);
  float p = std::stof(argv[5]);
  std::string start_date = argv[6];
  std::string end_date = argv[7];

  // Build the command to execute the Python script
  std::string python_command = "python script.py " + symbol + " " + start_date + " " + end_date;

    // Execute the Python script
  int python_script_result = std::system(python_command.c_str());


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

          data = StockData();
          date_set = false; // Reset the flag
      }
  }
}



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

          double diff, net;
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

  for (size_t i = 0; i < signals.size(); ++i) {
      if (signals[i] != 0) {
          order_file << result_data[i].date << "," << (signals[i] == 1 ? "BUY" : "SELL") << ",1," << result_data[i+1].price << "\n";
      }
  }
  order_file.close();

  }

  if(strategy == "DMA++"){

    std::string symbol = argv[2];
    int n = std::stoi(argv[3]);
    int x = std::stoi(argv[4]);
    float p = std::stof(argv[5]);
    int max_hold_days = std::stoi(argv[6]);
    double c1 = std::stod(argv[7]);
    double c2 = std::stod(argv[8]);
    std::string start_date = argv[9];
    std::string end_date = argv[10];

    // Build the command to execute the Python script
    std::string python_command = "python script.py " + symbol + " " + start_date + " " + end_date;

    // Execute the Python script
    int python_script_result = std::system(python_command.c_str());


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

            data = StockData();
            date_set = false; // Reset the flag
        }
    }
 }

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

    // Implement DMA++ strategy
    std::vector<int> signals;
    std::vector<double> prices(n + 1, 0.0); // stores prices for calculating ER
    double SF = 0.5; // Smoothing Factor (SF0)
    double AMA_prev = stock_data[0].price; // Adaptive Moving Average (AMA0)
    for (size_t i = 0; i < stock_data.size(); ++i) {
        if (stock_data[i].date >= start_date && stock_data[i].date <= last_trading_day) {
            // Calculate Efficiency Ratio (ER)
            for (int j = 0; j <= n; ++j) {
                prices[j] = stock_data[i + j].price;
            }
            double ER = calculateER(prices, n);

            // Calculate Smoothing Factor (SF)
            SF = calculateSF(SF, ER, c1, c2);

            // Calculate Adaptive Moving Average (AMA)
            double AMA = AMA_prev + SF * (stock_data[i].price - AMA_prev);

            // Determine buy/sell signal
            if ((stock_data[i].price - AMA) >= (p / 100.0) * AMA) {
                signals.push_back(1); // Buy
            } else if ((AMA - stock_data[i].price) >= (p / 100.0) * AMA) {
                signals.push_back(-1); // Sell
            } else {
                signals.push_back(0); // Hold
            }

            AMA_prev = AMA; // Update AMA for next iteration
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

    // Calculate daily cashflow
    std::ofstream cashflow_file("daily_cashflow.csv");
    cashflow_file << "Date,Cashflow\n";
    double cashflow = 0.0;
    int a = signals.size();
    int i = a - 1;
    int days_held = 0; // Counter for tracking days held
    while (i >= 0) {
        // Increment days held if position is held
        if (signals[i] != 0) {
            days_held++;
        }
        // Check if max_hold_days is reached and forcefully close position
        if (days_held >= max_hold_days) {
            signals[i] = -signals[i]; // Reverse signal to close position
            days_held = 0; // Reset days held counter
        }
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
  }

  if(strategy == "MACD"){

    std::string strategy = argv[1];
    std::string symbol = argv[2];
    int x = std::stoi(argv[3]);
    std::string start_date = argv[4];
    std::string end_date = argv[5];


    // Build the command to execute the Python script
    std::string python_command = "python script.py " + symbol + " " + start_date + " " + end_date;

    // Execute the Python script
    int python_script_result = std::system(python_command.c_str());


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

            data = StockData();
            date_set = false; // Reset the flag
        }
    }
 }

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

    // Implement MACD strategy
    std::vector<int> signals;
    for (size_t i = 0; i < stock_data.size(); ++i) {
        if (stock_data[i].date >= start_date && stock_data[i].date <= last_trading_day) {
            // Calculate Short EWM
            double short_ewm = 0.0;
            for (int j = 0; j <= 12; ++j) {
                short_ewm = (2.0 / (12.0 + 1)) * (stock_data[i+j].price - short_ewm) + short_ewm;
            }

            // Calculate Long EWM
            double long_ewm = 0.0;
            for (int j = 0; j <= 26; ++j) {
                long_ewm = (2.0 / (26.0 + 1)) * (stock_data[i+j].price - long_ewm) + long_ewm;
            }

            // Calculate MACD
            double macd = short_ewm - long_ewm;

            // Calculate Signal Line
            double signal_line = 0.0;
            for (int j = 0; j <= 9; ++j) {
                signal_line = (2.0 / (9.0 + 1)) * (macd - signal_line) + signal_line;
            }

            if (macd > signal_line) {
                signals.push_back(1);  // Buy
            } else if (macd < signal_line) {
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
            order_file << result_data[i].date << "," << (signals[i] == 1 ? "BUY" : "SELL") << ",1," << result_data[i].price << "\n";
        }
    }
    order_file.close();

  }

  if(strategy == "RSI"){

    std::string symbol = argv[2];
    int x = std::stoi(argv[3]); // Number of days for RSI calculation
    int n = std::stoi(argv[4]); // Placeholder for configurable parameter x
    int oversold_threshold = std::stoi(argv[5]); // Threshold for oversold condition
    int overbought_threshold = std::stoi(argv[6]); // Threshold for overbought condition
    std::string start_date = argv[7];
    std::string end_date = argv[8];

    // Build the command to execute the Python script
    std::string python_command = "python script.py " + symbol + " " + start_date + " " + end_date;

    // Execute the Python script
    int python_script_result = std::system(python_command.c_str());


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

            data = StockData();
            date_set = false; // Reset the flag
        }
    }
 }

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

  }

  if(strategy == "ADX"){

    std::string symbol = argv[2];
    int x = std::stoi(argv[3]);
    int n = std::stoi(argv[4]);
    double adx_threshold = std::stod(argv[5]);
    std::string start_date = argv[6];
    std::string end_date = argv[7];

    // Build the command to execute the Python script
    std::string python_command = "python script.py " + symbol + " " + start_date + " " + end_date;

    // Execute the Python script
    int python_script_result = std::system(python_command.c_str());


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


    for(auto it: stock_data){
      std:: cout<< it.date <<" " << it.close<< " ";
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
  }

  if(strategy == "PAIRS"){

   std::string symbol1 = argv[2];
   std::string symbol2 = argv[3];
   int x = std::stoi(argv[4]);
   int n = std::stoi(argv[5]);
   float threshold = std::stof(argv[6]);
   float stop_loss_threshold;
   std::string start_date;
   std::string end_date;

    std :: string str = argv[7];
    bool flag = true;  //We initialise flag as true.
    for (int i=0; i<str.length(); i++){
      if (isdigit(str[i]) == false){
        flag = false;
        break;
      }
    }
   if(!flag){

     start_date = argv[7];
     end_date = argv[8];

     // Build the command to execute the Python script
    std::string python_command1 = "python script.py " + symbol1 + " " + start_date + " " + end_date;

    // Execute the Python script
    int python_script_result1 = std::system(python_command1.c_str());

    // Build the command to execute the Python script
   std::string python_command2 = "python script.py " + symbol2 + " " + start_date + " " + end_date;

   // Execute the Python script
   int python_script_result2 = std::system(python_command2.c_str());


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


   std::ifstream input_file1("archive/" + symbol1 + ".txt");
   std::ifstream input_file2("archive/" + symbol2 + ".txt");

   if (!input_file1.is_open() || !input_file2.is_open()) {
       std::cerr << "Error: Unable to open input file(s)" << std::endl;
       return 1;
   }

   std::string line;
   std::vector<StockData> stock_data1;
   std::vector<StockData> stock_data2;
   StockData data1;
   StockData data2;
   bool date_set=false;

   while (std::getline(input_file1, line)) {
       std::istringstream iss(line);
       std::string key, value;
       iss >> key >> value;

       if (key == "DATE:") {
           // Set the date for the current StockData object
           data1.date = value;
           date_set = true;
       }
       else if (key == "CLOSE:") {
           // Set the price for the current StockData object
           data1.price1 = std::stod(value);

           // If both date and price are set, push the StockData object into the vector
           if (date_set) {
               stock_data1.push_back(data1);
               data1 = StockData(); // Create a new StockData object for the next pair of DATE and CLOSE lines
               date_set = false; // Reset the flag
           }
       }
   }

   while (std::getline(input_file2, line)) {
     std::istringstream iss(line);
     std::string key, value;
     iss >> key >> value;

     if (key == "DATE:") {
         // Set the date for the current StockData object
         data2.date = value;
         date_set = true;
     }
     else if (key == "CLOSE:") {
         // Set the price for the current StockData object
         data2.price2 = std::stod(value);

         // If both date and price are set, push the StockData object into the vector
         if (date_set) {
             stock_data2.push_back(data2);
             data2 = StockData(); // Create a new StockData object for the next pair of DATE and CLOSE lines
             date_set = false; // Reset the flag
         }
     }
   }
   // std:: cout << stock_data1.size() << stock_data2.size();
   // Check if the number of data points for both stocks match
   if (stock_data1.size() != stock_data2.size()) {
       std::cerr << "Error: Unequal number of data points for the given stock pair." << std::endl;
       return 1;
   }

   // Implement Pairs Trading Strategy
   std::vector<int> signals;
   for (size_t i = 0; i < stock_data1.size(); ++i) {
       if (stock_data1[i].date >= start_date && stock_data1[i].date <= end_date) {
           // Calculate spread
           double spread = stock_data1[i].price1 - stock_data2[i].price2;

           // Calculate rolling mean and std dev of the spread
           double sum = 0.0;
           double sum_sq = 0.0;

           for (int j = 0; j < n ; ++j) {
               sum += stock_data1[i + j+1].price1 - stock_data2[i + j+1].price2;
               sum_sq += pow(stock_data1[i +j+1].price1 - stock_data2[i+j+1].price2, 2);
           }

           double mean = sum / n;
           double std_dev = sqrt((sum_sq - n * pow(mean, 2)) / n);

           // Calculate z-score
           double z_score = (spread - mean) / std_dev;

           // Generate signals
           if (z_score > threshold) {
             std:: cout<< "sell";
               signals.push_back(-1);  // Sell spread (short S1, long S2)
           } else if (z_score < -threshold) {
             std:: cout<< "buy";
               signals.push_back(1);  // Buy spread (long S1, short S2)
           } else {
             std:: cout<< "hold";
               signals.push_back(0);  // Hold
           }
       }
   }

   /// Filter data for start_date to end_date
   std::vector<StockData> result_data1;
   std::vector<StockData> result_data2;

   for (size_t i = 0; i < stock_data1.size(); ++i) {
       if (stock_data1[i].date >= start_date && stock_data1[i].date <= end_date) {
           result_data1.push_back(stock_data1[i]);
           result_data2.push_back(stock_data2[i]);
       }
   }

   // Calculate daily cashflow
   std::ofstream cashflow_file("daily_cashflow.csv");
   cashflow_file << "Date,Cashflow\n";
   double cashflow = 0.0;
   int a = signals.size();
   int i = a - 1;

   while (i >= 0) {
       double spread = result_data1[i].price1 - result_data2[i].price2;
       cashflow += signals[i] * spread;
       cashflow_file << result_data1[i].date << "," << -1 * cashflow << "\n";
       i--;
   }
   cashflow_file.close();

   // Write to order statistics files
   std::ofstream order_file1("order_statistics_1.csv");
   std::ofstream order_file2("order_statistics_2.csv");

   order_file1 << "Date,Direction,Quantity,Price\n";
   order_file2 << "Date,Direction,Quantity,Price\n";

   for (size_t i = 0; i < signals.size(); ++i) {
       if (signals[i] != 0) {
           order_file1 << result_data1[i].date << "," << (signals[i] == 1 ? "BUY" : "SELL") << "," << x << "," << result_data1[i].price1 << "\n";
           order_file2 << result_data2[i].date << "," << (signals[i] == 1 ? "BUY" : "SELL") << "," << x << "," << result_data2[i].price2 << "\n";
       }
   }

   order_file1.close();
   order_file2.close();
  }

else{

  stop_loss_threshold = std::stof(argv[7]);
  start_date = argv[8];
  end_date = argv[9];

  // Build the command to execute the Python script
 std::string python_command1 = "python script.py " + symbol1 + " " + start_date + " " + end_date;

 // Execute the Python script
 int python_script_result1 = std::system(python_command1.c_str());

 // Build the command to execute the Python script
std::string python_command2 = "python script.py " + symbol2 + " " + start_date + " " + end_date;

// Execute the Python script
int python_script_result2 = std::system(python_command2.c_str());


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



  std::ifstream input_file1("archive/" + symbol1 + ".txt");
    std::ifstream input_file2("archive/" + symbol2 + ".txt");

    if (!input_file1.is_open() || !input_file2.is_open()) {
        std::cerr << "Error: Unable to open input file(s)" << std::endl;
        return 1;
    }

    std::string line;
    std::vector<StockData> stock_data1;
    std::vector<StockData> stock_data2;
    StockData data1;
    StockData data2;
    bool date_set=false;

    while (std::getline(input_file1, line)) {
        std::istringstream iss(line);
        std::string key, value;
        iss >> key >> value;

        if (key == "DATE:") {
            // Set the date for the current StockData object
            data1.date = value;
            date_set = true;
        }
        else if (key == "CLOSE:") {
            // Set the price for the current StockData object
            data1.price1 = std::stod(value);

            // If both date and price are set, push the StockData object into the vector
            if (date_set) {
                stock_data1.push_back(data1);
                data1 = StockData(); // Create a new StockData object for the next pair of DATE and CLOSE lines
                date_set = false; // Reset the flag
            }
        }
    }

    while (std::getline(input_file2, line)) {
      std::istringstream iss(line);
      std::string key, value;
      iss >> key >> value;

      if (key == "DATE:") {
          // Set the date for the current StockData object
          data2.date = value;
          date_set = true;
      }
      else if (key == "CLOSE:") {
          // Set the price for the current StockData object
          data2.price2 = std::stod(value);

          // If both date and price are set, push the StockData object into the vector
          if (date_set) {
              stock_data2.push_back(data2);
              data2 = StockData(); // Create a new StockData object for the next pair of DATE and CLOSE lines
              date_set = false; // Reset the flag
          }
      }
    }
    // std:: cout << stock_data1.size() << stock_data2.size();
    // Check if the number of data points for both stocks match
    if (stock_data1.size() != stock_data2.size()) {
        std::cerr << "Error: Unequal number of data points for the given stock pair." << std::endl;
        return 1;
    }

    // Implement Pairs Trading Strategy
    std::vector<int> signals;
    for (size_t i = 0; i < stock_data1.size(); ++i) {
        if (stock_data1[i].date >= start_date && stock_data1[i].date <= end_date) {
            // Calculate spread
            double spread = stock_data1[i].price1 - stock_data2[i].price2;

            // Calculate rolling mean and std dev of the spread
            double sum = 0.0;
            double sum_sq = 0.0;

            for (int j = 0; j < n ; ++j) {
                sum += stock_data1[i + j+1].price1 - stock_data2[i + j+1].price2;
                sum_sq += pow(stock_data1[i + j+1].price1 - stock_data2[i+j+1].price2, 2);
            }

            double mean = sum / n;
            double std_dev = sqrt((sum_sq - n * pow(mean, 2)) / n);

            // Calculate z-score
            double z_score = (spread - mean) / std_dev;

            // Generate signals
            if (z_score > threshold) {
              std:: cout<< "sell";
                signals.push_back(-1);  // Sell spread (short S1, long S2)
            } else if (z_score < -threshold) {
              std:: cout<< "buy";
                signals.push_back(1);  // Buy spread (long S1, short S2)
            } else {
              std:: cout<< "hold";
                signals.push_back(0);  // Hold
            }

            // Apply stop-loss based on stop_loss_threshold
            if (abs(z_score) > stop_loss_threshold) {
                std::cout << " (stop-loss triggered)";
                signals.back() = 0;  // Close position (Hold) if stop-loss is triggered
            }
        }
    }

    /// Filter data for start_date to end_date
    std::vector<StockData> result_data1;
    std::vector<StockData> result_data2;

    for (size_t i = 0; i < stock_data1.size(); ++i) {
        if (stock_data1[i].date >= start_date && stock_data1[i].date <= end_date) {
            result_data1.push_back(stock_data1[i]);
            result_data2.push_back(stock_data2[i]);
        }
    }

    // Calculate daily cashflow
    std::ofstream cashflow_file("daily_cashflow.csv");
    cashflow_file << "Date,Cashflow\n";
    double cashflow = 0.0;
    int a = signals.size();
    int i = a - 1;

    while (i >= 0) {
        double spread = result_data1[i].price1 - result_data2[i].price2;
        cashflow += signals[i] * spread;
        cashflow_file << result_data1[i].date << "," << -1 * cashflow << "\n";
        i--;
    }
    cashflow_file.close();

    // Write to order statistics files
    std::ofstream order_file1("order_statistics_1.csv");
    std::ofstream order_file2("order_statistics_2.csv");

    order_file1 << "Date,Direction,Quantity,Price\n";
    order_file2 << "Date,Direction,Quantity,Price\n";

    for (size_t i = 0; i < signals.size(); ++i) {
        if (signals[i] != 0) {
            order_file1 << result_data1[i].date << "," << (signals[i] == 1 ? "BUY" : "SELL") << "," << x << "," << result_data1[i].price1 << "\n";
            order_file2 << result_data2[i].date << "," << (signals[i] == 1 ? "BUY" : "SELL") << "," << x << "," << result_data2[i].price2 << "\n";
        }
    }

    order_file1.close();
    order_file2.close();
}

}

  return 0;
}
