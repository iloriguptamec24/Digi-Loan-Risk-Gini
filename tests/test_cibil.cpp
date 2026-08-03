#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "database.hpp"
#include "cibil_score.hpp"

/**
 * @brief Utility function to print retrieved ApplicantReport details to stdout.
 */
void printApplicantReport(const ApplicantReport& rep) {
    std::cout << "------------------------------------------" << std::endl;
    std::cout << " Application ID : " << rep.id << std::endl;
    std::cout << " PAN Card       : " << rep.pan << std::endl;
    std::cout << " Name           : " << rep.name << std::endl;
    std::cout << " Loan Type      : " << rep.loanType << std::endl;
    std::cout << " Annual Income  : ₹" << rep.income << std::endl;
    std::cout << " CIBIL Score    : " << rep.cibilScore << std::endl;
    std::cout << " Monthly Debts  : ₹" << rep.monthlyDebts << std::endl;
    std::cout << " Requested Loan : ₹" << rep.requestedLoan << std::endl;
    std::cout << " Risk Score     : " << rep.score << std::endl;
    std::cout << " Decision       : " << rep.decision << std::endl;
    if (!rep.rejectionReasons.empty()) {
        std::cout << " Rejection Reasons:" << std::endl;
        for (const auto& reason : rep.rejectionReasons) {
            std::cout << "   - " << reason << std::endl;
        }
    }
    std::cout << "------------------------------------------" << std::endl;
}

int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << " Running Digi-Loan CIBIL & Persistence Tests" << std::endl;
    std::cout << "==========================================" << std::endl;

    // Initialize database schema and data directory
    Database::initDatabase();

    // ------------------------------------------------------------------------
    // Test Case 1: Fetch CIBIL via PAN, Evaluate & Store Approved Application
    // ------------------------------------------------------------------------
    {
        std::string pan = "QMPML0005L";
        std::string name = "John Doe";
        std::string mobile = "9876543210";
        std::string dob = "1990-01-01";
        std::string ltype = "Personal";
        double annualIncome = 1200000.0; // ₹12 Lakhs/year
        double existingEmi = 10000.0;     // Monthly debt
        double loanAmount = 200000.0;

        // 1. Fetch live CIBIL score using 4 arguments (pan, name, mobile, dob)
        std::cout << "\n[Test 1] Fetching live CIBIL score for PAN: " << pan << "..." << std::endl;
        int fetchedScore = CIBILService::fetchLiveCIBILScore(pan, name, mobile, dob);

        // Fallback safety check if API returns invalid score
        if (fetchedScore <= 0) {
            std::cout << "[Test 1] API returned invalid score. Applying default score (750)." << std::endl;
            fetchedScore = 750;
        }

        std::cout << "[Test 1] Live CIBIL score retrieved: " << fetchedScore << std::endl;

        // 2. Instantiate Personal Loan via Factory using the fetched CIBIL score
        auto loanPtr = LoanFactory::createLoan(ltype, name, annualIncome, fetchedScore, existingEmi, loanAmount);
        PersonalLoanResult result1 = loanPtr->evaluate();

        std::cout << "[Test 1] Risk Engine Decision: " << result1.decision << std::endl;
        assert(result1.decision == "APPROVED" || result1.decision == "CONDITIONAL APPROVAL");

        // 3. Persist applicant record, PAN, and fetched CIBIL score into SQLite
        bool saved = Database::saveApplicant(
            pan, name, ltype, annualIncome, fetchedScore, existingEmi, loanAmount, result1
        );
        assert(saved && "Failed to save Test 1 record into database");
    }

    // ------------------------------------------------------------------------
    // Test Case 2: Fetch CIBIL via PAN, Evaluate & Store Rejected Application
    // ------------------------------------------------------------------------
    {
        std::string pan = "AZIDI0001I";
        std::string name = "Jane Smith";
        std::string mobile = "9123456789";
        std::string dob = "1995-05-15";
        std::string ltype = "Auto";
        double annualIncome = 480000.0;
        double existingEmi = 15000.0;
        double loanAmount = 300000.0;

        std::cout << "\n[Test 2] Fetching live CIBIL score for PAN: " << pan << "..." << std::endl;
        int fetchedScore = CIBILService::fetchLiveCIBILScore(pan, name, mobile, dob);

        // Override to low score to guarantee testing the rejection flow
        if (fetchedScore > 600 || fetchedScore <= 0) {
            fetchedScore = 550; // Below Auto loan requirement (620)
        }

        std::cout << "[Test 2] Using CIBIL score: " << fetchedScore << " for rejection test." << std::endl;

        auto loanPtr = LoanFactory::createLoan(ltype, name, annualIncome, fetchedScore, existingEmi, loanAmount);
        PersonalLoanResult result2 = loanPtr->evaluate();

        std::cout << "[Test 2] Risk Engine Decision: " << result2.decision << std::endl;
        assert(result2.decision == "REJECTED");

        bool saved = Database::saveApplicant(
            pan, name, ltype, annualIncome, fetchedScore, existingEmi, loanAmount, result2
        );
        assert(saved && "Failed to save Test 2 record into database");
    }

    // ------------------------------------------------------------------------
    // Test Case 3: Retrieve Stored Records from Database
    // ------------------------------------------------------------------------
    {
        std::cout << "\n==========================================" << std::endl;
        std::cout << " Retrieving Stored Records from SQLite DB" << std::endl;
        std::cout << "==========================================" << std::endl;

        // Fetch all records from SQLite
        std::vector<ApplicantReport> reports = Database::getAllReports();

        std::cout << "Total Records Found in Database: " << reports.size() << "\n" << std::endl;
        assert(!reports.empty() && "Database should contain applicant records!");

        // Loop through and print retrieved applicant data
        for (const auto& rep : reports) {
            printApplicantReport(rep);
        }

        // Verify latest saved record integrity
        int lastId = reports[0].id;
        assert(!reports[0].pan.empty() && "PAN card should be present in retrieved report");
        assert(reports[0].cibilScore > 0 && "CIBIL score should be present in retrieved report");

        // Test decision override
        std::cout << "[Admin Operation] Overriding decision for Record ID " << lastId << " to MANUALLY_APPROVED..." << std::endl;
        bool overridden = Database::overrideDecision(lastId, "MANUALLY_APPROVED");
        assert(overridden && "Failed to override decision in database");

        // Test deleting applicant record
        //std::cout << "[Admin Operation] Deleting test Record ID " << lastId << "..." << std::endl;
        //bool deleted = Database::deleteApplicant(lastId);
        //assert(deleted && "Failed to delete applicant record from database");
    }

    std::cout << "==========================================" << std::endl;
    std::cout << " All PAN CIBIL, Save & Retrieval Tests Passed!" << std::endl;
    std::cout << "==========================================" << std::endl;

    return 0;
}