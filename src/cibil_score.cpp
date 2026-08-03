/**
 * @file cibil_score.cpp
 * @brief Implementation of the CIBIL Service module for fetching live credit bureau scores.
 * 
 * This file handles HTTP communication with third-party credit score servers using libcurl,
 * constructs structured JSON requests using nlohmann::json, and parses multi-schema JSON
 * responses with type safety and fallback guarantees.
 */

#include "cibil_score.hpp"
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

/**
 * @brief Memory write callback function required by libcurl.
 * 
 * When libcurl receives chunked HTTP response data from the remote endpoint,
 * it repeatedly invokes this callback to append raw stream bytes into a C++ string buffer.
 * 
 * @param contents Pointer to the incoming memory block delivered by cURL.
 * @param size Size of each data element (typically 1 byte).
 * @param nmemb Number of data elements in the chunk.
 * @param userp Custom pointer passed via CURLOPT_WRITEDATA (cast to std::string*).
 * @return size_t Total number of bytes successfully written into the target buffer.
 */
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalBytes = size * nmemb;
    // Cast the void pointer back to our std::string target buffer and append the received chunk
    static_cast<std::string*>(userp)->append(static_cast<char*>(contents), totalBytes);
    return totalBytes;
}

/**
 * @brief Helper function to safely extract an integer credit score from JSON.
 * 
 * Remote mock/production credit APIs may represent credit score values either as raw integers
 * (e.g., 750) or stringified numbers (e.g., "750"). This utility handles both data types safely
 * to prevent nlohmann::json type mismatch exceptions (`type_error`).
 * 
 * @param scoreVal The nlohmann::json node containing the score value.
 * @return int Extracted score value, or 0 if extraction fails.
 */
static int parseScoreValue(const nlohmann::json& scoreVal) {
    // Case 1: JSON payload supplies a direct numeric value (e.g., {"score": 750})
    if (scoreVal.is_number()) {
        return scoreVal.get<int>();
    } 
    // Case 2: JSON payload supplies a stringified number (e.g., {"score": "750"})
    else if (scoreVal.is_string()) {
        try {
            return std::stoi(scoreVal.get<std::string>());
        } catch (...) {
            // Conversion failed (e.g. string was non-numeric "N/A" or empty)
            return 0;
        }
    }
    return 0; // Return zero if node type is invalid or null
}

/**
 * @brief Connects to external CIBIL mock API and retrieves credit bureau score for a borrower.
 * 
 * Performs identity mapping, constructs standard credit inquiry payload, fires an HTTP POST
 * request via libcurl, and dynamically searches response structures for the credit score.
 * 
 * @param pan Borrower's 10-character Permanent Account Number.
 * @param name Borrower's full name.
 * @param mobile Borrower's 10-digit registered mobile number.
 * @param dob Borrower's Date of Birth (YYYY-MM-DD).
 * @return int Evaluated CIBIL score (Range: 300 - 900), or fallback default (699) on failure.
 */
int CIBILService::fetchLiveCIBILScore(
    const std::string& pan, 
    const std::string& name, 
    const std::string& mobile, 
    const std::string& dob
) {
    // Initialize libcurl context handle for single HTTP transfer session
    CURL* curl = curl_easy_init();
    
    // Buffer to store raw response string received from the server
    std::string readBuffer;
    
    // Standard industry fallback credit score used when API connection or parsing fails
    int score = 699; 

    if (curl) {
        // Target endpoint for CIBIL credit inquiry service
        std::string url = "https://cibil-mock-server.onrender.com/api/v1/cibil/score";
        
        // ------------------------------------------------------------------
        // STEP 1: NAME SPLITTING LOGIC
        // Convert full name (e.g., "Priya Kumar") into first and last name components
        // ------------------------------------------------------------------
        std::string firstName = name;
        std::string lastName = "";
        std::stringstream ss(name);
        
        if (ss >> firstName) {
            // Extract remaining characters into lastName if multiple words exist
            std::getline(ss >> std::ws, lastName);
        }
        // Fallback: If single word name provided, duplicate firstName as lastName for schema compliance
        if (lastName.empty()) {
            lastName = firstName;
        }

        // ------------------------------------------------------------------
        // STEP 2: CONSTRUCT JSON REQUEST PAYLOAD
        // Build nested structure expected by credit bureau mock endpoints
        // ------------------------------------------------------------------
        nlohmann::json reqJson;
        
        // Create PAN identifier sub-object
        nlohmann::json panObj = {
            {"type", "PAN"},
            {"value", pan}
        };

        // Populate required root payload attributes
        reqJson["identifiers"] = nlohmann::json::array({ panObj });
        reqJson["mobile"] = mobile;
        reqJson["first_name"] = firstName;
        reqJson["last_name"] = lastName;
        reqJson["dob"] = dob;

        // Serialize JSON object into string format for HTTP transmission
        std::string jsonStr = reqJson.dump();

        // ------------------------------------------------------------------
        // STEP 3: CONFIGURE LIBCURL TRANSFER OPTIONS
        // ------------------------------------------------------------------
        struct curl_slist* headers = NULL;
        // Declare JSON body payload header
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Target API URL endpoint
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        
        // Specify HTTP POST method and attach JSON string payload
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        
        // Attach custom request headers
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // Bind custom stream writer callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        
        // Target buffer memory reference for the callback function
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // Enforce maximum network response timeout (15 seconds) to prevent hanging
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

        // ------------------------------------------------------------------
        // STEP 4: EXECUTE HTTP POST REQUEST
        // ------------------------------------------------------------------
        CURLcode res = curl_easy_perform(curl);

        // Verify network transfer completed successfully without socket or DNS errors
        if (res == CURLE_OK) {
            std::cout << "\n📩 [DEBUG RAW API RESPONSE]:\n" << readBuffer << "\n" << std::endl;

            try {
                // Parse raw string buffer into structured nlohmann::json object
                auto resJson = nlohmann::json::parse(readBuffer);
                
                // Normalization: Unwrap array if server wraps single response object inside a list [...]
                if (resJson.is_array() && !resJson.empty()) {
                    resJson = resJson[0];
                }

                int extractedScore = 0;

                // --------------------------------------------------------------
                // STEP 5: DYNAMIC MULTI-SCHEMA JSON SCORE EXTRACTION
                // Evaluates multiple schema patterns used across API revisions
                // --------------------------------------------------------------
                
                // Schema Pattern A: {"credit_score": {"score": 750}}
                if (resJson.contains("credit_score") && resJson["credit_score"].contains("score")) {
                    extractedScore = parseScoreValue(resJson["credit_score"]["score"]);
                }
                // Schema Pattern B: {"data": {"credit_score": {"score": 750}}}
                else if (resJson.contains("data") && resJson["data"].contains("credit_score") && resJson["data"]["credit_score"].contains("score")) {
                    extractedScore = parseScoreValue(resJson["data"]["credit_score"]["score"]);
                }
                // Schema Pattern C: Direct key {"score": 750}
                else if (resJson.contains("score")) {
                    extractedScore = parseScoreValue(resJson["score"]);
                } 

                // --------------------------------------------------------------
                // STEP 6: EVALUATE RESULT & FALLBACK ASSIGNMENT
                // --------------------------------------------------------------
                if (extractedScore > 0) {
                    score = extractedScore;
                    std::cout << "✅ [CIBIL API Success] Extracted Score: " << score << std::endl;
                } else {
                    std::cerr << "⚠️ [CIBIL API Warning] Score key missing or invalid. Using default fallback: " << score << std::endl;
                }

            } catch (const std::exception& e) {
                // Catch malformed JSON string parsing exceptions gracefully
                std::cerr << "⚠️ [CIBIL API Error] JSON Exception: " << e.what() << std::endl;
            }
        } else {
            // Log cURL transfer error details (e.g. network host unreachable, timeout)
            std::cerr << "⚠️ [CIBIL API Error] CURL failed: " << curl_easy_strerror(res) << std::endl;
        }

        // ------------------------------------------------------------------
        // STEP 7: CLEANUP RESOURCE MEMORY ALLOCATIONS
        // ------------------------------------------------------------------
        curl_slist_free_all(headers); // Deallocate cURL header list memory
        curl_easy_cleanup(curl);      // Terminate cURL session context
    }

    // Return successfully fetched CIBIL score or fallback default
    return score;
}