#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include "crow.h"

struct PersonalLoanResult {
    double score;
    std::string decision;
    std::string riskTier;
    double maxLoanCapacity;
    std::vector<std::string> rejectionReasons;
};

class LoanDatabase {
private:
    sqlite3* db;

public:
    LoanDatabase(const std::string& db_path);
    ~LoanDatabase();

    bool createTables();
    bool addApplicant(const std::string& name, double annualIncome, int cibilScore, double monthlyDebts, double requestedLoan);
    bool overrideDecision(int applicantId, const std::string& newDecision, const std::string& adminId);
    crow::json::wvalue getApplicantsAsJson();
    static PersonalLoanResult evaluatePersonalLoan(int cibilScore, double annualIncome, double monthlyDebts, double requestedLoan);
};

#endif
