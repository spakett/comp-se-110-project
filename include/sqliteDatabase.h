#pragma once
#include <sqlite3.h>
#include <string>
#include "IDatabasePort.h"

// Caches raw API responses in a SQLite database
class SqliteDatabase : public IDatabasePort {
    public:
        explicit SqliteDatabase(const std::string& filePath, int ttlSeconds = 3600);
        ~SqliteDatabase() override;

        SqliteDatabase(const SqliteDatabase&) = delete;
        SqliteDatabase& operator=(const SqliteDatabase&) = delete;

        bool saveCache(const std::string& endpointKey, const std::string&jsonPayload) override;
        std::string loadCache(const std::string& endpointKey) override;
        void clearExpiredCache() override;

    private:
        sqlite3* m_db = nullptr;
        int m_ttlSeconds;

};