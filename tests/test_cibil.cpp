#include <iostream>
#include <string>
#include <memory>
#include "cibil_score.hpp"
#include "database.hpp"

int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << "🧪 TESTING CIBILService::fetchLiveCIBILScore" << std::endl;
    std::cout << "==========================================" << std::endl;

    // ------------------------------------------------------------------
    // 1. DATABASE INITIALIZATION
    // Ensure data directory exists and SQLite schema is created on startup.
    // ------------------------------------------------------------------
    std::cout << "\n🛠️ Initializing Database..." << std::endl;
    Database::initDatabase();

    // ------------------------------------------------------------------
    // 2. APPLICANT TEST DATA
    // ------------------------------------------------------------------
    std::string pan = "PSFGF0003W";
    std::string name = "Priya Kumari";
    std::string mobile = "9876543210";
    std::string dob = "1995-05-15";
    std::string ltype = "AutoLoan";

    double loanAmount = 4000000.0;
    double existingEmi = 500.0;
    double monthlyIncome = 70000.0;

    std::cout << "\n[Test 1] Fetching live score for:" << std::endl;
    std::cout << " - PAN:            " << pan << std::endl;
    std::cout << " - Name:           " << name << std::endl;
    std::cout << " - Mobile:         " << mobile << std::endl;
    std::cout << " - DOB:            " << dob << std::endl;
    std::cout << " - Loan Type:      " << ltype << std::endl;
    std::cout << " - Requested Amt:  ₹" << loanAmount << std::endl;

    // ------------------------------------------------------------------
    // 3. FETCH LIVE CIBIL SCORE
    // ------------------------------------------------------------------
    int score = CIBILService::fetchLiveCIBILScore(pan, name, mobile, dob);

    // ------------------------------------------------------------------
    // 4. EVALUATE LOAN & SAVE APPLICANT RECORD
    // ------------------------------------------------------------------
    
    auto loanObj1 = LoanFactory::createLoan(ltype, name, monthlyIncome, score, existingEmi, loanAmount);

    if (loanObj1) {
        PersonalLoanResult result1 = loanObj1->evaluate();

        // Save application record & decision result into SQLite database
        Database::saveApplicant(name, ltype, loanAmount, score, existingEmi, monthlyIncome, result1);
        std::cout << "💾 [DB Success] Applicant record saved successfully." << std::endl;
    } else {
        std::cerr << "❌ [Error] Failed to instantiate loan object from LoanFactory!" << std::endl;
    }

    // ------------------------------------------------------------------
    // 5. VALIDATION ASSERTION
    // ------------------------------------------------------------------
    std::cout << "\n📊 RESULT:" << std::endl;
    std::cout << "Returned CIBIL Score: " << score << std::endl;

    if (score >= 300 && score <= 900) {
        std::cout << "✅ TEST PASSED: CIBIL score is within valid range (300-900)." << std::endl;
    } else {
        std::cout << "❌ TEST FAILED: Invalid CIBIL score returned!" << std::endl;
    }

    std::cout << "==========================================" << std::endl;
    return 0;
}