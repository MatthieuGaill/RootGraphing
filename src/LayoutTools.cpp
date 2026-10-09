#include "LayoutTools.hh"

#include <algorithm>
#include <cmath>
#include <cctype>
#include "THLimitsFinder.h"

namespace ROOTEnhancedGraphing {
namespace Layout {

Sizes Sizes::Scaled(Double_t scale) const {
    Sizes s = *this;
    for (Double_t* v : {&s.label, &s.title, &s.tick, &s.labelOffset, &s.titleGap, &s.outer, &s.minRight, &s.minTop, &s.legend}) {
        *v *= scale;
    }
    s.scale = scale;
    return s;
}

namespace {
// Helvetica advance widths (1/1000 em) for ASCII 32..126
constexpr Short_t kHelveticaWidths[95] = {
    278, 278, 355, 556, 556, 889, 667, 191, 333, 333, 389, 584, 278, 333, 278, 278, // ' ' .. '/'
    556, 556, 556, 556, 556, 556, 556, 556, 556, 556,                               // '0' .. '9'
    278, 278, 584, 584, 584, 556, 1015,                                             // ':' .. '@'
    667, 667, 722, 722, 667, 611, 778, 722, 278, 500, 667, 556, 833,                // 'A' .. 'M'
    722, 778, 667, 778, 722, 667, 611, 722, 667, 944, 667, 667, 611,                // 'N' .. 'Z'
    278, 278, 278, 469, 556, 333,                                                   // '[' .. '`'
    556, 556, 500, 556, 556, 278, 556, 556, 222, 222, 500, 222, 833,                // 'a' .. 'm'
    556, 556, 556, 556, 333, 500, 278, 556, 500, 722, 500, 500, 500,                // 'n' .. 'z'
    334, 260, 334, 584                                                              // '{' .. '~'
};

Double_t CharWidthEm(char c) {
    if (c < 32 || c > 126) return 0.556;
    return kHelveticaWidths[c - 32] / 1000.;
}

TString FormatValue(Double_t v, Int_t decimals) {
    TString s = TString::Format("%.*f", decimals, v);
    if (s.BeginsWith("-") && s.Atof() == 0.) s.Remove(0, 1); // no "-0"
    return s;
}

Int_t DecimalsFor(Double_t step) {
    for (Int_t n = 0; n < 9; ++n) {
        const Double_t x = step * std::pow(10., n);
        if (std::abs(x - std::round(x)) < 1e-6 * std::max(1., x)) return n;
    }
    return 9;
}
} // namespace

Double_t TextWidthEm(const char* text) {
    // Rough TLatex support: #symbols count as one character, ^{} and _{} are scaled down
    Double_t width = 0.;
    std::vector<Double_t> scales{1.};
    for (const char* p = text; p && *p; ++p) {
        const char c = *p;
        if (c == '#') {
            while (std::isalpha(static_cast<unsigned char>(p[1]))) ++p;
            width += 0.6 * scales.back();
        } else if ((c == '^' || c == '_') && p[1] == '{') {
            scales.push_back(0.7 * scales.back());
            ++p;
        } else if (c == '^' || c == '_') {
            if (p[1]) width += 0.7 * scales.back() * CharWidthEm(*++p);
        } else if (c == '{') {
            scales.push_back(scales.back());
        } else if (c == '}') {
            if (scales.size() > 1) scales.pop_back();
        } else {
            width += scales.back() * CharWidthEm(c);
        }
    }
    return width;
}

AxisLabels PredictLabels(Double_t min, Double_t max, Bool_t log, Int_t ndiv) {
    AxisLabels result;
    if (!(max > min)) return result;

    std::vector<Double_t> values;
    if (log && min > 0.) {
        const Int_t kmin = static_cast<Int_t>(std::ceil(std::log10(min) - 1e-9));
        const Int_t kmax = static_cast<Int_t>(std::floor(std::log10(max) + 1e-9));
        if (kmin <= kmax) {
            result.useExponent = (kmax > 5 || kmin < -4);
            for (Int_t k = kmin; k <= kmax; ++k) {
                if (result.useExponent) result.labels.push_back(TString::Format("10^{%d}", k));
                else result.labels.push_back(FormatValue(std::pow(10., k), std::max(0, -k)));
            }
        } else {
            log = kFALSE; // less than a decade: ROOT labels it like a linear axis
        }
    }

    if (!log) {
        Double_t binLow = 0., binHigh = 0., step = 0.;
        Int_t nbins = 0;
        THLimitsFinder::Optimize(min, max, ndiv % 100, binLow, binHigh, nbins, step, "");
        if (step <= 0.) return result;
        const Int_t decimals = DecimalsFor(step);
        const Double_t eps = 1e-9 * (max - min);
        for (Int_t i = 0; i <= nbins; ++i) {
            Double_t v = binLow + i * step;
            if (v < min - eps || v > max + eps) continue;
            if (std::abs(v) < 1e-9 * step) v = 0.;
            result.labels.push_back(FormatValue(v, decimals));
        }
        const Double_t maxAbs = std::max(std::abs(min), std::abs(max));
        const Int_t intDigits = (maxAbs >= 1.) ? static_cast<Int_t>(std::floor(std::log10(maxAbs))) + 1 : 1;
        result.useExponent = (intDigits > 5 || decimals > 4);
    }

    if (result.useExponent && !log) {
        // ROOT rescales the labels (e.g. "2.5" + "x10^{6}"): assume short labels
        result.maxWidthEm = result.lastWidthEm = TextWidthEm("-0.00");
        return result;
    }
    for (const auto& label : result.labels) {
        result.maxWidthEm = std::max(result.maxWidthEm, TextWidthEm(label.Data()));
    }
    if (!result.labels.empty()) result.lastWidthEm = TextWidthEm(result.labels.back().Data());
    return result;
}

Double_t YTitleOffset(Double_t centerDistPx, Double_t titleSizePx) {
    return (centerDistPx + 0.12 * titleSizePx) / (1.6 * titleSizePx);
}

Double_t XTitleOffset(Double_t centerDistPx, Double_t titleSizePx) {
    return (centerDistPx - 0.05 * titleSizePx) / (1.6 * titleSizePx);
}

} // namespace Layout
} // namespace ROOTEnhancedGraphing
