#include "database.hpp"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <filesystem>

// --- LOAN EVALUATION POLYMORPHIC METHODS ---
PersonalLoanResult PersonalLoan::evaluate() const {
    PersonalLoanResult res;
    res.ltiRatio = (annualIncome > 0) ? (requestedLoan / annualIncome) : 999.0;
    res.foirRatio = (annualIncome > 0) ? ((monthlyDebts * 12.0) / annualIncome) : 999.0;
    res.bankRiskFactor = 1.35;

    int rawCibil = cibilObj.getRawScore();
    res.score = (rawCibil * 0.6) + ((1.0 / res.ltiRatio) * 100.0) - (res.bankRiskFactor * 20.0);

    if (rawCibil < 650) res.rejectionReasons.push_back("CIBIL score below 650 requirement");
    if (res.foirRatio > 0.50) res.rejectionReasons.push_back("FOIR exceeds 50% threshold");
    if (res.ltiRatio > 4.0) res.rejectionReasons.push_back("LTI ratio exceeds 4.0x income limit");

    if (res.rejectionReasons.empty()) {
        res.decision = "APPROVED";
    } else if (res.rejectionReasons.size() == 1 && rawCibil >= 620) {
        res.decision = "CONDITIONAL APPROVAL";
    } else {
        res.decision = "REJECTED";
    }
    return res;
}

PersonalLoanResult EducationLoan::evaluate() const {
    PersonalLoanResult res;
    res.ltiRatio = (annualIncome > 0) ? (requestedLoan / annualIncome) : 999.0;
    res.foirRatio = (annualIncome > 0) ? ((monthlyDebts * 12.0) / annualIncome) : 999.0;
    res.bankRiskFactor = 1.00;

    int rawCibil = cibilObj.getRawScore();
    res.score = (rawCibil * 0.65) + ((1.0 / res.ltiRatio) * 110.0);

    if (rawCibil < 600) res.rejectionReasons.push_back("Co-borrower CIBIL below 600 limit");
    if (res.ltiRatio > 6.0) res.rejectionReasons.push_back("Education loan cap exceeded");

    res.decision = res.rejectionReasons.empty() ? "APPROVED" : "REJECTED";
    return res;
}

PersonalLoanResult AutoLoan::evaluate() const {
    PersonalLoanResult res;
    res.ltiRatio = (annualIncome > 0) ? (requestedLoan / annualIncome) : 999.0;
    res.foirRatio = (annualIncome > 0) ? ((monthlyDebts * 12.0) / annualIncome) : 999.0;
    res.bankRiskFactor = 1.10;

    int rawCibil = cibilObj.getRawScore();
    res.score = (rawCibil * 0.60) + ((1.0 / res.ltiRatio) * 90.0);

    if (rawCibil < 620) res.rejectionReasons.push_back("Auto loan CIBIL below 620");
    if (res.foirRatio > 0.55) res.rejectionReasons.push_back("FOIR exceeds 55%");

    res.decision = res.rejectionReasons.empty() ? "APPROVED" : "REJECTED";
    return res;
}

PersonalLoanResult HomeLoan::evaluate() const {
    PersonalLoanResult res;
    res.ltiRatio = (annualIncome > 0) ? (requestedLoan / annualIncome) : 999.0;
    res.foirRatio = (annualIncome > 0) ? ((monthlyDebts * 12.0) / annualIncome) : 999.0;
    res.bankRiskFactor = 0.85;

    int rawCibil = cibilObj.getRawScore();
    res.score = (rawCibil * 0.70) + ((1.0 / res.ltiRatio) * 120.0);

    if (rawCibil < 620) res.rejectionReasons.push_back("Home loan CIBIL threshold missed (620+)");
    if (res.foirRatio > 0.60) res.rejectionReasons.push_back("Home FOIR limit exceeded (60%)");

    res.decision = res.rejectionReasons.empty() ? "APPROVED" : "REJECTED";
    return res;
}

PersonalLoanResult SmallBusinessLoan::evaluate() const {
    PersonalLoanResult res;
    res.ltiRatio = (annualIncome > 0) ? (requestedLoan / annualIncome) : 999.0;
    res.foirRatio = (annualIncome > 0) ? ((monthlyDebts * 12.0) / annualIncome) : 999.0;
    res.bankRiskFactor = 1.25;

    int rawCibil = cibilObj.getRawScore();
    res.score = (rawCibil * 0.55) + ((1.0 / res.ltiRatio) * 80.0);

    if (rawCibil < 680) res.rejectionReasons.push_back("Commercial risk CIBIL below 680");
    if (res.foirRatio > 0.45) res.rejectionReasons.push_back("Business FOIR exceeds 45%");

    res.decision = res.rejectionReasons.empty() ? "APPROVED" : "REJECTED";
    return res;
}

// --- FACTORY PATTERN ---
std::unique_ptr<Loan> LoanFactory::createLoan(
    const std::string& type, 
    const std::string& name, 
    double income, 
    int cibil, 
    double debts, 
    double loanAmt
) {
    if (type == "Education") return std::make_unique<EducationLoan>(name, income, cibil, debts, loanAmt);
    if (type == "Auto")      return std::make_unique<AutoLoan>(name, income, cibil, debts, loanAmt);
    if (type == "Home")      return std::make_unique<HomeLoan>(name, income, cibil, debts, loanAmt);
    if (type == "Business")  return std::make_unique<SmallBusinessLoan>(name, income, cibil, debts, loanAmt);
    return std::make_unique<PersonalLoan>(name, income, cibil, debts, loanAmt);
}

// --- DATABASE OPERATIONS ---
static const std::string DB_PATH = "data/digi_loan_risk.db";

void Database::initDatabase() {
    std::filesystem::create_directories("data");
    sqlite3* db;
    if (sqlite3_open(DB_PATH.c_str(), &db) == SQLITE_OK) {
        const char* sql = "CREATE TABLE IF NOT EXISTS applicants ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "name TEXT NOT NULL, "
                          "loan_type TEXT NOT NULL, "
                          "income REAL NOT NULL, "
                          "cibil_score INTEGER NOT NULL, "
                          "monthly_debts REAL NOT NULL, "
                          "requested_loan REAL NOT NULL, "
                          "lti_ratio REAL NOT NULL, "
                          "foir_ratio REAL NOT NULL, "
                          "risk_factor REAL NOT NULL, "
                          "adjusted_score REAL NOT NULL, "
                          "decision TEXT NOT NULL, "
                          "rejection_reasons TEXT);";
        sqlite3_exec(db, sql, 0, 0, 0);
        sqlite3_close(db);
    }
}

bool Database::saveApplicant(
    const std::string& name,
    const std::string& loanType,
    double income,
    int cibilScore,
    double monthlyDebts,
    double requestedLoan,
    const PersonalLoanResult& result
) {
    sqlite3* db;
    if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) return false;

    std::stringstream reasonsSS;
    for (size_t i = 0; i < result.rejectionReasons.size(); ++i) {
        reasonsSS << result.rejectionReasons[i];
        if (i + 1 < result.rejectionReasons.size()) reasonsSS << "; ";
    }

    const char* sql = "INSERT INTO applicants (name, loan_type, income, cibil_score, monthly_debts, requested_loan, "
                      "lti_ratio, foir_ratio, risk_factor, adjusted_score, decision, rejection_reasons) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, loanType.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, income);
    sqlite3_bind_int(stmt, 4, cibilScore);
    sqlite3_bind_double(stmt, 5, monthlyDebts);
    sqlite3_bind_double(stmt, 6, requestedLoan);
    sqlite3_bind_double(stmt, 7, result.ltiRatio);
    sqlite3_bind_double(stmt, 8, result.foirRatio);
    sqlite3_bind_double(stmt, 9, result.bankRiskFactor);
    sqlite3_bind_double(stmt, 10, result.score);
    sqlite3_bind_text(stmt, 11, result.decision.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 12, reasonsSS.str().c_str(), -1, SQLITE_STATIC);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return success;
}

std::vector<ApplicantReport> Database::getAllReports() {
    std::vector<ApplicantReport> reports;
    sqlite3* db;
    if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) return reports;

    const char* sql = "SELECT id, name, loan_type, income, cibil_score, monthly_debts, requested_loan, "
                      "lti_ratio, foir_ratio, risk_factor, adjusted_score, decision, rejection_reasons "
                      "FROM applicants ORDER BY id DESC;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ApplicantReport rep;
            rep.id = sqlite3_column_int(stmt, 0);
            rep.name = (const char*)sqlite3_column_text(stmt, 1);
            rep.loanType = (const char*)sqlite3_column_text(stmt, 2);
            rep.income = sqlite3_column_double(stmt, 3);
            rep.cibilScore = sqlite3_column_int(stmt, 4);
            rep.monthlyDebts = sqlite3_column_double(stmt, 5);
            rep.requestedLoan = sqlite3_column_double(stmt, 6);
            rep.ltiRatio = sqlite3_column_double(stmt, 7);
            rep.foirRatio = sqlite3_column_double(stmt, 8);
            rep.bankRiskFactor = sqlite3_column_double(stmt, 9);
            rep.score = sqlite3_column_double(stmt, 10);
            rep.decision = (const char*)sqlite3_column_text(stmt, 11);

            const char* reasons = (const char*)sqlite3_column_text(stmt, 12);
            if (reasons && std::string(reasons).length() > 0) {
                std::stringstream ss(reasons);
                std::string item;
                while (std::getline(ss, item, ';')) {
                    if (!item.empty() && item[0] == ' ') item.erase(0, 1);
                    rep.rejectionReasons.push_back(item);
                }
            }

            reports.push_back(rep);
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return reports;
}

bool Database::overrideDecision(int id, const std::string& newDecision) {
    sqlite3* db;
    if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) return false;

    const char* sql = "UPDATE applicants SET decision = ?, rejection_reasons = 'OVERRIDDEN BY ADMIN' WHERE id = ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    sqlite3_bind_text(stmt, 1, newDecision.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return success;
}