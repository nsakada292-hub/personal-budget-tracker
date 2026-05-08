#pragma once

#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class TransactionType {
    INCOME,
    EXPENSE
};

struct Transaction {
    int id;
    TransactionType type;
    std::string category;
    double amount;
    std::chrono::system_clock::time_point date;
    std::string note;

    // Default constructor
    Transaction() : id(0), type(TransactionType::EXPENSE), amount(0.0) {
        date = std::chrono::system_clock::now();
    }

    // Full constructor
    Transaction(int id, TransactionType type, const std::string& category,
                double amount, const std::chrono::system_clock::time_point& date,
                const std::string& note);

    std::string getDateString() const;
    std::string getMonthString() const;
    std::string getTypeString() const;

    // Convert to/from nlohmann JSON
    json toJson() const;
    static Transaction fromJson(const json& j);
    static int generateId();
};

