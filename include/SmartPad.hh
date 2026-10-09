#ifndef SMARTPAD_H
#define SMARTPAD_H

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <cstdint>
#include "TPad.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"
#include "TBox.h"
#include "TLatex.h"
#include "LayoutTools.hh"

namespace ROOTEnhancedGraphing {
class SmartPad : public TPad {
public:
    enum GraphDimension {
        kUndefined,
        k1D,
        k2D
    };
    SmartPad(const char* name, const char* title, Double_t xlow, Double_t ylow, Double_t xup, Double_t yup);
    ~SmartPad() override = default;

friend class PadContainer;

private:
    // Pad position in the canvas (NDC)
    const Double_t fxlow;
    const Double_t fylow;
    const Double_t fxup;
    const Double_t fyup;

    // User ranges: only the ones flagged in fSetRanges are used, the others are automatic
    uint8_t fSetRanges = 0;
    std::array<Double_t, 4> fRanges = {0., 0., 0., 0.}; // xmin, xmax, ymin, ymax
    // Frame ranges actually drawn (user ranges + automatic ones)
    std::array<Double_t, 4> fFrame = {0., 1., 0., 1.};

    GraphDimension dimension = kUndefined;

    enum class Kind : uint8_t { kHist1D, kGraph, kHist2D };
    struct Drawable {
        TObject* obj = nullptr;
        TString option;   // option for drawing
        TString legEntry; // legend label (not drawn yet)
        Kind kind = Kind::kHist1D;
        TH1* Hist() const { return static_cast<TH1*>(obj); }
        TGraph* Graph() const { return static_cast<TGraph*>(obj); }
    };
    std::vector<Drawable> fDrawables;

    // Occupancy of the frame, in a fNbinX x fNbinY grid (bit iy of fGrid[ix]).
    // A cell is occupied when it is below the highest drawn point of its column (fTop, frame fraction).
    static constexpr Int_t fNbinX = 64;
    static constexpr Int_t fNbinY = 64;
    uint64_t fGrid[fNbinX] = {0};
    std::array<Double_t, fNbinX> fTop{};

    // Legend (1D pads only), made of the drawables with a legend entry
    Double_t fLegendTextSize = -1.;              // pixels for an 800x600 canvas, <= 0: canvas default
    Bool_t fLegendUserPos = kFALSE;
    std::array<Double_t, 4> fLegendNDC{};        // user position: x1, y1, x2, y2 in pad NDC
    std::array<Double_t, 4> fLegendFrame{};      // automatic position: x1, y1, x2, y2 in frame fraction
    Bool_t fYmaxFixed = kFALSE;                  // the legend may not add headroom
    Double_t fLegendTextPx = 0.;                 // text size actually used (reduced if the legend is too wide)
    Bool_t fLegendOverlaps = kFALSE;

    Int_t fShowGrid = -1; // grid on 1D pads: -1 canvas default, 0 off, 1 on
    Layout::Options fOptions; // canvas options of the current drawing

    // Layout of the current drawing, in pixels
    struct Margins {
        Double_t left = 0., right = 0., bottom = 0., top = 0.;
    };
    Layout::Sizes fSizes;
    Double_t fPadWpx = 0.;
    Double_t fPadHpx = 0.;
    Margins fMarginsPx;
    Layout::AxisLabels fXLabels, fYLabels, fZLabels;

private:
    void ComputeFrame();
    Margins RequiredMargins() const;
    void ApplyMargins(const Margins& marginsPx);
    void StyleAxis(TAxis* axis, char which) const;
    void FillGrid();
    void ClearGrid() {std::fill(std::begin(fGrid), std::end(fGrid), 0ULL);}
    Bool_t CheckDims(GraphDimension dim);
    Bool_t HasDrawables() const { return !fDrawables.empty(); }
    Bool_t HasZPalette() const;
    Double_t GetXWidth() const {return fxup - fxlow;}
    Double_t GetYWidth() const {return fyup - fylow;}
    TAxis* GetFirstXaxis() const;
    TAxis* GetFirstYaxis() const;
    Double_t FrameWpx() const { return fPadWpx - fMarginsPx.left - fMarginsPx.right; }
    Double_t FrameHpx() const { return fPadHpx - fMarginsPx.bottom - fMarginsPx.top; }
    Double_t ToFracX(Double_t x) const; // user coordinate to frame fraction
    Double_t ToFracY(Double_t y) const;

    Bool_t HasLegend() const;
    Double_t LegendTextSizePx() const;
    void LegendSizePx(Double_t textPx, Double_t& widthPx, Double_t& heightPx, Double_t& symbolPx) const;
    static TString LegendOption(const Drawable& d);
    void DrawLegend();
    Double_t OutsideTickPx() const { return fOptions.ticksOutside ? fSizes.tick : 0.; }
    void DrawOutsideTicks();

    // Called by PadContainer when the page is drawn, in this order
    void Prepare(Double_t canvasWpx, Double_t canvasHpx, const Layout::Sizes& sizes, const Layout::Options& options);
    Bool_t PlaceLegend(); // after the margins; kTRUE if the y range had to grow to fit the legend
    void DrawAll();

public:
    Bool_t AddDrawable(TObject* obj, Option_t* option, const TString& legEntry = ""); // Add a TH1, TH2 or TGraph
    void SetRange(Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax);
    void ResetRanges() { fSetRanges = 0; }

    void SetXMin(Double_t x1){
        fRanges[0] = x1;
        fSetRanges |= 0x1;
    }

    void SetXMax(Double_t x2) {
        fRanges[1] = x2;
        fSetRanges |= 0x2;
    }
    void SetYMin(Double_t y1) {
        fRanges[2] = y1;
        fSetRanges |= 0x4;
    }
    void SetYMax(Double_t y2) {
        fRanges[3] = y2;
        fSetRanges |= 0x8;
    }

    // Legend text size in pixels for an 800x600 canvas (scaled with the canvas size), <= 0: canvas default
    void SetLegendTextSize(Double_t sizePx) { fLegendTextSize = sizePx; }
    // Fixed legend position in pad NDC, instead of the automatic placement
    void SetLegendPosition(Double_t x1, Double_t y1, Double_t x2, Double_t y2) {
        fLegendNDC = {x1, y1, x2, y2};
        fLegendUserPos = kTRUE;
    }
    void SetAutoLegendPosition() { fLegendUserPos = kFALSE; }

    // Grid (1D pads only), overriding the canvas default
    void ShowGrid(Bool_t show = kTRUE) { fShowGrid = show; }
    void UseDefaultGrid() { fShowGrid = -1; }

    void PrintInfo() const;
    void PrintGrid() const;
};

} // namespace ROOTEnhancedGraphing

#endif
