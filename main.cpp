// Disable specific MSVC warnings at the top
#ifdef _MSC_VER
#pragma warning(disable: 4996)  // localtime
#pragma warning(disable: 4267)  // size_t conversion
#pragma warning(disable: 4101)  // unreferenced variable
#endif

#include <iostream>
#include <string>
#include <limits>
#include <iomanip>
#include <vector>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include "customer.h"
#include "menu.h"
#include "order.h"
#include "payment.h"
#include "delivery.h"
#include "owner.h"
#include "receipt.h"
#include "analytics.h"

using namespace std;

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define CYAN    "\033[36m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pause() {
    cout << "\n" << YELLOW << "Press Enter to continue..." << RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

void printCentered(string text, int width = 80) {
    int textLen = static_cast<int>(text.length());
    int padding = (width - textLen) / 2;
    if (padding > 0) {
        cout << string(padding, ' ') << text << endl;
    }
    else {
        cout << text << endl;
    }
}

// FORWARD DECLARATIONS - CRITICAL!
void customerMenu(Customer& customer, Menu& menu, Order& order, Payment& payment, Receipt& receipt, int customerID);
void riderMenu(Delivery& delivery, int riderID);
void ownerMenu(Owner& owner, Menu& menu, Analytics& analytics, Receipt& receipt);

int main() {
    sql::Driver* driver;
    std::unique_ptr<sql::Connection> con;

    try {
        driver = get_driver_instance();
        con.reset(driver->connect("tcp://127.0.0.1:3306", "root", ""));
        con->setSchema("fooddelivery");
        cout << GREEN << "? Connected to database successfully!" << RESET << endl;
    }
    catch (sql::SQLException& e) {
        cout << "Connection failed!" << endl;
        cout << "# ERR: " << e.what() << endl;
        system("pause");
        return 1;
    }

    // Create objects
    Customer customer(con.get());
    Menu menu(con.get());
    Order order(con.get());
    Payment payment(con.get());
    Delivery delivery(con.get());
    Owner owner(con.get());
    Receipt receipt(con.get());
    Analytics analytics(con.get());

    int choice;

    while (true) {
        clearScreen();

        cout << "\n";
        cout << BOLD << CYAN;
        cout << " |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|\n";
        cout << " |                                                                                                                     |\n";
        cout << " |                                          FOODIE EXPRESS DELIVERY SYSTEM                                             |\n";
        cout << " |                                             Your Food, Our Priority!                                                |\n";
        cout << " |                                                                                                                     |\n";
        cout << " |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|\n";
        cout << RESET;

        cout << "\n" << YELLOW;
        printCentered("      ============================================================================================================");
        cout << RESET;

        cout << "\n";
        cout << "                                                " << GREEN << " 1. Customer Login" << RESET << endl;
        cout << "                                                " << GREEN << " 2. Customer Registration\n" << RESET << endl;
        cout << "                                                " << CYAN << " 3. Rider Login\n" << RESET << endl;
        cout << "                                                " << MAGENTA << " 4. Owner Login\n" << RESET << endl;
        cout << "                                                " << YELLOW << " 5. View Menu (Guest)\n" << RESET << endl;
        cout << "                                                 " << RED << "0. Exit System" << endl;

        cout << "\n" << YELLOW;
        printCentered("      ============================================================================================================");
        cout << RESET;

        cout << "\n          " << BOLD << "Enter your choice ? " << RESET;
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a number." << endl;
            pause();
            continue;
        }

        switch (choice) {
        case 1: {
            string phone, password;
            cout << "\n" << BOLD << CYAN << "=== CUSTOMER LOGIN ===" << RESET << endl;
            cout << " Phone Number: ";
            cin >> phone;
            cout << " Password: ";
            cin >> password;

            int customerID = customer.loginCustomer(phone, password);
            if (customerID != -1) {
                pause();
                customerMenu(customer, menu, order, payment, receipt, customerID);
            }
            else {
                pause();
            }
            break;
        }

        case 2: {
            string name, phone, password, address;
            cout << "\n" << BOLD << GREEN << "=====CUSTOMER REGISTRATION =====" << RESET << endl;
            cin.ignore();
            cout << "  Name: ";
            getline(cin, name);
            cout << "  Phone Number: ";
            getline(cin, phone);
            cout << "  Password: ";
            getline(cin, password);
            cout << "  Address: ";
            getline(cin, address);

            customer.registerCustomer(name, phone, password, address);
            pause();
            break;
        }

        case 3: {
            string phone, password;
            cout << "\n" << BOLD << CYAN << "==== RIDER LOGIN ====" << RESET << endl;
            cout << " Phone Number: ";
            cin >> phone;
            cout << " Password: ";
            cin >> password;

            int riderID = delivery.loginRider(phone, password);
            if (riderID != -1) {
                pause();
                riderMenu(delivery, riderID);
            }
            else {
                pause();
            }
            break;
        }

        case 4: {
            string username, password;
            cout << "\n" << BOLD << MAGENTA << "==== OWNER LOGIN =====" << RESET << endl;
            cout << " Username: ";
            cin >> username;
            cout << " Password: ";
            cin >> password;

            if (owner.loginOwner(username, password)) {
                pause();
                ownerMenu(owner, menu, analytics, receipt);
            }
            else {
                pause();
            }
            break;
        }

        case 5: {
            menu.displayMenu();
            pause();
            break;
        }

        case 0: {
            cout << "\n" << GREEN;
            printCentered("+++++++++++++++++++++++++++++++++++++++++");
            printCentered("+  Thank you for using Foodie Express!  +");
            printCentered("+      Have a delicious day!            +");
            printCentered("+++++++++++++++++++++++++++++++++++++++++");
            cout << RESET << "\n";
            con->close();
            return 0;
        }

        default: {
            cout << "Invalid choice! Please try again." << endl;
            pause();
        }
        }
    }

    return 0;
}

void customerMenu(Customer& customer, Menu& menu, Order& order, Payment& payment, Receipt& receipt, int customerID) {
    int choice;
    while (true) {
        clearScreen();
        cout << BOLD << GREEN;
        cout << "  *****************************************************************\n";
        cout << "  *                       CUSTOMER PORTAL                         *\n";
        cout << "  *****************************************************************\n";
        cout << RESET;

        cout << "\n    MENU OPTIONS:\n";
        cout << " 1. View Menu\n";
        cout << " 2. Search Menu\n";
        cout << " 3. Manage Cart (Add/Delete/Clear)\n";
        cout << " 4. View Cart\n";
        cout << " 5. Place Order & Checkout\n";
        cout << " 6. View Order History\n";
        cout << " 7. View Profile\n";
        cout << " 8. Update Address\n";
        cout << " 0. Logout\n";
        cout << "\nEnter choice: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input!" << endl;
            pause();
            continue;
        }

        switch (choice) {
        case 1: {
            menu.displayMenu();
            pause();
            break;
        }

        case 2: {
            string keyword;
            cout << "Enter search keyword: ";
            cin.ignore();
            getline(cin, keyword);
            menu.searchMenu(keyword);
            pause();
            break;
        }

        case 3: {
            int cartChoice;
            do {
                clearScreen();
                cout << BOLD << CYAN << "============================ AVAILABLE MENU ===============================" << RESET << endl;
                menu.displayMenu();

                cout << "\n" << BOLD << YELLOW << "+-+-+-+-+-+-+- YOUR CART +-+-+-+-+-+-+-+-+-" << RESET << endl;
                order.viewCart();

                cout << "  \n" << BOLD << "Actions:" << RESET << endl;
                cout << "       "<<"1.Add Item to Cart" << endl;
                cout << "       "<<"2.Delete Specific Item" << endl;
                cout << "       "<<"3.Clear Entire Cart" << endl;
                cout << "       "<<"0.Back to Menu" << endl;
                cout << "       "<<"Choice: ";
                cin >> cartChoice;

                if (cartChoice == 1) {
                    int menuID, qty;
                    cout << "\nEnter Menu ID (0 to cancel): ";
                    cin >> menuID;
                    if (menuID != 0) {
                        cout << "Enter Quantity: ";
                        cin >> qty;

                        double price = menu.getMenuPrice(menuID);
                        string name = menu.getMenuName(menuID);

                        if (price > 0) {
                            order.addToCart(menuID, qty, price, name);
                        }
                        else {
                            cout << "Invalid Menu ID!" << endl;
                        }
                        pause();
                    }
                }
                else if (cartChoice == 2) {
                    if (!order.isCartEmpty()) {
                        int idToDel;
                        cout << "\nEnter Menu ID to remove: ";
                        cin >> idToDel;
                        order.deleteCartItem(idToDel);
                    }
                    pause();
                }
                else if (cartChoice == 3) {
                    order.clearCart();
                    pause();
                }

            } while (cartChoice != 0);
            break;
        }

        case 4: {
            order.viewCart();
            pause();
            break;
        }

              // FIXED CUSTOMER MENU - Place Order Section (Case 5)
             // Replace case 5 in your customerMenu() function with this:

        case 5: {
            if (order.getCartSize() == 0) {
                cout << RED << "Cart is empty! Add items first." << RESET << endl;
                pause();
                break;
            }

            // Show cart
            order.viewCart();

            // Ask for delivery address confirmation/update
            cout << "\n" << YELLOW << "??? DELIVERY INFORMATION ???" << RESET << endl;
            customer.viewProfile(customerID);

            cout << "\n" << BOLD << "Is this delivery address correct? (y/n): " << RESET;
            char addressConfirm;
            cin >> addressConfirm;

            if (addressConfirm == 'n' || addressConfirm == 'N') {
                string newAddress;
                cout << "\n" << YELLOW << "Enter new delivery address: " << RESET;
                cin.ignore();
                getline(cin, newAddress);

                if (customer.updateAddress(customerID, newAddress)) {
                    cout << GREEN << "? Delivery address updated!" << RESET << endl;
                }
            }

            // Confirm order
            cout << "\n" << BOLD << YELLOW << "Confirm order? (y/n): " << RESET;
            char confirm;
            cin >> confirm;

            if (confirm == 'y' || confirm == 'Y') {
                int orderID = order.createOrder(customerID);
                if (orderID != -1) {
                    payment.displayPaymentMethods();
                    int paymentChoice;
                    cout << "Select payment method (1-4): ";
                    cin >> paymentChoice;

                    string paymentMethod;
                    switch (paymentChoice) {
                    case 1: paymentMethod = "Cash"; break;
                    case 2: paymentMethod = "Online Banking"; break;
                    case 3: paymentMethod = "Credit Card"; break;
                    case 4: paymentMethod = "E-Wallet"; break;
                    default: paymentMethod = "Cash";
                    }

                    double total = order.getCartTotal();
                    if (payment.createPayment(orderID, paymentMethod, total)) {
                        // Generate receipt (will clear screen and show in new page)
                        receipt.generateReceipt(orderID, customerID, paymentMethod, total);
                        order.clearCart();
                    }
                }
            }
            else {
                cout << "\n" << YELLOW << "Order cancelled. You can:" << RESET << endl;
                cout << "- Modify your cart (option 3)" << endl;
                cout << "- Update delivery address (option 8)" << endl;
                cout << "- Try placing order again (option 5)" << endl;
                pause();
            }
            break;
        }

        case 6: {
            order.viewOrderHistory(customerID);
            pause();
            break;
        }

        case 7: {
            customer.viewProfile(customerID);
            pause();
            break;
        }

        case 8: {
            string newAddress;
            cout << "Enter new address: ";
            cin.ignore();
            getline(cin, newAddress);
            customer.updateAddress(customerID, newAddress);
            pause();
            break;
        }

        case 0: {
            cout << "Logging out..." << endl;
            return;
        }

        default: {
            cout << "Invalid choice!" << endl;
            pause();
        }
        }
    }
}

void riderMenu(Delivery& delivery, int riderID) {
    int choice;

    while (true) {
        clearScreen();
        cout << BOLD << CYAN;
        cout << "  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n";
        cout << "  |                    RIDER DASHBOARD                            |\n";
        cout << "  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n";
        cout << RESET;

        cout << "\n1. View Available Orders\n";
        cout << "2. Accept Order\n";
        cout << "3. View My Deliveries\n";
        cout << "4. Update Delivery Status\n";
        cout << "5.  Complete Order\n";
        cout << "6. View History\n";
        cout << "0. Logout\n";
        cout << "\nEnter choice: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            pause();
            continue;
        }

        switch (choice) {
        case 1: {
            delivery.viewAvailableOrders();
            pause();
            break;
        }

        case 2: {
            delivery.viewAvailableOrders();
            int orderID;
            cout << "\nEnter Order ID (0 to cancel): ";
            cin >> orderID;
            if (orderID != 0) {
                delivery.acceptOrder(orderID, riderID);
            }
            pause();
            break;
        }

        case 3: {
            delivery.viewMyDeliveries(riderID);
            pause();
            break;
        }

        case 4: {
            int activeOrders = delivery.viewMyDeliveries(riderID);
            if (activeOrders > 0) {
                int orderID;
                cout << "\nEnter Order ID: ";
                cin >> orderID;

                cout << "\nSelect Status:\n";
                cout << "1. Preparing\n";
                cout << "2. Out for Delivery\n";
                cout << "3. Arrived\n";
                cout << "Choice: ";

                int statusChoice;
                cin >> statusChoice;

                string status = (statusChoice == 1) ? "Preparing" :
                    (statusChoice == 3) ? "Arrived" : "Out for Delivery";

                delivery.updateDeliveryStatus(orderID, status);
            }
            pause();
            break;
        }

        case 5: {
            int activeOrders = delivery.viewMyDeliveries(riderID);
            if (activeOrders > 0) {
                int orderID;
                cout << "\nEnter Order ID to complete: ";
                cin >> orderID;
                delivery.completeDelivery(orderID);
            }
            pause();
            break;
        }

        case 6: {
            delivery.viewDeliveryHistory(riderID);
            pause();
            break;
        }

        case 0: {
            return;
        }

        default: {
            pause();
        }
        }
    }
}

// GANTI MENU DISPLAY dalam ownerMenu() dengan ni:

void ownerMenu(Owner& owner, Menu& menu, Analytics& analytics, Receipt& receipt) {
    int choice;

    while (true) {
        clearScreen();
        cout << BOLD << MAGENTA;
        cout << "   -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-\n";
        cout << "  ||                     OWNER DASHBOARD                         ||\n";
        cout << "   -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-\n";
        cout << RESET;

        cout << "\n" << CYAN << "==== MENU MANAGEMENT ===" << RESET << endl;
        cout << "1. View All Menu Items\n";
        cout << "2. Add Menu Item\n";
        cout << "3. Update Menu Item\n";
        cout << "4. Delete Menu Item\n";

        cout << "\n" << GREEN << "=====STOCK MANAGEMENT ===" << RESET << endl;
        cout << "5. Update Stock\n";
        cout << "6. View Low Stock Alert\n";

        cout << "\n" << YELLOW << "===== ANALYTICS & REPORTS ===" << RESET << endl;
        cout << "7.  Category Sales Table\n";
        cout << "8.  Sales Summary Report\n";
        cout << "9.  Peak Hours Bar Chart\n";
        cout << "10.  Top Selling Items Table\n";

        cout << "\n" << BLUE << "===== RECEIPT MANAGEMENT ===" << RESET << endl;
        cout << "11.  Search Receipts by Customer\n";
        cout << "12.  View All Receipts History\n";
        cout << "13.  View Specific Receipt Details\n";

        cout << "\n" << MAGENTA << "===== OTHER MANAGEMENT ===" << RESET << endl;
        cout << "14. View All Orders\n";
        cout << "15. View All Customers\n";
        cout << "16. View All Riders\n";

        cout << "\n0. Logout\n";
        cout << "\nEnter choice: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            pause();
            continue;
        }

        switch (choice) {
        case 1: {
            owner.viewAllMenuItems();
            pause();
            break;
        }

        case 2: {
            string name, description;
            double price;
            int categoryID, stock;

            owner.viewAllCategories();
            cout << "\n=== ADD MENU ===" << endl;
            cin.ignore();
            cout << "Name: "; getline(cin, name);
            cout << "Price: RM"; cin >> price;
            cout << "Stock: "; cin >> stock;
            cin.ignore();
            cout << "Description: "; getline(cin, description);
            cout << "Category ID: "; cin >> categoryID;

            owner.addMenuItem(name, price, description, categoryID, stock);
            pause();
            break;
        }

        case 3: {
            int menuID, categoryID, stock;
            string name, description;
            double price;

            owner.viewAllMenuItems();
            cout << "\nEnter Menu ID: "; cin >> menuID;
            cin.ignore();
            cout << "Name: "; getline(cin, name);
            cout << "Price: RM"; cin >> price;
            cout << "Stock: "; cin >> stock;
            cin.ignore();
            cout << "Description: "; getline(cin, description);
            owner.viewAllCategories();
            cout << "Category ID: "; cin >> categoryID;

            owner.updateMenuItem(menuID, name, price, description, categoryID, stock);
            pause();
            break;
        }

        case 4: {
            int menuID;
            owner.viewAllMenuItems();
            cout << "\nEnter Menu ID: "; cin >> menuID;
            char confirm;
            cout << "Are you sure? (y/n): "; cin >> confirm;
            if (confirm == 'y' || confirm == 'Y') {
                owner.deleteMenuItem(menuID);
            }
            pause();
            break;
        }

        case 5: {
            int menuID, stock;
            owner.viewAllMenuItems();
            cout << "\nEnter Menu ID: "; cin >> menuID;
            cout << "New Stock: "; cin >> stock;
            owner.updateStock(menuID, stock);
            pause();
            break;
        }

        case 6: {
            owner.viewLowStockItems();
            pause();
            break;
        }

        case 7: {
            analytics.showCategoryPerformance();
            pause();
            break;
        }

        case 8: {
            analytics.showSalesSummary();
            pause();
            break;
        }

        case 9: {
            analytics.showPeakHoursBarChart();
            pause();
            break;
        }

        case 10: {
            analytics.showTopSellingTable();
            pause();
            break;
        }

        case 11: {
            // ? SEARCH BY CUSTOMER NAME
            string customerName;
            cout << "\n" << BOLD << CYAN << "=== SEARCH RECEIPTS BY CUSTOMER ===" << RESET << endl;
            cout << "Enter customer name: ";
            cin.ignore();
            getline(cin, customerName);

            if (!customerName.empty()) {
                receipt.searchReceiptsByCustomer(customerName);
            }
            else {
                cout << RED << "Customer name cannot be empty!" << RESET << endl;
            }
            pause();
            break;
        }

        case 12: {
            receipt.viewAllReceipts();
            pause();
            break;
        }

        case 13: {
            // ? SHOW ALL RECEIPTS FIRST, THEN LET OWNER CHOOSE
            receipt.viewAllReceipts();  // Tunjuk list dulu

            int receiptID;
            cout << "\n" << YELLOW << "Enter Receipt ID to view details (0 to cancel): " << RESET;
            cin >> receiptID;

            if (receiptID != 0) {
                receipt.viewReceiptDetails(receiptID);
            }
            else {
                cout << GREEN << "Cancelled." << RESET << endl;
            }
            pause();
            break;
        }

        case 14: {
            owner.viewAllOrders();
            pause();
            break;
        }

        case 15: {
            owner.viewAllCustomers();
            pause();
            break;
        }

        case 16: {
            owner.viewAllRiders();
            pause();
            break;
        }

        case 0: {
            return;
        }

        default: {
            pause();
        }
        }
    }
}