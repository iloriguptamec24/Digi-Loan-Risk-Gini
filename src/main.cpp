#include "crow.h"
#include "database.hpp"
#include "cibil_score.hpp"
#include <iostream>
#include <sstream>

/**
 * @brief Helper utility to attach cross-origin resource sharing (CORS) headers to responses.
 */
inline void applyCorsHeaders(crow::response& res) {
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

int main() {
    // ------------------------------------------------------------------
    // 0. CROW LOGGER CONFIGURATION
    // ------------------------------------------------------------------
    crow::logger::setLogLevel(crow::LogLevel::Debug);

    // ------------------------------------------------------------------
    // 1. DATABASE INITIALIZATION
    // ------------------------------------------------------------------
    Database::initDatabase();

    // Create the primary Crow microservice application engine
    crow::SimpleApp app;

    // ==================================================================
    // GLOBAL OPTIONS ROUTE (CORS Preflight Requests)
    // ==================================================================
    CROW_ROUTE(app, "/<path>")
    .methods(crow::HTTPMethod::Options)
    ([](const crow::request&, crow::response& res, std::string) {
        applyCorsHeaders(res);
        res.code = 200;
        res.end();
    });

    // ==================================================================
    // ROUTE 1: GET /
    // Serves static web dashboard (public/index.html)
    // ==================================================================
    CROW_ROUTE(app, "/")
    ([](const crow::request&, crow::response& res) {
        CROW_LOG_INFO << "Serving static frontend file: public/index.html";
        res.set_static_file_info("public/index.html");
        res.end();
    });

    // ==================================================================
    // ROUTE 2: POST /api/applicant
    // Accepts PAN details, fetches CIBIL score, and runs Risk Engine
    // ==================================================================
    CROW_ROUTE(app, "/api/applicant").methods(crow::HTTPMethod::Post)
    ([](const crow::request& req, crow::response& res) {
        applyCorsHeaders(res);
        
        auto body = crow::json::load(req.body);
        if (!body) {
            CROW_LOG_ERROR << "Failed to parse JSON body in /api/applicant";
            res.code = 400;
            res.write("Invalid JSON payload");
            res.end();
            return;
        }

        // Validate mandatory parameters
        if (!body.has("name") || !body.has("loanType") || !body.has("pan") ||
            !body.has("income") || !body.has("monthlyDebts") || !body.has("loanAmount")) {
            CROW_LOG_ERROR << "Missing required parameters in request payload";
            res.code = 400;
            res.write("Missing required applicant parameters (PAN required)");
            res.end();
            return;
        }

        // Extract parameters safely
        std::string name = std::string(body["name"].s());
        std::string loanType = std::string(body["loanType"].s());
        std::string pan = std::string(body["pan"].s());
        std::string mobile = body.has("mobile") ? std::string(body["mobile"].s()) : "";
        std::string dob = body.has("dob") ? std::string(body["dob"].s()) : "";

        double income = body["income"].d();
        double monthlyDebts = body["monthlyDebts"].d();
        double loanAmount = body["loanAmount"].d();

        // --------------------------------------------------------------
        // CIBIL SCORE LOOKUP VIA PAN
        // --------------------------------------------------------------
        CROW_LOG_INFO << "Fetching live CIBIL score for Applicant: " << name << " | PAN: " << pan;
        
        int cibilScore = CIBILService::fetchLiveCIBILScore(pan, name, mobile, dob);
        
        // Fallback handling if score query fails
        if (cibilScore <= 0) {
            CROW_LOG_WARNING << "CIBIL score fetch returned invalid value (" << cibilScore << "). Applying default score.";
            cibilScore = 777;
        }

        CROW_LOG_INFO << "Evaluated CIBIL Score: " << cibilScore << " for PAN: " << pan;

        // --------------------------------------------------------------
        // OOP RISK EVALUATION ENGINE
        // --------------------------------------------------------------
        auto loanObj = LoanFactory::createLoan(loanType, name, income, cibilScore, monthlyDebts, loanAmount);
        PersonalLoanResult result = loanObj->evaluate();

        CROW_LOG_INFO << "Risk assessment decision for " << name << ": " << result.decision;

        // Persist record in SQLite database
        Database::saveApplicant(pan, name, loanType, income, cibilScore, monthlyDebts, loanAmount, result);

        // Construct JSON response
        crow::json::wvalue resJson;
        resJson["status"] = "SUCCESS";
        resJson["pan"] = pan;
        resJson["fetchedCibilScore"] = cibilScore;
        resJson["decision"] = result.decision;

        res.code = 201;
        res.set_header("Content-Type", "application/json");
        res.write(resJson.dump());
        res.end();
    });

    // ==================================================================
    // ROUTE 3: GET /api/reports
    // Returns full report history for the Decision Matrix UI
    // ==================================================================
    CROW_ROUTE(app, "/api/reports").methods(crow::HTTPMethod::Get)
    ([](const crow::request&, crow::response& res) {
        applyCorsHeaders(res);

        auto reports = Database::getAllReports();
        crow::json::wvalue::list reportList;

        for (const auto& rep : reports) {
            crow::json::wvalue item;
            item["id"] = rep.id;
            item["pan"] = rep.pan;
            item["name"] = rep.name;
            item["loanType"] = rep.loanType;
            item["income"] = rep.income;
            item["cibilScore"] = rep.cibilScore;
            item["monthlyDebts"] = rep.monthlyDebts;
            item["requestedLoan"] = rep.requestedLoan;
            item["ltiRatio"] = rep.ltiRatio;
            item["foirRatio"] = rep.foirRatio;
            item["bankRiskFactor"] = rep.bankRiskFactor;
            item["score"] = rep.score;
            item["decision"] = rep.decision;

            crow::json::wvalue::list reasonsList;
            for (const auto& reason : rep.rejectionReasons) {
                reasonsList.push_back(reason);
            }
            item["rejectionReasons"] = std::move(reasonsList);
            reportList.push_back(std::move(item));
        }

        crow::json::wvalue resJson;
        resJson["status"] = "success";
        resJson["data"] = std::move(reportList);

        res.code = 200;
        res.set_header("Content-Type", "application/json");
        res.write(resJson.dump());
        res.end();
    });

    // ==================================================================
    // ROUTE 4: POST /api/admin/override
    // Allows decision overriding for administrative actions
    // ==================================================================
    CROW_ROUTE(app, "/api/admin/override").methods(crow::HTTPMethod::Post)
    ([](const crow::request& req, crow::response& res) {
        applyCorsHeaders(res);

        auto body = crow::json::load(req.body);
        if (!body || !body.has("id") || !body.has("overrideDecision")) {
            res.code = 400;
            res.write("Invalid payload");
            res.end();
            return;
        }

        int id = body["id"].i();
        std::string newDecision = std::string(body["overrideDecision"].s());

        bool success = Database::overrideDecision(id, newDecision);
        if (success) {
            crow::json::wvalue resJson;
            resJson["status"] = "OVERRIDDEN";
            res.code = 200;
            res.set_header("Content-Type", "application/json");
            res.write(resJson.dump());
        } else {
            res.code = 500;
            res.write("Database update failed");
        }
        res.end();
    });

    // ==================================================================
    // ROUTE 5: DELETE /api/admin/delete
    // Deletes an applicant record from SQLite
    // ==================================================================
    CROW_ROUTE(app, "/api/admin/delete").methods(crow::HTTPMethod::Delete)
    ([](const crow::request& req, crow::response& res) {
        applyCorsHeaders(res);

        auto body = crow::json::load(req.body);
        if (!body || !body.has("id")) {
            res.code = 400;
            res.write("Missing application ID in delete payload");
            res.end();
            return;
        }

        int id = body["id"].i();
        bool success = Database::deleteApplicant(id);

        if (success) {
            crow::json::wvalue resJson;
            resJson["status"] = "DELETED";
            resJson["id"] = id;
            res.code = 200;
            res.set_header("Content-Type", "application/json");
            res.write(resJson.dump());
        } else {
            res.code = 500;
            res.write("Failed to delete record from database");
        }
        res.end();
    });

    // ------------------------------------------------------------------
    // START CROW WEB SERVER
    // ------------------------------------------------------------------
    std::cout << "=======================================================\n";
    std::cout << "🚀 DIGI-LOAN-RISK-GINI RUNNING AT http://localhost:18080\n";
    std::cout << "=======================================================\n";

    app.port(18080).bindaddr("0.0.0.0").multithreaded().run();
    return 0;
}