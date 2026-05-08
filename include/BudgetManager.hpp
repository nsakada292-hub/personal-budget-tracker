#pragma once

#include "Transaction.hpp"
#include "AuthManager.hpp"
#include <vector>
#include <string>

class BudgetManager {
private:
    std::vector<Transaction> transactions; // std::vector as required
    bool readOnly;
    std::chrono::system_clock::time_point parseDate(const std::string& dateStr);

public:
    BudgetManager(bool readOnly = false);

    void addTransaction();
    void viewTransactions(const std::string& filterMonth = "",
                          const std::string& filterCategory = "",
                          const std::string& filterType = "");
    void monthlySummary();
    void categoryBreakdown();
    void editTransaction();
    void deleteTransaction();
    void checkWarnings();
    void filterMenu();
    void save(const User& user);    // saves to Data/budget.json
    void load(const User& user);    // loads from Data/budget.json
    void loadSampleData();
};

