#ifndef CIBIL_SCORE_HPP
#define CIBIL_SCORE_HPP

#include <string>

enum class CIBILRatingTier {
    EXCELLENT,  // 775 - 900
    GOOD,       // 725 - 774
    FAIR,       // 650 - 724
    SUBPRIME    // 300 - 649 (Hard Reject Threshold in India)
};

// =================================================================
// OOP CONCEPT: ENCAPSULATION
// Private internal state (score) accessed through controlled methods.
// =================================================================
class CIBILScore {
private:
    int score;

public:
    explicit CIBILScore(int scoreVal = 300);

    int getValue() const;
    bool isValid() const;
    bool isSubprime(int minimumThreshold = 650) const;
    
    CIBILRatingTier getRatingTier() const;
    std::string getRatingTierString() const;
    double getNormalizedSubScore() const;
};

#endif
