#ifndef SMARTCANVAS_H
#define SMARTCANVAS_H

#include <map>
#include "TStyle.h"
#include "PadContainer.hh"

namespace ROOTEnhancedGraphing {

// Multi-page canvas saved to <savePath>.pdf. Batch mode is enabled (no window).
// The style only applies while drawing: gStyle is left untouched.
class SmartCanvas : public TCanvas {
public:
    SmartCanvas(const char* name, const char* title, const char* savePath_, Int_t ww, Int_t wh, Bool_t fverbose = false);
    ~SmartCanvas() override;

    void ApplyCustomStyle();
    TStyle* GetStyle() const { return fStyle.get(); } // to tweak the style before DrawAndSave
    // Default legend text size of all pads, in pixels for an 800x600 canvas (scaled with the canvas size).
    // A pad can override it with SmartPad::SetLegendTextSize.
    void SetLegendTextSize(Double_t sizePx) { fSizes.legend = sizePx; }
    // Grid on the 1D pads (on by default). A pad can override it with SmartPad::ShowGrid.
    // Its look comes from the style: GetStyle()->SetGridColor(...), SetGridStyle(...), SetGridWidth(...)
    void ShowGrid(Bool_t show = kTRUE) { fOptions.grid = show; }
    // Ticks pointing out of the frame (default: inside)
    void SetTicksOutside(Bool_t outside = kTRUE) { fOptions.ticksOutside = outside; }
    // Ticks also on the top and right sides (default: on). Both off and outside: matplotlib-like axes.
    void SetMirrorTicks(Bool_t mirror = kTRUE) { fOptions.mirrorTicks = mirror; }
    void DrawAndSave();

private:
    TString fSavePath;
    Bool_t fverbose = false;
    std::unique_ptr<TStyle> fStyle;
    Layout::Sizes fSizes; // text sizes and spacings, in pixels for an 800x600 canvas
    Layout::Options fOptions;
    std::map<Int_t, std::unique_ptr<PadContainer>> fPages;
    Int_t fActivePageIndex = -1;

    static const char* EnableBatch(const char* name);
    Bool_t CheckNewPage(Int_t index);

public:
    using TCanvas::GetPad;

    // Page creation or setter. On failure, no page is active until the next successful SetPage.
    Bool_t SetPage(Int_t index); // set existing page
    Bool_t SetPage(Int_t index, Option_t* layout); // create page with layout: "LR", "UD", or "SINGLE"
    // "LR": scale is the width of the left pad, "UD": scale is the height of the bottom pad
    Bool_t SetPage(Int_t index, Double_t scale, Option_t* layout);
    // Custom pads, each given as {xlow, ylow, xup, yup} in canvas NDC
    Bool_t SetPage(Int_t index, const std::vector<std::array<Double_t, 4>>& pad_set);

    PadContainer* GetPage(Int_t index);

    SmartPad* GetPad(Int_t index, Int_t padIndex);

    Bool_t AddDrawable(Int_t padIndex, TObject* obj, Option_t* option, TString legEntry = "");
    void PrintInfo();

};

}

#endif // SMARTCANVAS_H
