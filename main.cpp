#include "SmartCanvas.hh"
#include "TStyle.h"
#include "TROOT.h"
#include "TRandom.h"



int main() {
    // Create histogram with random gaussian data
    TH1D* h1 = new TH1D("h1", "Gaussian Histogram;X-axis;Y-axis", 100, -5, 5);
    for (int i = 0; i < 10000; ++i) {
        h1->Fill(gRandom->Gaus());
    }
    // another with poisson data
    TH1D* h2 = new TH1D("h2", "Poisson Histogram;X-axis;Y-axis", 100, 0, 20);
    for (int i = 0; i < 10000; ++i) {
        h2->Fill(gRandom->Poisson(5));
    }
    // Landau distribution
    TH1D* h3 = new TH1D("h3", "Landau Histogram;X-axis;Y-axis", 100, -5, 5);
    for (int i = 0; i < 10000; ++i) {
        h3->Fill(gRandom->Landau(1, 4));
    }
    ROOTEnhancedGraphing::SmartCanvas* c1 = new ROOTEnhancedGraphing::SmartCanvas("c1", "Title", "output", 800, 600, false);
    c1->SetPage(0, "LR"); // for Left-Right pads layout
    c1->AddDrawable(0, h1, "hist");
    c1->AddDrawable(1, h2, "hist");
    c1->AddDrawable(1, h1, "hist");
    
    c1->SetPage(1, "UD"); // for Up-Down pads layout
    c1->AddDrawable(0, h1, "hist");
    c1->AddDrawable(1, h3, "hist");

    // c1->PrintInfo();

    c1->DrawAndSave();
    // c1->GetPad(0, 1)->PrintGrid();
    c1->Close();

    return 0;
}