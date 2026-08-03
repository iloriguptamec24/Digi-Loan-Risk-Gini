/**
 * @file cibil_score.hpp
 * @brief Header definition for CIBIL score modeling and third-party API integration.
 * 
 * Provides the `CIBILScore` class to encapsulate raw credit score evaluation logic, 
 * and `CIBILService` for making network requests to remote CIBIL bureau servers.
 */

#ifndef CIBIL_SCORE_HPP
#define CIBIL_SCORE_HPP

#pragma once

#include <string>

/**
 * @class CIBILScore
 * @brief Encapsulates a raw CIBIL credit score and provides utility methods for risk categorization.
 */
class CIBILScore {
private:
    int rawScore; ///< Raw CIBIL score integer (Typically ranging from 300 to 900)

public:
    /**
     * @brief Construct a new CIBILScore object.
     * @param score Raw CIBIL score integer.
     */
    explicit CIBILScore(int score) : rawScore(score) {}

    /**
     * @brief Retrieves the raw integer credit score.
     * @return int The stored CIBIL score.
     */
    int getRawScore() const { return rawScore; }

    /**
     * @brief Checks if the borrower's score falls into the subprime/high-risk category.
     * @param threshold Score boundary below which borrower is considered subprime (Default: 650).
     * @return true If score is below threshold.
     * @return false If score meets or exceeds threshold.
     */
    bool isSubprime(int threshold = 650) const { return rawScore < threshold; }

    /**
     * @brief Returns normalized score value for algorithmic risk calculations.
     * @return double Score converted to floating point.
     */
    double getNormalizedSubScore() const { return static_cast<double>(rawScore); }
};

/**
 * @class CIBILService
 * @brief Static service wrapper for executing HTTP requests against credit bureau servers.
 */
class CIBILService {
public:
    /**
     * @brief Connects to an external CIBIL mock server via libcurl POST and retrieves live credit score.
     * 
     * Constructs a structured JSON request containing identity verification parameters (PAN, Name, Mobile, DOB),
     * transmits it over HTTP, and parses the response with fallback guarantees.
     * 
     * @param pan Borrower's 10-character Permanent Account Number (e.g. "ABCDE1234F").
     * @param name Borrower's full name.
     * @param mobile Registered 10-digit mobile number.
     * @param dob Borrower's Date of Birth in YYYY-MM-DD format.
     * @return int Evaluated live CIBIL score (300-900), or fallback default (699) on network failure.
     */
        static int fetchLiveCIBILScore(
        const std::string& pan,
        const std::string& name,
        const std::string& mobile = "", // Default to empty string
        const std::string& dob = ""     // Default to empty string
    );
};

#endif // CIBIL_SCORE_HPP