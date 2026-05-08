#include "Transaction.hpp"

Transaction::Transaction(int id, TransactionType type, const std::string& category,
                         double amount, const std::chrono::system_clock::time_point& date,
                         const std::string& note)
    : id(id), type(type), category(category), amount(amount), date(date), note(note) {}

std::string Transaction::getDateString() const {
    auto time = std::chrono::system_clock::to_time_t(date);
    std::tm tm = *std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string Transaction::getMonthString() const {
    return getDateString().substr(0, 7);
}

std::string Transaction::getTypeString() const {
    return type == TransactionType::INCOME ? "income" : "expense";
}

// Convert Transaction -> nlohmann JSON
json Transaction::toJson() const {
    return json{
        {"id",       id},
        {"type",     getTypeString()},
        {"category", category},
        {"amount",   amount},
        {"date",     getDateString()},
        {"note",     note}
    };
}

// Convert nlohmann JSON -> Transaction
Transaction Transaction::fromJson(const json& j) {
    Transaction t;
    t.id       = j.at("id").get<int>();
    t.category = j.at("category").get<std::string>();
    t.amount   = j.at("amount").get<double>();
    t.note     = j.value("note", "-");

    std::string typeStr = j.at("type").get<std::string>();
    t.type = (typeStr == "income") ? TransactionType::INCOME : TransactionType::EXPENSE;

    // Parse date string "YYYY-MM-DD"
    std::string dateStr = j.at("date").get<std::string>();
    std::tm tm = {};
    std::stringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (!ss.fail())
        t.date = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    return t;
}

int Transaction::generateId() {
    static int counter = 0;
    return ++counter;
}