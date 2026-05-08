#include "AuthManager.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

AuthManager::AuthManager() {
    // Create Data folder if it doesn't exist
    std::filesystem::create_directories("Data");
    load();
    if (users.empty()) {
        User u;
        u.username = "admin";
        u.password = "1234";
        u.isGuest  = false;
        users["admin"] = u;
        save();
        std::cout << "\n  [!] Default account created.\n";
        std::cout << "      Username : admin\n";
        std::cout << "      Password : 1234\n\n";
    }
}

bool AuthManager::login(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it != users.end())
        return it->second.password == password;
    return false;
}

bool AuthManager::registerUser(const std::string& username, const std::string& password) {
    if (users.find(username) != users.end()) return false;
    User u;
    u.username = username;
    u.password = password;
    u.isGuest  = false;
    users[username] = u;
    save();
    return true;
}

User& AuthManager::getUser(const std::string& username) {
    return users[username];
}

User AuthManager::getGuestUser() {
    User guest;
    guest.username = "Guest";
    guest.password = "";
    guest.isGuest  = true;
    return guest;
}

// Save users to Data/users.json
void AuthManager::save() {
    json root = json::array();
    for (auto& pair : users) {
        root.push_back({
            {"username", pair.second.username},
            {"password", pair.second.password}
        });
    }
    std::ofstream file(filename);
    if (!file) { std::cout << "  [!] Could not save users.json\n"; return; }
    file << root.dump(2);
    file.close();
}

// Load users from Data/users.json
void AuthManager::load() {
    std::ifstream file(filename);
    if (!file) return;

    try {
        json root = json::parse(file);
        for (auto& u : root) {
            User user;
            user.username = u.at("username").get<std::string>();
            user.password = u.at("password").get<std::string>();
            user.isGuest  = false;
            users[user.username] = user;
        }
    } catch (...) {
        std::cout << "  [!] Could not read users.json\n";
    }
    file.close();
}