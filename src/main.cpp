#include "crow.h"
#include "database.hpp"
#include <jwt-cpp/jwt.h>
#include <iostream>

struct UserSession {
    std::string userId;
    std::string role;
    bool isAuthenticated = false;
};

UserSession authenticateSSOToken(const crow::request& req) {
    UserSession session;
    auto authHeader = req.get_header_value("Authorization");

    if (authHeader.empty() || authHeader.find("Bearer ") != 0) {
        return session;
    }

    std::string token = authHeader.substr(7);

    // Mock verification for testing frontend controls
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

    return session;
}

int main() {
    crow::SimpleApp app;
    LoanDatabase db("data/digi_loan_risk.db");
    db.createTables();

    // Serve Frontend Landing Page
    CROW_ROUTE(app, "/")
    ([](const crow::request&, crow::response& res) {
        res.set_static_file_info("public/index.html");
        res.end();
    });

    // API: Fetch All Applications
    CROW_ROUTE(app, "/api/reports").methods(crow::HTTPMethod::GET)
    ([&db](const crow::request& req) {
        UserSession user = authenticateSSOToken(req);
        if (!user.isAuthenticated) return crow::response(401, "SSO Login Required");
        return crow::response(200, db.getApplicantsAsJson());
    });

    // API: Submit Application
    CROW_ROUTE(app, "/api/applicant").methods(crow::HTTPMethod::POST)
    ([&db](const crow::request& req) {
        UserSession user = authenticateSSOToken(req);
        if (!user.isAuthenticated) return crow::response(401, "SSO Login Required");

        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        std::string name = body["name"].s();
        double income = body["income"].d();
        int cibilScore = body["cibilScore"].i();
        double monthlyDebts = body["monthlyDebts"].d();
        double loanAmount = body["loanAmount"].d();

        if (db.addApplicant(name, income, cibilScore, monthlyDebts, loanAmount)) {
            crow::json::wvalue res;
            res["status"] = "success";
            return crow::response(201, res);
        }
        return crow::response(500, "Database insertion failed");
    });

    // API: ADMIN ONLY Override
    CROW_ROUTE(app, "/api/admin/override").methods(crow::HTTPMethod::POST)
    ([&db](const crow::request& req) {
        UserSession user = authenticateSSOToken(req);
        if (!user.isAuthenticated) return crow::response(401, "SSO Login Required");
        if (user.role != "admin") return crow::response(403, "Forbidden: Admin privileges required");

        auto body = crow::json::load(req.body);
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

    app.port(18080).multithreaded().run();
}
