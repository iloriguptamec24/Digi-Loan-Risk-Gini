#include "database.hpp"
#include <iostream>
#include <algorithm>

// Factory Pattern Implementation
std::unique_ptr<Loan> LoanFactory::createLoan(const std::string& type, const std::string& name, double income, int cibil, double debts, double loanAmt) {
    if (type == "Education") return std::make_unique<EducationLoan>(name, income, cibil, debts, loanAmt);
    if (type == "Auto")      return std::make_unique<AutoLoan>(name, income, cibil, debts, loanAmt);
    if (type == "Home")      return std::make_unique<HomeLoan>(name, income, cibil, debts, loanAmt);
    if (type == "Business")  return std::make_unique<SmallBusinessLoan>(name, income, cibil, debts, loanAmt);
    return std::make_unique<PersonalLoan>(name, income, cibil, debts, loanAmt);
}

// -----------------------------------------------------------------
// 1. Personal Loan (Unsecured Risk Multiplier: 1.35)
// -----------------------------------------------------------------
PersonalLoanResult PersonalLoan::evaluate() const {
    PersonalLoanResult res;
    res.bankRiskFactor = 1.35;
    double monthlyIncome = annualIncome / 12.0;
    double dti = (monthlyIncome > 0) ? (monthlyDebts / monthlyIncome) * 100.0 : 100.0;
    double lti = (annualIncome > 0) ? (requestedLoan / annualIncome) : 99.0;

    if (cibilObj.isSubprime(650)) res.rejectionReasons.push_back("Personal Loan: CIBIL < 650 (" + cibilObj.getRatingTierString() + ")");
    if (dti > 45.0)       res.rejectionReasons.push_back("Personal Loan: FOIR/DTI > 45%");
    if (lti > 0.35)       res.rejectionReasons.push_back("Personal Loan: Loan > 35% of Gross Income");

    if (!res.rejectionReasons.empty()) {
        res.score = 0.0; res.decision = "REJECTED"; res.riskTier = "HIGH";
        res.maxLoanCapacity = annualIncome * 0.10;
        return res;
    }

    double cibilSub = cibilObj.getNormalizedSubScore();
    double dtiSub   = (dti <= 20) ? 100 : (dti <= 35) ? 75 : 40;
    double ltiSub   = (lti <= 0.10) ? 100 : 70;
    
    double rawScore = (cibilSub * 0.40) + (dtiSub * 0.30) + (ltiSub * 0.30);
    res.score = std::max(0.0, rawScore / res.bankRiskFactor);

    res.maxLoanCapacity = (annualIncome * 0.35 * (res.score / 100.0));
    res.decision = (res.score >= 58.0) ? "APPROVED" : (res.score >= 45.0) ? "CONDITIONAL APPROVAL" : "REJECTED";
    res.riskTier = (res.score >= 58.0) ? "LOW" : "MEDIUM";
    return res;
}

// -----------------------------------------------------------------
// 2. Education Loan (Priority Sector Risk Multiplier: 1.10)
// -----------------------------------------------------------------
PersonalLoanResult EducationLoan::evaluate() const {
    PersonalLoanResult res;
    res.bankRiskFactor = 1.10;
    double monthlyIncome = annualIncome / 12.0;
    double dti = (monthlyIncome > 0) ? (monthlyDebts / monthlyIncome) * 100.0 : 100.0;

    if (cibilObj.isSubprime(600)) res.rejectionReasons.push_back("Education Loan: Co-borrower CIBIL < 600");
    if (dti > 50.0)       res.rejectionReasons.push_back("Education Loan: Household DTI > 50%");

    if (!res.rejectionReasons.empty()) {
        res.score = 0.0; res.decision = "REJECTED"; res.riskTier = "HIGH";
        return res;
    }

    double cibilSub = cibilObj.getNormalizedSubScore();
    double dtiSub   = (dti <= 30) ? 100 : 65;
    double rawScore = (cibilSub * 0.35) + (dtiSub * 0.35) + 30.0;

    res.score = rawScore / res.bankRiskFactor;
    res.maxLoanCapacity = annualIncome * 1.5;
    res.decision = (res.score >= 55.0) ? "APPROVED" : "CONDITIONAL APPROVAL";
    res.riskTier = "MEDIUM";
    return res;
}

// -----------------------------------------------------------------
// 3. Auto Loan (Depreciating Asset Risk Multiplier: 1.15)
// -----------------------------------------------------------------
PersonalLoanResult AutoLoan::evaluate() const {
    PersonalLoanResult res;
    res.bankRiskFactor = 1.15;
    double monthlyIncome = annualIncome / 12.0;
    double dti = (monthlyIncome > 0) ? (monthlyDebts / monthlyIncome) * 100.0 : 100.0;

    if (cibilObj.isSubprime(620)) res.rejectionReasons.push_back("Auto Loan: CIBIL < 620");
    if (dti > 50.0)       res.rejectionReasons.push_back("Auto Loan: FOIR DTI > 50%");

    if (!res.rejectionReasons.empty()) {
        res.score = 0.0; res.decision = "REJECTED"; res.riskTier = "HIGH";
        return res;
    }

    double cibilSub = cibilObj.getNormalizedSubScore();
    double dtiSub   = (dti <= 35) ? 100 : 60;
    double rawScore = (cibilSub * 0.35) + (dtiSub * 0.45) + 20.0;

    res.score = rawScore / res.bankRiskFactor;
    res.maxLoanCapacity = annualIncome * 0.85;
    res.decision = (res.score >= 60.0) ? "APPROVED" : "CONDITIONAL APPROVAL";
    res.riskTier = "MEDIUM";
    return res;
}

// -----------------------------------------------------------------
// 4. Home Loan (Secured Property Asset Risk Multiplier: 0.85)
// -----------------------------------------------------------------
PersonalLoanResult HomeLoan::evaluate() const {
    PersonalLoanResult res;
    res.bankRiskFactor = 0.85;
    double monthlyIncome = annualIncome / 12.0;
    double dti = (monthlyIncome > 0) ? (monthlyDebts / monthlyIncome) * 100.0 : 100.0;

    if (cibilObj.isSubprime(620)) res.rejectionReasons.push_back("Home Loan: CIBIL < 620");
    if (dti > 55.0)       res.rejectionReasons.push_back("Home Loan: FOIR DTI > 55%");

    if (!res.rejectionReasons.empty()) {
        res.score = 0.0; res.decision = "REJECTED"; res.riskTier = "HIGH";
        return res;
    }

    double cibilSub = cibilObj.getNormalizedSubScore();
    double dtiSub   = (dti <= 40) ? 100 : 70;
    double rawScore = (cibilSub * 0.35) + (dtiSub * 0.45) + 20.0;

    res.score = std::min(100.0, rawScore / res.bankRiskFactor);
    res.maxLoanCapacity = annualIncome * 5.0;
    res.decision = (res.score >= 65.0) ? "APPROVED" : "CONDITIONAL APPROVAL";
    res.riskTier = "LOW";
    return res;
}

// -----------------------------------------------------------------
// 5. Small Business Loan (Commercial Volatility Risk Multiplier: 1.25)
// -----------------------------------------------------------------
PersonalLoanResult SmallBusinessLoan::evaluate() const {
    PersonalLoanResult res;
    res.bankRiskFactor = 1.25;
    double monthlyIncome = annualIncome / 12.0;
    double dti = (monthlyIncome > 0) ? (monthlyDebts / monthlyIncome) * 100.0 : 100.0;

    if (cibilObj.isSubprime(680)) res.rejectionReasons.push_back("Small Business Loan: CIBIL < 680");
    if (dti > 40.0)       res.rejectionReasons.push_back("Small Business Loan: DTI > 40%");

    if (!res.rejectionReasons.empty()) {
        res.score = 0.0; res.decision = "REJECTED"; res.riskTier = "HIGH";
        return res;
    }

    double cibilSub = cibilObj.getNormalizedSubScore();
    double dtiSub   = (dti <= 25) ? 100 : 60;
    double liqSub   = std::clamp(((monthlyIncome - monthlyDebts) / 50000.0) * 100.0, 0.0, 100.0);

    double rawScore = (cibilSub * 0.35) + (dtiSub * 0.40) + (liqSub * 0.25);
    res.score = rawScore / res.bankRiskFactor;

    res.maxLoanCapacity = annualIncome * 1.2;
    res.decision = (res.score >= 62.0) ? "APPROVED" : "CONDITIONAL APPROVAL";
    res.riskTier = "MEDIUM";
    return res;
}

// SQLite Database Operations
LoanDatabase::LoanDatabase(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) db = nullptr;
}

LoanDatabase::~LoanDatabase() {
    if (db) sqlite3_close(db);
}

bool LoanDatabase::createTables() {
    const char* sql = "CREATE TABLE IF NOT EXISTS applicants ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "name TEXT NOT NULL, "
                      "loan_type TEXT DEFAULT 'Personal', "
                      "income REAL NOT NULL, "
                      "cibil_score INTEGER NOT NULL, "
                      "monthly_debts REAL NOT NULL, "
                      "loan_amount REAL NOT NULL, "
                      "override_status TEXT DEFAULT '');";
    char* errMsg = nullptr;
    return (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) == SQLITE_OK);
}

bool LoanDatabase::addApplicant(const std::string& name, const std::string& loanType, double annualIncome, int cibilScore, double monthlyDebts, double requestedLoan) {
    std::string sql = "INSERT INTO applicants (name, loan_type, income, cibil_score, monthly_debts, loan_amount) VALUES ('" +
                      name + "', '" + loanType + "', " + std::to_string(annualIncome) + ", " + std::to_string(cibilScore) + 
                      ", " + std::to_string(monthlyDebts) + ", " + std::to_string(requestedLoan) + ");";
    char* errMsg = nullptr;
    return (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) == SQLITE_OK);
}

bool LoanDatabase::overrideDecision(int applicantId, const std::string& newDecision, const std::string& adminId) {
    std::string sql = "UPDATE applicants SET override_status = '" + newDecision + " (by Admin: " + adminId + ")' WHERE id = " + std::to_string(applicantId) + ";";
    char* errMsg = nullptr;
    return (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) == SQLITE_OK);
}

crow::json::wvalue LoanDatabase::getApplicantsAsJson() {
    const char* sql = "SELECT id, name, loan_type, income, cibil_score, monthly_debts, loan_amount, override_status FROM applicants;";
    sqlite3_stmt* stmt;
    crow::json::wvalue list = crow::json::wvalue::list();
    int index = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::string loanType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            double income = sqlite3_column_double(stmt, 3);
            int cibilScore = sqlite3_column_int(stmt, 4);
            double monthlyDebts = sqlite3_column_double(stmt, 5);
            double loanAmount = sqlite3_column_double(stmt, 6);
            const char* overrideTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

            // =================================================================
            // OOP CONCEPT: POLYMORPHISM IN ACTION
            // Factory returns unique_ptr<Loan> (Base pointer), calling overriden
            // evaluate() at runtime based on loan type.
            // =================================================================
            std::unique_ptr<Loan> loanObj = LoanFactory::createLoan(loanType, name, income, cibilScore, monthlyDebts, loanAmount);
            PersonalLoanResult eval = loanObj->evaluate();

            crow::json::wvalue item;
            item["id"] = sqlite3_column_int(stmt, 0);
            item["name"] = name;
            item["loanType"] = loanType;
            item["cibilScore"] = cibilScore;
            item["bankRiskFactor"] = eval.bankRiskFactor;
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
