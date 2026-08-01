#include "cibil_score.hpp"
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int CIBILService::fetchLiveCIBILScore(const std::string& pan, const std::string& name, const std::string& mobile, const std::string& dob) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    int score = 700; // Fallback score if API is unreachable

    if (curl) {
        std::string url = "https://cibil-mock-server.onrender.com/api/v1/cibil/score";
        
        nlohmann::json reqJson;
        reqJson["PAN"] = pan;
        reqJson["name"] = name;
        reqJson["mobile"] = mobile;
        reqJson["dob"] = dob;
        std::string jsonStr = reqJson.dump();

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 12L);

        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            try {
                auto resJson = nlohmann::json::parse(readBuffer);
                if (resJson.contains("data") && resJson["data"].contains("cibilScore")) {
                    score = resJson["data"]["cibilScore"].get<int>();
                    std::cout << "✅ [CIBIL API Success] Fetched CIBIL Score for PAN " << pan << ": " << score << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "⚠️ Failed to parse CIBIL API JSON response: " << e.what() << std::endl;
            }
        } else {
            std::cerr << "⚠️ CURL Request failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return score;
}
