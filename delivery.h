#ifndef DELIVERY_H
#define DELIVERY_H

#include <iostream>
#include <string>
#include <memory>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

class Delivery {
private:
    sql::Connection* conn;

public:
    Delivery(sql::Connection* connection) : conn(connection) {}

    int loginRider(std::string phone, std::string password) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "SELECT DeliveryID FROM delivery WHERE PhoneNUM=? AND Rider_pass=? AND Rider_Active='Y'"));
            pstmt->setString(1, phone);
            pstmt->setString(2, password);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            if (res->next()) return res->getInt("DeliveryID");
            return -1;
        }
        catch (sql::SQLException& e) { return -1; }
    }

    void viewAvailableOrders() {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "SELECT o.OrdersID, o.OrdersDate, c.Customer_Name, c.Customer_Address FROM orders o "
                "JOIN customer c ON o.CustomerID = c.CustomerID "
                "WHERE o.Orders_status IN ('Pending', 'Confirmed') AND o.DeliveryID IS NULL"));
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            std::cout << "\n=== Available Orders ===" << std::endl;
            while (res->next()) {
                std::cout << "ID: " << res->getInt("OrdersID") << " | Customer: " << res->getString("Customer_Name") << std::endl;
            }
        }
        catch (sql::SQLException& e) { std::cerr << e.what(); }
    }

    bool acceptOrder(int orderID, int deliveryID) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "UPDATE orders SET DeliveryID=?, Orders_status='Out for Delivery' WHERE OrdersID=? AND DeliveryID IS NULL"));
            pstmt->setInt(1, deliveryID);
            pstmt->setInt(2, orderID);
            return pstmt->executeUpdate() > 0;
        }
        catch (sql::SQLException& e) { return false; }
    }

    // FUNGSI PENTING: Untuk hilangkan error viewMyDeliveries
    int viewMyDeliveries(int deliveryID) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "SELECT o.OrdersID, o.Orders_status, c.Customer_Name FROM orders o "
                "JOIN customer c ON o.CustomerID = c.CustomerID WHERE o.DeliveryID=? AND o.Orders_status != 'Completed'"));
            pstmt->setInt(1, deliveryID);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            std::cout << "\n=== My Current Deliveries ===" << std::endl;

            int count = 0;
            while (res->next()) {
                std::cout << "Order ID: " << res->getInt("OrdersID")
                    << " | Status: " << res->getString("Orders_status")
                    << " | Customer: " << res->getString("Customer_Name") << std::endl;
                count++;
            }

            if (count == 0) {
                std::cout << "You have no active deliveries at the moment." << std::endl;
            }
            return count; // Pulangkan jumlah pesanan
        }
        catch (sql::SQLException& e) {
            std::cerr << "Database error: " << e.what() << std::endl;
            return 0;
        }
    }

    // FUNGSI PENTING: Untuk hilangkan error updateDeliveryStatus
    bool updateDeliveryStatus(int orderID, std::string status) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "UPDATE orders SET Orders_status=? WHERE OrdersID=?"));
            pstmt->setString(1, status);
            pstmt->setInt(2, orderID);
            return pstmt->executeUpdate() > 0;
        }
        catch (sql::SQLException& e) { return false; }
    }

    bool completeDelivery(int orderID) {
        return updateDeliveryStatus(orderID, "Completed");
    }

    // FUNGSI PENTING: Untuk hilangkan error viewDeliveryHistory
    void viewDeliveryHistory(int deliveryID) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "SELECT OrdersID, OrdersDate FROM orders WHERE DeliveryID=? AND Orders_status='Completed'"));
            pstmt->setInt(1, deliveryID);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            std::cout << "\n=== My History ===" << std::endl;
            while (res->next()) {
                std::cout << "Order ID: " << res->getInt("OrdersID") << " | Date: " << res->getString("OrdersDate") << std::endl;
            }
        }
        catch (sql::SQLException& e) { std::cerr << e.what(); }
    }
};

#endif