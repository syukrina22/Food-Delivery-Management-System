#ifndef OWNER_H
#define OWNER_H

#include <iostream>
#include <string>
#include <iomanip>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace std;

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define YELLOW  "\033[33m"
#define ORANGE  "\033[38;5;208m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"
#define RED     "\033[31m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

class Owner {
private:
    sql::Connection* con;

public:
    Owner(sql::Connection* conn) : con(conn) {}

    bool loginOwner(string username, string password) {
        try {
            sql::PreparedStatement* pstmt = con->prepareStatement("SELECT StaffName FROM owner WHERE Username = ? AND Password = ?");
            pstmt->setString(1, username);
            pstmt->setString(2, password);
            sql::ResultSet* res = pstmt->executeQuery();
            if (res->next()) {
                cout << GREEN << BOLD << "\n Welcome!!!, " << res->getString("StaffName") << "!" << RESET << endl;
                delete res; delete pstmt;
                return true;
            }
            delete res; delete pstmt;
            return false;
        }
        catch (sql::SQLException& e) { return false; }
    }

    // MENU MANAGEMENT (UPDATED WITH STOCK)
    void viewAllMenuItems() {
        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery(
            "SELECT m.MenuID, m.Menu_Name, m.Price, m.Stock, m.Menu_Description, c.CategoryName "
            "FROM menu m JOIN category c ON m.CategoryID = c.CategoryID ORDER BY c.CategoryName ASC"
        );
        cout << "\n" << BOLD << CYAN << "=== FOOD MENU (WITH STOCK) ===" << RESET << endl;
        cout << left << setw(5) << "ID" << setw(20) << "Name" << setw(10) << "Price"
            << setw(8) << "Stock" << "Category" << endl;
        cout << string(60, '-') << endl;

        while (res->next()) {
            int stock = res->getInt("Stock");
            string stockColor = (stock > 10) ? GREEN : (stock > 0 ? YELLOW : RED);

            cout << left << setw(5) << res->getInt("MenuID")
                << setw(20) << res->getString("Menu_Name")
                << "RM" << setw(7) << res->getDouble("Price")
                << stockColor << setw(8) << stock << RESET
                << res->getString("CategoryName") << endl;
        }
        delete res; delete stmt;
    }

    // **UPDATED** Add menu WITH stock
    void addMenuItem(string n, double p, string d, int c, int stock) {
        sql::PreparedStatement* pstmt = con->prepareStatement(
            "INSERT INTO menu (Menu_Name, Price, Menu_Description, CategoryID, Stock) VALUES (?, ?, ?, ?, ?)"
        );
        pstmt->setString(1, n);
        pstmt->setDouble(2, p);
        pstmt->setString(3, d);
        pstmt->setInt(4, c);
        pstmt->setInt(5, stock);
        pstmt->executeUpdate();
        cout << GREEN << "[System] Add menu with " << stock << " unit stock!" << RESET << endl;
        delete pstmt;
    }

    // **UPDATED** Update menu WITH stock
    void updateMenuItem(int id, string n, double p, string d, int c, int stock) {
        sql::PreparedStatement* pstmt = con->prepareStatement(
            "UPDATE menu SET Menu_Name=?, Price=?, Menu_Description=?, CategoryID=?, Stock=? WHERE MenuID=?"
        );
        pstmt->setString(1, n);
        pstmt->setDouble(2, p);
        pstmt->setString(3, d);
        pstmt->setInt(4, c);
        pstmt->setInt(5, stock);
        pstmt->setInt(6, id);
        pstmt->executeUpdate();
        cout << GREEN << "[System] Update menu!" << RESET << endl;
        delete pstmt;
    }

    void deleteMenuItem(int id) {
        sql::PreparedStatement* pstmt = con->prepareStatement("DELETE FROM menu WHERE MenuID = ?");
        pstmt->setInt(1, id); pstmt->executeUpdate();
        cout << RED << "[System] Delete Menu!" << RESET << endl;
        delete pstmt;
    }

    // **NEW** Update stock only (for restocking)
    void updateStock(int menuID, int newStock) {
        try {
            sql::PreparedStatement* pstmt = con->prepareStatement(
                "UPDATE menu SET Stock=? WHERE MenuID=?"
            );
            pstmt->setInt(1, newStock);
            pstmt->setInt(2, menuID);
            pstmt->executeUpdate();
            cout << GREEN << "[System] Update Stock To " << newStock << " unit!" << RESET << endl;
            delete pstmt;
        }
        catch (sql::SQLException& e) {
            cout << RED << "[Error] Failed to update stock: " << e.what() << RESET << endl;
        }
    }

    // **NEW** View low stock items (stock <= 10)
    void viewLowStockItems() {
        try {
            sql::Statement* stmt = con->createStatement();
            sql::ResultSet* res = stmt->executeQuery(
                "SELECT MenuID, Menu_Name, Stock FROM menu WHERE Stock <= 10 ORDER BY Stock ASC"
            );

            cout << "\n" << BOLD << RED << "=== WARNING: LOW STOCK! ===" << RESET << endl;
            cout << left << setw(6) << "ID" << setw(30) << "Menu Name" << "Stock" << endl;
            cout << string(50, '-') << endl;

            int count = 0;
            while (res->next()) {
                count++;
                int stock = res->getInt("Stock");
                string color = (stock == 0) ? RED : YELLOW;

                cout << left << setw(6) << res->getInt("MenuID")
                    << setw(30) << res->getString("Menu_Name")
                    << color << stock << RESET << endl;
            }

            if (count == 0) {
                cout << GREEN << "All items have sufficient stock!" << RESET << endl;
            }
            else {
                cout << string(50, '-') << endl;
                cout << BOLD << "Total items with low stock: " << count << RESET << endl;
            }

            delete res; delete stmt;
        }
        catch (sql::SQLException& e) {
            cout << RED << "[Error] Failed check stock: " << e.what() << RESET << endl;
        }
    }

    // CATEGORY MANAGEMENT (unchanged)
    void viewAllCategories() {
        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery("SELECT * FROM category");
        cout << BOLD << "\n--- KATEGORI ---" << RESET << endl;
        while (res->next()) cout << res->getInt("CategoryID") << ". " << res->getString("CategoryName") << endl;
        delete res; delete stmt;
    }

    void addCategory(string n) {
        sql::PreparedStatement* pstmt = con->prepareStatement("INSERT INTO category (CategoryName) VALUES (?)");
        pstmt->setString(1, n); pstmt->executeUpdate();
        cout << GREEN << "[System] Kategori ditambah!" << RESET << endl;
        delete pstmt;
    }

    void updateCategory(int id, string n) {
        sql::PreparedStatement* pstmt = con->prepareStatement("UPDATE category SET CategoryName=? WHERE CategoryID=?");
        pstmt->setString(1, n); pstmt->setInt(2, id); pstmt->executeUpdate();
        cout << GREEN << "[System] update category!" << RESET << endl;
        delete pstmt;
    }

    void deleteCategory(int id) {
        sql::PreparedStatement* pstmt = con->prepareStatement("DELETE FROM category WHERE CategoryID=?");
        pstmt->setInt(1, id); pstmt->executeUpdate();
        cout << RED << "[System] Delete category!" << RESET << endl;
        delete pstmt;
    }

    // ORDERS & SALES (unchanged)
    void viewSalesSummary() {
        try {
            sql::Statement* stmt = con->createStatement();
            sql::ResultSet* res = stmt->executeQuery(
                "SELECT p.PaymentID, p.Amount, c.Customer_Name FROM payment p "
                "JOIN orders o ON p.OrdersID = o.OrdersID JOIN customer c ON o.CustomerID = c.CustomerID"
            );
            double total = 0;
            cout << "\n" << BOLD << GREEN << "=== SALES DETAILS ===" << RESET << endl;
            while (res->next()) {
                double val = res->getDouble("Amount");
                total += val;
                cout << "ID:" << res->getInt("PaymentID") << " | " << res->getString("Customer_Name") << " : RM" << fixed << setprecision(2) << val << endl;
            }
            cout << "------------------------------------------" << endl;
            cout << BOLD << YELLOW << "TOTAL SALES: RM " << total << RESET << endl;
            delete res; delete stmt;
        }
        catch (sql::SQLException& e) {
            cout << RED << "[Ralat] Failed to read sales data.: " << e.what() << RESET << endl;
        }
    }

    void viewAllOrders() {
        try {
            sql::Statement* stmt = con->createStatement();
            sql::ResultSet* res = stmt->executeQuery("SELECT o.OrdersID, o.Orders_status, c.Customer_Name FROM orders o JOIN customer c ON o.CustomerID = c.CustomerID");
            cout << "\n" << BOLD << BLUE << "=== ORDER LIST ===" << RESET << endl;
            while (res->next()) {
                cout << "Order ID: " << res->getInt("OrdersID") << " | Status: " << res->getString("Orders_status")
                    << " | Customer Name: " << res->getString("Customer_Name") << endl;
            }
            delete res; delete stmt;
        }
        catch (sql::SQLException& e) {
            cout << RED << "[Error] Failed to read : " << e.what() << RESET << endl;
        }
    }

    void viewOrderDetails(int id) {
        try {
            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT m.Menu_Name, oi.Quantity FROM order_item oi JOIN menu m ON oi.MenuID = m.MenuID WHERE oi.OrdersID = ?"
            );
            pstmt->setInt(1, id);
            sql::ResultSet* res = pstmt->executeQuery();
            cout << "\nItems for Order #" << id << ":" << endl;
            while (res->next()) {
                cout << "- " << res->getString("Menu_Name") << " (x" << res->getInt("Quantity") << ")" << endl;
            }
            delete res; delete pstmt;
        }
        catch (sql::SQLException& e) {
            cout << RED << "[Error] Failed to read order: " << e.what() << RESET << endl;
        }
    }

    // USER MANAGEMENT (unchanged)
    void viewAllCustomers() {
        try {
            sql::Statement* stmt = con->createStatement();
            sql::ResultSet* res = stmt->executeQuery(
                "SELECT CustomerID, Customer_Name, PhoneNUM, Customer_Address FROM customer ORDER BY CustomerID ASC"
            );

            cout << "\n" << BOLD << MAGENTA << "><><><><><><><><><><><><><><><><><><><><><><><><><><><><" << RESET << endl;
            cout << BOLD << MAGENTA << "//             LIST OF REGISTERED CUSTOMERS                      //" << RESET << endl;
            cout << BOLD << MAGENTA << "><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>><" << RESET << endl;

            cout << left << setw(6) << "ID" << setw(30) << "Customer Name" << setw(18) << "Phone number" << "Address" << endl;
            cout << string(90, '-') << endl;

            int count = 0;
            while (res->next()) {
                count++;
                cout << left
                    << setw(6) << res->getInt("CustomerID")
                    << setw(30) << res->getString("Customer_Name")
                    << setw(18) << res->getString("PhoneNUM")
                    << res->getString("Customer_Address") << endl;
            }

            cout << string(90, '-') << endl;
            cout << BOLD << GREEN << "Total Customer: " << count << RESET << endl;

            delete res;
            delete stmt;
        }
        catch (sql::SQLException& e) {
            cout << RED << "[Error] Failed to read customer data.: " << e.what() << RESET << endl;
        }
    }

    void viewAllRiders() {
        try {
            sql::Statement* stmt = con->createStatement();
            sql::ResultSet* res = stmt->executeQuery(
                "SELECT DeliveryID, Rider_Name, PhoneNUM, Rider_Active FROM delivery ORDER BY DeliveryID ASC"
            );

            cout << "\n" << BOLD << CYAN << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << RESET << endl;
            cout << BOLD << CYAN << "||                LIST OF REGISTERED RIDER                      ||" << RESET << endl;
            cout << BOLD << CYAN << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << RESET << endl;

            cout << left << setw(6) << "ID" << setw(30) << "Rider Name" << setw(18) << "Phone Number" << "Status" << endl;
            cout << string(90, '-') << endl;

            int count = 0;
            while (res->next()) {
                count++;
                string status = res->getString("Rider_Active");
                string statusText = (status == "Y" || status == "y") ?
                    (string(GREEN) + "Active" + RESET) :
                    (string(RED) + "Inactive" + RESET);

                cout << left
                    << setw(6) << res->getInt("DeliveryID")
                    << setw(30) << res->getString("Rider_Name")
                    << setw(18) << res->getString("PhoneNUM")
                    << statusText << endl;
            }

            cout << string(90, '-') << endl;
            cout << BOLD << GREEN << "Total Rider: " << count << RESET << endl;

            delete res;
            delete stmt;
        }
        catch (sql::SQLException& e) {
            cout << RED << "[Error] Failed to read rider data: " << e.what() << RESET << endl;
        }
    }
};

#endif