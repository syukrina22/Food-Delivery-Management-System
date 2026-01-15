#ifndef MENU_H
#define MENU_H

#include <iostream>
#include <string>
#include <memory>
#include <iomanip>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

class Menu {
private:
    sql::Connection* conn;

public:
    Menu(sql::Connection* connection) : conn(connection) {}

    // Display all menu items WITH STOCK
    void displayMenu() {
        try {
            std::unique_ptr<sql::Statement> stmt(conn->createStatement());
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(
                "SELECT m.MenuID, m.Menu_Name, m.Price, m.Menu_Description, m.Stock, c.CategoryName "
                "FROM menu m JOIN category c ON m.CategoryID = c.CategoryID "
                "ORDER BY c.CategoryName, m.Menu_Name"
            ));

            std::string CYAN = "\033[36m";
            std::string GREEN = "\033[32m";
            std::string YELLOW = "\033[33m";
            std::string RED = "\033[31m";
            std::string BOLD = "\033[1m";
            std::string RESET = "\033[0m";

            std::string indent = "     ";

            std::cout << "\n" << indent << YELLOW << BOLD
                << "================================== FOOD MENU ==================================="
                << RESET << std::endl;

            std::cout << indent << CYAN << std::left
                << std::setw(6) << "ID"
                << std::setw(22) << "NAME"
                << std::setw(12) << "PRICE"
                << std::setw(10) << "STOCK"
                << std::setw(25) << "DESCRIPTION"
                << std::setw(15) << "CATEGORY" << RESET << std::endl;

            std::cout << indent << std::string(90, '-') << std::endl;

            while (res->next()) {
                int stock = res->getInt("Stock");
                std::string stockColor = (stock > 10) ? GREEN : (stock > 0 ? YELLOW : RED);
                std::string stockStatus = (stock > 0) ? std::to_string(stock) : "OUT";

                std::cout << indent << std::left
                    << std::setw(6) << res->getInt("MenuID")
                    << std::setw(22) << res->getString("Menu_Name");

                std::string priceStr = "RM " + std::to_string(res->getDouble("Price")).substr(0,
                    std::to_string(res->getDouble("Price")).find(".") + 3);
                std::cout << GREEN << std::setw(12) << priceStr << RESET;

                std::cout << stockColor << std::setw(10) << stockStatus << RESET;

                std::cout << std::left
                    << std::setw(25) << res->getString("Menu_Description")
                    << std::setw(15) << res->getString("CategoryName") << std::endl;
            }
            std::cout << indent << std::string(90, '=') << std::endl;
        }
        catch (sql::SQLException& e) {
            std::cerr << "Query failed: " << e.what() << std::endl;
        }
    }

    // In menu.h - Update searchMenu function display

    void searchMenu(std::string keyword) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement(
                    "SELECT m.MenuID, m.Menu_Name, m.Price, m.Menu_Description, m.Stock, c.CategoryName "
                    "FROM menu m JOIN category c ON m.CategoryID = c.CategoryID "
                    "WHERE m.Menu_Name LIKE ?"
                )
            );
            pstmt->setString(1, "%" + keyword + "%");

            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            std::cout << "\n=== Search Results ===" << std::endl;
            int count = 0;
            while (res->next()) {
                int stock = res->getInt("Stock");
                std::cout << "ID: " << res->getInt("MenuID") << " | "
                    << res->getString("Menu_Name") << " - RM"
                    << std::fixed << std::setprecision(2) << res->getDouble("Price")
                    << " | Stock: " << stock
                    << " (" << res->getString("CategoryName") << ")" << std::endl;
                std::cout << "   " << res->getString("Menu_Description") << std::endl;
                count++;
            }

            if (count == 0) {
                std::cout << "No menu items found matching '" << keyword << "'" << std::endl;
            }
        }
        catch (sql::SQLException& e) {
            std::cerr << "Query failed: " << e.what() << std::endl;
        }
    }

    // Get menu price
    double getMenuPrice(int menuID) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("SELECT Price FROM menu WHERE MenuID=?")
            );
            pstmt->setInt(1, menuID);

            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            if (res->next()) {
                return res->getDouble("Price");
            }
            return 0.0;
        }
        catch (sql::SQLException& e) {
            std::cerr << "Query failed: " << e.what() << std::endl;
            return 0.0;
        }
    }

    // Get menu name
    std::string getMenuName(int menuID) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("SELECT Menu_Name FROM menu WHERE MenuID=?")
            );
            pstmt->setInt(1, menuID);

            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            if (res->next()) {
                return res->getString("Menu_Name");
            }
            return "";
        }
        catch (sql::SQLException& e) {
            return "";
        }
    }

    // **NEW** Check if menu item has sufficient stock
    bool checkStock(int menuID, int quantity) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("SELECT Stock FROM menu WHERE MenuID=?")
            );
            pstmt->setInt(1, menuID);

            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            if (res->next()) {
                int currentStock = res->getInt("Stock");
                return currentStock >= quantity;
            }
            return false;
        }
        catch (sql::SQLException& e) {
            std::cerr << "Stock check failed: " << e.what() << std::endl;
            return false;
        }
    }

    // **NEW** Get current stock level
    int getStock(int menuID) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("SELECT Stock FROM menu WHERE MenuID=?")
            );
            pstmt->setInt(1, menuID);

            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            if (res->next()) {
                return res->getInt("Stock");
            }
            return 0;
        }
        catch (sql::SQLException& e) {
            return 0;
        }
    }

    // **NEW** Deduct stock (called when order is placed)
    bool deductStock(int menuID, int quantity) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("UPDATE menu SET Stock = Stock - ? WHERE MenuID=? AND Stock >= ?")
            );
            pstmt->setInt(1, quantity);
            pstmt->setInt(2, menuID);
            pstmt->setInt(3, quantity);

            int affected = pstmt->executeUpdate();
            return affected > 0;
        }
        catch (sql::SQLException& e) {
            std::cerr << "Stock deduction failed: " << e.what() << std::endl;
            return false;
        }
    }
};

#endif