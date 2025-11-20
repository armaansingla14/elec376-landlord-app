#include "LandlordCtrl.h"
#include <fstream>
#include <algorithm>
#include <map>
#include <vector>
#include <cmath>
#include <ctime> 

static std::string lower(std::string s){
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

// Helper: load reviews.json and compute per-landlord (sum, count)
static std::map<std::string, std::pair<double,int>> computeLandlordRatings()
{
    std::map<std::string, std::pair<double,int>> ratings;
    Json::Value reviewDb;
    std::ifstream reviewFile("data/reviews.json");
    if (reviewFile.good()) {
        reviewFile >> reviewDb;
    } else {
        std::ifstream reviewFile2("../data/reviews.json");
        if (reviewFile2.good()) {
            reviewFile2 >> reviewDb;
        } else {
            reviewDb["reviews"] = Json::Value(Json::arrayValue);
        }
    }

    for (const auto &review : reviewDb["reviews"]) {
        std::string landlordId = review["landlord_id"].asString();
        int rating = review["rating"].asInt();
        if (ratings.find(landlordId) == ratings.end()) {
            ratings[landlordId] = std::make_pair(0.0, 0);
        }
        ratings[landlordId].first += rating;
        ratings[landlordId].second += 1;
    }

    return ratings;
}

void LandlordCtrl::search(const drogon::HttpRequestPtr &req,
                          std::function<void (const drogon::HttpResponsePtr &)> &&cb) {
    auto q = req->getParameter("name");
    std::string query = lower(q);

    Json::Value db;
    {
        std::lock_guard<std::mutex> lk(mu_);
        std::ifstream f(dbPath_);
        if(!f.good()){
            auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value(Json::objectValue));
            resp->setStatusCode(drogon::k500InternalServerError);
            (*resp->getJsonObject())["error"] = "landlords database missing";
            cb(resp);
            return;
        }
        f >> db;
    }

    // Compute ratings map once and attach to results
    auto landlordRatings = computeLandlordRatings();

    Json::Value results(Json::arrayValue);
    const auto &arr = db["landlords"];
    for(const auto &ll : arr){
        std::string name = ll["name"].asString();
        if(query.empty() || lower(name).find(query) != std::string::npos){
            Json::Value landlord = ll;
            std::string landlordId = ll["landlord_id"].asString();
            double avgRating = 0.0;
            int reviewCount = 0;
            auto it = landlordRatings.find(landlordId);
            if(it != landlordRatings.end() && it->second.second > 0){
                avgRating = it->second.first / it->second.second;
                reviewCount = it->second.second;
            }
            landlord["average_rating"] = std::round(avgRating * 100.0) / 100.0;
            landlord["review_count"] = reviewCount;
            results.append(landlord);
        }
    }

    // Create a json object to send back
    Json::Value body(Json::objectValue);
    // define entry "results" to the array of names we just captured
    body["results"] = results;
    // format response to a drogon http response object
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);

    // now inside the reference (call back), but the response object inside
    cb(resp);
}

void LandlordCtrl::stats(const drogon::HttpRequestPtr &req,
                        std::function<void (const drogon::HttpResponsePtr &)> &&cb) {
    Json::Value db;
    {
        std::lock_guard<std::mutex> lk(mu_);
        std::ifstream f(dbPath_);
        if(!f.good()){
            auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value(Json::objectValue));
            resp->setStatusCode(drogon::k500InternalServerError);
            (*resp->getJsonObject())["error"] = "landlords database missing";
            cb(resp);
            return;
        }
        f >> db;
    }

    int landlordCount = 0;
    int propertyCount = 0;
    int unitCount = 0;

    const auto &arr = db["landlords"];
    for(const auto &ll : arr){
        landlordCount++;
        const auto &properties = ll["properties"];
        for(const auto &property : properties){
            propertyCount++;
            const auto &units = property["unit_details"];
            for(const auto &unit : units){
                unitCount++;
            }
        }
    }

    Json::Value body(Json::objectValue);
    body["landlords"] = landlordCount;
    body["properties"] = propertyCount;
    body["units"] = unitCount;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    cb(resp);
}

void LandlordCtrl::leaderboard(const drogon::HttpRequestPtr &req,
                                std::function<void (const drogon::HttpResponsePtr &)> &&cb) {
    // Load landlords data
    Json::Value landlordDb;
    {
        std::lock_guard<std::mutex> lk(mu_);
        std::ifstream f(dbPath_);
        if(!f.good()){
            auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value(Json::objectValue));
            resp->setStatusCode(drogon::k500InternalServerError);
            (*resp->getJsonObject())["error"] = "landlords database missing";
            cb(resp);
            return;
        }
        f >> landlordDb;
    }

    // Compute ratings map once (sum,count) using helper
    auto landlordRatings = computeLandlordRatings();

    // Create leaderboard entries
    Json::Value results(Json::arrayValue);
    std::vector<std::pair<std::string, double>> sortedLandlords;

    for (const auto &ll : landlordDb["landlords"]) {
        std::string landlordId = ll["landlord_id"].asString();
        double avgRating = 0.0;
        int reviewCount = 0;

        if (landlordRatings.find(landlordId) != landlordRatings.end()) {
            avgRating = landlordRatings[landlordId].first / landlordRatings[landlordId].second;
            reviewCount = landlordRatings[landlordId].second;
        }

        // Create leaderboard entry
        Json::Value entry = ll;
        entry["average_rating"] = std::round(avgRating * 100.0) / 100.0; // Round to 2 decimal places
        entry["review_count"] = reviewCount;
        sortedLandlords.push_back({landlordId, avgRating});
        results.append(entry);
    }

    // Sort results by average rating (highest to lowest)
    // We need to reorder the results array
    std::sort(sortedLandlords.begin(), sortedLandlords.end(), 
              [](const auto &a, const auto &b) { return a.second > b.second; });

    Json::Value sortedResults(Json::arrayValue);
    for (const auto &sorted : sortedLandlords) {
        for (const auto &entry : results) {
            if (entry["landlord_id"].asString() == sorted.first) {
                sortedResults.append(entry);
                break;
            }
        }
    }

    Json::Value body(Json::objectValue);
    body["leaderboard"] = sortedResults;
    
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    cb(resp);
}

void LandlordCtrl::submitRequest(const drogon::HttpRequestPtr &req,
                            std::function<void (const drogon::HttpResponsePtr &)> &&cb)
{
        auto json = req->getJsonObject();
    if (!json) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value(Json::objectValue));
        resp->setStatusCode(drogon::k400BadRequest);
        (*resp->getJsonObject())["error"] = "Request body must be JSON";
        cb(resp);
        return;
    }

    std::string landlordName   = (*json)["landlord_name"].asString();
    std::string requesterName  = (*json)["user_name"].asString();
    std::string requesterEmail = (*json)["user_email"].asString();
    std::string propertyInfo   = (*json)["property_address"].asString();
    std::string details        = (*json)["details"].asString();

    if (landlordName.empty() || requesterEmail.empty()) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value(Json::objectValue));
        resp->setStatusCode(drogon::k400BadRequest);
        (*resp->getJsonObject())["error"] =
            "landlord_name and user_email are required";
        cb(resp);
        return;
    }

    Json::Value db;
    {
        std::lock_guard<std::mutex> lk(mu_);

        // Load existing requests file if present
        std::ifstream f(requestDbPath_);
        if (f.good()) {
            f >> db;
        } else {
            db["requests"] = Json::Value(Json::arrayValue);
        }

        if (!db.isMember("requests") || !db["requests"].isArray()) {
            db["requests"] = Json::Value(Json::arrayValue);
        }

        // Compute next id
        int nextId = 1;
        for (const auto &reqItem : db["requests"]) {
            if (reqItem.isMember("id")) {
                nextId = std::max(nextId, reqItem["id"].asInt() + 1);
            }
        }

        // Timestamp
        std::time_t now = std::time(nullptr);
        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&now));

        Json::Value newReq(Json::objectValue);
        newReq["id"] = nextId;
        newReq["landlord_name"] = landlordName;
        newReq["user_name"] = requesterName;
        newReq["user_email"] = requesterEmail;
        newReq["property_address"] = propertyInfo;
        newReq["details"] = details;
        newReq["created_at"] = buf;
        newReq["status"] = "pending";

        db["requests"].append(newReq);

        // Save back to file
        std::ofstream out(requestDbPath_);
        if (!out.good()) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value(Json::objectValue));
            resp->setStatusCode(drogon::k500InternalServerError);
            (*resp->getJsonObject())["error"] = "Failed to write landlord_requests database";
            cb(resp);
            return;
        }
        out << db;
    }

    Json::Value body(Json::objectValue);
    body["ok"] = true;
    body["message"] = "Landlord request submitted";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    cb(resp);
}
