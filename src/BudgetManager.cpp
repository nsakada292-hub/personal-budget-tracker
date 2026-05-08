#include "BudgetManager.hpp"
#include <tabulate/table.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <map>
#include <algorithm>
#include <ctime>
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#endif

using json = nlohmann::json;
using namespace tabulate;

#define RST   "\033[0m"
#define BOLD  "\033[1m"
#define CYN   "\033[36m"
#define GRN   "\033[32m"
#define RED   "\033[31m"
#define YLW   "\033[33m"
#define MGT   "\033[35m"
#define WHT   "\033[37m"

// Center padding helper
static std::string cp(int width = 20) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int termWidth = 120;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        termWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int pad = (termWidth - width) / 2;
    if (pad < 0) pad = 0;
    return std::string(pad, ' ');
#else
    return std::string(20, ' ');
#endif
}

// Print a tabulate table with padding
static void printTable(Table& table, const std::string& pad) {
    std::ostringstream oss;
    oss << table;
    std::istringstream lines(oss.str());
    std::string line;
    while (std::getline(lines, line))
        std::cout << pad << line << "\n";
}

BudgetManager::BudgetManager(bool readOnly) : readOnly(readOnly) {}

std::chrono::system_clock::time_point BudgetManager::parseDate(const std::string& dateStr) {
    std::tm tm = {};
    std::stringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (!ss.fail())
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    return std::chrono::system_clock::now();
}

// =============================================
//  Add transaction
// =============================================
void BudgetManager::addTransaction() {
    if (readOnly) {
        std::cout << "\n  " << RED << BOLD << "⚠️  Guest mode is read-only." << RST << "\n";
        return;
    }

    Transaction t;
    std::string pad = cp(40);
    std::cout << "\n" << pad << CYN << BOLD << "--- Add New Transaction ---" << RST << "\n";

    int typeChoice;
    std::cout << pad << "Type:\n" << pad << "  1. Income\n" << pad << "  2. Expense\n" << pad << "Choose (1/2): ";
    std::cin >> typeChoice;
    std::cin.ignore();
    t.type = (typeChoice == 1) ? TransactionType::INCOME : TransactionType::EXPENSE;

    std::cout << pad << "Category: ";
    std::getline(std::cin, t.category);
    if (t.category.empty()) t.category = "general";

    std::cout << pad << "Amount ($): ";
    while (!(std::cin >> t.amount) || t.amount <= 0) {
        std::cout << pad << RED << "⚠️  Enter a valid positive amount: " << RST;
        std::cin.clear(); std::cin.ignore(256, '\n');
    }
    std::cin.ignore();

    std::cout << pad << "Date (YYYY-MM-DD): ";
    std::string dateStr;
    std::getline(std::cin, dateStr);
    std::tm tm = {};
    std::stringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (!ss.fail()) {
        t.date = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    } else {
        t.date = std::chrono::system_clock::now();
        std::cout << pad << RED << "⚠️  Invalid date. Using today." << RST << "\n";
    }

    std::cout << pad << "Note (Enter to skip): ";
    std::getline(std::cin, t.note);
    if (t.note.empty()) t.note = "-";

    t.id = Transaction::generateId();
    transactions.push_back(t);
    std::cout << "\n" << pad << GRN << BOLD << "[+] Transaction added! (ID: " << t.id << ")" << RST << "\n";
    checkWarnings();
}

// =============================================
//  View transactions using tabulate
// =============================================
void BudgetManager::viewTransactions(const std::string& filterMonth,
                                      const std::string& filterCategory,
                                      const std::string& filterType) {
    if (transactions.empty()) {
        std::cout << "\n  " << RED << BOLD << "⚠️  No transactions found." << RST << "\n";
        return;
    }

    std::vector<Transaction> filtered;
    for (const auto& t : transactions) {
        bool ok = true;
        if (!filterMonth.empty()    && t.getMonthString() != filterMonth) ok = false;
        if (!filterType.empty()     && t.getTypeString()  != filterType)  ok = false;
        if (!filterCategory.empty()) {
            std::string tCat = t.category, fCat = filterCategory;
            std::transform(tCat.begin(), tCat.end(), tCat.begin(), ::tolower);
            std::transform(fCat.begin(), fCat.end(), fCat.begin(), ::tolower);
            if (tCat.find(fCat) == std::string::npos) ok = false;
        }
        if (ok) filtered.push_back(t);
    }

    if (filtered.empty()) {
        std::cout << "\n  " << RED << BOLD << "⚠️  No transactions match filter." << RST << "\n";
        return;
    }

    std::string pad = cp(80);

    // Column widths
    int w0=4, w1=10, w2=14, w3=11, w4=12, w5=14;

    // Pad string to exact width
    auto pw = [](std::string s, int w) -> std::string {
        if ((int)s.size() >= w) return s.substr(0, w);
        return s + std::string(w - s.size(), ' ');
    };

    // Border line
    auto borderLine = [&]() {
        std::cout << pad << CYN << BOLD
            << "+" << std::string(w0+2,'-')
            << "+" << std::string(w1+2,'-')
            << "+" << std::string(w2+2,'-')
            << "+" << std::string(w3+2,'-')
            << "+" << std::string(w4+2,'-')
            << "+" << std::string(w5+2,'-')
            << "+" << RST << "\n";
    };

    std::cout << "\n";
    borderLine();

    // Header
    std::cout << pad
        << CYN << BOLD << "| " << pw("ID",w0)       << " | " << pw("Type",w1)
        <<                  " | " << pw("Category",w2) << " | " << pw("Amount",w3)
        <<                  " | " << pw("Date",w4)     << " | " << pw("Note",w5)
        << " |" << RST << "\n";
    borderLine();

    // Data rows
    for (const auto& t : filtered) {
        std::ostringstream amt;
        amt << "$" << std::fixed << std::setprecision(2) << t.amount;
        std::string typeCol = (t.type == TransactionType::INCOME) ? GRN : RED;

        std::cout << pad
            << CYN  << BOLD << "| " << RST
            << WHT  << BOLD << pw(std::to_string(t.id), w0) << RST
            << CYN  << BOLD << " | " << RST
            << typeCol << BOLD << pw(t.getTypeString(), w1)  << RST
            << CYN  << BOLD << " | " << RST
            << WHT  << BOLD << pw(t.category, w2)           << RST
            << CYN  << BOLD << " | " << RST
            << YLW  << BOLD << pw(amt.str(), w3)            << RST
            << CYN  << BOLD << " | " << RST
            << CYN  << BOLD << pw(t.getDateString(), w4)    << RST
            << CYN  << BOLD << " | " << RST
            << WHT  << BOLD << pw(t.note, w5)               << RST
            << CYN  << BOLD << " |"                         << RST << "\n";
        borderLine();
    }

    std::cout << pad << CYN << "Showing: " << filtered.size() << " / " << transactions.size() << " record(s)" << RST << "\n";
}

// =============================================
//  Filter menu
// =============================================
void BudgetManager::filterMenu() {
    std::string pad = cp(40);
    std::cout << "\n" << pad << CYN << BOLD << "--- Filter Transactions ---" << RST << "\n";
    std::cout << pad << "1. Filter by Month\n";
    std::cout << pad << "2. Filter by Category\n";
    std::cout << pad << "3. Filter by Type (income/expense)\n";
    std::cout << pad << "4. Show All\n";
    std::cout << pad << "Choose: ";

    int choice; std::cin >> choice; std::cin.ignore();
    if (choice == 1) {
        std::cout << pad << "Enter month (YYYY-MM): ";
        std::string m; std::getline(std::cin, m);
        viewTransactions(m, "", "");
    } else if (choice == 2) {
        std::cout << pad << "Enter category: ";
        std::string c; std::getline(std::cin, c);
        viewTransactions("", c, "");
    } else if (choice == 3) {
        std::cout << pad << "Enter type (income/expense): ";
        std::string tp; std::getline(std::cin, tp);
        viewTransactions("", "", tp);
    } else {
        viewTransactions();
    }
}

// =============================================
//  Monthly summary
// =============================================
void BudgetManager::monthlySummary() {
    if (transactions.empty()) {
        std::cout << "\n  " << RED << BOLD << "⚠️  No transactions to summarize." << RST << "\n";
        return;
    }

    std::map<std::string, double> incomeMap, expenseMap;
    for (const auto& t : transactions) {
        if (t.type == TransactionType::INCOME) incomeMap[t.getMonthString()] += t.amount;
        else expenseMap[t.getMonthString()] += t.amount;
    }

    std::map<std::string, bool> allMonths;
    for (auto& p : incomeMap)  allMonths[p.first] = true;
    for (auto& p : expenseMap) allMonths[p.first] = true;

    std::string pad = cp(70);
    int w0=10, w1=12, w2=12, w3=12, w4=12;

    auto pw = [](std::string s, int w) -> std::string {
        if ((int)s.size() >= w) return s.substr(0, w);
        return s + std::string(w - s.size(), ' ');
    };
    auto borderLine = [&]() {
        std::cout << pad << CYN << BOLD
            << "+" << std::string(w0+2,'-')
            << "+" << std::string(w1+2,'-')
            << "+" << std::string(w2+2,'-')
            << "+" << std::string(w3+2,'-')
            << "+" << std::string(w4+2,'-')
            << "+" << RST << "\n";
    };

    double grandIncome = 0, grandExpense = 0;
    // collect rows first
    struct MRow { std::string month, inc, exp, bal, status; bool over; };
    std::vector<MRow> rows;
    for (auto& p : allMonths) {
        std::string m = p.first;
        double inc = incomeMap.count(m) ? incomeMap[m] : 0;
        double exp = expenseMap.count(m) ? expenseMap[m] : 0;
        double bal = inc - exp;
        grandIncome += inc; grandExpense += exp;
        std::ostringstream iS, eS, bS;
        iS << "$" << std::fixed << std::setprecision(2) << inc;
        eS << "$" << std::fixed << std::setprecision(2) << exp;
        bS << "$" << std::fixed << std::setprecision(2) << bal;
        rows.push_back({m, iS.str(), eS.str(), bS.str(), (bal<0)?"Over budget!":"OK", bal<0});
    }

    std::cout << "\n" << pad << CYN << BOLD << "MONTHLY SUMMARY" << RST << "\n";
    borderLine();
    // Header
    std::cout << pad << CYN << BOLD
        << "| " << pw("Month",w0)   << " | " << pw("Income",w1)
        << " | " << pw("Expense",w2) << " | " << pw("Balance",w3)
        << " | " << pw("Status",w4)  << " |" << RST << "\n";
    borderLine();
    // Data rows
    for (auto& r : rows) {
        std::string bc = r.over ? RED : GRN;
        std::cout << pad
            << CYN << BOLD << "| " << RST << WHT  << BOLD << pw(r.month, w0) << RST
            << CYN << BOLD << " | " << RST << GRN  << BOLD << pw(r.inc,   w1) << RST
            << CYN << BOLD << " | " << RST << RED  << BOLD << pw(r.exp,   w2) << RST
            << CYN << BOLD << " | " << RST << bc   << BOLD << pw(r.bal,   w3) << RST
            << CYN << BOLD << " | " << RST << bc   << BOLD << pw(r.status,w4) << RST
            << CYN << BOLD << " |" << RST << "\n";
        borderLine();
    }
    // Total row
    std::ostringstream tI, tE, tB;
    tI << "$" << std::fixed << std::setprecision(2) << grandIncome;
    tE << "$" << std::fixed << std::setprecision(2) << grandExpense;
    tB << "$" << std::fixed << std::setprecision(2) << (grandIncome - grandExpense);
    std::cout << pad
        << CYN << BOLD << "| " << RST << YLW << BOLD << pw("TOTAL", w0) << RST
        << CYN << BOLD << " | " << RST << YLW << BOLD << pw(tI.str(), w1) << RST
        << CYN << BOLD << " | " << RST << YLW << BOLD << pw(tE.str(), w2) << RST
        << CYN << BOLD << " | " << RST << YLW << BOLD << pw(tB.str(), w3) << RST
        << CYN << BOLD << " | " << RST << YLW << BOLD << pw("", w4)       << RST
        << CYN << BOLD << " |" << RST << "\n";
    borderLine();
}

// =============================================
//  Category breakdown
// =============================================
void BudgetManager::categoryBreakdown() {
    if (transactions.empty()) {
        std::cout << "\n  " << RED << BOLD << "⚠️  No transactions found." << RST << "\n";
        return;
    }

    std::map<std::string, double> catMap;
    double totalExpense = 0;

    for (const auto& t : transactions) {
        if (t.type == TransactionType::EXPENSE) {
            std::string cat = t.category;
            std::transform(cat.begin(), cat.end(), cat.begin(), ::tolower);
            catMap[cat] += t.amount;
            totalExpense += t.amount;
        }
    }

    if (totalExpense == 0) { std::cout << "\n  " << RED << "⚠️  No expenses found." << RST << "\n"; return; }

    std::vector<std::pair<std::string, double>> sorted(catMap.begin(), catMap.end());
    std::sort(sorted.begin(), sorted.end(),
        [](const std::pair<std::string,double>& a, const std::pair<std::string,double>& b) {
            return a.second > b.second;
        });

    std::string pad = cp(70);
    int w0=14, w1=12, w2=12, w3=22;

    auto pw = [](std::string s, int w) -> std::string {
        if ((int)s.size() >= w) return s.substr(0, w);
        return s + std::string(w - s.size(), ' ');
    };
    auto borderLine = [&]() {
        std::cout << pad << CYN << BOLD
            << "+" << std::string(w0+2,'-')
            << "+" << std::string(w1+2,'-')
            << "+" << std::string(w2+2,'-')
            << "+" << std::string(w3+2,'-')
            << "+" << RST << "\n";
    };

    std::cout << "\n" << pad << CYN << BOLD << "CATEGORY BREAKDOWN" << RST << "\n";
    borderLine();
    // Header
    std::cout << pad << CYN << BOLD
        << "| " << pw("Category",w0)   << " | " << pw("Amount",w1)
        << " | " << pw("Percentage",w2) << " | " << pw("Bar",w3)
        << " |" << RST << "\n";
    borderLine();
    // Data rows
    for (auto& p : sorted) {
        double pct = (p.second / totalExpense) * 100.0;
        int bars = (int)(pct / 5);
        std::string bar = "";
        for (int i = 0; i < bars; i++) bar += "#";
        for (int i = bars; i < 20; i++) bar += "-";

        std::ostringstream aS, pS;
        aS << "$" << std::fixed << std::setprecision(2) << p.second;
        pS << std::fixed << std::setprecision(1) << pct << "%";

        std::cout << pad
            << CYN << BOLD << "| " << RST << WHT << BOLD << pw(p.first,  w0) << RST
            << CYN << BOLD << " | " << RST << YLW << BOLD << pw(aS.str(), w1) << RST
            << CYN << BOLD << " | " << RST << MGT << BOLD << pw(pS.str(), w2) << RST
            << CYN << BOLD << " | " << RST << GRN << BOLD << pw(bar,      w3) << RST
            << CYN << BOLD << " |" << RST << "\n";
        borderLine();
    }
    // Total row
    std::ostringstream tS;
    tS << "$" << std::fixed << std::setprecision(2) << totalExpense;
    std::cout << pad
        << CYN << BOLD << "| " << RST << YLW << BOLD << pw("TOTAL",   w0) << RST
        << CYN << BOLD << " | " << RST << YLW << BOLD << pw(tS.str(), w1) << RST
        << CYN << BOLD << " | " << RST << YLW << BOLD << pw("100%",   w2) << RST
        << CYN << BOLD << " | " << RST << YLW << BOLD << pw("",       w3) << RST
        << CYN << BOLD << " |" << RST << "\n";
    borderLine();
}

// =============================================
//  Warning banner
// =============================================
void BudgetManager::checkWarnings() {
    std::map<std::string, double> incomeMap, expenseMap;
    for (const auto& t : transactions) {
        if (t.type == TransactionType::INCOME) incomeMap[t.getMonthString()] += t.amount;
        else expenseMap[t.getMonthString()] += t.amount;
    }

    std::string wpad = cp(40);
    bool warned = false;
    for (auto& p : expenseMap) {
        double inc = incomeMap.count(p.first) ? incomeMap[p.first] : 0;
        if (p.second > inc) {
            if (!warned) { std::cout << "\n" << wpad << RED << BOLD << "⚠️  WARNING " << RST << "\n"; warned = true; }
            std::cout << wpad << RED << " " << p.first
                      << ": Expenses ($" << std::fixed << std::setprecision(2) << p.second
                      << ") exceed Income ($" << inc << ")!" << RST << "\n";
        }
    }
    if (!warned && !transactions.empty())
        std::cout << "\n" << wpad << GRN << BOLD << "✅  Budget is on track!" << RST << "\n";
}

// =============================================
//  Edit transaction
// =============================================
void BudgetManager::editTransaction() {
    if (readOnly) { std::cout << "\n  " << RED << "⚠️ Guest mode is read-only." << RST << "\n"; return; }
    if (transactions.empty()) { std::cout << "\n  " << RED << "⚠️ No transactions to edit." << RST << "\n"; return; }

    viewTransactions();
    std::string epad = cp(40);
    std::cout << "\n" << epad << "Enter ID to edit: ";
    int id; std::cin >> id; std::cin.ignore();

    auto it = std::find_if(transactions.begin(), transactions.end(),
        [id](const Transaction& t) { return t.id == id; });
    if (it == transactions.end()) { std::cout << "  " << RED << "⚠️ ID not found." << RST << "\n"; return; }

    std::cout << "\n" << epad << CYN << "--- Edit Transaction (ID: " << id << ") ---" << RST << "\n";
    std::cout << epad << "Leave blank to keep current value.\n\n";

    std::string input;
    std::cout << epad << "Category [" << it->category << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) it->category = input;

    std::cout << epad << "Amount [$" << it->amount << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) { try { it->amount = std::stod(input); } catch (...) {} }

    std::cout << epad << "Date [" << it->getDateString() << "] (YYYY-MM-DD): ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        std::tm tm = {};
        std::stringstream ss(input);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        if (!ss.fail())
            it->date = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    }

    std::cout << epad << "Note [" << it->note << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) it->note = input;

    std::cout << "\n" << epad << GRN << BOLD << "[+] Transaction " << id << " updated!" << RST << "\n";
}

// =============================================
//  Delete transaction
// =============================================
void BudgetManager::deleteTransaction() {
    if (readOnly) { std::cout << "\n  " << RED << "[!] Guest mode is read-only." << RST << "\n"; return; }
    if (transactions.empty()) { std::cout << "\n  " << RED << "[!] No transactions to delete." << RST << "\n"; return; }

    viewTransactions();
    std::string dpad = cp(40);
    std::cout << "\n" << dpad << "Enter ID to delete: ";
    int id; std::cin >> id;

    auto it = std::find_if(transactions.begin(), transactions.end(),
        [id](const Transaction& t) { return t.id == id; });
    if (it == transactions.end()) { std::cout << "  " << RED << "[!] ID not found." << RST << "\n"; return; }
    transactions.erase(it);
    std::cout << dpad << GRN << BOLD << "[+] Transaction " << id << " deleted." << RST << "\n";
}

// =============================================
//  Sample data for guest
// =============================================
void BudgetManager::loadSampleData() {
    transactions.clear();
    auto makeDate = [](const std::string& s) {
        std::tm tm = {};
        std::stringstream ss(s);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    };
    transactions.push_back({1, TransactionType::INCOME,  "salary",    3000.0, makeDate("2026-03-01"), "March salary"});
    transactions.push_back({2, TransactionType::EXPENSE, "food",       500.0, makeDate("2026-03-05"), "groceries"});
    transactions.push_back({3, TransactionType::EXPENSE, "rent",       800.0, makeDate("2026-03-01"), "monthly rent"});
    transactions.push_back({4, TransactionType::EXPENSE, "transport",  150.0, makeDate("2026-03-10"), "bus pass"});
    transactions.push_back({5, TransactionType::INCOME,  "freelance",  500.0, makeDate("2026-03-15"), "web project"});
    transactions.push_back({6, TransactionType::EXPENSE, "food",       200.0, makeDate("2026-03-20"), "dining out"});
    std::cout << "\n  " << GRN << BOLD << "[+] Sample data loaded!" << RST << "\n";
}

// =============================================
//  Save
// =============================================
void BudgetManager::save(const User& user) {
    if (user.isGuest) { std::cout << "  " << RED << "[!] Guest data is not saved." << RST << "\n"; return; }

    std::filesystem::create_directories("Data");
    std::string filename = "Data/budget.json";

    json root;
    std::ifstream inFile(filename);
    if (inFile.is_open()) {
        try { root = json::parse(inFile); } catch (...) { root = json::object(); }
        inFile.close();
    }
    if (!root.is_object()) root = json::object();

    json txArray = json::array();
    for (const auto& t : transactions) txArray.push_back(t.toJson());
    root[user.username]["transactions"] = txArray;

    std::ofstream outFile(filename);
    if (!outFile) { std::cout << "  " << RED << "[!] Could not save." << RST << "\n"; return; }
    outFile << root.dump(2);
    outFile.close();
    std::cout << "  " << GRN << BOLD << "📥  Saved! (user: " << user.username << ")" << RST << "\n";
}

// =============================================
//  Load
// =============================================
void BudgetManager::load(const User& user) {
    if (user.isGuest) { loadSampleData(); return; }

    std::string filename = "Data/budget.json";
    std::ifstream file(filename);
    if (!file) {
        std::cout << "  " << RED << "[!] No saved data. Starting fresh." << RST << "\n";
        return;
    }

    try {
        json root = json::parse(file);
        if (!root.contains(user.username)) {
            std::cout << "  " << RED << "[!] No data for '" << user.username << "'. Starting fresh." << RST << "\n";
            return;
        }
        transactions.clear();
        for (auto& j : root[user.username]["transactions"])
            transactions.push_back(Transaction::fromJson(j));
        std::cout << "  " << GRN << BOLD << "[+] Loaded " << transactions.size()
                  << " transaction(s) for '" << user.username << "'." << RST << "\n";
    } catch (...) {
        std::cout << "  " << RED << "[!] Error reading Data/budget.json" << RST << "\n";
    }
    file.close();
}