#include <iostream>
#include <string>
#include "cibil_score.hpp"

int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << "🧪 TESTING CIBILService::fetchLiveCIBILScore" << std::endl;
    std::cout << "==========================================" << std::endl;

    // Test Case 1: Standard Applicant Data
    std::string pan = "AZIDI0001I";
    std::string name = "Priya Kumar";
    std::string mobile = "9876543210";
    std::string dob = "1995-05-15";

    std::cout << "\n[Test 1] Fetching live score for:" << std::endl;
    std::cout << " - PAN:    " << pan << std::endl;
    std::cout << " - Name:   " << name << std::endl;
    std::cout << " - Mobile: " << mobile << std::endl;
    std::cout << " - DOB:    " << dob << std::endl;

    int score = CIBILService::fetchLiveCIBILScore(pan, name, mobile, dob);

    std::cout << "\n📊 RESULT:" << std::endl;
    std::cout << "Returned CIBIL Score: " << score << std::endl;

    // Validation Assertions
    if (score >= 300 && score <= 900) {
        std::cout << "✅ TEST PASSED: CIBIL score is within valid range (300-900)." << std::endl;
    } else {
        std::cout << "❌ TEST FAILED: Invalid CIBIL score returned!" << std::endl;
    }

    std::cout << "==========================================" << std::endl;
    return 0;
}