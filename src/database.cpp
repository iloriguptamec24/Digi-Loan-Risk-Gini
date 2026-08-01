#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <vector>
#include <memory>
#include "cibil_score.hpp"

struct PersonalLoanResult {
    double ltiRatio;
    double foirRatio;
    double bankRiskFactor;
    double score;
    std::string decision;
    std::vector<std::string> rejectionReasons;
};

struct ApplicantReport {
    int id;
    std::string name;
    std::string loanType;
    double income;
    int cibilScore;
    double monthlyDebts;
    double requestedLoan;
    double ltiRatio;
    double foirRatio;
    double bankRiskFactor;
    double score;
    std::string decision;
    std::vector<std::string> rejectionReasons;
};

class Loan {
protected:
    std::string name;
    double annualIncome;
    CIBILScore cibilObj;
    double monthlyDebts;
    double requestedLoan;

public:
    Loan(std::string name, double income, int cibil, double debts, double loanAmt)
        : name(name), annualIncome(income), cibilObj(cibil), monthlyDebts(debts), requestedLoan(loanAmt) {}

    virtual ~Loan() = default;
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

class LoanFactory {
public:
    static std::unique_ptr<Loan> createLoan(
        const std::string& type, 
        const std::string& name, 
        double income, 
        int cibil, 
        double debts, 
        double loanAmt
    );
};

class Database {
public:
    static void initDatabase();
    static bool saveApplicant(
        const std::string& name,
        const std::string& loanType,
        double income,
        int cibilScore,
        double monthlyDebts,
        double requestedLoan,
        const PersonalLoanResult& result
    );
    static std::vector<ApplicantReport> getAllReports();
    static bool overrideDecision(int id, const std::string& newDecision);
};

#endif
