#include <iostream>
#include <sqlite3.h>


int main() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    const char* db_filename = "chinook.db";
    const char* sql_query = "SELECT artistid, name FROM artists;";

    // Open the database
    if (sqlite3_open(db_filename, &db) != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    // Prepare the SQL statement
    if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }

    // Execute the query and loop through the result set
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        std::cout << "ID: " << id << ", Name: " << name << std::endl;
    }

    // Clean up
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}
