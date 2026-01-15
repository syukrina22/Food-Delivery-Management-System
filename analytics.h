#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <iostream>
#include <string>
#include <iomanip>
#include <map>
#include <vector>
#include <algorithm>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace std;

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define CYAN    "\033[36m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define RED     "\033[31m"
#define MAGENTA "\033[35m"

class Analytics {
private:
    sql::Connection* con;

    void printTableLine(int width = 70) {
        cout << "+";
        for (int i = 0; i < width; i++) cout << "-";
        cout << "+" << endl;
    }

    void printHeader(string title) {
        cout << "\n" << BOLD << CYAN;
        printTableLine(70);
        cout << "| " << left << setw(68) << title << " |" << endl;
        printTableLine(70);
        cout << RESET;
    }

public:
    Analytics(sql::Connection* conn) : con(conn) {}

    // 1. CATEGORY PERFORMANCE - TABLE FORMAT
    void showCategoryPerformance() {
        printHeader("1. GENERATE SALES TABLE BY CATEGORY");

        try {
            sql::Statement* stmt = con->createStatement();
            sql::ResultSet* res = stmt->executeQuery(
                "SELECT c.CategoryID, c.CategoryName, "
                "SUM(oi.Quantity) as total_quantity, "
                "SUM(oi.Quantity * m.Price) as total_sales "
                "FROM category c "
                "JOIN menu m ON c.CategoryID = m.CategoryID "
                "JOIN order_item oi ON m.MenuID = oi.MenuID "
                "GROUP BY c.CategoryID, c.CategoryName "
                "ORDER BY total_sales DESC"
            );

            cout << "\n";
            printTableLine(70);
            cout << "| " << left << setw(15) << "Category ID"
                << "| " << setw(20) << "Category Name"
                << "| " << setw(12) << "Quantity"
                << "| " << setw(15) << "Total Sales" << " |" << endl;
            printTableLine(70);

            double grandTotal = 0.0;
            while (res->next()) {
                int catID = res->getInt("CategoryID");
                string catName = res->getString("CategoryName");
                int qty = res->getInt("total_quantity");
                double sales = res->getDouble("total_sales");
                grandTotal += sales;

                cout << "| " << left << setw(15) << catID
                    << "| " << setw(20) << catName
                    << "| " << setw(12) << qty
                    << "| RM" << setw(14) << fixed << setprecision(2) << sales << " |" << endl;
            }
            printTableLine(70);

            cout << BOLD << GREEN << "\nGRAND TOTAL: RM" << fixed << setprecision(2) << grandTotal << RESET << endl;

            delete res;
            delete stmt;
        }
        catch (sql::SQLException& e) {
            cerr << RED << "Error: " << e.what() << RESET << endl;
        }
    }

    // 2. SALES SUMMARY - TABLE FORMAT
    void showSalesSummary() {
        printHeader("2. GENERATE SALES SUMMARY");

        try {
            sql::Statement* stmt = con->createStatement();

            // Total Monthly Sales
            sql::ResultSet* monthRes = stmt->executeQuery(
                "SELECT IFNULL(SUM(Amount), 0) as total FROM payment "
                "WHERE MONTH(PaymentDate) = MONTH(NOW()) AND YEAR(PaymentDate) = YEAR(NOW())"
            );
            double monthlySales = 0;
            if (monthRes->next()) monthlySales = monthRes->getDouble("total");
            delete monthRes;

            // Total Inventory Value
            sql::ResultSet* invRes = stmt->executeQuery(
                "SELECT IFNULL(SUM(Stock * Price), 0) as total FROM menu"
            );
            double inventoryValue = 0;
            if (invRes->next()) inventoryValue = invRes->getDouble("total");
            delete invRes;

            // Calculate Profit Margin (example: 25%)
            double profitMargin = 25.0;

            cout << "\n";
            cout << "1. Total Monthly Sales: RM" << fixed << setprecision(2) << monthlySales << endl;
            cout << "2. Total Inventory Value: RM" << fixed << setprecision(2) << inventoryValue << endl;
            cout << "3. Profit Margin: " << profitMargin << "%" << endl;

            delete stmt;
        }
        catch (sql::SQLException& e) {
            cerr << RED << "Error: " << e.what() << RESET << endl;
        }
    }

    // 3. PEAK HOURS BAR CHART (UPDATED - replaces monthly)
    void showPeakHoursBarChart() {
        printHeader("3. GENERATE TEXT BAR CHART - PEAK HOURS ANALYSIS");

        try {
            sql::Statement* stmt = con->createStatement();
            sql::ResultSet* res = stmt->executeQuery(
                "SELECT HOUR(OrdersDate) as hour, COUNT(*) as total "
                "FROM orders "
                "GROUP BY HOUR(OrdersDate) "
                "ORDER BY hour"
            );

            cout << "\n";

            map<int, int> hourlyOrders;
            int maxOrders = 0;
            while (res->next()) {
                int hour = res->getInt("hour");
                int total = res->getInt("total");
                hourlyOrders[hour] = total;
                if (total > maxOrders) maxOrders = total;
            }

            // Display bar chart by hour order
            for (auto& entry : hourlyOrders) {
                int hour = entry.first;
                int orders = entry.second;

                // Calculate number of stars (scale to max 50 stars)
                int numStars = (maxOrders > 0) ? (int)((orders * 1.0 / maxOrders) * 50) : 0;
                if (numStars < 1 && orders > 0) numStars = 1;

                cout << "Hour " << setw(2) << setfill('0') << hour << ":00 : ";
                cout << setfill(' ');

                // Color: RED for peak, GREEN for others
                if (orders == maxOrders) {
                    cout << RED;
                }
                else {
                    cout << GREEN;
                }

                for (int i = 0; i < numStars; i++) cout << "*";
                cout << RESET << " (" << orders << " orders)" << endl;
            }

            // Find and display peak hour
            auto peak = max_element(hourlyOrders.begin(), hourlyOrders.end(),
                [](const pair<int, int>& a, const pair<int, int>& b) {
                    return a.second < b.second;
                });

            cout << "\n" << BOLD << RED << "PEAK HOUR: "
                << setw(2) << setfill('0') << peak->first << ":00" << setfill(' ')
                << " (" << peak->second << " orders)"
                << RESET << endl;

            delete res;
            delete stmt;
        }
        catch (sql::SQLException& e) {
            cerr << RED << "Error: " << e.what() << RESET << endl;
        }
    }

    // 4. TOP SELLING ITEMS - TABLE FORMAT
    void showTopSellingTable() {
        printHeader("4. TOP SELLING ITEMS TABLE");

        try {
            sql::Statement* stmt = con->createStatement();
            sql::ResultSet* res = stmt->executeQuery(
                "SELECT m.MenuID, m.Menu_Name, "
                "COUNT(DISTINCT oi.OrdersID) as times_ordered, "
                "SUM(oi.Quantity) as total_sold, "
                "SUM(oi.Quantity * m.Price) as revenue "
                "FROM order_item oi "
                "JOIN menu m ON oi.MenuID = m.MenuID "
                "GROUP BY m.MenuID, m.Menu_Name "
                "ORDER BY total_sold DESC "
                "LIMIT 10"
            );

            cout << "\n";
            printTableLine(85);
            cout << "| " << left << setw(6) << "Rank"
                << "| " << setw(10) << "Menu ID"
                << "| " << setw(25) << "Menu Name"
                << "| " << setw(13) << "Times Order"
                << "| " << setw(11) << "Total Sold"
                << "| " << setw(12) << "Revenue" << " |" << endl;
            printTableLine(85);

            int rank = 1;
            while (res->next()) {
                int menuID = res->getInt("MenuID");
                string menuName = res->getString("Menu_Name");
                int timesOrdered = res->getInt("times_ordered");
                int totalSold = res->getInt("total_sold");
                double revenue = res->getDouble("revenue");

                string rankStr = (rank == 1) ? "1st" : (rank == 2) ? "2nd" : (rank == 3) ? "3rd" : to_string(rank) + "th";

                cout << "| " << left << setw(6) << rankStr
                    << "| " << setw(10) << menuID
                    << "| " << setw(25) << menuName
                    << "| " << setw(13) << timesOrdered
                    << "| " << setw(11) << totalSold
                    << "| $" << setw(11) << fixed << setprecision(2) << revenue << " |" << endl;
                rank++;
            }
            printTableLine(85);

            delete res;
            delete stmt;
        }
        catch (sql::SQLException& e) {
            cerr << RED << "Error: " << e.what() << RESET << endl;
        }
    }
};

#endif