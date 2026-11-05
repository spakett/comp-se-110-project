#include <string>
#include <optional>
#pragma once

// Core uses this to command SQLite to manage caching
class IDatabasePort {
public:
    virtual ~IDatabasePort() = default;

    virtual bool saveCache(const std::string& endpointKey, const std::string& jsonPayload) = 0;
    virtual std::string loadCache(const std::string& endpointKey) = 0;
    virtual void clearExpiredCache() = 0;
};