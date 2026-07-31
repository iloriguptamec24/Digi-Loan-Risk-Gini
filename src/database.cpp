#include "database.hpp"
#include <iostream>
#include <algorithm>

LoanDatabase::LoanDatabase(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Database Open Error: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

LoanDatabase::~LoanDatabase() {
    if (db) sqlite3_close(db);
}

bool LoanDatabase::createTables() {
    const char* sql = "CREATE TABLE IF NOT EXISTS applicants ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "name TEXT NOT NULL, "
                      "income REAL NOT NULL, "
                      "cibil_score INTEGER NOT NULL, "
                      "monthly_debts REAL NOT NULL, "
                      "loan_amount REAL NOT NULL, "
                      "override_status TEXT DEFAULT '');";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool LoanDatabase::addApplicant(const std::string& name, double annualIncome, int cibilScore, double monthlyDebts, double requestedLoan) {
    std::string sql = "INSERT INTO applicants (name, income, cibil_score, monthly_debts, loan_amount) VALUES ('" +
                      name + "', " + std::to_string(annualIncome) + ", " + std::to_string(cibilScore) + 
                      ", " + std::to_string(monthlyDebts) + ", " + std::to_string(requestedLoan) + ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Insert Error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool LoanDatabase::overrideDecision(int applicantId, const std::string& newDecision, const std::string& adminId) {
    std::string sql = "UPDATE applicants SET override_status = '" + newDecision + " (by Admin: " + adminId + ")' WHERE id = " + std::to_string(applicantId) + ";";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Override Error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

PersonalLoanResult LoanDatabase::evaluatePersonalLoan(int cibilScore, double annualIncome, double monthlyDebts, double requestedLoan) {
    PersonalLoanResult res;
    double monthlyIncome = annualIncome / 12.0;
    double dti = (monthlyIncome > 0) ? (monthlyDebts / monthlyIncome) * 100.0 : 100.0;
    double lti = (annualIncome > 0) ? (requestedLoan / annualIncome) : 99.0;

    // Hard Stop Rule Checks (Subprime / Over-leveraged)
    if (cibilScore < 650) res.rejectionReasons.push_back("CIBIL Score < 650 (Below Indian Banking Threshold)");
    if (dti > 45.0) res.rejectionReasons.push_back("Debt-to-Income (DTI) > 45% (FOIR limit breached)");
    if (lti > 0.35) res.rejectionReasons.push_back("Requested loan > 35% of gross annual income");

    if (!res.rejectionReasons.empty()) {
        res.score = 0.0;
        res.decision = "REJECTED";
        res.riskTier = "HIGH";
        res.maxLoanCapacity = annualIncome * 0.10;
        return res;
    }

    // Weighted Scoring Engine Calculations
    double cibilSubScore = (cibilScore >= 775) ? 100.0 : (cibilScore >= 725) ? 85.0 : (cibilScore >= 675) ? 65.0 : 45.0;
    double dtiSubScore = (dti <= 20.0) ? 100.0 : (dti <= 35.0) ? 75.0 : 40.0;
    double ltiSubScore = (lti <= 0.10) ? 100.0 : (lti <= 0.20) ? 80.0 : 50.0;

    double netDisposable = monthlyIncome - monthlyDebts;
    double liquiditySubScore = std::clamp((netDisposable / 25000.0) * 100.0, 0.0, 100.0);

    res.score = (cibilSubScore * 0.40) + (dtiSubScore * 0.30) + (ltiSubScore * 0.15) + (liquiditySubScore * 0.15);
    res.maxLoanCapacity = annualIncome * 0.35 * (res.score / 100.0);

    if (res.score >= 78.0) {
        res.decision = "APPROVED";
        res.riskTier = "LOW";
    } else if (res.score >= 62.0) {
        res.decision = "CONDITIONAL APPROVAL";
        res.riskTier = "MEDIUM";
    } else {
        res.decision = "REJECTED";
        res.riskTier = "HIGH";
        res.rejectionReasons.push_back("Composite score below underwriting threshold");
    }

    return res;
}

crow::json::wvalue LoanDatabase::getApplicantsAsJson() {
    const char* sql = "SELECT id, name, income, cibil_score, monthly_debts, loan_amount, override_status FROM applicants;";
    sqlite3_stmt* stmt;

    crow::json::wvalue list = crow::json::wvalue::list();
    int index = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            double income = sqlite3_column_double(stmt, 2);
            int cibilScore = sqlite3_column_int(stmt, 3);
            double monthlyDebts = sqlite3_column_double(stmt, 4);
            double loanAmount = sqlite3_column_double(stmt, 5);
            const char* overrideTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

            PersonalLoanResult eval = evaluatePersonalLoan(cibilScore, income, monthlyDebts, loanAmount);

            crow::json::wvalue item;
            item["id"] = sqlite3_column_int(stmt, 0);
            item["name"] = name;
            item["cibilScore"] = cibilScore;
            item["loanAmount"] = loanAmount;
            item["score"] = eval.score;
            item["decision"] = (overrideTxt && std::string(overrideTxt).length() > 0) ? std::string(overrideTxt) : eval.decision;
            item["maxLoanCapacity"] = eval.maxLoanCapacity;

            crow::json::wvalue reasons = crow::json::wvalue::list();
            for (size_t i = 0; i < eval.rejectionReasons.size(); ++i) {
                reasons[i] = eval.rejectionReasons[i];
            }
            item["rejectionReasons"] = std::move(reasons);

            list[index++] = std::move(item);
        }
    }
    sqlite3_finalize(stmt);
    return list;
}
