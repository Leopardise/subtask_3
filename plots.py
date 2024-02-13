import matplotlib.pyplot as plt
import csv

# Read daily_cashflow.csv
cashflow_dates = []
cashflow_values = []
with open('daily_cashflow.csv', 'r') as cashflow_file:
    reader = csv.reader(cashflow_file)
    next(reader)  # Skip header
    for row in reader:
        cashflow_dates.append(row[0])
        cashflow_values.append(float(row[1]))

# Plot daily cashflow with rotated X-axis labels
plt.figure(figsize=(10, 5))
plt.plot(cashflow_dates, cashflow_values, label='Daily Cashflow')
plt.xlabel('Date')
plt.ylabel('Cashflow')
plt.title('Daily Cashflow')
plt.legend()
plt.xticks(rotation=45, ha='right')  # Rotate X-axis labels for better readability
plt.tight_layout()  # Adjust layout for better display
plt.grid(True)
plt.show()

# Read order_statistics.csv
order_dates = []
order_directions = []
order_prices = []
with open('order_statistics.csv', 'r') as order_file:
    reader = csv.reader(order_file)
    next(reader)  # Skip header
    for row in reader:
        order_dates.append(row[0])
        order_directions.append(row[1])
        order_prices.append(float(row[3]))

# Plot order statistics
plt.figure(figsize=(10, 5))
for i in range(len(order_dates)):
    if order_directions[i] == 'BUY':
        plt.scatter(order_dates[i], order_prices[i], color='green', label='Buy', marker='^')
    else:
        plt.scatter(order_dates[i], order_prices[i], color='red', label='Sell', marker='v')

plt.xlabel('Date')
plt.ylabel('Price')
plt.title('Order Statistics')
plt.legend()
plt.xticks(rotation=45, ha='right')  # Rotate X-axis labels for better readability
plt.tight_layout()  # Adjust layout for better display
plt.grid(True)
plt.show()
