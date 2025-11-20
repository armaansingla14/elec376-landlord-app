#include "AdminCtrl.h"
#include <fstream>

bool AdminCtrl::isAdmin(const std::string &email) {
    // load users.json and check admin flag
    Json::Value arr(Json::arrayValue);
    {
        std::lock_guard<std::mutex> lk(mu_);
        std::ifstream f(usersPath_);
        if(f.good()) f >> arr;
    }

    for(const auto &u : arr) {
        if(u["email"].asString() == email) {
            return u.get("admin", 0).asInt() == 1;
        }
    }
    return false;
}

void AdminCtrl::getReported(const drogon::HttpRequestPtr &req, std::function<void (const drogon::HttpResponsePtr &)> &&cb) {
    auto authHeader = req->getHeader("authorization");
    auto email = parseToken(authHeader);
    if(email.empty() || !isAdmin(email)) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value(Json::objectValue));
        resp->setStatusCode(drogon::k401Unauthorized);
        (*resp->getJsonObject())["error"] = "missing/invalid token or not admin";
        cb(resp);
        return;
    }

    Json::Value root(Json::objectValue);
    {
        std::lock_guard<std::mutex> lk(mu_);
        std::ifstream f(reportedPath_);
        if(f.good()) {
            f >> root;
        } else {
            root["reports"] = Json::Value(Json::arrayValue);
        }
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    cb(resp);
}

void AdminCtrl::approve(const drogon::HttpRequestPtr &req, std::function<void (const drogon::HttpResponsePtr &)> &&cb, const std::string &id) {
    auto authHeader = req->getHeader("authorization");
    auto email = parseToken(authHeader);
    if(email.empty() || !isAdmin(email)) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value(Json::objectValue));
        resp->setStatusCode(drogon::k401Unauthorized);
        (*resp->getJsonObject())["error"] = "missing/invalid token or not admin";
        cb(resp);
        return;
    }

    Json::Value root;
    {
        std::lock_guard<std::mutex> lk(mu_);
        std::ifstream f(reportedPath_);
        if(f.good()) f >> root;
        else root["reports"] = Json::Value(Json::arrayValue);

        bool found = false;
        for(auto &r : root["reports"]) {
            if(r["id"].asString() == id) {
                r["status"] = "approved";
                found = true;
                break;
            }
        }
        if(found) {
            std::ofstream out(reportedPath_, std::ios::trunc);
            out << root.toStyledString();
        } else {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value(Json::objectValue));
            resp->setStatusCode(drogon::k404NotFound);
            (*resp->getJsonObject())["error"] = "report not found";
            cb(resp);
            return;
        }
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value(Json::objectValue));
    (*resp->getJsonObject())["message"] = "report approved";
    cb(resp);
}

void AdminCtrl::deny(const drogon::HttpRequestPtr &req, std::function<void (const drogon::HttpResponsePtr &)> &&cb, const std::string &id) {
    auto authHeader = req->getHeader("authorization");
    auto email = parseToken(authHeader);
    if(email.empty() || !isAdmin(email)) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value(Json::objectValue));
        resp->setStatusCode(drogon::k401Unauthorized);
        (*resp->getJsonObject())["error"] = "missing/invalid token or not admin";
        cb(resp);
        return;
    }

    Json::Value root;
    {
        std::lock_guard<std::mutex> lk(mu_);
        std::ifstream f(reportedPath_);
        if(f.good()) f >> root;
        else root["reports"] = Json::Value(Json::arrayValue);

        bool found = false;
        for(auto &r : root["reports"]) {
            if(r["id"].asString() == id) {
                r["status"] = "denied";
                found = true;
                break;
            }
        }
        if(found) {
            std::ofstream out(reportedPath_, std::ios::trunc);
            out << root.toStyledString();
        } else {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value(Json::objectValue));
            resp->setStatusCode(drogon::k404NotFound);
            (*resp->getJsonObject())["error"] = "report not found";
            cb(resp);
            return;
        }
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value(Json::objectValue));
    (*resp->getJsonObject())["message"] = "report denied";
    cb(resp);
}
