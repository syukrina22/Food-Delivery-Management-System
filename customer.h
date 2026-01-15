#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <iostream>
#include <string>
#include <memory>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>



class Customer {
private:
    sql::Connection* conn;

public:
    Customer(sql::Connection* connection) : conn(connection) {}

    // Register new customer
    bool registerCustomer(std::string name, std::string phone, std::string password, std::string address) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("INSERT INTO customer (Customer_Name, PhoneNUM, Customer_pass, Customer_Address) VALUES (?, ?, ?, ?)")
            );
            pstmt->setString(1, name);
            pstmt->setString(2, phone);
            pstmt->setString(3, password);
            pstmt->setString(4, address);
            pstmt->executeUpdate();

            std::cout << "Customer registered successfully!" << std::endl;
            return true;
        }
        catch (sql::SQLException& e) {
            std::cerr << "Registration failed: " << e.what() << std::endl;
            std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
            return false;
        }
    }

    // Login customer
    int loginCustomer(std::string phone, std::string password) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("SELECT CustomerID FROM customer WHERE PhoneNUM=? AND Customer_pass=?")
            );
            pstmt->setString(1, phone);
            pstmt->setString(2, password);

            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());


            if (res->next()) {
                int customerID = res->getInt("CustomerID");
                std::cout << "Login successful! Welcome back." << std::endl;
                return customerID;
            }


            std::cout << "Invalid phone number or password!" << std::endl;
            return -1;
        }
        catch (sql::SQLException& e) {
            std::cerr << "Login failed: " << e.what() << std::endl;
            return -1;
        }
    }

    // View customer profile
    void viewProfile(int customerID) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("SELECT Customer_Name, PhoneNUM, Customer_Address FROM customer WHERE CustomerID=?")
            );
            pstmt->setInt(1, customerID);

            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            std::cout << "\n=== Customer Profile ===" << std::endl;
            if (res->next()) {
                std::cout << "Name: " << res->getString("Customer_Name") << std::endl;
                std::cout << "Phone: " << res->getString("PhoneNUM") << std::endl;
                std::cout << "Address: " << res->getString("Customer_Address") << std::endl;
            }
        }
        catch (sql::SQLException& e) {
            std::cerr << "Query failed: " << e.what() << std::endl;
        }
    }

    // Update customer address
    bool updateAddress(int customerID, std::string newAddress) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("UPDATE customer SET Customer_Address=? WHERE CustomerID=?")
            );
            pstmt->setString(1, newAddress);
            pstmt->setInt(2, customerID);
            pstmt->executeUpdate();

            std::cout << "Address updated successfully!" << std::endl;
            return true;
        }
        catch (sql::SQLException& e) {
            std::cerr << "Update failed: " << e.what() << std::endl;
            return false;
        }
    }
};

#endif
