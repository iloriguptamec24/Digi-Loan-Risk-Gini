#include "cibil_score.hpp"
#include <algorithm>

CIBILScore::CIBILScore(int scoreVal) : score(scoreVal) {}

int CIBILScore::getValue() const { return score; }

bool CIBILScore::isValid() const { return score >= 300 && score <= 900; }

bool CIBILScore::isSubprime(int minimumThreshold) const { return score < minimumThreshold; }

CIBILRatingTier CIBILScore::getRatingTier() const {
    if (score >= 775) return CIBILRatingTier::EXCELLENT;
    if (score >= 725) return CIBILRatingTier::GOOD;
    if (score >= 650) return CIBILRatingTier::FAIR;
    return CIBILRatingTier::SUBPRIME;
}

std::string CIBILScore::getRatingTierString() const {
    switch (getRatingTier()) {
        case CIBILRatingTier::EXCELLENT: return "EXCELLENT";
        case CIBILRatingTier::GOOD:      return "GOOD";
        case CIBILRatingTier::FAIR:      return "FAIR";
        case CIBILRatingTier::SUBPRIME:   return "SUBPRIME";
        default:                         return "UNKNOWN";
    }
}

double CIBILScore::getNormalizedSubScore() const {
    if (score >= 775) return 100.0;
    if (score >= 725) return 85.0;
    if (score >= 675) return 65.0;
    if (score >= 650) return 45.0;
    return 0.0;
}
