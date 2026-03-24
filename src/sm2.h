#pragma once
#include <QString>
#include <QDate>
#include <cmath>
#include <algorithm>

struct SM2Result {
    int interval;
    int repetitions;
    double easeFactor;
    QString nextReview; // YYYY-MM-DD
};

// Direct port of the SM-2 algorithm from the React prototype.
// q: 0=Forgot, 3=Hard, 4=Good, 5=Easy
inline SM2Result sm2(int q, int interval, int repetitions, double easeFactor) {
    if (q < 3) {
        repetitions = 0;
        interval    = 1;
    } else {
        repetitions++;
        if      (repetitions == 1) interval = 1;
        else if (repetitions == 2) interval = 6;
        else                       interval = static_cast<int>(std::round(interval * easeFactor));
        easeFactor = std::max(1.3, easeFactor + 0.1 - (5 - q) * (0.08 + (5 - q) * 0.02));
    }
    QString nextReview = QDate::currentDate().addDays(interval).toString("yyyy-MM-dd");
    return {interval, repetitions, easeFactor, nextReview};
}

inline int qualityFromRating(const QString &rating) {
    if (rating == "Forgot") return 0;
    if (rating == "Hard")   return 3;
    if (rating == "Good")   return 4;
    if (rating == "Easy")   return 5;
    return 4;
}
