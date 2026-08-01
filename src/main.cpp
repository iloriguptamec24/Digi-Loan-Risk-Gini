#include "crow.h"
#include "database.hpp"
#include <jwt-cpp/jwt.h>
#include <iostream>
#include <filesystem>

// Structure to track user session details extracted from SSO tokens
struct UserSession {
    std::string userId;
    std::string role;
    bool isAuthenticated = false;
};

// Middleware function to inspect incoming Authorization Headers
UserSession authenticateSSOToken(const crow::request& req) {
    UserSession session;
    auto authHeader = req.get_header_value("Authorization");

    // Check if header is missing or improperly formatted
    if (authHeader.empty() || authHeader.find("Bearer ") != 0) {
        return session; // Returns session with isAuthenticated = false
    }

    std::string token = authHeader.substr(7); // Strip "Bearer "

    // Handle simulation tokens passed by the frontend SSO switcher
    if (token == "ADMIN_MOCK_JWT_TOKEN") {
        session.userId = "admin_user_01";
        session.role = "admin";
        session.isAuthenticated = true;
        return session;
    } else if (token == "USER_MOCK_JWT_TOKEN") {
        session.userId = "underwriter_01";
        session.role = "underwriter";
        session.isAuthenticated = true;
        return session;
    }

    // Standard JWT decoding fallback
    try {
        auto decoded = jwt::decode(token);
        session.userId = decoded.get_payload_claim("sub").as_string();
        session.role = decoded.get_payload_claim("role").as_string();
        session.isAuthenticated = true;
    } catch (...) {
        session.isAuthenticated = false;
    }

    return session;
}

int main() {
    crow::SimpleApp app;

    // 1. Ensure the 'data' directory exists programmatically to avoid SQLite path errors
    try {
        std::filesystem::create_directories("data");
    } catch (const std::exception& e) {
        std::cerr << "Directory warning: " << e.what() << std::endl;
    }

    // 2. Initialize Database Connection
    LoanDatabase db("data/digi_loan_risk.db");
    if (!db.createTables()) {
        std::cerr << "Failed to initialize database tables.\n";
        return 1;
    }

    // -------------------------------------------------------------
    // Route 1: Serve Static Web UI Dashboard
    // -------------------------------------------------------------
    CROW_ROUTE(app, "/")
    ([](const crow::request&, crow::response& res) {
        res.set_static_file_info("public/index.html");
        res.end();
    });

    // -------------------------------------------------------------
    // Route 2: GET API - Fetch All Evaluated Applications
    // -------------------------------------------------------------
    CROW_ROUTE(app, "/api/reports").methods(crow::HTTPMethod::GET)
    ([&db](const crow::request& req) {
        UserSession user = authenticateSSOToken(req);
        if (!user.isAuthenticated) {
            return crow::response(401, "SSO Login Required");
        }
        return crow::response(200, db.getApplicantsAsJson());
    });

    // -------------------------------------------------------------
    // Route 3: POST API - Submit New Loan Application
    // -------------------------------------------------------------
    CROW_ROUTE(app, "/api/applicant").methods(crow::HTTPMethod::POST)
    ([&db](const crow::request& req) {
        UserSession user = authenticateSSOToken(req);
        if (!user.isAuthenticated) {
            return crow::response(401, "SSO Login Required");
        }

        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON payload");
        }

        std::string name = body["name"].s();
        
        // Explicitly extract loanType as std::string to resolve type ambiguity
        std::string loanType = "Personal";
        if (body.has("loanType")) {
            loanType = std::string(body["loanType"].s());
        }

        double income = body["income"].d();
        int cibilScore = body["cibilScore"].i();
        double monthlyDebts = body["monthlyDebts"].d();
        double loanAmount = body["loanAmount"].d();

        // Pass all 6 arguments to addApplicant()
        if (db.addApplicant(name, loanType, income, cibilScore, monthlyDebts, loanAmount)) {
            crow::json::wvalue res;
            res["status"] = "success";
            return crow::response(201, res);
        }
        return crow::response(500, "Database insertion failed");
    });

    // -------------------------------------------------------------
    // Route 4: POST API - Admin-Only Decision Override
    // -------------------------------------------------------------
    CROW_ROUTE(app, "/api/admin/override").methods(crow::HTTPMethod::POST)
    ([&db](const crow::request& req) {
        UserSession user = authenticateSSOToken(req);
        if (!user.isAuthenticated) {
            return crow::response(401, "SSO Login Required");
        }
        if (user.role != "admin") {
            return crow::response(403, "Forbidden: Admin privileges required");
        }

        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON payload");
        }

        int applicantId = body["id"].i();
        std::string newDecision = body["overrideDecision"].s();

        if (db.overrideDecision(applicantId, newDecision, user.userId)) {
            crow::json::wvalue res;
            res["status"] = "success";
            return crow::response(200, res);
        }
        return crow::response(500, "Failed to apply decision override");
    });

    std::cout << "\n=======================================================\n";
    std::cout << "🚀 DIGI-LOAN-RISK-GINI RUNNING AT http://localhost:18080\n";
    std::cout << "=======================================================\n\n";

    // Launch asynchronous multithreaded server
    app.port(18080).multithreaded().run();
}