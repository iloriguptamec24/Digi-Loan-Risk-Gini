#ifndef CIBIL_SCORE_HPP
#define CIBIL_SCORE_HPP

#include <string>

class CIBILScore {
private:
    int rawScore;

public:
    explicit CIBILScore(int score) : rawScore(score) {}
    int getRawScore() const { return rawScore; }
    bool isSubprime(int threshold = 650) const { return rawScore < threshold; }
    double getNormalizedSubScore() const { return static_cast<double>(rawScore); }
};

class CIBILService {
public:
    // Calls external CIBIL mock server via curl POST and returns the live CIBIL score
    static int fetchLiveCIBILScore(const std::string& pan, const std::string& name, const std::string& mobile, const std::string& dob);
};

#endif
