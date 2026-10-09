#ifndef SMARTPAD_H
#define SMARTPAD_H

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <limits>
#include <unordered_map>
#include "TPad.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"
#include "TBox.h"
#include "TLatex.h"
#include "TLegend.h"

namespace ROOTEnhancedGraphing {
class SmartPad : public TPad {
public:
    enum GraphDimension {
        kUndefined,
        k1D,
        k2D,
        k3D
    };
    SmartPad(const char* name, const char* title, Double_t xlow, Double_t ylow, Double_t xup, Double_t yup);
    virtual ~SmartPad();

friend class PadContainer;

private:
    TString name;
    const Double_t fxlow;
    const Double_t fylow;
    const Double_t fxup;
    const Double_t fyup;

    Double_t fstepX;
    Double_t fstepY;

    uint8_t fSetRanges = 0;
    std::array<Double_t, 4> fRanges = {
        std::numeric_limits<Double_t>::max(),   // xmin starts at +infinity
        std::numeric_limits<Double_t>::lowest(), // xmax starts at -infinity
        std::numeric_limits<Double_t>::max(),   // ymin starts at +infinity
        std::numeric_limits<Double_t>::lowest()  // ymax starts at -infinity
    }; // xmin, xmax, ymin, ymax



    GraphDimension dimension = kUndefined;

    struct Drawable {
        TH1* h1 = nullptr;
        TGraph* gr = nullptr;
        TH2* h2 = nullptr;
        TString option; // option for drawing
        uint8_t type; // 1: TH1, 2: TGraph, 3: TH2
    };
    std::vector<Drawable> fDrawables;

    // binning in NDC: not to be changed
    static constexpr uint8_t fNbinX = 64;
    static constexpr uint8_t fNbinY = fNbinX;

    uint64_t fGrid[fNbinX] = {0}; 



private:
    Int_t SetFinalRanges();
    void CheckDims(const GraphDimension& dimension);
    void ClearGrid() {std::fill(std::begin(fGrid), std::end(fGrid), 0ULL);}
    Int_t GetFirstType() const {
        if (fDrawables.size() == 0) return 0;
        return fDrawables[0].type;
    }
    Double_t GetXWidth() const {return fxup - fxlow;}
    Double_t GetYWidth() const {return fyup - fylow;} 

private:
    TAxis* GetXaxis();
    TAxis* GetYaxis();

public:
    void Add(TObject*& obj, Option_t* option); // Add a TH1, TGraph, etc
    void DrawAll();
    void SetRange(Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax);

public:
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

    void Print();
    void PrintGrid();


};

} // namespace BetterGraphing

#endif
