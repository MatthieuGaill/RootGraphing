
#include "SmartPad.hh"
#include "TLegendEntry.h"


namespace ROOTEnhancedGraphing {
class PadContainer {
public:
    enum PadMode {
        kUndefined = 0,
        kSingle,
        kLeftRight,
        kUpDown,
        kCustom
    };

    PadContainer(Int_t index);
    virtual ~PadContainer();

friend class SmartCanvas;

private:
    Int_t fIndex; // index of page (e.g PadContainer)
    Int_t fNPads;
    std::vector<std::unique_ptr<SmartPad>> fPads;
    PadMode mode;

private:
    constexpr static Double_t fYLabelSize = 0.033;
    constexpr static Double_t fYTitleLabelOffset = 0.015;
    constexpr static Double_t fYTitleLabelSize = 0.041;
    constexpr static Double_t fYExtraMargin = -0.025;
    constexpr static Double_t fXLabelSize = 0.033;
    constexpr static Double_t fXTitleLabelOffset = 0.036;
    constexpr static Double_t fXTitleLabelSize = 0.038;
    constexpr static Double_t fXExtraMargin = 0.0;

    Bool_t fSameYMargin = false;

private: // Boxes and Legends
    struct DrawableBox {
        Int_t padIndex;
        TBox* box = nullptr;
        TLegend* leg = nullptr;
        Double_t x1, y1, x2, y2;
        Bool_t AreCoordsSet = false;
        TString option;
        uint8_t type; // 0: TBox, 1: TLegend
    };
    std::vector<DrawableBox> fDrawableBoxes;


    void SetLegend(Option_t* option);
    void SetLegend(Double_t x1, Double_t y1, Double_t x2, Double_t y2, Option_t* option);
    void ClearLegend(Int_t legIndex);
    TLegend* GetLegend();
    Double_t EstimateLegendWidth(TLegend* leg);

private:
    TString GetStringFromMode(){
        if (mode == kUndefined) return "Undefined";
        if (mode == kSingle) return "Single";
        if (mode == kLeftRight) return "Left-Right";
        if (mode == kUpDown) return "Up-Down";
        if (mode == kCustom) return "Custom";
        return "Unknown";
    }

private:
    void SetMode(PadMode mode) { this->mode = mode;}
    void AddPad(const char* name, const char* title, Double_t x1, Double_t y1, Double_t x2, Double_t y2);
    void cd(Int_t padIndex);
    void DrawAllPads(TVirtualPad* parent);

public:
    SmartPad* getPad(Int_t padIndex) {
        if (padIndex < 0 || padIndex >= fPads.size()) {
            std::cerr << "Pad index out of range!" << std::endl;
            return nullptr;
        }
        return fPads[padIndex].get();
    }
    void Print();
};
}