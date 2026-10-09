#ifndef PADCONTAINER_H
#define PADCONTAINER_H

#include "SmartPad.hh"
#include "TError.h"


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
    std::vector<std::unique_ptr<SmartPad>> fPads;
    PadMode fMode = kUndefined;

private:
    TString GetStringFromMode() const {
        if (fMode == kUndefined) return "Undefined";
        if (fMode == kSingle) return "Single";
        if (fMode == kLeftRight) return "Left-Right";
        if (fMode == kUpDown) return "Up-Down";
        if (fMode == kCustom) return "Custom";
        return "Unknown";
    }

private:
    void SetMode(PadMode mode) { fMode = mode;}
    Bool_t AddPad(const char* name, const char* title, Double_t x1, Double_t y1, Double_t x2, Double_t y2);
    void cd(Int_t padIndex);
    void ApplyAlignedMargins();
    void DrawAllPads(TVirtualPad* parent, const Layout::Sizes& baseSizes);

public:
    SmartPad* GetPad(Int_t padIndex) const {
        if (padIndex < 0 || padIndex >= static_cast<Int_t>(fPads.size())) {
            ::Error("PadContainer::GetPad", "Pad index %d out of range on page %d (%zu pads).", padIndex, fIndex, fPads.size());
            return nullptr;
        }
        return fPads[padIndex].get();
    }
    Int_t GetNPads() const { return static_cast<Int_t>(fPads.size()); }
    void PrintInfo() const;
};
}

#endif // PADCONTAINER_H
