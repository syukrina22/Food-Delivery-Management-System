#ifndef DATABASE_H
#define DATABASE_H

#include <memory>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/connection.h>

// Global database connection
extern sql::Driver* driver;
extern std::unique_ptr<sql::Connection> conn;

// Database functions
bool connectDatabase();
void closeDatabase();

#endif
