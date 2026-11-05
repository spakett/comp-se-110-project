#include "sqliteDatabase.h"

#include <iostream>

SqliteDatabase::SqliteDatabase(const std::string& filePath, int ttlSeconds) : m_ttlSeconds(ttlSeconds) {
    const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
    if (sqlite3_open_v2(filePath.c_str(), &m_db, flags, nullptr) != SQLITE_OK) {
        std::cerr << "Could not open cache database: " << sqlite3_errmsg(m_db) << std::endl;
        sqlite3_close(m_db);
        m_db = nullptr;
        return;
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS cache ("
        "endpoint_key TEXT PRIMARY KEY, "
        "json_payload TEXT NOT NULL, "
        "expires_at INTEGER NOT NULL)";
    if (sqlite3_exec(m_db, sql, nullptr, nullptr, nullptr) != SQLITE_OK) {
        std::cerr << "Could not create cache table: " << sqlite3_errmsg(m_db) << std::endl;
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

SqliteDatabase::~SqliteDatabase() {
    sqlite3_close(m_db);
}

bool SqliteDatabase::saveCache(const std::string& endpointKey, const std::string& jsonPayload) {
    if (!m_db)
        return false;

    const char* sql =
        "INSERT OR REPLACE INTO cache (endpoint_key, json_payload, expires_at) "
        "VALUES (?, ?, CAST(strftime('%s', 'now') AS INTEGER) + ?)";
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &statement, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(statement, 1, endpointKey.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, jsonPayload.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 3, m_ttlSeconds);

    const bool saved = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    return saved;
}

std::string SqliteDatabase::loadCache(const std::string& endpointKey) {
    if (!m_db)
        return "";

    const char* sql =
        "SELECT json_payload FROM cache "
        "WHERE endpoint_key = ? AND expires_at > CAST(strftime('%s', 'now') AS INTEGER)";
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &statement, nullptr) != SQLITE_OK)
        return "";

    sqlite3_bind_text(statement, 1, endpointKey.c_str(), -1, SQLITE_TRANSIENT);

    std::string payload;
    if (sqlite3_step(statement) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(statement, 0);
        if (text)
            payload = reinterpret_cast<const char*>(text);
    }
    sqlite3_finalize(statement);
    return payload;
}

void SqliteDatabase::clearExpiredCache() {
    if (!m_db)
        return;

    const char* sql = "DELETE FROM cache WHERE expires_at <= CAST(strftime('%s', 'now') AS INTEGER)";
    sqlite3_exec(m_db, sql, nullptr, nullptr, nullptr);
}