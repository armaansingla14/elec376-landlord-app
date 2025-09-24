#include <drogon/drogon.h>
#include <filesystem>
#include <string>

int main(){
    drogon::app().addListener("127.0.0.1", 8080);
    drogon::app().setThreadNum(1);
    drogon::app().setLogLevel(trantor::Logger::kInfo);
    drogon::app().enableSession();
    drogon::app().registerPostHandlingAdvice([](const drogon::HttpRequestPtr &,
                                       const drogon::HttpResponsePtr &resp) {
            auto mutableResp = std::const_pointer_cast<drogon::HttpResponse>(resp);
            mutableResp->addHeader("Access-Control-Allow-Origin", "http://127.0.0.1:5173");
            mutableResp->addHeader("Access-Control-Allow-Credentials", "true");
            mutableResp->addHeader("Access-Control-Allow-Headers", "Authorization, Content-Type");
            mutableResp->addHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
        });

    
}