
/*

███████╗███╗   ███╗ █████╗ ██████╗ ████████╗    ██████╗  █████╗ ██████╗ 
██╔════╝████╗ ████║██╔══██╗██╔══██╗╚══██╔══╝    ██╔══██╗██╔══██╗██╔══██╗
███████╗██╔████╔██║███████║██████╔╝   ██║       ██████╔╝███████║██║  ██║
╚════██║██║╚██╔╝██║██╔══██║██╔══██╗   ██║       ██╔═══╝ ██╔══██║██║  ██║
███████║██║ ╚═╝ ██║██║  ██║██║  ██║   ██║       ██║     ██║  ██║██████╔╝
╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝       ╚═╝     ╚═╝  ╚═╝╚═════╝ 
                                                                        

*/

#include "SmartPad.hh"
#include "TLegendEntry.h"

namespace ROOTEnhancedGraphing {

SmartPad::SmartPad(const char* name, const char* title, Double_t xlow_, Double_t ylow_, Double_t xup_, Double_t yup_)
    : TPad(name, title, xlow_, ylow_, xup_, yup_),
      fxlow(xlow_), fylow(ylow_), fxup(xup_), fyup(yup_)
{
    // Check
    if (fxlow >= fxup || fylow >= fyup) {
        std::cerr << "Error: Invalid pad coordinates (" << name << "): Ensure xlow < xup and ylow < yup." << std::endl;
        exit(1);
    }
    if (fxlow < 0 || fxlow > 1 || fxup < 0 || fxup > 1 || fylow < 0 || fylow > 1 || fyup < 0 || fyup > 1) {
        std::cerr << "Error: Pad coordinates (" << name << ") must be in the range [0, 1]." << std::endl;
        exit(1);
    }
    fDrawables.clear();
}

SmartPad::~SmartPad() {
    // Destructor — fDrawables vector cleans up automatically
}

///////////////////////////////////////
// DRAWABLES: TH1, TGraph, TH2, etc. //
///////////////////////////////////////

void SmartPad::Add(TObject*& obj, Option_t* option) {
    TString opt = option;

    if (obj == nullptr) {
        std::cerr << "Error: Null object provided to Add method." << std::endl;
        return;
    }
    Double_t ymin_;
    Double_t ymax_;
    Double_t xmin_;
    Double_t xmax_;
    TAxis* x_axis = nullptr;
    TAxis* y_axis = nullptr;
    // Use dynamic_cast or InheritsFrom to check actual type at runtime
    if (TH1* h1_ = dynamic_cast<TH1*>(obj)) {
        CheckDims(k1D);
        if (opt.IsNull()) opt = "HIST";
        fDrawables.push_back({h1_, nullptr, nullptr, opt, 1});
        // Ensure the histogram picks up the current global style
        h1_->UseCurrentStyle();
        ymin_ = h1_->GetMinimum();
        ymax_ = h1_->GetMaximum();
        xmin_ = h1_->GetXaxis()->GetXmin();
        xmax_ = h1_->GetXaxis()->GetXmax();
        x_axis = h1_->GetXaxis();
        y_axis = h1_->GetYaxis();
    } else if (TGraph* gr_ = dynamic_cast<TGraph*>(obj)) {
        CheckDims(k1D);
        if (opt.IsNull()) opt = "LP";
        fDrawables.push_back({nullptr, gr_, nullptr, opt, 2});
        // Ensure the graph picks up the current global style
        gr_->UseCurrentStyle();
        ymin_ = gr_->GetMinimum();
        ymax_ = gr_->GetMaximum();
        xmin_ = gr_->GetXaxis()->GetXmin();
        xmax_ = gr_->GetXaxis()->GetXmax();
        x_axis = gr_->GetXaxis();
        y_axis = gr_->GetYaxis();
    } else if (TH2* h2_ = dynamic_cast<TH2*>(obj)) {
        CheckDims(k2D);
        // if (opt.IsNull()) opt = "COLZ";
        fDrawables.push_back({nullptr, nullptr, h2_, opt, 3});
        h2_->UseCurrentStyle();
        x_axis = h2_->GetXaxis();
        y_axis = h2_->GetYaxis();
    } else {
        std::cerr << "Warning: Unknown object type. Must inherit either from TH1, TGraph, or TH2 (for now)." << std::endl;
        fDrawables.clear();
        // fDrawableBoxes.clear();
        exit(1);
    }

    if (dimension == k1D) {
        if (xmin_ < fRanges[0]) fRanges[0] = xmin_;
        if (xmax_ > fRanges[1]) fRanges[1] = xmax_;
        if (ymin_ < fRanges[2]) fRanges[2] = ymin_;
        if (ymax_ > fRanges[3]) fRanges[3] = ymax_;
    }

    x_axis->CenterTitle();
    y_axis->CenterTitle();
    x_axis->SetTitleOffset(1.2);
    y_axis->SetTitleOffset(1.4);
}

TAxis* SmartPad::GetXaxis(){
    Drawable& d0 = fDrawables[0];
    if (d0.type == 1) return d0.h1->GetXaxis();
    else if (d0.type == 2) return d0.gr->GetXaxis();
    else if (d0.type == 3) return d0.h2->GetXaxis();
    else {
        std::cerr << "Error: No drawable with X axis found!" << std::endl;
        return nullptr;
    }
}

TAxis* SmartPad::GetYaxis(){
    Drawable& d0 = fDrawables[0];
    if (d0.type == 1) return d0.h1->GetYaxis();
    else if (d0.type == 2) return d0.gr->GetYaxis();
    else if (d0.type == 3) return d0.h2->GetYaxis();
    else {
        std::cerr << "Error: No drawable with Y axis found!" << std::endl;
        return nullptr;
    }
}

void SmartPad::CheckDims(const GraphDimension& dim){
    if (dimension == kUndefined) dimension = dim;
    else if (dimension != dim){
        std::cerr << "Error: Mixed graph/histogram dimensions in the same pad!" << std::endl;
        exit(1);
    } 
}

void SmartPad::SetRange(Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax) {
    fRanges[0] = xmin; fSetRanges |= 0x1;
    fRanges[1] = xmax; fSetRanges |= 0x2;
    fRanges[2] = ymin; fSetRanges |= 0x4;
    fRanges[3] = ymax; fSetRanges |= 0x8;
}

Int_t SmartPad::SetFinalRanges() {
    Double_t rangeX = fRanges[1] - fRanges[0];
    Double_t rangeY = fRanges[3] - fRanges[2];

    fRanges[1] += 0.05 * rangeX * !(fSetRanges & 0x2);
    fRanges[0] -= 0.05 * rangeX * !(fSetRanges & 0x1);
    rangeX = fRanges[1] - fRanges[0];

    fRanges[3] += 0.05 * rangeY * !(fSetRanges & 0x8);
    fRanges[2] -= 0.01 * rangeY * !(fSetRanges & 0x4);
    rangeY = fRanges[3] - fRanges[2];

    fstepX = rangeX / fNbinX;
    fstepY = rangeY / fNbinY;

    // Return a robust estimate of maximum Y tick-label character count.
    const Double_t maxAbsY = std::max(std::abs(fRanges[2]), std::abs(fRanges[3]));
    const Int_t signChars = (fRanges[2] < 0.0 || fRanges[3] < 0.0) ? 1 : 0;
    const Int_t intDigits = (maxAbsY >= 1.0) ? static_cast<Int_t>(std::floor(std::log10(maxAbsY))) + 1 : 1;

    Int_t decimals = 0;
    if (rangeY < 1.0) decimals = 2;
    else if (rangeY < 10.0) decimals = 1;

    const Int_t dotChars = (decimals > 0) ? 1 : 0;
    const Int_t labelChars = signChars + intDigits + dotChars + decimals;
    return std::max(2, std::min(labelChars, 10));
    
}

void SmartPad::DrawAll() {
    if (fDrawables.size() == 0) {
        std::cerr << "Nothing to draw. Add drawable TObject with Add" << std::endl;
        return;
    }

    bool isFirst = true;
    

    Drawable& d0 = fDrawables[0];
    // Draw first Object, to set axes
    // Debug: print axis/font settings before drawing to diagnose style issues
    // if (d0.type == 1 && d0.h1) {
        // std::cout << "[DEBUG] Before Draw: H1 name=" << d0.h1->GetName()
        //           << ", X axis label font=" << d0.h1->GetXaxis()->GetLabelFont()
        //           << ", label size=" << d0.h1->GetXaxis()->GetLabelSize()
        //           << ", title font=" << d0.h1->GetXaxis()->GetTitleFont()
        //           << ", title size=" << d0.h1->GetXaxis()->GetTitleSize()
        //           << std::endl;
    // }

    // Check to ensure "A" option is included for the first drawable if it's  TGraph
    if (d0.type==2 && !d0.option.Contains("A", TString::kIgnoreCase)){
        d0.option += " A";
    }
    if (d0.type == 1){
        d0.h1->GetXaxis()->SetLimits(fRanges[0], fRanges[1]);
        d0.h1->GetYaxis()->SetLimits(fRanges[2], fRanges[3]);
        d0.h1->GetYaxis()->SetRangeUser(fRanges[2], fRanges[3]);
        d0.h1->Draw(d0.option);
    } else if (d0.type == 2){
        d0.gr->GetXaxis()->SetLimits(fRanges[0], fRanges[1]);
        d0.gr->GetYaxis()->SetLimits(fRanges[2], fRanges[3]);
        d0.gr->GetYaxis()->SetRangeUser(fRanges[2], fRanges[3]);
        d0.gr->Draw(d0.option);
    } else{
        std::cerr << "No TH2 for DrawAll yet!" << std::endl;
        exit(1);
    }
    

    for (uint8_t ix = 0; ix < fNbinX; ++ix) {
        Double_t xTrue = fRanges[0] + ix * fstepX;

        for (auto& d : fDrawables) {
            Double_t yTrue = 0.0;
            if (d.type == 2) yTrue = d.gr->Eval(xTrue);
            else if (d.type == 1) {
                Int_t bin = d.h1->GetXaxis()->FindBin(xTrue);
                yTrue = d.h1->GetBinContent(bin);
            } else{
                std::cerr << "Should not happen" << std::endl;
                exit(1);
            }
            uint8_t iy = static_cast<uint8_t>((yTrue - fRanges[2]) / fstepY);
            if (iy >= fNbinY || iy < 0) {
                std::cerr << "Warning: y index out of bounds for binning (" << (int)iy << ") in pad " << GetName() 
                << " for value " << yTrue << " (Y range: [" << fRanges[2] << ", " << fRanges[3] << "], step: " << fstepY << ")" << std::endl;
                if (iy == 64) fGrid[ix] |= (1ULL << 63);
            } else{
                fGrid[ix] |= (1ULL << iy);
            }
        }
    }
    

    // Draw all drawable
    for (auto& d : fDrawables) {
        if (isFirst) {
            isFirst = false;
            continue;
        }
        if (d.option.Contains("A", TString::kIgnoreCase)){ // do not draw axis multiple times
            d.option.ReplaceAll("A", "");
        }
        TString opt = d.option;
        if (!opt.Contains("SAME", TString::kIgnoreCase)) { // ensure SAME is added
            opt += " SAME";
        }
        if (d.type==1) d.h1->Draw(opt.Data());
        else d.gr->Draw(opt.Data());
    }
    this->Modified();
    this->Update();
}


void SmartPad::Print(){
    std::cout << "   --- Pad \"" << GetName() << "\" (" << fxlow << "," << fylow << ") to (" << fxup << "," << fyup << ") ---" << std::endl;
    for (size_t idraw=0; idraw < fDrawables.size(); idraw++){
        TObject* obj = nullptr;
        Drawable& d = fDrawables[idraw];
        if (d.type == 1) obj = d.h1;
        else if (d.type == 2) obj = d.gr;
        else if (d.type == 3) obj = d.h2;
        if (obj){
            std::cout << "           - Drawable " << idraw << ": " << obj->GetName() << " of type " << obj->ClassName() << std::endl;
        }
    }
}

void SmartPad::PrintGrid() {
    std::cout << "   Grid for pad \"" << GetName() << "\" (" << (int)fNbinX << "x" << (int)fNbinY << "):" << std::endl;
    // Print from top (y=63) to bottom (y=0) so it looks like a normal plot
    for (int iy = fNbinY - 1; iy >= 0; --iy) {
        std::cout << "   ";
        for (uint8_t ix = 0; ix < fNbinX; ++ix) {
            bool bitSet = (fGrid[ix] >> iy) & 1ULL;
            std::cout << (bitSet ? "🮆" : "🮐");
        }
        std::cout << std::endl;
    }
}

}