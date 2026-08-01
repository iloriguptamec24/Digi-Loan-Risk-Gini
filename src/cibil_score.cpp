#include "cibil_score.hpp"
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int CIBILService::fetchLiveCIBILScore(
    const std::string& pan, 
    const std::string& name, 
    const std::string& mobile, 
    const std::string& dob
) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    int score = 700; // Fallback score

    if (curl) {
        std::string url = "https://cibil-mock-server.onrender.com/api/v1/cibil/score";
        
        // 1. Split full name into first and last name
        std::string firstName = name;
        std::string lastName = "";
        std::stringstream ss(name);
        if (ss >> firstName) {
            std::getline(ss >> std::ws, lastName);
        }
        if (lastName.empty()) lastName = firstName;

        // 2. Build JSON Request Payload with nested "identifiers" array
        nlohmann::json reqJson;
        
        // Create identifier object
        nlohmann::json panObj = {
            {"type", "PAN"},
            {"value", pan}
        };

        reqJson["identifiers"] = nlohmann::json::array({ panObj });
        reqJson["mobile"] = mobile;
        reqJson["first_name"] = firstName;
        reqJson["last_name"] = lastName;
        reqJson["dob"] = dob;

        std::string jsonStr = reqJson.dump();

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            std::cout << "\n📩 [DEBUG RAW API RESPONSE]:\n" << readBuffer << "\n" << std::endl;

            try {
                auto resJson = nlohmann::json::parse(readBuffer);
                
                if (resJson.is_array() && !resJson.empty()) {
                    resJson = resJson[0];
                }

                // Extract score from credit_score -> score
                if (resJson.contains("credit_score") && resJson["credit_score"].contains("score")) {
                    score = resJson["credit_score"]["score"].get<int>();
                    std::cout << "✅ [CIBIL API Success] Extracted Score: " << score << std::endl;
                }
                else if (resJson.contains("data") && resJson["data"].contains("credit_score") && resJson["data"]["credit_score"].contains("score")) {
                    score = resJson["data"]["credit_score"]["score"].get<int>();
                    std::cout << "✅ [CIBIL API Success] Extracted Score: " << score << std::endl;
                }
                else if (resJson.contains("score")) {
                    score = resJson["score"].get<int>();
                    std::cout << "✅ [CIBIL API Success] Extracted Score: " << score << std::endl;
                } 
                else {
                    std::cerr << "⚠️ [CIBIL API Warning] Score key missing in response. Using default: " << score << std::endl;
                }

            } catch (const std::exception& e) {
                std::cerr << "⚠️ [CIBIL API Error] JSON Exception: " << e.what() << std::endl;
            }
        } else {
            std::cerr << "⚠️ [CIBIL API Error] CURL failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return score;
}