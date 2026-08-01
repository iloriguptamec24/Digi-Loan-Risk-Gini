#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include "crow.h"
#include "cibil_score.hpp"

// Output evaluation structure
struct PersonalLoanResult {
    double score;
    std::string decision;
    std::string riskTier;
    double bankRiskFactor; // Indian Banking Product Risk Weight
    double maxLoanCapacity;
    std::vector<std::string> rejectionReasons;
};

// =================================================================
// OOP CONCEPT: ABSTRACTION & INHERITANCE
// 'Loan' is an abstract base class. Subclasses inherit fields and override evaluate().
// =================================================================
class Loan {
protected:
    std::string name;
    double annualIncome;
    CIBILScore cibilObj; // OOP Composition & Encapsulation
    double monthlyDebts;
    double requestedLoan;

public:
    Loan(std::string name, double income, int cibil, double debts, double loanAmt)
        : name(name), annualIncome(income), cibilObj(cibil), monthlyDebts(debts), requestedLoan(loanAmt) {}

    virtual ~Loan() = default;

    // =================================================================
    // OOP CONCEPT: POLYMORPHISM
    // Pure virtual method overriden by each specific loan type.
    // =================================================================
    virtual PersonalLoanResult evaluate() const = 0;
};

class PersonalLoan : public Loan {
public:
    using Loan::Loan;
    PersonalLoanResult evaluate() const override;
};

class EducationLoan : public Loan {
public:
    using Loan::Loan;
    PersonalLoanResult evaluate() const override;
};

class AutoLoan : public Loan {
public:
    using Loan::Loan;
    PersonalLoanResult evaluate() const override;
};

class HomeLoan : public Loan {
public:
    using Loan::Loan;
    PersonalLoanResult evaluate() const override;
};

class SmallBusinessLoan : public Loan {
public:
    using Loan::Loan;
    PersonalLoanResult evaluate() const override;
};

// =================================================================
// OOP CONCEPT: FACTORY PATTERN
// Encapsulates object instantiation and returns a polymorphic pointer.
// =================================================================
class LoanFactory {
public:
    static std::unique_ptr<Loan> createLoan(const std::string& type, const std::string& name, double income, int cibil, double debts, double loanAmt);
};

class LoanDatabase {
private:
    sqlite3* db;

public:
    LoanDatabase(const std::string& db_path);
    ~LoanDatabase();

    bool createTables();
    bool addApplicant(const std::string& name, const std::string& loanType, double annualIncome, int cibilScore, double monthlyDebts, double requestedLoan);
    bool overrideDecision(int applicantId, const std::string& newDecision, const std::string& adminId);
    crow::json::wvalue getApplicantsAsJson();
};

#endif
