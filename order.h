#ifndef ORDER_H
#define ORDER_H

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <iomanip>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

struct OrderItem {
    int menuID;
    int quantity;
    double price;
    std::string menuName;
};

class Order {
private:
    sql::Connection* conn;
    std::vector<OrderItem> cart;

public:
    Order(sql::Connection* connection) : conn(connection) {}

    // **UPDATED** Add to cart WITH stock validation
    void addToCart(int menuID, int quantity, double price, std::string menuName) {
        // Check stock availability
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("SELECT Stock FROM menu WHERE MenuID=?")
            );
            pstmt->setInt(1, menuID);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            if (res->next()) {
                int availableStock = res->getInt("Stock");

                if (availableStock < quantity) {
                    std::cout << "\n[WARNING] Insufficient stock! Available: " << availableStock
                        << " | You requested: " << quantity << std::endl;

                    if (availableStock > 0) {
                        std::cout << "Max you can add: " << availableStock << " units" << std::endl;
                    }
                    else {
                        std::cout << "This item is OUT OF STOCK!" << std::endl;
                    }
                    return; // Don't add to cart
                }

                // Stock is sufficient, add to cart
                OrderItem item;
                item.menuID = menuID;
                item.quantity = quantity;
                item.price = price;
                item.menuName = menuName;
                cart.push_back(item);
                std::cout << "\n[SUCCESS] Added " << quantity << "x " << menuName << " to cart" << std::endl;
            }
        }
        catch (sql::SQLException& e) {
            std::cerr << "Stock check failed: " << e.what() << std::endl;
        }
    }

    void viewCart() {
        if (cart.empty()) {
            std::cout << "Your cart is empty!" << std::endl;
            return;
        }

        std::cout << "\n=== Your Cart ===" << std::endl;
        double total = 0.0;

        std::cout << std::left << std::setw(10) << "Menu ID"
            << std::setw(20) << "Name"
            << std::setw(10) << "Qty"
            << "Total" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        for (const auto& item : cart) {
            double subtotal = item.price * item.quantity;
            std::cout << std::left << std::setw(10) << item.menuID
                << std::setw(20) << item.menuName
                << " x" << std::setw(8) << item.quantity
                << "RM" << std::fixed << std::setprecision(2) << subtotal << std::endl;
            total += subtotal;
        }
        std::cout << std::string(50, '-') << std::endl;
        std::cout << "Total Amount: RM" << total << std::endl;
    }

    void clearCart() {
        cart.clear();
        std::cout << "Cart cleared!" << std::endl;
    }

    void deleteCartItem(int menuID) {
        bool found = false;
        for (auto it = cart.begin(); it != cart.end(); ++it) {
            if (it->menuID == menuID) {
                std::cout << "\n[REMOVED] " << it->menuName << " has been removed from your cart." << std::endl;
                cart.erase(it);
                found = true;
                break;
            }
        }
        if (!found) {
            std::cout << "\n[ERROR] Item with ID " << menuID << " not found in your cart." << std::endl;
        }
    }

    bool isCartEmpty() {
        return cart.empty();
    }

    // **UPDATED** Create order WITH stock deduction
    int createOrder(int customerID) {
        if (cart.empty()) {
            std::cout << "Cannot create order. Cart is empty!" << std::endl;
            return -1;
        }

        try {
            // STEP 1: Revalidate stock before placing order
            for (const auto& item : cart) {
                std::unique_ptr<sql::PreparedStatement> checkStmt(
                    conn->prepareStatement("SELECT Stock FROM menu WHERE MenuID=?")
                );
                checkStmt->setInt(1, item.menuID);
                std::unique_ptr<sql::ResultSet> res(checkStmt->executeQuery());

                if (res->next()) {
                    int currentStock = res->getInt("Stock");
                    if (currentStock < item.quantity) {
                        std::cout << "\n[ERROR] " << item.menuName << " no longer has sufficient stock!" << std::endl;
                        std::cout << "Available: " << currentStock << " | Required: " << item.quantity << std::endl;
                        std::cout << "Please update your cart before placing order." << std::endl;
                        return -1;
                    }
                }
            }

            // STEP 2: Create order record
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("INSERT INTO orders (CustomerID, Orders_status) VALUES (?, 'Pending')")
            );
            pstmt->setInt(1, customerID);
            pstmt->executeUpdate();

            // Get the new order ID
            std::unique_ptr<sql::Statement> stmt(conn->createStatement());
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT LAST_INSERT_ID() as OrderID"));

            int orderID = 0;
            if (res->next()) {
                orderID = res->getInt("OrderID");
            }

            // STEP 3: Insert order items AND deduct stock
            for (const auto& item : cart) {
                // Insert order item
                std::unique_ptr<sql::PreparedStatement> itemStmt(
                    conn->prepareStatement("INSERT INTO order_item (OrdersID, MenuID, Quantity) VALUES (?, ?, ?)")
                );
                itemStmt->setInt(1, orderID);
                itemStmt->setInt(2, item.menuID);
                itemStmt->setInt(3, item.quantity);
                itemStmt->executeUpdate();

                // Deduct stock
                std::unique_ptr<sql::PreparedStatement> stockStmt(
                    conn->prepareStatement("UPDATE menu SET Stock = Stock - ? WHERE MenuID=?")
                );
                stockStmt->setInt(1, item.quantity);
                stockStmt->setInt(2, item.menuID);
                stockStmt->executeUpdate();

                std::cout << "[INFO] Deducted " << item.quantity << " units from " << item.menuName << std::endl;
            }

            std::cout << "\n[SUCCESS] Order created successfully! Order ID: " << orderID << std::endl;
            return orderID;
        }
        catch (sql::SQLException& e) {
            std::cerr << "Order creation failed: " << e.what() << std::endl;
            return -1;
        }
    }

    void viewOrderHistory(int customerID) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement(
                    "SELECT o.OrdersID, o.OrdersDate, o.Orders_status, "
                    "COALESCE(d.Rider_Name, 'Not Assigned') as RiderName "
                    "FROM orders o LEFT JOIN delivery d ON o.DeliveryID = d.DeliveryID "
                    "WHERE o.CustomerID=? ORDER BY o.OrdersDate DESC"
                )
            );
            pstmt->setInt(1, customerID);

            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            std::cout << "\n=== Order History ===" << std::endl;
            while (res->next()) {
                int orderID = res->getInt("OrdersID");
                std::cout << "Order ID: " << orderID << std::endl;
                std::cout << "Date: " << res->getString("OrdersDate") << std::endl;
                std::cout << "Status: " << res->getString("Orders_status") << std::endl;
                std::cout << "Rider: " << res->getString("RiderName") << std::endl;

                std::unique_ptr<sql::PreparedStatement> itemStmt(
                    conn->prepareStatement(
                        "SELECT m.Menu_Name, oi.Quantity, m.Price "
                        "FROM order_item oi JOIN menu m ON oi.MenuID = m.MenuID "
                        "WHERE oi.OrdersID=?"
                    )
                );
                itemStmt->setInt(1, orderID);

                std::unique_ptr<sql::ResultSet> itemRes(itemStmt->executeQuery());

                double total = 0.0;
                std::cout << "Items:" << std::endl;
                while (itemRes->next()) {
                    double subtotal = itemRes->getDouble("Price") * itemRes->getInt("Quantity");
                    std::cout << "  - " << itemRes->getString("Menu_Name")
                        << " x" << itemRes->getInt("Quantity")
                        << " = RM" << std::fixed << std::setprecision(2) << subtotal << std::endl;
                    total += subtotal;
                }
                std::cout << "Total: RM" << std::fixed << std::setprecision(2) << total << std::endl;
                std::cout << std::string(50, '-') << std::endl;
            }
        }
        catch (sql::SQLException& e) {
            std::cerr << "Query failed: " << e.what() << std::endl;
        }
    }

    double getCartTotal() {
        double total = 0.0;
        for (const auto& item : cart) {
            total += item.price * item.quantity;
        }
        return total;
    }

    int getCartSize() {
        return cart.size();
    }
};

#endif