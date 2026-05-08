#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#endif
#include <cxxopts.hpp>
#include <tabulate/table.hpp>
#include "AuthManager.hpp"
#include "BudgetManager.hpp"

#ifdef _WIN32
#include <conio.h>
#endif

using namespace tabulate;

#define RESET   "\033[0m"
#define GREEN   "\033[92m"
#define CYAN    "\033[96m"
#define YELLOW  "\033[93m"
#define MAGENTA "\033[95m"
#define RED     "\033[91m"
#define BOLD    "\033[1m"
void enableANSI() {
#ifdef _WIN32
    system("chcp 65001 > nul");
    system("");
#endif
}

// Get terminal width and return padding to center text
std::string centerPad(int textWidth) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int termWidth = 120; // default
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        termWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int pad = (termWidth - textWidth) / 2;
    if (pad < 0) pad = 0;
    return std::string(pad, ' ');
#else
    return std::string(20, ' ');
#endif
}

// =============================================
//  HIDDEN PASSWORD INPUT
// =============================================
std::string getPassword() {
    std::string password = "";
    char ch;
#ifdef _WIN32
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        } else {
            password += ch;
            std::cout << "*";
        }
    }
#else
    while ((ch = getchar()) != '\n') {
        if (ch == 127 || ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        } else {
            password += ch;
            std::cout << "*";
        }
    }
#endif
    std::cout << "\n";
    return password;
}

// =============================================
//  SCHOOL BANNER - clean and beautiful
// =============================================
void printSchoolBanner() {
    std::cout << "\n";
    std::cout << CYAN << BOLD;
    std::cout << "  +===================================================================+\n";
    std::cout << "  |                                                                   |\n";
    std::cout << "  |                                                                   |\n";
    std::cout << "  |                                                                   |\n";
    std::cout << "  |                                                                   |\n";
    std::cout << "  |                                                                   |\n";
    std::cout << "  |                                                                   |\n";
    std::cout << "  |                                                                   |\n";
    std::cout << "  |                                                                   |\n";
    std::cout << "  |                                                                   |\n";
    std::cout << "  |  \033[43m\033[30m" << BOLD;
    std::cout << "  PRE-UNIVERSITY SCHOLARS (6TH)  |  C++ FINAL PROJECT  |  2026" <<"   |"; 
    std::cout << RESET << CYAN << BOLD <<" \n";
    std::cout << "  |                                                                   |\n";
    std::cout << "  +===================================================================+\n";
    std::cout << RESET;

    // Print ISTAD art OVER the empty lines using cursor up
    // Move cursor up 11 lines then print art
    std::cout << "\033[11A"; // move up 11 lines
    std::cout << "\033[6C";  // move right 6 columns (past the "  |   ")

    std::cout << CYAN << BOLD;
    std::cout << "  ██╗███████╗████████╗ █████╗ ██████╗ \n";
    std::cout << "\033[6C";
    std::cout << "  ██║██╔════╝╚══██╔══╝██╔══██╗██╔══██╗\n";
    std::cout << "\033[6C";
    std::cout << "  ██║███████╗   ██║   ███████║██║  ██║\n";
    std::cout << "\033[6C";
    std::cout << "  ██║╚════██║   ██║   ██╔══██║██║  ██║\n";
    std::cout << "\033[6C";
    std::cout << "  ██║███████║   ██║   ██║  ██║██████╔╝\n";
    std::cout << "\033[6C";
    std::cout << "  ╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═════╝ \n";
    std::cout << RESET;

    // Move cursor down to end of box
    std::cout << "\033[6B\n";
}
// =============================================
//  BIG ASCII ART - "BUDGET"
// =============================================
void printBigBudget() {
    std::string pad = centerPad(52);
    std::cout << MAGENTA << BOLD << "\n";
    std::cout << pad << "██████╗ ██╗   ██╗██████╗  ██████╗ ███████╗████████╗\n";
    std::cout << pad << "██╔══██╗██║   ██║██╔══██╗██╔════╝ ██╔════╝╚══██╔══╝\n";
    std::cout << pad << "██████╔╝██║   ██║██║  ██║██║  ███╗█████╗     ██║   \n";
    std::cout << pad << "██╔══██╗██║   ██║██║  ██║██║   ██║██╔══╝     ██║   \n";
    std::cout << pad << "██████╔╝╚██████╔╝██████╔╝╚██████╔╝███████╗   ██║   \n";
    std::cout << pad << "╚═════╝  ╚═════╝ ╚═════╝  ╚═════╝ ╚══════╝   ╚═╝   \n";
    std::cout << RESET;
}

// =============================================
//  BIG ASCII ART - "MANAGER"
// =============================================
void printBigManager() {
    std::string pad = centerPad(67);
    std::cout << GREEN << BOLD << "\n";
    std::cout << pad << "████████╗██████╗  █████╗  ██████╗██╗  ██╗███████╗██████╗ \n";
    std::cout << pad << "╚══██╔══╝██╔══██╗██╔══██╗██╔════╝██║ ██╔╝██╔════╝██╔══██╗\n";
    std::cout << pad << "   ██║   ██████╔╝███████║██║     █████╔╝ █████╗  ██████╔╝\n";
    std::cout << pad << "   ██║   ██╔══██╗██╔══██║██║     ██╔═██╗ ██╔══╝  ██╔══██╗\n";
    std::cout << pad << "   ██║   ██║  ██║██║  ██║╚██████╗██║  ██╗███████╗██║  ██║\n";
    std::cout << pad << "   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝\n";
    std::cout << RESET;
}

// =============================================
//  BIG ASCII ART - "DASHBOARD"
// =============================================
void printBigDashboard() {
    std::string pad = centerPad(76);
    std::cout << MAGENTA << BOLD << "\n";
    std::cout << pad << "██████╗  █████╗ ███████╗██╗  ██╗██████╗  ██████╗  █████╗ ██████╗ ██████╗ \n";
    std::cout << pad << "██╔══██╗██╔══██╗██╔════╝██║  ██║██╔══██╗██╔═══██╗██╔══██╗██╔══██╗██╔══██╗\n";
    std::cout << pad << "██║  ██║███████║███████╗███████║██████╔╝██║   ██║███████║██████╔╝██║  ██║\n";
    std::cout << pad << "██║  ██║██╔══██║╚════██║██╔══██║██╔══██╗██║   ██║██╔══██║██╔══██╗██║  ██║\n";
    std::cout << pad << "██████╔╝██║  ██║███████║██║  ██║██████╔╝╚██████╔╝██║  ██║██║  ██║██████╔╝\n";
    std::cout << pad << "╚═════╝ ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝\n";
    std::cout << RESET;
}

// =============================================
//  LOADING BAR
// =============================================
void loadingBar(const std::string& label) {
    std::string pad = centerPad(56);
    std::cout << "\n" << pad << GREEN << BOLD << label << RESET << "\n";
    std::cout << pad << "[";
    for (int i = 0; i <= 50; i++) {
        std::cout << "\033[42m " << RESET;
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::cout << "] " << GREEN << BOLD << "100%  Done!" << RESET << "\n\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
}

// =============================================
//  LOGIN MENU
// =============================================
void printLoginMenu() {
    std::string pad = centerPad(52);
    std::cout << GREEN << BOLD;
    std::cout << "\n" << pad << "####################################################\n";
    std::cout << pad << "#                                                  #\n";
    std::cout << pad << "#    ✨ >>> 🔐 SELECT LOGIN OPTION 🔐 <<< ✨       #\n";
    std::cout << pad << "#                                                  #\n";
    std::cout << pad << "####################################################\n\n";
    std::cout << RESET;

    Table table;
    table.add_row({"No", "Options"});
    table[0].format().font_style({FontStyle::bold}).font_color(Color::cyan);
    table.add_row({"1", " --> Login"});
    table.add_row({"2", " [+] Register New Account"});
    table.add_row({"3", " [~] Continue as Guest (read-only)"});
    table.add_row({"4", " [x] Exit"});
    table.column(0).format().width(5);
    table.column(1).format().width(42);
    table[1][1].format().font_color(Color::green);
    table[2][1].format().font_color(Color::cyan);
    table[3][1].format().font_color(Color::yellow);
    table[4][1].format().font_color(Color::red);
    // Print manually with colors + centering
    std::string sep = pad + CYAN + BOLD + "├─────┼──────────────────────────────────────────┤\n" + RESET;
    std::string top = pad + CYAN + BOLD + "┌─────┬──────────────────────────────────────────┐\n" + RESET;
    std::string bot = pad + CYAN + BOLD + "└─────┴──────────────────────────────────────────┘\n" + RESET;
    std::string hdr = pad + CYAN + BOLD + "│ No  │ Options                                  │\n" + RESET;
    std::cout << top << hdr;
    std::cout << sep;
    std::cout << pad << CYAN << BOLD << "│  1  │ " << GREEN  << BOLD << " 🔐  Login                               " << RESET << CYAN << BOLD <<"│\n" << RESET;
    std::cout << sep;
    std::cout << pad << CYAN << BOLD << "│  2  │ " << CYAN   << BOLD << " ✅  Register New Account                " << RESET << CYAN << BOLD <<"│\n" << RESET;
    std::cout << sep;
    std::cout << pad << CYAN << BOLD << "│  3  │ " << YELLOW << BOLD << " 👤  Continue as Guest (read-only)       " << RESET << CYAN << BOLD <<"│\n" << RESET;
    std::cout << sep;
    std::cout << pad << CYAN << BOLD << "│  4  │ " << RED    << BOLD << " ❌  Exit                                " << RESET << CYAN << BOLD <<"│\n" << RESET;
    std::cout << bot;
    std::cout << RESET;
    std::cout << "\n" << pad << GREEN << BOLD << "🎯  Choose your option (1-4): " << RESET;
}

// =============================================
//  MAIN MENU
// =============================================
void printMainMenu(const std::string& username, bool isGuest) {
    printBigDashboard();
    std::string pad = centerPad(52);
    std::cout << pad << GREEN << BOLD << "👋  Current user: " << username;
    if (isGuest) std::cout << " (Guest - Read Only)";
    std::cout << RESET << "\n\n";

    Table table;
    table.add_row({"No", "Options"});
    table[0].format().font_style({FontStyle::bold}).font_color(Color::cyan);
    table.add_row({"1", " 💰  Add Transaction"});
    table.add_row({"2", " 📋  View / Filter Transactions"});
    table.add_row({"3", " 📅  Monthly Summary"});
    table.add_row({"4", " 📊  Category Breakdown"});
    table.add_row({"5", " ✏️  Edit Transaction"});
    table.add_row({"6", " 🗑️  Delete Transaction"});
    table.add_row({"7", " ⚠️  Check Budget Warnings"});
    table.add_row({"8", " 🚪   Save & Logout"});
    table.column(0).format().width(5);
    table.column(1).format().width(42);
    table[1][1].format().font_color(Color::green);
    table[2][1].format().font_color(Color::cyan);
    table[3][1].format().font_color(Color::yellow);
    table[4][1].format().font_color(Color::yellow);
    table[5][1].format().font_color(Color::magenta);
    table[6][1].format().font_color(Color::red);
    table[7][1].format().font_color(Color::red);
    table[8][1].format().font_color(Color::cyan);
    // Print manually with colors + centering
    std::string sep = pad + CYAN + BOLD + "├─────┼──────────────────────────────────────────┤\n" + RESET;
    std::string top = pad + CYAN + BOLD + "┌─────┬──────────────────────────────────────────┐\n" + RESET;
    std::string bot = pad + CYAN + BOLD + "└─────┴──────────────────────────────────────────┘\n" + RESET;
    std::string hdr = pad + CYAN + BOLD + "│ No  │ Options                                  │\n" + RESET;
    std::cout << top << hdr;
std::cout << sep;
std::cout << pad << CYAN << BOLD << "│  1  │ " << GREEN   << BOLD << " 💰  Add Transaction                     " << RESET << CYAN << BOLD << "│\n" << RESET;
std::cout << sep;
std::cout << pad << CYAN << BOLD << "│  2  │ " << CYAN    << BOLD << " 📋  View / Filter Transactions          " << RESET << CYAN << BOLD << "│\n" << RESET;
std::cout << sep;
std::cout << pad << CYAN << BOLD << "│  3  │ " << YELLOW  << BOLD << " 📅  Monthly Summary                     " << RESET << CYAN << BOLD << "│\n" << RESET;
std::cout << sep;
std::cout << pad << CYAN << BOLD << "│  4  │ " << YELLOW  << BOLD << " 📊  Category Breakdown                  " << RESET << CYAN << BOLD << "│\n" << RESET;
std::cout << sep;
std::cout << pad << CYAN << BOLD << "│  5  │ " << MAGENTA << BOLD << " ✏️   Edit Transaction                    " << RESET << CYAN << BOLD << "│\n" << RESET;
std::cout << sep;
std::cout << pad << CYAN << BOLD << "│  6  │ " << RED     << BOLD << " 🗑️  Delete Transaction                  " << RESET << CYAN << BOLD << "│\n" << RESET;
std::cout << sep;
std::cout << pad << CYAN << BOLD << "│  7  │ " << RED     << BOLD << " ⚠️   Check Budget Warnings               " << RESET << CYAN << BOLD << "│\n" << RESET;
std::cout << sep;
std::cout << pad << CYAN << BOLD << "│  8  │ " << CYAN    << BOLD << " 🚪  Save & Logout                       " << RESET << CYAN << BOLD << "│\n" << RESET;
std::cout << bot;
std::cout << "\n" << pad << GREEN << BOLD << "🎯  Choose your option (1-8): " << RESET;
}

// =============================================
//  AUTH
// =============================================
bool handleAuth(AuthManager& auth, User*& outUser, User& guestHolder) {
    printLoginMenu();
    int choice; std::cin >> choice;
    std::string username, password;

    if (choice == 1) {
        std::string p = centerPad(20);
        std::cout << "\n" << p << GREEN << "👤  Username: " << RESET;
        std::cin >> username;
        std::cout << p << GREEN << "🔑  Password: " << RESET;
        std::cin.ignore();
        password = getPassword();
        if (!auth.login(username, password)) {
            std::cout << "\n  " << RED << BOLD << "⚠️ Invalid username or password." << RESET << "\n";
            return false;
        }
        outUser = &auth.getUser(username);
        printBigManager();
        loadingBar("Loading MANAGER Dashboard");
        return true;
    } else if (choice == 2) {
        std::string p = centerPad(20);
        std::cout << "\n" << p << GREEN << "👤  New Username: " << RESET;
        std::cin >> username;
        std::cout << p << GREEN << "🔑  New Password: " << RESET;
        std::cin.ignore();
        password = getPassword();
        if (!auth.registerUser(username, password)) {
            std::cout << "\n  " << RED << BOLD << "⚠️  Username already exists." << RESET << "\n";
            return false;
        }
        outUser = &auth.getUser(username);
        printBigManager();
        loadingBar("Setting up your account");
        return true;
    } else if (choice == 3) {
        guestHolder = auth.getGuestUser();
        outUser = &guestHolder;
        printBigManager();
        loadingBar("Loading Guest Mode");
        return true;
    } else {
        return false;
    }
}

// =============================================
//  BUDGET MENU LOOP
// =============================================
void budgetMenu(BudgetManager& manager, AuthManager& auth, User& user) {
    int choice;
    do {
        printMainMenu(user.username, user.isGuest);
        std::cin >> choice;
        switch (choice) {
            case 1: manager.addTransaction();    break;
            case 2: manager.filterMenu();        break;
            case 3: manager.monthlySummary();    break;
            case 4: manager.categoryBreakdown(); break;
            case 5: manager.editTransaction();   break;
            case 6: manager.deleteTransaction(); break;
            case 7: manager.checkWarnings();     break;
            case 8: {
                manager.save(user);
                auth.save();
                std::string gpad = centerPad(68);
                std::string bpad = centerPad(54);
                std::cout << "\n" << gpad << RED << BOLD << "// EXIT SCREEN\n\n";
                std::cout << gpad << "██████╗  ██████╗  ██████╗ ██████╗ ██████╗ ██╗   ██╗███████╗\n";
                std::cout << gpad << "██╔════╝ ██╔═══██╗██╔═══██╗██╔══██╗██╔══██╗╚██╗ ██╔╝██╔════╝\n";
                std::cout << gpad << "██║  ███╗██║   ██║██║   ██║██║  ██║██████╔╝ ╚████╔╝ █████╗  \n";
                std::cout << gpad << "██║   ██║██║   ██║██║   ██║██║  ██║██╔══██╗  ╚██╔╝  ██╔══╝  \n";
                std::cout << gpad << "╚██████╔╝╚██████╔╝╚██████╔╝██████╔╝██████╔╝   ██║   ███████╗\n";
                std::cout << gpad << " ╚═════╝  ╚═════╝  ╚═════╝ ╚═════╝ ╚═════╝    ╚═╝   ╚══════╝\n";
                std::cout << RESET << "\n";
                std::cout << bpad << RED << BOLD << "+---------------------------------------------------+\n";
                std::cout << bpad << "|                                                   |\n";
                std::cout << bpad << "|    Thank you for using Personal Budget Tracker!   |\n";
                std::cout << bpad << "|    See you next time, " << user.username << "!";
                for (int i = user.username.length(); i < 27; i++) std::cout << " ";
                std::cout << "|\n";
                std::cout << bpad << "|                                                   |\n";
                std::cout << bpad << "+---------------------------------------------------+\n";
                std::cout << RESET << "\n";
                } break;
            default:
                std::cout << "\n  " << RED << BOLD << "⚠️  Invalid option. Choose 1-8." << RESET << "\n";
        }
    } while (choice != 8);
}

// =============================================
//  MAIN
// =============================================
int main(int argc, char* argv[]) {
    enableANSI();

    cxxopts::Options options("budget", "Personal Budget Manager v2.0");
    options.add_options()
        ("h,help",       "Show all available options")
        ("v,version",    "Show program version")
        ("u,user",       "Auto-login username",          cxxopts::value<std::string>())
        ("p,pass",       "Auto-login password",          cxxopts::value<std::string>())
        ("g,guest",      "Auto-login as Guest (read-only)")
        ("l,list",       "Show all transactions and exit")
        ("s,summary",    "Show monthly summary and exit")
        ("b,breakdown",  "Show category breakdown and exit")
        ("w,warnings",   "Show budget warnings and exit")
        ("m,month",      "Filter transactions by month (YYYY-MM)", cxxopts::value<std::string>())
        ("c,category",   "Filter transactions by category",        cxxopts::value<std::string>())
        ("t,type",       "Filter transactions by type",            cxxopts::value<std::string>());

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << "\n" << GREEN << BOLD;
        std::cout << "  ####################################################\n";
        std::cout << "  #       Personal Budget Manager v2.0 - Help       #\n";
        std::cout << "  ####################################################\n\n";
        std::cout << RESET;
        Table table;
        table.add_row({"Option", "Short", "Description", "Example"});
        table[0].format().font_style({FontStyle::bold}).font_color(Color::cyan);
        table.add_row({"--help",      "-h", "Show this help",            "budget --help"});
        table.add_row({"--version",   "-v", "Show version",              "budget --version"});
        table.add_row({"--user",      "-u", "Auto-login username",        "budget -u admin -p 1234"});
        table.add_row({"--pass",      "-p", "Auto-login password",        "budget -u admin -p 1234"});
        table.add_row({"--guest",     "-g", "Login as Guest",            "budget --guest"});
        table.add_row({"--list",      "-l", "Show all transactions",     "budget -u admin -p 1234 -l"});
        table.add_row({"--summary",   "-s", "Show monthly summary",      "budget -u admin -p 1234 -s"});
        table.add_row({"--breakdown", "-b", "Show category breakdown",   "budget -u admin -p 1234 -b"});
        table.add_row({"--warnings",  "-w", "Show budget warnings",      "budget -u admin -p 1234 -w"});
        table.add_row({"--month",     "-m", "Filter by month (YYYY-MM)", "budget -u admin -p 1234 -m 2026-03"});
        table.add_row({"--category",  "-c", "Filter by category",        "budget -u admin -p 1234 -c food"});
        table.add_row({"--type",      "-t", "Filter by type",            "budget -u admin -p 1234 -t income"});
        table.column(0).format().width(14);
        table.column(1).format().width(8);
        table.column(2).format().width(28);
        table.column(3).format().width(32);
        std::cout << table << "\n";
        return 0;
    }

    if (result.count("version")) {
        std::cout << GREEN << BOLD << "\n  Personal Budget Manager v2.0\n" << RESET;
        std::cout << "  Built with: nlohmann/json, tabulate, cxxopts\n\n";
        return 0;
    }

    // Show school banner then BUDGET MANAGER
    printSchoolBanner();
    printBigBudget();
    printBigManager();

    AuthManager auth;
    User* currentUser = nullptr;
    User guestHolder;

    if (result.count("guest")) {
        guestHolder = auth.getGuestUser();
        currentUser = &guestHolder;
        loadingBar("Loading Guest Mode");
    }

    if (!currentUser && result.count("user") && result.count("pass")) {
        std::string u = result["user"].as<std::string>();
        std::string p = result["pass"].as<std::string>();
        if (auth.login(u, p)) {
            currentUser = &auth.getUser(u);
            loadingBar("Loading MANAGER Dashboard");
        } else {
            std::cout << "  " << RED << "⚠️  Auto-login failed." << RESET << "\n\n";
        }
    }

    while (currentUser == nullptr) {
        if (!handleAuth(auth, currentUser, guestHolder)) {
            std::cout << "\n  Try again? (1=Yes / 0=No): ";
            int retry; std::cin >> retry;
            if (retry != 1) {
                std::cout << "\n  " << GREEN << "👋  Goodbye!\n\n" << RESET;
                return 0;
            }
            printSchoolBanner();
            printBigBudget();
            printBigManager();
        }
    }

    BudgetManager manager(currentUser->isGuest);
    manager.load(*currentUser);

    if (result.count("list") || result.count("month") ||
        result.count("category") || result.count("type")) {
        std::string month    = result.count("month")    ? result["month"].as<std::string>()    : "";
        std::string category = result.count("category") ? result["category"].as<std::string>() : "";
        std::string type     = result.count("type")     ? result["type"].as<std::string>()     : "";
        manager.viewTransactions(month, category, type);
        manager.save(*currentUser);
        return 0;
    }
    if (result.count("summary"))   { manager.monthlySummary();    return 0; }
    if (result.count("breakdown")) { manager.categoryBreakdown(); return 0; }
    if (result.count("warnings"))  { manager.checkWarnings();     return 0; }

    budgetMenu(manager, auth, *currentUser);
    return 0;
}