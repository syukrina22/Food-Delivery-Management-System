#ifndef PAYMENT_H
#define PAYMENT_H

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

class Payment {
private:
    sql::Connection* conn;

public:
    Payment(sql::Connection* connection) : conn(connection) {}

    // Create payment
    bool createPayment(int orderID, std::string paymentMethod, double amount) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement(
                    "INSERT INTO payment (OrdersID, PaymentMethod, Amount, PaymentStatus) VALUES (?, ?, ?, 'Pending')"
                )
            );
            pstmt->setInt(1, orderID);
            pstmt->setString(2, paymentMethod);
            pstmt->setDouble(3, amount);
            pstmt->executeUpdate();

            std::cout << "Payment record created successfully!" << std::endl;
            return true;
        }
        catch (sql::SQLException& e) {
            std::cerr << "Payment creation failed: " << e.what() << std::endl;
            return false;
        }
    }

    // Process payment
    bool processPayment(int paymentID, std::string status) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement(
                    "UPDATE payment SET PaymentStatus=?, PaymentDate=NOW() WHERE PaymentID=?"
                )
            );
            pstmt->setString(1, status);
            pstmt->setInt(2, paymentID);
            pstmt->executeUpdate();

            // Update order status if payment completed
            if (status == "Paid" || status == "Completed") {
                std::unique_ptr<sql::PreparedStatement> orderStmt(
                    conn->prepareStatement(
                        "UPDATE orders SET Orders_status='Confirmed' "
                        "WHERE OrdersID=(SELECT OrdersID FROM payment WHERE PaymentID=?)"
                    )
                );
                orderStmt->setInt(1, paymentID);
                orderStmt->executeUpdate();
            }

            std::cout << "Payment processed successfully!" << std::endl;
            return true;
        }
        catch (sql::SQLException& e) {
            std::cerr << "Payment update failed: " << e.what() << std::endl;
            return false;
        }
    }

    // View payment details
    void viewPaymentDetails(int orderID) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement(
                    "SELECT PaymentID, PaymentMethod, PaymentDate, PaymentStatus, Amount "
                    "FROM payment WHERE OrdersID=?"
                )
            );
            pstmt->setInt(1, orderID);

            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            std::cout << "\n=== Payment Details ===" << std::endl;
            if (res->next()) {
                std::cout << "Payment ID: " << res->getInt("PaymentID") << std::endl;
                std::cout << "Method: " << res->getString("PaymentMethod") << std::endl;

                sql::SQLString dateStr = res->getString("PaymentDate");
                std::cout << "Date: " << (res->isNull("PaymentDate") ? "Pending" : dateStr.asStdString()) << std::endl;

                std::cout << "Status: " << res->getString("PaymentStatus") << std::endl;
                std::cout << "Amount: RM" << std::fixed << std::setprecision(2)
                    << res->getDouble("Amount") << std::endl;
            }
            else {
                std::cout << "No payment record found for this order." << std::endl;
            }
        }
        catch (sql::SQLException& e) {
            std::cerr << "Query failed: " << e.what() << std::endl;
        }
    }

    // Display payment methods
    void displayPaymentMethods() {
        std::cout << "\n=== Payment Methods ===" << std::endl;
        std::cout << "1. Cash" << std::endl;
        std::cout << "2. Online Banking" << std::endl;
        std::cout << "3. Credit Card" << std::endl;
        std::cout << "4. E-Wallet" << std::endl;
    }

    // Get payment ID by order ID
    int getPaymentID(int orderID) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("SELECT PaymentID FROM payment WHERE OrdersID=?")
            );
            pstmt->setInt(1, orderID);

            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            if (res->next()) {
                return res->getInt("PaymentID");
            }
            return -1;
        }
        catch (sql::SQLException& e) {
            return -1;
        }
    }
};

#endif