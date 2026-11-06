#include <cassert>
#include <cstdio>
#include <iostream>
#include "sqliteDatabase.h"

namespace {

const char* kTestFile = "databaseTest.tmp";

int countRows(const std::string& filePath) {
    sqlite3* db = nullptr;
    sqlite3_open(filePath.c_str(), &db);
    sqlite3_stmt* statement = nullptr;
    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM cache", -1, &statement, nullptr);
    sqlite3_step(statement);
    const int count = sqlite3_column_int(statement, 0);
    sqlite3_finalize(statement);
    sqlite3_close(db);
    return count;
}

};

int main() {
    // Saved data can be read back
    {
        SqliteDatabase database(":memory:");
        assert(database.saveCache("key", "{\"pm2_5\": 4.2}"));
        assert(database.loadCache("key") == "{\"pm2_5\": 4.2}");
    }
    // An unknown key gives an empty string
    {
        SqliteDatabase database(":memory:");
        assert(database.loadCache("unknown").empty());
    }
    // Saving the same key again replaces the old data
    {
        SqliteDatabase database(":memory:");
        database.saveCache("key", "old");
        database.saveCache("key", "new");
        assert(database.loadCache("key") == "new");
    }
    // Quotes in the data do not break the SQL
    {
        SqliteDatabase database(":memory:");
        const std::string tricky = "{\"name\": \"it's'); DROP TABLE cache;--\"}";
        assert(database.saveCache("it's", tricky));
        assert(database.loadCache("it's") == tricky);
    }
    // Expired data is not returned
    {
        SqliteDatabase database(":memory:", 0);
        database.saveCache("key", "data");
        assert(database.loadCache("key").empty());
    }
    // Data is still there after reopening the file
    std::remove(kTestFile);
    {
        SqliteDatabase database(kTestFile);
        database.saveCache("key", "data");
    }
    {
        SqliteDatabase database(kTestFile);
        assert(database.loadCache("key") == "data");
    }
    // Cleaning removes expired rows and keeps fresh ones
    {
        SqliteDatabase expiring(kTestFile, 0);
        expiring.saveCache("expired", "data");
        assert(countRows(kTestFile) == 2);
        expiring.clearExpiredCache();
        assert(countRows(kTestFile) == 1);
    }
    std::remove(kTestFile);
    // A database that cannot be opened does not crash the app
    {
        SqliteDatabase database("/no/such/folder/cache.db");
        assert(!database.saveCache("key", "data"));
        assert(database.loadCache("key").empty());
        database.clearExpiredCache();
    }

    std::cout << "All database tests passed" << std::endl;
    return 0;
}