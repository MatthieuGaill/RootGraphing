#ifndef LAYOUTTOOLS_H
#define LAYOUTTOOLS_H

#include <vector>
#include "TString.h"

// Pixel-based layout helpers shared by SmartPad and PadContainer.
// All text uses ROOT precision-3 fonts (size in pixels), so a given size looks
// identical in every pad, whatever the layout.
namespace ROOTEnhancedGraphing {
namespace Layout {

// Text font: Helvetica, precision 3 (size in pixels)
constexpr Int_t kFont = 43;
// Axis divisions used for every frame axis
constexpr Int_t kNdivisions = 505;
// Pixel sizes below are tuned for a canvas whose smaller side is kRefPixels
// (an 800x600 canvas gives a 796x572 drawing area)
constexpr Double_t kRefPixels = 572.;

struct Sizes {
    Double_t label = 17.;       // tick label size
    Double_t title = 19.;       // axis title size
    Double_t tick = 9.;         // tick length
    Double_t labelOffset = 5.;  // axis to tick labels
    Double_t titleGap = 9.;     // tick labels to axis title
    Double_t outer = 7.;        // axis title to pad edge
    Double_t minRight = 12.;    // smallest right margin
    Double_t minTop = 10.;      // smallest top margin
    Double_t legend = 18.;      // legend text size
    Double_t scale = 1.;        // canvas size / reference size, set by Scaled()

    Sizes Scaled(Double_t scale) const;
};

// Canvas-wide drawing options, given to every pad
struct Options {
    Bool_t grid = kTRUE;          // grid on the 1D pads (a pad can override it)
    Bool_t ticksOutside = kTRUE; // ticks pointing out of the frame
    Bool_t mirrorTicks = kFALSE;   // ticks also on the top and right sides
};

// Width of a (TLatex) string in units of the font size, from the Helvetica metrics
Double_t TextWidthEm(const char* text);

// Tick labels ROOT is expected to draw for an axis range
struct AxisLabels {
    std::vector<TString> labels;
    Bool_t useExponent = false; // labels too long: let ROOT factor out a x10^n
    Double_t maxWidthEm = 0.;   // widest label, in units of the font size
    Double_t lastWidthEm = 0.;  // width of the largest label (sticks out at the axis end)
};
AxisLabels PredictLabels(Double_t min, Double_t max, Bool_t log, Int_t ndiv = kNdivisions);

// ROOT places an axis title centre at 1.6 * titleSize * titleOffset pixels from the axis
// (calibrated on PDF output). These return the offset for a wanted title centre distance.
Double_t YTitleOffset(Double_t centerDistPx, Double_t titleSizePx);
Double_t XTitleOffset(Double_t centerDistPx, Double_t titleSizePx);

} // namespace Layout
} // namespace ROOTEnhancedGraphing

#endif // LAYOUTTOOLS_H
