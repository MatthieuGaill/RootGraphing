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

namespace ROOTEnhancedGraphing {


SmartCanvas::SmartCanvas(const char* name, const char* title, const char* savePath_, Int_t ww, Int_t wh, Bool_t verbose)
    : TCanvas(name, title, ww, wh), fTitle(title), fSavePath(savePath_), fActivePageIndex(-1), fWidth(ww), fHeight(wh), fverbose(verbose)
    {
    gROOT->SetBatch(kTRUE); // Enable batch mode
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    gStyle->SetPalette(kBird);
    gStyle->SetLegendBorderSize(0);
    gStyle->SetLegendFillColor(0);
    
    // Apply canvas-specific styling
    ApplyCustomStyle();
    // Force the current style onto already-created objects (histograms, graphs, etc.)
    gROOT->ForceStyle();
    fPages.clear();
    if (!verbose) gErrorIgnoreLevel = kWarning;
}

SmartCanvas::~SmartCanvas() {
    fPages.clear();
}

void SmartCanvas::ApplyCustomStyle() {
    // Canvas-specific settings (margins, etc.)
    // SetLeftMargin(0.05);
    // SetRightMargin(0.05);
    // SetTopMargin(0.05);
    // SetBottomMargin(0.05);
    // SetFrameBorderMode(0);
    // SetBorderMode(0);
    // SetBorderSize(0);

    gStyle->SetFrameLineWidth(1);
    gStyle->SetFrameLineColor(kGray+3);


    gStyle->SetNdivisions(505, "x");
    gStyle->SetNdivisions(505, "y");
    gStyle->SetNdivisions(505, "z");
    gStyle->SetAxisColor(22, "x");
    gStyle->SetAxisColor(1, "y");
    gStyle->SetAxisColor(1, "z");

    gStyle->SetLabelColor(1, "x");
    gStyle->SetLabelColor(1, "y");
    gStyle->SetLabelColor(1, "z");
    gStyle->SetLabelFont(42, "x");
    gStyle->SetLabelFont(42, "y");
    gStyle->SetLabelFont(42, "z");
    // gStyle->SetLabelOffset(0.003, "x");
    // gStyle->SetLabelOffset(0.003, "y");
    // gStyle->SetLabelOffset(0.003, "z");
    // gStyle->SetLabelSize(0.032, "x");
    // gStyle->SetLabelSize(0.032, "y");
    // gStyle->SetLabelSize(0.032, "z");

    // gStyle->SetTickLength(0.02, "x");
    // gStyle->SetTickLength(0.02, "y");
    // gStyle->SetTickLength(0.01, "z");

    gStyle->SetTitleAlign(22);
    // gStyle->SetTitleSize(0.03, "x");
    // gStyle->SetTitleSize(0.03, "y");
    // gStyle->SetTitleSize(0.03, "z");
    // gStyle->SetTitleColor(12, "x");
    // gStyle->SetTitleColor(12, "y");
    // gStyle->SetTitleColor(12, "z");
    // gStyle->SetTitleFont(42, "x");
    // gStyle->SetTitleFont(42, "y");
    // gStyle->SetTitleFont(42, "z");
    gStyle->SetTitleFillColor(0);
    gStyle->SetTitleTextColor(13);
    gStyle->SetTitleFont(62);

    // Canvas and pad
    gStyle->SetCanvasColor(0);      // White canvas
    gStyle->SetPadColor(0);         // White pad
    gStyle->SetPadBorderMode(0);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetFrameBorderMode(0);

    // Ticks and grid
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);
    gStyle->SetGridStyle(3);
    gStyle->SetPadGridX(0);
    gStyle->SetPadGridY(0);
    // Font
    gStyle->SetTextFont(42);        // Helvetica
    gStyle->SetLabelFont(42, "XYZ");
    gStyle->SetTitleFont(42, "XYZ");

    // Size
    // gStyle->SetTitleSize(0.05, "XYZ");
    // gStyle->SetLabelSize(0.045, "XYZ");
    // gStyle->SetTitleOffset(1.2, "X");
    // gStyle->SetTitleOffset(1.4, "Y");

    // Stats box
    gStyle->SetOptStat(0);          // No stats box
    gStyle->SetStatFont(42);
    gStyle->SetStatBorderSize(1);
    gStyle->SetStatX(0.9);
    gStyle->SetStatY(0.9);
    gStyle->SetStatW(0.15);
    gStyle->SetStatH(0.1);
}

void SmartCanvas::SetPage(Int_t index) {
    SetupPage(index);
    if (fPages.find(index) == fPages.end()) {
        std::cerr << " Page index not found! Build a Page with another SetPage" << std::endl;
        return;
    }
    fActivePageIndex = index;
}

void SmartCanvas::SetPage(Int_t index, Option_t* layout) {
    SetupPage(index);
    if (fPages.find(index) != fPages.end()) {
        std::cerr << " Double page index " << index << " (already exists)." << std::endl;
        return;
    }
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
        std::cout << "Any other options then 'LR' or 'UD' will create a one-pad layout." << std::endl;
        padContainer->SetMode(PadContainer::kSingle);
        padContainer->AddPad("pad1", "Single Pad", 0, 0, 1, 1);
    }
    fPages.emplace(index, std::move(padContainer));
    fActivePageIndex = index;
}

void SmartCanvas::SetPage(Int_t index, Double_t scale, Option_t* layout) {
    SetupPage(index);
    if (fPages.find(index) != fPages.end()) {
        std::cerr << " Double page index " << index << " (already exists)." << std::endl;
        return;
    }
    TString layoutStr(layout);
    layoutStr.ToUpper();

    if (scale <= 0 || scale >= 1) {
        std::cerr << __func__ << ": Scale must be between 0 and 1." << std::endl;
        return;
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
        std::cerr << "Wrong layout option provided. Available options are 'LR' and 'UD'." << std::endl;
        return;
    }
    fPages.emplace(index, std::move(padContainer));
    fActivePageIndex = index;
}

void SmartCanvas::SetPage(Int_t index, std::vector<std::array<Double_t, 4>> pad_set, Option_t* layout) {
    SetupPage(index);
    if (fPages.find(index) != fPages.end()) {
        std::cerr << " Double page index " << index << " (already exists)." << std::endl;
        return;
    }
    TString layoutStr(layout);
    layoutStr.ToUpper();

    if (pad_set.size() < 1) {
        std::cerr << __func__ << ": At least one pad definition must be provided." << std::endl;
        return;
    }

    std::unique_ptr<PadContainer> padContainer = std::make_unique<PadContainer>(index);

    int i = 0;
    char PadName[100];
    for (const auto& padDef : pad_set) {
        sprintf(PadName, "Pad%d", i);
        padContainer->AddPad(PadName, PadName, padDef[0], padDef[1], padDef[2], padDef[3]);
        i++;
    }

    fPages.emplace(index, std::move(padContainer));
    fActivePageIndex = index;
}

void SmartCanvas::AddDrawable(Int_t padIndex, TObject* obj, Option_t* option, TString legEntry) {
    /*
    Adds a drawable object (TH1, TGraph, etc.) to the specified pad in the active page.
    Validates the active page and pad index before adding.
    */
    if (fPages.find(fActivePageIndex) == fPages.end()) {
        std::cerr << __func__ << ": Active page index not found! Set a valid page first." << std::endl;
        return;
    }
    PadContainer* padContainer = fPages[fActivePageIndex].get();
    SmartPad* pad = padContainer->getPad(padIndex);
    if (!pad) {
        std::cerr << __func__ << ": Pad index not found in the active page!" << std::endl;
        return;
    }
    pad->Add(obj, option);
}

void SmartCanvas::SetupPage(Int_t index) {
    if (index < 0 ) {
        std::cerr << " Page index must be non-negative." << std::endl;
        return;
    }
}

void SmartCanvas::DrawAndSave() {
    this->Print(Form("%s.pdf[", fSavePath));
    for (auto& pagePair : fPages) {
        Int_t pageIndex = pagePair.first;
        PadContainer* padContainer = pagePair.second.get();

        this->Clear();    // Clear canvas before drawing new page
        this->cd();       // Make canvas the current pad
        padContainer->DrawAllPads(this);  // Pass canvas pointer
        this->Update();   // Update canvas after all pads are drawn

        // Save each page
        this->Print(Form("%s.pdf", fSavePath));
    }
    this->Print(Form("%s.pdf]", fSavePath));
    // this->Close();
    if (fverbose) {
        std::cout << "Saved canvas to " << fSavePath << ".pdf" << std::endl;
    }
}

PadContainer* SmartCanvas::GetPage(Int_t index) {
    if (fPages.find(index) != fPages.end()) {
        return fPages[index].get();
    } else {
        std::cerr << "Page index not found!" << std::endl;
        exit(1);
        return nullptr;
    }
}

SmartPad* SmartCanvas::GetPad(Int_t index, Int_t padIndex) {
    PadContainer* padContainer = GetPage(index);
    if (padContainer) {
        return padContainer->getPad(padIndex);
    } else {
        std::cerr << "No Pad with index " << padIndex << " found on Page " << index << std::endl;
        return nullptr;
    }
}

void SmartCanvas::PrintInfo(){
    std::cout << "---=========== Canvas " << fTitle << " ===========---\n" << std::endl;
    for (auto& [key, page] : fPages) {
        page->Print();
    }
    std::cout << "---=================================---\n" << std::endl;
}

}