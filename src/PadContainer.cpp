/*

██████╗  █████╗ ██████╗      ██████╗ ██████╗ ███╗   ██╗████████╗ █████╗ ██╗███╗   ██╗███████╗██████╗ 
██╔══██╗██╔══██╗██╔══██╗    ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔══██╗██║████╗  ██║██╔════╝██╔══██╗
██████╔╝███████║██║  ██║    ██║     ██║   ██║██╔██╗ ██║   ██║   ███████║██║██╔██╗ ██║█████╗  ██████╔╝
██╔═══╝ ██╔══██║██║  ██║    ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══██║██║██║╚██╗██║██╔══╝  ██╔══██╗
██║     ██║  ██║██████╔╝    ╚██████╗╚██████╔╝██║ ╚████║   ██║   ██║  ██║██║██║ ╚████║███████╗██║  ██║
╚═╝     ╚═╝  ╚═╝╚═════╝      ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝
                                                                                                                                                                                                                          
*/

#include "PadContainer.hh"

namespace ROOTEnhancedGraphing {

PadContainer::PadContainer(Int_t index)
    : fIndex(index)
{
    fPads.clear();
    fNPads = 0;
}

PadContainer::~PadContainer() {
    // unique_ptr automatically cleans up fPads
}

void PadContainer::AddPad(const char* name, const char* title, Double_t x1, Double_t y1, Double_t x2, Double_t y2) {
    fPads.push_back(std::make_unique<SmartPad>(name, title, x1, y1, x2, y2));
    fNPads++;
}

void PadContainer::cd(Int_t padIndex) {
    if (padIndex < 0 || padIndex >= fPads.size()) {
        std::cerr << "Pad index out of range!" << std::endl;
        return;
    }
    fPads[padIndex]->cd();
}

void PadContainer::DrawAllPads(TVirtualPad* parent) {

    if (mode == kUpDown) fSameYMargin = true;

    Double_t YMargin = 0.0;
    Double_t XMargin = 0.0;
    Double_t yTitleOffsetRoot = 0.0;
    Double_t xTitleOffsetRoot = 0.0;
 

    for (auto& pad : fPads) {
        // Get axis and remove potential exponent
        TAxis* xAxis = pad->GetXaxis();
        TAxis* yAxis = pad->GetYaxis();
        xAxis->SetNoExponent();
        yAxis->SetNoExponent();
        if (pad->GetFirstType() == 0) {
            std::cerr << "Warning: Pad \"" << pad->GetName() << "\" has no drawables added. Skipping." << std::endl;
            continue;
        }

        Int_t Ncharacters = pad->SetFinalRanges();
        Double_t padWidth = pad->GetXWidth();
        Double_t padHeight = pad->GetYWidth();
                
        if (YMargin == 0.0 || !fSameYMargin) {
            // Title size
            YMargin = fYTitleLabelSize;
            XMargin = fXTitleLabelSize;

            // Label size
            YMargin += fYLabelSize * Ncharacters - fYLabelSize*(0.5 - padWidth)/0.4; 
            XMargin += fXLabelSize;

            // Title offset is a ROOT axis-unit multiplier, not a margin component.
            yTitleOffsetRoot = (fYTitleLabelOffset * Ncharacters) / fYTitleLabelSize + (0.5 - padWidth)/0.6;
            xTitleOffsetRoot = fXTitleLabelOffset / fXTitleLabelSize;
            YMargin += fYTitleLabelOffset;
            XMargin += fXTitleLabelOffset;

            // Extra margin
            YMargin += fYExtraMargin + (0.5 - padWidth)/0.5;
            XMargin += fXExtraMargin;
        }

        yAxis->SetLabelSize(fYLabelSize);
        xAxis->SetLabelSize(fXLabelSize);
        yAxis->SetTitleOffset(yTitleOffsetRoot);
        xAxis->SetTitleOffset(xTitleOffsetRoot);
        yAxis->SetTitleSize(fYTitleLabelSize);
        xAxis->SetTitleSize(fXTitleLabelSize);
        
        pad->SetLeftMargin(YMargin);
        pad->SetRightMargin(0.005);
        pad->SetBottomMargin(XMargin);
        pad->SetTopMargin(0.04);
        
    }
    for (auto& pad : fPads) { 
        parent->cd();     // Make sure parent is current before drawing subpad
        pad->Draw();      // Draw the pad onto the parent
        pad->cd();        // cd into the pad
        pad->DrawAll();   // Draw content inside the pad
    }
}


/////////////////////////////
// LEGENDS AND OTHER BOXES //
/////////////////////////////

void PadContainer::SetLegend(Option_t* option){
    DrawableBox boxInfo;
    boxInfo.type = 1; // TLegend
    boxInfo.leg = new TLegend();
    boxInfo.option = option;
    fDrawableBoxes.push_back(boxInfo);
}

void PadContainer::SetLegend(Double_t x1, Double_t y1, Double_t x2, Double_t y2, Option_t* option){

}

TLegend* PadContainer::GetLegend() {
    for (const auto& boxInfo : fDrawableBoxes) {
        if (boxInfo.type == 1) return boxInfo.leg;
    }
    std::cerr << "GetLegend(): No TLegend found!" << std::endl;
    return nullptr;
}


Double_t PadContainer::EstimateLegendWidth(TLegend* leg) {
    if (!leg) return 0.0;
    
    // Get text attributes from legend
    Double_t textSize = leg->GetTextSize();
    Int_t textFont = leg->GetTextFont();
    
    TLatex latex;
    latex.SetTextSize(textSize);
    latex.SetTextFont(textFont);
    
    Double_t maxWidth = 0.0;
    
    // Iterate over all entries in the legend
    TList* primitives = leg->GetListOfPrimitives();
    if (primitives) {
        TIter next(primitives);
        TObject* obj;
        while ((obj = next())) {
            TLegendEntry* entry = dynamic_cast<TLegendEntry*>(obj);
            if (entry) {
                const char* label = entry->GetLabel();
                Double_t w = latex.GetXsize();  // or use GetBBox
                if (w > maxWidth) maxWidth = w;
            }
        }
    }
    
    // Add padding for the symbol/marker box (typically ~0.05-0.08 in NDC)
    Double_t symbolWidth = 0.08;  // estimate for "LP" style marker+line
    Double_t padding = 0.02;      // left/right padding
    
    return maxWidth + symbolWidth + 2 * padding;
}

// void PadContainer::DrawAllBoxes() {

//     for (auto& [index, boxInfo] : fDrawableBoxes) {
//         if (boxInfo.type == 0 && boxInfo.box) { // TBox
//             boxInfo.box->Draw(boxInfo.option.Data());
//         } else if (boxInfo.type == 1 && boxInfo.leg) { // TLegend
//             boxInfo.leg->Draw(boxInfo.option.Data());
//         }
//     }
// }



void PadContainer::ClearLegend(Int_t legIndex) {
    for (size_t i=0; i < fDrawableBoxes.size(); i++) {
        if (fDrawableBoxes[i].type == 1 && i == legIndex) {
            delete fDrawableBoxes[i].leg;
            fDrawableBoxes.erase(fDrawableBoxes.begin() + i);
            return;
        }
    }
}



void PadContainer::Print(){
    std::cout << "--------- Page " << fIndex << ", " << fNPads << " Pads, Mode: " << GetStringFromMode() << " --------" << std::endl;
    for (size_t ipad=0; ipad < fPads.size(); ipad++){
        fPads[ipad]->Print();
    }
}

}