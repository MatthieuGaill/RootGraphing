/*

██████╗  █████╗ ██████╗      ██████╗ ██████╗ ███╗   ██╗████████╗ █████╗ ██╗███╗   ██╗███████╗██████╗ 
██╔══██╗██╔══██╗██╔══██╗    ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔══██╗██║████╗  ██║██╔════╝██╔══██╗
██████╔╝███████║██║  ██║    ██║     ██║   ██║██╔██╗ ██║   ██║   ███████║██║██╔██╗ ██║█████╗  ██████╔╝
██╔═══╝ ██╔══██║██║  ██║    ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══██║██║██║╚██╗██║██╔══╝  ██╔══██╗
██║     ██║  ██║██████╔╝    ╚██████╗╚██████╔╝██║ ╚████║   ██║   ██║  ██║██║██║ ╚████║███████╗██║  ██║
╚═╝     ╚═╝  ╚═╝╚═════╝      ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝
                                                                                                                                                                                                                          
*/

#include "PadContainer.hh"
#include <cmath>

namespace ROOTEnhancedGraphing {

PadContainer::PadContainer(Int_t index)
    : fIndex(index)
{
}

PadContainer::~PadContainer() {
    // unique_ptr automatically cleans up fPads
}

Bool_t PadContainer::AddPad(const char* name, const char* title, Double_t x1, Double_t y1, Double_t x2, Double_t y2) {
    if (!(x1 < x2) || !(y1 < y2) || x1 < 0 || x2 > 1 || y1 < 0 || y2 > 1) {
        ::Error("PadContainer::AddPad", "Pad \"%s\": invalid coordinates (%g,%g)-(%g,%g), need 0 <= low < up <= 1.",
                name, x1, y1, x2, y2);
        return kFALSE;
    }
    fPads.push_back(std::make_unique<SmartPad>(name, title, x1, y1, x2, y2));
    return kTRUE;
}

void PadContainer::cd(Int_t padIndex) {
    if (SmartPad* pad = GetPad(padIndex)) pad->cd();
}

void PadContainer::ApplyAlignedMargins() {
    const size_t n = fPads.size();
    std::vector<SmartPad::Margins> margins(n);
    for (size_t i = 0; i < n; ++i) {
        if (fPads[i]->HasDrawables()) margins[i] = fPads[i]->RequiredMargins();
    }

    // Pads stacked in a column share their left/right margins, pads in a row their bottom/top ones,
    // so that the frames line up
    auto same = [](Double_t a, Double_t b) { return std::abs(a - b) < 1e-6; };
    std::vector<SmartPad::Margins> aligned = margins;
    for (size_t i = 0; i < n; ++i) {
        const SmartPad* pi = fPads[i].get();
        for (size_t j = 0; j < n; ++j) {
            const SmartPad* pj = fPads[j].get();
            if (j == i || !pj->HasDrawables()) continue;
            if (same(pi->fxlow, pj->fxlow) && same(pi->fxup, pj->fxup)) {
                aligned[i].left = std::max(aligned[i].left, margins[j].left);
                aligned[i].right = std::max(aligned[i].right, margins[j].right);
            }
            if (same(pi->fylow, pj->fylow) && same(pi->fyup, pj->fyup)) {
                aligned[i].bottom = std::max(aligned[i].bottom, margins[j].bottom);
                aligned[i].top = std::max(aligned[i].top, margins[j].top);
            }
        }
    }

    for (size_t i = 0; i < n; ++i) {
        if (fPads[i]->HasDrawables()) fPads[i]->ApplyMargins(aligned[i]);
    }
}

void PadContainer::DrawAllPads(TVirtualPad* parent, const Layout::Sizes& baseSizes, const Layout::Options& options) {
    // Text sizes are in pixels, scaled with the canvas so that the style does not depend on its size
    const Double_t canvasW = parent->GetWw();
    const Double_t canvasH = parent->GetWh();
    const Layout::Sizes sizes = baseSizes.Scaled(std::min(canvasW, canvasH) / Layout::kRefPixels);

    for (auto& pad : fPads) {
        pad->Prepare(canvasW, canvasH, sizes, options);
        if (!pad->HasDrawables()) {
            ::Warning("PadContainer::DrawAllPads", "Pad \"%s\" of page %d has no drawables added. Skipping.", pad->GetName(), fIndex);
        }
    }

    // Making room for a legend changes the y range, hence possibly the labels and the margins: iterate
    for (Int_t iter = 0; iter < 4; ++iter) {
        ApplyAlignedMargins();
        Bool_t changed = kFALSE;
        for (auto& pad : fPads) {
            if (pad->HasDrawables() && pad->PlaceLegend()) changed = kTRUE;
        }
        if (!changed) break;
    }

    for (auto& pad : fPads) {
        if (!pad->HasDrawables()) continue;
        parent->cd();     // Make sure parent is current before drawing subpad
        pad->Draw();      // Draw the pad onto the parent
        pad->DrawAll();   // Draw content inside the pad
    }
}


void PadContainer::PrintInfo() const {
    std::cout << "--------- Page " << fIndex << ", " << fPads.size() << " Pads, Mode: " << GetStringFromMode() << " --------" << std::endl;
    for (const auto& pad : fPads) {
        pad->PrintInfo();
    }
}

}
