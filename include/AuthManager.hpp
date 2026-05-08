#pragma once

#include <string>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct User {
    std::string username;
    std::string password;
    bool isGuest = false;
};

class AuthManager {
private:
    std::map<std::string, User> users;
    const std::string filename = "Data/users.json";

public:
    AuthManager();
    bool login(const std::string& username, const std::string& password);
    bool registerUser(const std::string& username, const std::string& password);
    User& getUser(const std::string& username);
    User getGuestUser();
    void save();
    void load();
};

