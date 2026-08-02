#include "crow.h"
#include "database.hpp"
#include "cibil_score.hpp"
#include <iostream>

int main() {
    // ------------------------------------------------------------------
    // 1. DATABASE INITIALIZATION
    // Ensure data directory exists and SQLite schema is created on startup.
    // ------------------------------------------------------------------
    Database::initDatabase();

    // Create the primary Crow microservice application engine
    crow::SimpleApp app;

    // ==================================================================
    // ROUTE 1: GET /
    // Serves the static web dashboard (public/index.html)
    // ==================================================================
    CROW_ROUTE(app, "/")
    ([](const crow::request&, crow::response& res) {
        // Serves index.html copied to public/ folder during the CMake build step
        res.set_static_file_info("public/index.html");
        res.end();
    });

    // ==================================================================
    // ROUTE 2: POST /api/applicant
    // Receives applicant details, queries CIBIL API, runs OOP Risk Engine,
    // and saves the underwriting decision into SQLite.
    // ==================================================================
    CROW_ROUTE(app, "/api/applicant").methods("POST"_method)
    ([](const crow::request& req) {
        // Parse incoming HTTP JSON request payload
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON payload");

        // Extract required common financial fields
        std::string name = std::string(body["name"].s());
        std::string loanType = std::string(body["loanType"].s());
        double income = body["income"].d();
        double monthlyDebts = body["monthlyDebts"].d();
        double loanAmount = body["loanAmount"].d();

        int cibilScore = 800; // Default fallback score

        // --------------------------------------------------------------
        // CIBIL SCORE EVALUATION
        // Case A: Guest mode submission with identity parameters (PAN, Mobile, DOB)
        //         -> Fetch real-time score via external CIBIL REST API call.
        // Case B: Underwriter/Admin mode submission
        //         -> Use direct CIBIL score input from the request.
        // --------------------------------------------------------------
        if (body.has("pan") && body.has("mobile") && body.has("dob")) {
            std::string pan = std::string(body["pan"].s());
            std::string mobile = std::string(body["mobile"].s());
            std::string dob = std::string(body["dob"].s());

            // Outbound libcurl HTTP POST call to cibil-mock-server
            cibilScore = CIBILService::fetchLiveCIBILScore(pan, name, mobile, dob);
        } else if (body.has("cibilScore")) {
            cibilScore = body["cibilScore"].i();
        }

        // --------------------------------------------------------------
        // OOP RISK EVALUATION ENGINE
        // 1. Use LoanFactory (Creational Design Pattern) to instantiate loan subclass.
        // 2. Perform polymorphic evaluation (evaluate()) to derive risk score.
        // --------------------------------------------------------------
        auto loanObj = LoanFactory::createLoan(loanType, name, income, cibilScore, monthlyDebts, loanAmount);
        PersonalLoanResult result = loanObj->evaluate();

        // Save application record & decision result into SQLite database
        Database::saveApplicant(name, loanType, income, cibilScore, monthlyDebts, loanAmount, result);

        // Construct JSON response payload
        crow::json::wvalue resJson;
        resJson["status"] = "SUCCESS";
        resJson["fetchedCibilScore"] = cibilScore;
        resJson["decision"] = result.decision;

        return crow::response(201, resJson);
    });

    // ==================================================================
    // ROUTE 3: GET /api/reports
    // Retrieves all evaluated applicant records for staff inspection.
    // ==================================================================
    CROW_ROUTE(app, "/api/reports").methods("GET"_method)
    ([](const crow::request& req) {
        // Query database for all applicant records ordered by ID descending
        auto reports = Database::getAllReports();
        crow::json::wvalue::list reportList;

        // Map internal C++ structs to JSON array response
        for (const auto& rep : reports) {
            crow::json::wvalue item;
            item["id"] = rep.id;
            item["name"] = rep.name;
            item["loanType"] = rep.loanType;
            item["income"] = rep.income;
            item["cibilScore"] = rep.cibilScore;
            item["monthlyDebts"] = rep.monthlyDebts;
            item["requestedLoan"] = rep.requestedLoan;
            item["bankRiskFactor"] = rep.bankRiskFactor;
            item["score"] = rep.score;
            item["decision"] = rep.decision;

            // Map rejection reasons list
            crow::json::wvalue::list reasonsList;
            for (size_t i = 0; i < rep.rejectionReasons.size(); ++i) {
                reasonsList.push_back(rep.rejectionReasons[i]);
            }
            item["rejectionReasons"] = std::move(reasonsList);
            reportList.push_back(std::move(item));
        }

        crow::json::wvalue resJson;
        resJson = std::move(reportList);
        return crow::response(200, resJson);
    });

    // ==================================================================
    // ROUTE 4: POST /api/admin/override
    // Admin privilege route to manually override system decision.
    // ==================================================================
    CROW_ROUTE(app, "/api/admin/override").methods("POST"_method)
    ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("id") || !body.has("overrideDecision")) {
            return crow::response(400, "Invalid payload");
        }

        int id = body["id"].i();
        std::string newDecision = std::string(body["overrideDecision"].s());

        // Update database record status
        bool success = Database::overrideDecision(id, newDecision);
        if (success) {
            crow::json::wvalue resJson;
            resJson["status"] = "OVERRIDDEN";
            return crow::response(200, resJson);
        } else {
            return crow::response(500, "Database update failed");
        }
    });

    // ------------------------------------------------------------------
    // START CROW WEB SERVER
    // Listens on HTTP Port 18080 with multi-threaded async execution.
    // ------------------------------------------------------------------
    std::cout << "=======================================================\n";
    std::cout << "🚀 DIGI-LOAN-RISK-GINI RUNNING AT http://localhost:18080\n";
    std::cout << "=======================================================\n";

    app.port(18080).multithreaded().run();
    return 0;
}
