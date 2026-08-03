#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <vector>
#include <memory>
#include "cibil_score.hpp"

/**
 * @struct PersonalLoanResult
 * @brief Encapsulates the output metrics and evaluation decision produced by a Loan object.
 */
struct PersonalLoanResult {
    double ltiRatio;                           ///< Loan-to-Income (LTI) ratio calculated for the applicant
    double foirRatio;                          ///< Fixed Obligation to Income Ratio (FOIR) percentage
    double bankRiskFactor;                     ///< Calculated risk score specific to the lending institution
    double score;                              ///< Overall composite credit evaluation score
    std::string decision;                      ///< Underwriting verdict (e.g., "APPROVED", "REJECTED", "REFER")
    std::vector<std::string> rejectionReasons; ///< List of policy compliance violations if rejected
};

/**
 * @struct ApplicantReport
 * @brief Data transfer object representing a persistent applicant record retrieved from the database.
 */
struct ApplicantReport {
    int id;                                    ///< Unique database primary key identifier
    std::string pan;                           ///< Permanent Account Number (PAN) of the applicant
    std::string name;                          ///< Full legal name of the applicant
    std::string loanType;                      ///< Product segment (e.g., "Personal", "Home", "Auto")
    double income;                             ///< Stated annual income in local currency
    int cibilScore;                            ///< Applicant's CIBIL credit bureau score (300–900)
    double monthlyDebts;                       ///< Total existing monthly debt obligations
    double requestedLoan;                      ///< Principal loan amount requested
    double ltiRatio;                           ///< Saved Loan-to-Income ratio metric
    double foirRatio;                          ///< Saved Fixed Obligation to Income Ratio metric
    double bankRiskFactor;                     ///< Saved bank internal risk factor metric
    double score;                              ///< Saved credit decision engine composite score
    std::string decision;                      ///< Final saved decision ("APPROVED", "REJECTED", etc.)
    std::vector<std::string> rejectionReasons; ///< Deserialized list of rejection reasons
};

/**
 * @class Loan
 * @brief Abstract base class defining common state and evaluation interface for all loan domains.
 */
class Loan {
protected:
    std::string name;                          ///< Applicant name
    double annualIncome;                       ///< Stated annual gross income
    CIBILScore cibilObj;                       ///< CIBIL domain object handling credit score rules
    double monthlyDebts;                       ///< Current monthly recurring debt obligations
    double requestedLoan;                      ///< Desired principal amount

public:
    /**
     * @brief Constructs a base Loan object with core financial metrics.
     * @param name Name of applicant.
     * @param income Gross annual income.
     * @param cibil Raw CIBIL score integer.
     * @param debts Total monthly debt payments.
     * @param loanAmt Requested principal loan amount.
     */
    Loan(std::string name, double income, int cibil, double debts, double loanAmt)
        : name(name), annualIncome(income), cibilObj(cibil), monthlyDebts(debts), requestedLoan(loanAmt) {}

    virtual ~Loan() = default;

    /**
     * @brief Evaluates risk parameters and computes credit outcome.
     * @return PersonalLoanResult containing score metrics and decision outcome.
     */
    virtual PersonalLoanResult evaluate() const = 0;
};

// ============================================================================
// Derived Loan Products
// Implement polymorphic underwriting rules for specific financial products.
// ============================================================================

/**
 * @class PersonalLoan
 * @brief Evaluates unsecured personal loan applications.
 */
class PersonalLoan : public Loan {
public:
    using Loan::Loan;
    PersonalLoanResult evaluate() const override;
};

/**
 * @class EducationLoan
 * @brief Evaluates student and education-related loan applications.
 */
class EducationLoan : public Loan {
public:
    using Loan::Loan;
    PersonalLoanResult evaluate() const override;
};

/**
 * @class AutoLoan
 * @brief Evaluates secured auto/vehicle purchase loan applications.
 */
class AutoLoan : public Loan {
public:
    using Loan::Loan;
    PersonalLoanResult evaluate() const override;
};

/**
 * @class HomeLoan
 * @brief Evaluates secured residential mortgage/home loan applications.
 */
class HomeLoan : public Loan {
public:
    using Loan::Loan;
    PersonalLoanResult evaluate() const override;
};

/**
 * @class SmallBusinessLoan
 * @brief Evaluates commercial and small business credit line applications.
 */
class SmallBusinessLoan : public Loan {
public:
    using Loan::Loan;
    PersonalLoanResult evaluate() const override;
};

/**
 * @class LoanFactory
 * @brief Factory class implementing the Factory Pattern to instantiate concrete Loan instances.
 */
class LoanFactory {
public:
    /**
     * @brief Instantiates a concrete Loan instance according to the requested loan product string.
     * @param type Category string ("Personal", "Education", "Auto", "Home", "SmallBusiness").
     * @param name Applicant name.
     * @param income Stated annual gross income.
     * @param cibil Raw CIBIL bureau score integer.
     * @param debts Total existing monthly debt obligations.
     * @param loanAmt Requested principal loan amount.
     * @return std::unique_ptr<Loan> Polymorphic smart pointer to the derived loan instance.
     */
    static std::unique_ptr<Loan> createLoan(
        const std::string& type, 
        const std::string& name, 
        double income, 
        int cibil, 
        double debts, 
        double loanAmt
    );
};

/**
 * @class Database
 * @brief Data Access Layer (DAL) managing database connections, queries, and mutations.
 */
class Database {
public:
    /**
     * @brief Initializes database tables, indexes, and connections if they do not already exist.
     */
    static void initDatabase();
    
    /**
     * @brief Persists a new loan application record into the persistent storage backend.
     * 
     * @param pan Applicant's unique Permanent Account Number (PAN).
     * @param name Full name of the applicant.
     * @param loanType Product category identifier string.
     * @param income Annual gross income.
     * @param cibilScore CIBIL credit score integer.
     * @param monthlyDebts Total monthly debt payments.
     * @param requestedLoan Requested loan principal amount.
     * @param result Computed evaluation metrics and risk engine decision output.
     * @return true If the record was inserted successfully, false on error.
     */
    static bool saveApplicant(
        const std::string& pan,
        const std::string& name,
        const std::string& loanType,
        double income,
        int cibilScore,
        double monthlyDebts,
        double requestedLoan,
        const PersonalLoanResult& result
    );
    
    /**
     * @brief Fetches all saved applicant assessment reports from the database.
     * @return std::vector<ApplicantReport> Collection of populated ApplicantReport structures.
     */
    static std::vector<ApplicantReport> getAllReports();

    /**
     * @brief Overrides an automated underwriting decision for a specific loan record by ID.
     * 
     * @param id Database primary key identifier of the targeted application.
     * @param newDecision Overridden status string (e.g., "MANUALLY_APPROVED").
     * @return true If update succeeded, false if application ID was missing or operation failed.
     */
    static bool overrideDecision(int id, const std::string& newDecision);
    
    /**
     * @brief Removes an applicant record permanently from persistent storage by ID.
     * 
     * @param id Database primary key identifier of the applicant to delete.
     * @return true If deletion succeeded, false if record was not found or deletion failed.
     */
    static bool deleteApplicant(int id);
};

#endif // DATABASE_HPP