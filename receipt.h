#ifndef RECEIPT_H
#define RECEIPT_H

#include <iostream>
#include <string>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

using namespace std;

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define CYAN    "\033[36m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"
#define WHITE   "\033[97m"
#define RED     "\033[31m"

class Receipt {
private:
    sql::Connection* con;

    string getCurrentDateTime() {
        time_t now = time(0);
        tm timeinfo = {};

#ifdef _WIN32
        localtime_s(&timeinfo, &now);
#else
        localtime_r(&now, &timeinfo);
#endif

        char buffer[80];
        strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
        return string(buffer);
    }

    void printDoubleLine(int length = 70) {
        cout << string(length, '=') << endl;
    }

    void printSingleLine(int length = 70) {
        cout << string(length, '-') << endl;
    }

    void printCentered(string text, int width = 70) {
        int textLen = static_cast<int>(text.length());
        int padding = (width - textLen) / 2;
        if (padding > 0) {
            cout << string(padding, ' ') << text << endl;
        }
        else {
            cout << text << endl;
        }
    }

    void clearScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void waitForUser() {
        cout << "\n" << YELLOW << "Press Enter to continue..." << RESET;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }

public:
    Receipt(sql::Connection* conn) : con(conn) {}

    void generateReceipt(int orderID, int customerID, string paymentMethod, double totalAmount) {
        try {
            // Get customer details
            sql::PreparedStatement* custStmt = con->prepareStatement(
                "SELECT Customer_Name, PhoneNUM, Customer_Address FROM customer WHERE CustomerID=?"
            );
            custStmt->setInt(1, customerID);
            sql::ResultSet* custRes = custStmt->executeQuery();

            string custName = "", custPhone = "", custAddress = "";
            if (custRes->next()) {
                custName = custRes->getString("Customer_Name");
                custPhone = custRes->getString("PhoneNUM");
                custAddress = custRes->getString("Customer_Address");
            }
            delete custRes;
            delete custStmt;

            // Get order items
            sql::PreparedStatement* itemStmt = con->prepareStatement(
                "SELECT m.Menu_Name, m.Price, oi.Quantity "
                "FROM order_item oi JOIN menu m ON oi.MenuID = m.MenuID "
                "WHERE oi.OrdersID=?"
            );
            itemStmt->setInt(1, orderID);
            sql::ResultSet* itemRes = itemStmt->executeQuery();

            // Clear screen for clean receipt display
            clearScreen();

            // Build receipt content string for database
            stringstream receiptContent;
            receiptContent << "FOODIE EXPRESS DELIVERY SERVICE\n";
            receiptContent << "Order ID: #" << orderID << "\n";
            receiptContent << "Date: " << getCurrentDateTime() << "\n";
            receiptContent << "Customer: " << custName << "\n\n";

            // Print receipt to console (ENHANCED VERSION)
            printDoubleLine();
            cout << BOLD << CYAN;
            printCentered(".->->->->->->->->->->->->->->->->->->->->->->->->->");
            printCentered("||       FOODIE EXPRESS DELIVERY SERVICE          ||");
            printCentered("||             Your Food, Our Priority!           ||");
            printCentered(".->->->->->->->->->->->->->->->->->->->->->->->->->");
            cout << RESET;
            printDoubleLine();

            cout << YELLOW;
            printCentered(" 123, Jalan Makan, Taman Sedap, 50000 KL");
            printCentered(" Tel: 03-1234 5678 | Email: order@foodie.my");
            printCentered(" www.foodieexpress.com.my");
            cout << RESET;
            printSingleLine();

            cout << BOLD << WHITE << "\n  RECEIPT / RESIT" << RESET << endl;
            printSingleLine();

            // Order ID with green color
            cout << left << setw(30) << "  Order ID / No Pesanan:"
                << BOLD << GREEN << "#" << orderID << RESET << endl;
            cout << left << setw(30) << "  Date / Tarikh:"
                << getCurrentDateTime() << endl;
            printSingleLine();

            cout << BOLD << MAGENTA << "\n   CUSTOMER DETAILS / MAKLUMAT PELANGGAN" << RESET << endl;
            printSingleLine();
            cout << left << setw(25) << "  Name / Nama:" << custName << endl;
            cout << left << setw(25) << "  Phone / Telefon:" << custPhone << endl;
            cout << left << setw(25) << "  Address / Alamat:" << custAddress << endl;
            printSingleLine();

            cout << BOLD << CYAN << "\n    ORDER ITEMS / SENARAI PESANAN" << RESET << endl;
            printDoubleLine();

            // Table header
            cout << left << setw(5) << "  No"
                << setw(25) << "Item"
                << setw(10) << "Qty"
                << setw(15) << "Price"
                << "Subtotal" << endl;
            printSingleLine();

            double subtotal = 0.0;
            int itemNo = 1;

            while (itemRes->next()) {
                string menuName = itemRes->getString("Menu_Name");
                double price = itemRes->getDouble("Price");
                int qty = itemRes->getInt("Quantity");
                double itemTotal = price * qty;
                subtotal += itemTotal;

                cout << "  " << left << setw(3) << itemNo++
                    << setw(25) << menuName
                    << "x" << setw(9) << qty
                    << "RM" << setw(13) << fixed << setprecision(2) << price
                    << "RM" << itemTotal << endl;

                receiptContent << itemNo - 1 << ". " << menuName
                    << " x" << qty << " @ RM" << price
                    << " = RM" << itemTotal << "\n";
            }

            printSingleLine();

            // Calculate fees
            double serviceTax = subtotal * 0.06;
            double deliveryFee = 5.00;
            double grandTotal = subtotal + serviceTax + deliveryFee;

            // Price breakdown
            cout << right << setw(55) << "Subtotal: RM "
                << setw(10) << fixed << setprecision(2) << subtotal << endl;
            cout << right << setw(55) << "Service Tax (6%): RM "
                << setw(10) << serviceTax << endl;
            cout << right << setw(55) << "Delivery Fee: RM "
                << setw(10) << deliveryFee << endl;
            printDoubleLine();

            cout << BOLD << GREEN << right << setw(55) << "GRAND TOTAL: RM "
                << setw(10) << grandTotal << RESET << endl;
            printDoubleLine();

            // Payment info
            cout << YELLOW << "\n   Payment Method / Kaedah Bayaran: "
                << BOLD << paymentMethod << RESET << endl;
            cout << GREEN << "   Payment Status: PAID / DIBAYAR" << RESET << endl;
            printSingleLine();

            // Thank you message
            cout << CYAN << BOLD;
            printCentered(" THANK YOU FOR YOUR ORDER! ");
            printCentered("TERIMA KASIH ATAS PESANAN ANDA!");
            cout << RESET;
            printCentered("Please keep this receipt for your reference");
            printCentered("Sila simpan resit ini untuk rujukan anda");
            printDoubleLine();

            cout << GREEN;
            printCentered(" Track your order status in 'View Order History'");
            printCentered(" Estimated delivery: 30-45 minutes");
            cout << RESET;
            printDoubleLine();

            cout << "\n" << YELLOW << "   Follow us on social media for latest promotions!" << RESET << endl;
            printCentered("Facebook: @FoodieExpressMY | Instagram: @foodie_express");
            printDoubleLine();

            // Add to receipt content
            receiptContent << "\nSubtotal: RM" << subtotal << "\n";
            receiptContent << "Service Tax: RM" << serviceTax << "\n";
            receiptContent << "Delivery Fee: RM" << deliveryFee << "\n";
            receiptContent << "GRAND TOTAL: RM" << grandTotal << "\n";
            receiptContent << "Payment: " << paymentMethod << "\n";

            delete itemRes;
            delete itemStmt;

            // Save to database
            saveReceiptToDatabase(orderID, customerID, paymentMethod, grandTotal,
                subtotal, serviceTax, deliveryFee, receiptContent.str());

            cout << GREEN << "\n? Receipt saved to database successfully!" << RESET << endl;
            cout << GREEN << "Cart cleared!" << RESET << endl;

            // Wait for user before returning
            waitForUser();

        }
        catch (sql::SQLException& e) {
            cerr << RED << "Error generating receipt: " << e.what() << RESET << endl;
        }
    }

    void saveReceiptToDatabase(int orderID, int customerID, string paymentMethod,
        double totalAmount, double subTotal, double serviceTax,
        double deliveryFee, string receiptContent) {
        try {
            sql::PreparedStatement* pstmt = con->prepareStatement(
                "INSERT INTO receipt_history (OrdersID, CustomerID, PaymentMethod, "
                "TotalAmount, SubTotal, ServiceTax, DeliveryFee, ReceiptContent) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
            );

            pstmt->setInt(1, orderID);
            pstmt->setInt(2, customerID);
            pstmt->setString(3, paymentMethod);
            pstmt->setDouble(4, totalAmount);
            pstmt->setDouble(5, subTotal);
            pstmt->setDouble(6, serviceTax);
            pstmt->setDouble(7, deliveryFee);
            pstmt->setString(8, receiptContent);

            pstmt->executeUpdate();
            delete pstmt;
        }
        catch (sql::SQLException& e) {
            cerr << RED << "Error saving receipt: " << e.what() << RESET << endl;
        }
    }

    // Owner view receipts
    void viewAllReceipts() {
        try {
            sql::Statement* stmt = con->createStatement();
            sql::ResultSet* res = stmt->executeQuery(
                "SELECT rh.ReceiptID, rh.OrdersID, rh.GeneratedDate, "
                "c.Customer_Name, rh.PaymentMethod, rh.TotalAmount "
                "FROM receipt_history rh "
                "JOIN customer c ON rh.CustomerID = c.CustomerID "
                "ORDER BY rh.GeneratedDate DESC"
            );

            cout << "\n" << BOLD << CYAN;
            cout << "'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''\n";
            cout << "'              RECEIPT HISTORY / SEJARAH RESIT                '\n";
            cout << "'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''\n";
            cout << RESET;

            cout << "\n";
            cout << left << setw(12) << "Receipt ID"
                << setw(12) << "Order ID"
                << setw(22) << "Date/Time"
                << setw(20) << "Customer"
                << setw(18) << "Payment"
                << "Amount" << endl;
            cout << string(90, '-') << endl;

            int count = 0;
            while (res->next()) {
                count++;
                cout << left << setw(12) << res->getInt("ReceiptID")
                    << setw(12) << res->getInt("OrdersID")
                    << setw(22) << res->getString("GeneratedDate")
                    << setw(20) << res->getString("Customer_Name")
                    << setw(18) << res->getString("PaymentMethod")
                    << "RM" << fixed << setprecision(2) << res->getDouble("TotalAmount") << endl;
            }

            cout << string(90, '-') << endl;
            cout << BOLD << GREEN << "Total Receipts: " << count << RESET << endl;

            delete res;
            delete stmt;
        }
        catch (sql::SQLException& e) {
            cerr << RED << "Error: " << e.what() << RESET << endl;
        }
    }

    // View specific receipt (with beautiful display like customer)
    void viewReceiptDetails(int receiptID) {
        try {
            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT rh.*, c.Customer_Name, c.PhoneNUM, c.Customer_Address "
                "FROM receipt_history rh "
                "JOIN customer c ON rh.CustomerID = c.CustomerID "
                "WHERE rh.ReceiptID=?"
            );
            pstmt->setInt(1, receiptID);
            sql::ResultSet* res = pstmt->executeQuery();

            if (res->next()) {
                int orderID = res->getInt("OrdersID");
                string custName = res->getString("Customer_Name");
                string custPhone = res->getString("PhoneNUM");
                string custAddress = res->getString("Customer_Address");
                string paymentMethod = res->getString("PaymentMethod");
                double subtotal = res->getDouble("SubTotal");
                double serviceTax = res->getDouble("ServiceTax");
                double deliveryFee = res->getDouble("DeliveryFee");
                double grandTotal = res->getDouble("TotalAmount");
                string generatedDate = res->getString("GeneratedDate");

                // Display beautiful receipt (same as customer sees)
                clearScreen();

                printDoubleLine();
                cout << BOLD << CYAN;
                printCentered(":::::::::::::::::::::::::::::::::::::::::::::::::::");
                printCentered("::        FOODIE EXPRESS DELIVERY SERVICE        ::");
                printCentered("::             Your Food, Our Priority!          ::");
                printCentered(":::::::::::::::::::::::::::::::::::::::::::::::::::");
                cout << RESET;
                printDoubleLine();

                cout << YELLOW;
                printCentered(" 123, Jalan Makan, Taman Sedap, 50000 KL");
                printCentered(" Tel: 03-1234 5678 |  Email: order@foodie.my");
                cout << RESET;
                printSingleLine();

                cout << BOLD << WHITE << "\n  RECEIPT / RESIT" << RESET << endl;
                printSingleLine();
                cout << left << setw(30) << "  Order ID:"
                    << BOLD << GREEN << "#" << orderID << RESET << endl;
                cout << left << setw(30) << "  Date:" << generatedDate << endl;
                printSingleLine();

                cout << BOLD << MAGENTA << "\n   CUSTOMER DETAILS" << RESET << endl;
                printSingleLine();
                cout << left << setw(25) << "  Name:" << custName << endl;
                cout << left << setw(25) << "  Phone:" << custPhone << endl;
                cout << left << setw(25) << "  Address:" << custAddress << endl;
                printSingleLine();

                // Get order items
                sql::PreparedStatement* itemStmt = con->prepareStatement(
                    "SELECT m.Menu_Name, m.Price, oi.Quantity "
                    "FROM order_item oi JOIN menu m ON oi.MenuID = m.MenuID "
                    "WHERE oi.OrdersID=?"
                );
                itemStmt->setInt(1, orderID);
                sql::ResultSet* itemRes = itemStmt->executeQuery();

                cout << BOLD << CYAN << "\n    ORDER ITEMS" << RESET << endl;
                printDoubleLine();
                cout << left << setw(5) << "  No"
                    << setw(25) << "Item"
                    << setw(10) << "Qty"
                    << setw(15) << "Price"
                    << "Subtotal" << endl;
                printSingleLine();

                int itemNo = 1;
                while (itemRes->next()) {
                    string menuName = itemRes->getString("Menu_Name");
                    double price = itemRes->getDouble("Price");
                    int qty = itemRes->getInt("Quantity");
                    double itemTotal = price * qty;

                    cout << "  " << left << setw(3) << itemNo++
                        << setw(25) << menuName
                        << "x" << setw(9) << qty
                        << "RM" << setw(13) << fixed << setprecision(2) << price
                        << "RM" << itemTotal << endl;
                }
                delete itemRes;
                delete itemStmt;

                printSingleLine();
                cout << right << setw(55) << "Subtotal: RM " << setw(10) << subtotal << endl;
                cout << right << setw(55) << "Service Tax (6%): RM " << setw(10) << serviceTax << endl;
                cout << right << setw(55) << "Delivery Fee: RM " << setw(10) << deliveryFee << endl;
                printDoubleLine();
                cout << BOLD << GREEN << right << setw(55) << "GRAND TOTAL: RM " << setw(10) << grandTotal << RESET << endl;
                printDoubleLine();

                cout << YELLOW << "\n   Payment: " << BOLD << paymentMethod << RESET << endl;
                cout << GREEN << "   Status: PAID" << RESET << endl;
                printSingleLine();

            }
            else {
                cout << RED << "\nReceipt not found!" << RESET << endl;
            }

            delete res;
            delete pstmt;
        }
        catch (sql::SQLException& e) {
            cerr << RED << "Error: " << e.what() << RESET << endl;
        }
    }

    // ? TAMBAHAN: Fungsi untuk search receipts by customer
    void searchReceiptsByCustomer(string customerName) {
        try {
            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT rh.ReceiptID, rh.OrdersID, rh.GeneratedDate, "
                "c.Customer_Name, rh.PaymentMethod, rh.TotalAmount "
                "FROM receipt_history rh "
                "JOIN customer c ON rh.CustomerID = c.CustomerID "
                "WHERE c.Customer_Name LIKE ? "
                "ORDER BY rh.GeneratedDate DESC"
            );
            pstmt->setString(1, "%" + customerName + "%");
            sql::ResultSet* res = pstmt->executeQuery();

            cout << "\n" << BOLD << CYAN << "=== SEARCH RESULTS FOR: " << customerName << " ===" << RESET << endl;
            cout << left << setw(12) << "Receipt ID"
                << setw(12) << "Order ID"
                << setw(22) << "Date"
                << setw(20) << "Customer"
                << "Amount" << endl;
            cout << string(80, '-') << endl;

            int count = 0;
            while (res->next()) {
                count++;
                cout << left << setw(12) << res->getInt("ReceiptID")
                    << setw(12) << res->getInt("OrdersID")
                    << setw(22) << res->getString("GeneratedDate")
                    << setw(20) << res->getString("Customer_Name")
                    << "RM" << fixed << setprecision(2) << res->getDouble("TotalAmount") << endl;
            }

            if (count == 0) {
                cout << YELLOW << "No receipts found for customer: " << customerName << RESET << endl;
            }
            else {
                cout << string(80, '-') << endl;
                cout << GREEN << "Total found: " << count << RESET << endl;
            }

            delete res;
            delete pstmt;
        }
        catch (sql::SQLException& e) {
            cerr << RED << "Search error: " << e.what() << RESET << endl;
        }
    }
};

#endif