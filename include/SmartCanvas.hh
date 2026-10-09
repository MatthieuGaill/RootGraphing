#ifndef SMARTCANVAS_H
#define SMARTCANVAS_H

#include "PadContainer.hh"

namespace ROOTEnhancedGraphing {

class SmartCanvas : public TCanvas {
public:
    SmartCanvas(const char* name, const char* title, const char* savePath_, Int_t ww, Int_t wh, Bool_t fverbose = false);
    virtual ~SmartCanvas();

    void ApplyCustomStyle();
    void DrawAndSave();

private:
    const char* fTitle;
    const char* fSavePath;
    Bool_t fverbose = false;
    std::map<Int_t, std::unique_ptr<PadContainer>> fPages;
    Int_t fActivePageIndex;

    Int_t fWidth;
    Int_t fHeight;

public:

    // Page creation or setter
    void SetPage(Int_t index); // set existing page
    void SetPage(Int_t index, Option_t* layout); // create page with layout
    void SetPage(Int_t index, Double_t scale, Option_t* layout);
    void SetPage(Int_t index, std::vector<std::array<Double_t, 4>> pad_set, Option_t* layout);

    void SetupPage(Int_t index);


    PadContainer* GetPage(Int_t index);

    SmartPad* GetPad(Int_t index, Int_t padIndex);

    void AddDrawable(Int_t padIndex, TObject* obj, Option_t* option, TString legEntry = "");
    void PrintInfo();

};

}

#endif // SMARTCANVAS_H