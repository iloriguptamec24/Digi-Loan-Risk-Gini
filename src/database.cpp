#include "database.hpp"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <filesystem>

// ============================================================================
// --- LOAN EVALUATION POLYMORPHIC METHODS ---
// ============================================================================

/**
 * @brief Evaluates underwriting risk for an unsecured Personal Loan.
 * @details Checks CIBIL score (>= 650), FOIR (<= 50%), and LTI (<= 4.0x).
 *          Allows conditional approval if only one reason fails and CIBIL >= 620.
 */
PersonalLoanResult PersonalLoan::evaluate() const {
    PersonalLoanResult res;
    
    // Calculate core risk metrics
    res.ltiRatio = (annualIncome > 0) ? (requestedLoan / annualIncome) : 999.0;
    res.foirRatio = (annualIncome > 0) ? ((monthlyDebts * 12.0) / annualIncome) : 999.0;
    res.bankRiskFactor = 1.35; // Higher risk weight for unsecured personal debt

    int rawCibil = cibilObj.getRawScore();
    
    // Compute composite underwriting score
    res.score = (rawCibil * 0.6) + ((1.0 / res.ltiRatio) * 100.0) - (res.bankRiskFactor * 20.0);

    // Business rule checks
    if (rawCibil < 650) res.rejectionReasons.push_back("CIBIL score below 650 requirement");
    if (res.foirRatio > 0.50) res.rejectionReasons.push_back("FOIR exceeds 50% threshold");
    if (res.ltiRatio > 4.0) res.rejectionReasons.push_back("LTI ratio exceeds 4.0x income limit");

    // Decision logic
    if (res.rejectionReasons.empty()) {
        res.decision = "APPROVED";
    } else if (res.rejectionReasons.size() == 1 && rawCibil >= 620) {
        res.decision = "CONDITIONAL APPROVAL";
    } else {
        res.decision = "REJECTED";
    }
    return res;
}

/**
 * @brief Evaluates underwriting risk for Education Loans.
 * @details Features a lower CIBIL requirement (600+) and higher allowable LTI (6.0x)
 *          due to expected future earnings.
 */
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

/**
 * @brief Evaluates underwriting risk for Auto Loans.
 * @details Secured vehicle debt allows a slightly higher FOIR limit (55%).
 */
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

/**
 * @brief Evaluates underwriting risk for Home Loans.
 * @details Collateralized mortgage lending allows lower risk weights and FOIR up to 60%.
 */
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

/**
 * @brief Evaluates underwriting risk for Small Business Loans.
 * @details Imposes strict commercial requirements (CIBIL >= 680, FOIR <= 45%).
 */
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

// ============================================================================
// --- FACTORY PATTERN IMPLEMENTATION ---
// ============================================================================

/**
 * @brief Instantiates concrete derived Loan objects based on the requested category string.
 */
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

// ============================================================================
// --- DATABASE ACCESS LAYER IMPLEMENTATION ---
// ============================================================================

/// Path to the SQLite storage file on disk
static const std::string DB_PATH = "data/digi_loan_risk.db";

/**
 * @brief Creates data directories and initializes the SQLite schema if not already initialized.
 */
void Database::initDatabase() {
    // Ensure parent directory exists
    std::filesystem::create_directories("data");
    
    sqlite3* db;
    if (sqlite3_open(DB_PATH.c_str(), &db) == SQLITE_OK) {
        const char* sql = "CREATE TABLE IF NOT EXISTS applicants ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "pan TEXT NOT NULL, "
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

/**
 * @brief Inserts a new loan application record into the SQLite database using prepared statements.
 */
bool Database::saveApplicant(
    const std::string& pan,
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

    // Serialize vector of rejection reasons into a single semicolon-separated string
    std::stringstream reasonsSS;
    for (size_t i = 0; i < result.rejectionReasons.size(); ++i) {
        reasonsSS << result.rejectionReasons[i];
        if (i + 1 < result.rejectionReasons.size()) reasonsSS << "; ";
    }
    std::string serializedReasons = reasonsSS.str();

    const char* sql = "INSERT INTO applicants (pan, name, loan_type, income, cibil_score, monthly_debts, requested_loan, "
                      "lti_ratio, foir_ratio, risk_factor, adjusted_score, decision, rejection_reasons) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    // Bind parameter values safely to prevent SQL injection
    sqlite3_bind_text(stmt, 1, pan.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, loanType.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, income);
    sqlite3_bind_int(stmt, 5, cibilScore);
    sqlite3_bind_double(stmt, 6, monthlyDebts);
    sqlite3_bind_double(stmt, 7, requestedLoan);
    sqlite3_bind_double(stmt, 8, result.ltiRatio);
    sqlite3_bind_double(stmt, 9, result.foirRatio);
    sqlite3_bind_double(stmt, 10, result.bankRiskFactor);
    sqlite3_bind_double(stmt, 11, result.score);
    sqlite3_bind_text(stmt, 12, result.decision.c_str(), -1, SQLITE_STATIC);
    
    // Use SQLITE_TRANSIENT so SQLite makes an internal copy of the serialized string
    sqlite3_bind_text(stmt, 13, serializedReasons.c_str(), -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return success;
}

/**
 * @brief Retrieves all stored applicant evaluation records in descending order of creation.
 */
std::vector<ApplicantReport> Database::getAllReports() {
    std::vector<ApplicantReport> reports;
    sqlite3* db;
    if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) return reports;

    const char* sql = "SELECT id, pan, name, loan_type, income, cibil_score, monthly_debts, requested_loan, "
                      "lti_ratio, foir_ratio, risk_factor, adjusted_score, decision, rejection_reasons "
                      "FROM applicants ORDER BY id DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ApplicantReport rep;
            rep.id = sqlite3_column_int(stmt, 0);
            
            // Handle null safety for string columns
            const char* panTxt = (const char*)sqlite3_column_text(stmt, 1);
            rep.pan = panTxt ? panTxt : "";

            const char* nameTxt = (const char*)sqlite3_column_text(stmt, 2);
            rep.name = nameTxt ? nameTxt : "";

            const char* typeTxt = (const char*)sqlite3_column_text(stmt, 3);
            rep.loanType = typeTxt ? typeTxt : "";

            rep.income = sqlite3_column_double(stmt, 4);
            rep.cibilScore = sqlite3_column_int(stmt, 5);
            rep.monthlyDebts = sqlite3_column_double(stmt, 6);
            rep.requestedLoan = sqlite3_column_double(stmt, 7);
            rep.ltiRatio = sqlite3_column_double(stmt, 8);
            rep.foirRatio = sqlite3_column_double(stmt, 9);
            rep.bankRiskFactor = sqlite3_column_double(stmt, 10);
            rep.score = sqlite3_column_double(stmt, 11);
            
            const char* decTxt = (const char*)sqlite3_column_text(stmt, 12);
            rep.decision = decTxt ? decTxt : "";

            // Deserialize semicolon-separated rejection reasons into vector
            const char* reasons = (const char*)sqlite3_column_text(stmt, 13);
            if (reasons && std::string(reasons).length() > 0) {
                std::stringstream ss(reasons);
                std::string item;
                while (std::getline(ss, item, ';')) {
                    // Trim leading whitespace
                    if (!item.empty() && item[0] == ' ') item.erase(0, 1);
                    if (!item.empty()) rep.rejectionReasons.push_back(item);
                }
            }

            reports.push_back(rep);
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return reports;
}

/**
 * @brief Overrides an existing underwriting decision for a given record ID.
 */
bool Database::overrideDecision(int id, const std::string& newDecision) {
    sqlite3* db;
    if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) return false;

    const char* sql = "UPDATE applicants SET decision = ?, rejection_reasons = 'OVERRIDDEN BY ADMIN' WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, newDecision.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return success;
}

/**
 * @brief Permanently deletes an applicant record from the database by primary key ID.
 */
bool Database::deleteApplicant(int id) {
    sqlite3* db;
    if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) return false;

    const char* sql = "DELETE FROM applicants WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return success;
}