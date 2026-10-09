/*

███████╗███╗   ███╗ █████╗ ██████╗ ████████╗     ██████╗ █████╗ ███╗   ██╗██╗   ██╗ █████╗ ███████╗
██╔════╝████╗ ████║██╔══██╗██╔══██╗╚══██╔══╝    ██╔════╝██╔══██╗████╗  ██║██║   ██║██╔══██╗██╔════╝
███████╗██╔████╔██║███████║██████╔╝   ██║       ██║     ███████║██╔██╗ ██║██║   ██║███████║███████╗
╚════██║██║╚██╔╝██║██╔══██║██╔══██╗   ██║       ██║     ██╔══██║██║╚██╗██║╚██╗ ██╔╝██╔══██║╚════██║
███████║██║ ╚═╝ ██║██║  ██║██║  ██║   ██║       ╚██████╗██║  ██║██║ ╚████║ ╚████╔╝ ██║  ██║███████║
╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝        ╚═════╝╚═╝  ╚═╝╚═╝  ╚═══╝  ╚═══╝  ╚═╝  ╚═╝╚══════╝
                                                                                                   
                                                                  
*/

#include "SmartCanvas.hh"
#include "TROOT.h"
#include "TStyle.h"
#include "TError.h"
#include "TColor.h"

namespace ROOTEnhancedGraphing {


SmartCanvas::SmartCanvas(const char* name, const char* title, const char* savePath_, Int_t ww, Int_t wh, Bool_t verbose)
    : TCanvas(EnableBatch(name), title, ww, wh), fSavePath(savePath_), fverbose(verbose)
    {
    if (fSavePath.IsNull()) fSavePath = name;

    // Private style, starting from ROOT's "Modern" one. The name must be unique: TStyle deletes a homonym.
    static Int_t styleCounter = 0;
    const TString styleName = TString::Format("SmartCanvasStyle_%d", styleCounter++);
    fStyle = std::make_unique<TStyle>(styleName, "SmartCanvas style");
    if (const TStyle* modern = gROOT->GetStyle("Modern")) {
        modern->Copy(*fStyle);
        fStyle->SetName(styleName);
        fStyle->SetTitle("SmartCanvas style");
    }
    ApplyCustomStyle();
}

SmartCanvas::~SmartCanvas() {
    Clear();         // detach the pads of the last page (they are not deleted, see SmartPad)
    fPages.clear();  // then delete them
}

const char* SmartCanvas::EnableBatch(const char* name) {
    // Called before the TCanvas constructor, so that no window is ever opened
    gROOT->SetBatch(kTRUE);
    return name;
}

void SmartCanvas::ApplyCustomStyle() {
    // Text sizes, offsets, tick lengths and margins are set per pad, in pixels (see LayoutTools.hh)
    TStyle* s = fStyle.get();

    s->SetOptStat(0);
    s->SetOptTitle(0);
    s->SetPalette(kBird);
    s->SetLegendBorderSize(0);
    s->SetLegendFillColor(0);

    s->SetFrameLineWidth(1);
    s->SetFrameLineColor(kGray+3);
    s->SetFrameFillColor(0);


    s->SetNdivisions(505, "x");
    s->SetNdivisions(505, "y");
    s->SetNdivisions(505, "z");
    s->SetAxisColor(1, "x");
    s->SetAxisColor(1, "y");
    s->SetAxisColor(1, "z");

    s->SetLabelColor(1, "x");
    s->SetLabelColor(1, "y");
    s->SetLabelColor(1, "z");

    s->SetTitleAlign(22);
    s->SetTitleFillColor(0);
    s->SetTitleTextColor(13);
    s->SetTitleFont(62, "t"); // pad title (any option other than x, y, z)

    // Canvas and pad
    s->SetCanvasColor(0);      // White canvas
    s->SetPadColor(0);         // White pad
    s->SetPadBorderMode(0);
    s->SetCanvasBorderMode(0);
    s->SetFrameBorderMode(0);

    // Ticks and grid
    s->SetPadTickX(1);
    s->SetPadTickY(1);
    s->SetGridStyle(1);        // thin solid soft gray grid (switched on per pad)
    s->SetGridWidth(1);
    s->SetGridColor(TColor::GetColor("#dcdcdc"));
    s->SetPadGridX(0);
    s->SetPadGridY(0);
    // Font
    s->SetTextFont(42);        // Helvetica
    s->SetLabelFont(Layout::kFont, "XYZ");
    s->SetTitleFont(Layout::kFont, "XYZ");

    // Stats box
    s->SetStatFont(42);
    s->SetStatBorderSize(1);
    s->SetStatX(0.9);
    s->SetStatY(0.9);
    s->SetStatW(0.15);
    s->SetStatH(0.1);
}

Bool_t SmartCanvas::CheckNewPage(Int_t index) {
    fActivePageIndex = -1; // nothing must go to the previous page if this one is not created
    if (index < 0) {
        ::Error("SmartCanvas::SetPage", "Page index must be non-negative (got %d).", index);
        return kFALSE;
    }
    if (fPages.find(index) != fPages.end()) {
        ::Error("SmartCanvas::SetPage", "Page %d already exists: select it with SetPage(%d).", index, index);
        return kFALSE;
    }
    cd(); // the pads are created inside this canvas
    return kTRUE;
}

Bool_t SmartCanvas::SetPage(Int_t index) {
    if (fPages.find(index) == fPages.end()) {
        ::Error("SmartCanvas::SetPage", "Page %d not found! Build it with another SetPage.", index);
        fActivePageIndex = -1;
        return kFALSE;
    }
    fActivePageIndex = index;
    return kTRUE;
}

Bool_t SmartCanvas::SetPage(Int_t index, Option_t* layout) {
    if (!CheckNewPage(index)) return kFALSE;
    TString layoutStr(layout);
    layoutStr.ToUpper();

    std::unique_ptr<PadContainer> padContainer = std::make_unique<PadContainer>(index);
    if (layoutStr.Contains("LR")) {
        padContainer->SetMode(PadContainer::kLeftRight);
        padContainer->AddPad("pad1", "Left Pad", 0, 0, 0.5, 1);
        padContainer->AddPad("pad2", "Right Pad", 0.5, 0, 1, 1);
    } else if (layoutStr.Contains("UD")) {
        padContainer->SetMode(PadContainer::kUpDown);
        padContainer->AddPad("pad1", "Top Pad", 0, 0.5, 1, 1);
        padContainer->AddPad("pad2", "Bottom Pad", 0, 0, 1, 0.5);
    } else {
        if (!layoutStr.IsNull() && layoutStr != "SINGLE" && layoutStr != "S") {
            ::Warning("SmartCanvas::SetPage", "Unknown layout \"%s\": using a single pad ('LR', 'UD' or 'SINGLE').", layout);
        }
        padContainer->SetMode(PadContainer::kSingle);
        padContainer->AddPad("pad1", "Single Pad", 0, 0, 1, 1);
    }
    fPages.emplace(index, std::move(padContainer));
    fActivePageIndex = index;
    return kTRUE;
}

Bool_t SmartCanvas::SetPage(Int_t index, Double_t scale, Option_t* layout) {
    if (!CheckNewPage(index)) return kFALSE;
    TString layoutStr(layout);
    layoutStr.ToUpper();

    if (scale <= 0 || scale >= 1) {
        ::Error("SmartCanvas::SetPage", "Scale must be between 0 and 1 (got %g).", scale);
        return kFALSE;
    }

    std::unique_ptr<PadContainer> padContainer = std::make_unique<PadContainer>(index);
    if (layoutStr.Contains("LR")) {
        padContainer->SetMode(PadContainer::kLeftRight);
        padContainer->AddPad("pad1", "Left Pad", 0, 0, scale, 1);
        padContainer->AddPad("pad2", "Right Pad", scale, 0, 1, 1);
    } else if (layoutStr.Contains("UD")) {
        padContainer->SetMode(PadContainer::kUpDown);
        padContainer->AddPad("pad1", "Top Pad", 0, scale, 1, 1);
        padContainer->AddPad("pad2", "Bottom Pad", 0, 0, 1, scale);
    } else {
        ::Error("SmartCanvas::SetPage", "Wrong layout option \"%s\". Available options are 'LR' and 'UD'.", layout);
        return kFALSE;
    }
    fPages.emplace(index, std::move(padContainer));
    fActivePageIndex = index;
    return kTRUE;
}

Bool_t SmartCanvas::SetPage(Int_t index, const std::vector<std::array<Double_t, 4>>& pad_set) {
    if (!CheckNewPage(index)) return kFALSE;

    if (pad_set.empty()) {
        ::Error("SmartCanvas::SetPage", "At least one pad definition must be provided.");
        return kFALSE;
    }

    std::unique_ptr<PadContainer> padContainer = std::make_unique<PadContainer>(index);
    padContainer->SetMode(PadContainer::kCustom);
    for (size_t i = 0; i < pad_set.size(); ++i) {
        const auto& padDef = pad_set[i];
        const TString padName = TString::Format("Pad%zu", i);
        if (!padContainer->AddPad(padName, padName, padDef[0], padDef[1], padDef[2], padDef[3])) {
            ::Error("SmartCanvas::SetPage", "Page %d not created.", index);
            return kFALSE;
        }
    }

    fPages.emplace(index, std::move(padContainer));
    fActivePageIndex = index;
    return kTRUE;
}

Bool_t SmartCanvas::AddDrawable(Int_t padIndex, TObject* obj, Option_t* option, TString legEntry) {
    /*
    Adds a drawable object (TH1, TH2, TGraph) to the specified pad in the active page.
    Validates the active page and pad index before adding.
    */
    auto it = fPages.find(fActivePageIndex);
    if (it == fPages.end()) {
        ::Error("SmartCanvas::AddDrawable", "No active page! Create or select one with SetPage first.");
        return kFALSE;
    }
    SmartPad* pad = it->second->GetPad(padIndex);
    if (!pad) return kFALSE;
    return pad->AddDrawable(obj, option, legEntry);
}

void SmartCanvas::DrawAndSave() {
    if (fPages.empty()) {
        ::Warning("SmartCanvas::DrawAndSave", "No page to draw.");
        return;
    }

    // Draw with our style and quiet ROOT's info messages, restore both afterwards
    TStyle* previousStyle = gStyle;
    const Int_t previousErrorLevel = gErrorIgnoreLevel;
    fStyle->cd();
    if (!fverbose && gErrorIgnoreLevel < kWarning) gErrorIgnoreLevel = kWarning;

    SetFillColor(gStyle->GetCanvasColor());
    SetBorderMode(gStyle->GetCanvasBorderMode());

    const TString file = fSavePath + ".pdf";
    this->Print(file + "[");
    for (auto& pagePair : fPages) {
        PadContainer* padContainer = pagePair.second.get();

        this->Clear();    // Clear canvas before drawing new page (the pads are not deleted)
        this->cd();       // Make canvas the current pad
        padContainer->DrawAllPads(this, fSizes, fShowGrid);  // Pass canvas pointer
        this->Update();   // Update canvas after all pads are drawn

        // Save each page
        this->Print(file);
    }
    this->Print(file + "]");

    if (previousStyle) previousStyle->cd();
    gErrorIgnoreLevel = previousErrorLevel;
    if (fverbose) {
        std::cout << "Saved canvas to " << file << std::endl;
    }
}

PadContainer* SmartCanvas::GetPage(Int_t index) {
    auto it = fPages.find(index);
    if (it == fPages.end()) {
        ::Error("SmartCanvas::GetPage", "Page %d not found!", index);
        return nullptr;
    }
    return it->second.get();
}

SmartPad* SmartCanvas::GetPad(Int_t index, Int_t padIndex) {
    PadContainer* padContainer = GetPage(index);
    return padContainer ? padContainer->GetPad(padIndex) : nullptr;
}

void SmartCanvas::PrintInfo(){
    std::cout << "---=========== Canvas " << GetTitle() << " ===========---\n" << std::endl;
    for (auto& [key, page] : fPages) {
        page->PrintInfo();
    }
    std::cout << "---=================================---\n" << std::endl;
}

}
