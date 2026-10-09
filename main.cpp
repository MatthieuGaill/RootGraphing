#include "SmartCanvas.hh"
#include "TRandom.h"
#include "TGraphErrors.h"
#include "TH2D.h"



int main() {

    //////////////////////////////
    // Create objects to plot   //
    //////////////////////////////

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
    h2->SetLineColor(kRed+1); // user styling is kept
    // Landau distribution
    TH1D* h3 = new TH1D("h3", "Landau Histogram;X-axis;Y-axis", 100, -5, 45);
    for (int i = 0; i < 10000; ++i) {
        h3->Fill(gRandom->Landau(1, 4));
    }
    // Exponential decay with errors
    TGraphErrors* g1 = new TGraphErrors();
    for (int i = 0; i < 20; ++i) {
        double x = 0.5 * i;
        g1->SetPoint(i, x, 1000 * std::exp(-x / 3) + gRandom->Gaus(0, 20));
        g1->SetPointError(i, 0, 20);
    }
    g1->SetTitle("Decay;Time [s];Counts");
    g1->SetMarkerStyle(20);
    // 2D gaussian
    TH2D* h4 = new TH2D("h4", "2D Gaussian;X-axis;Y-axis;Entries", 50, -4, 4, 50, -4, 4);
    for (int i = 0; i < 100000; ++i) {
        h4->Fill(gRandom->Gaus(), gRandom->Gaus(0, 1.5));
    }

    //////////////////////////////
    // Plotting                 //
    //////////////////////////////

    ROOTEnhancedGraphing::SmartCanvas* c1 = new ROOTEnhancedGraphing::SmartCanvas("c1", "Title", "output", 800, 600, false);
    c1->SetPage(0, "LR"); // for Left-Right pads layout
    c1->AddDrawable(0, h1, "hist", "Gaussian");
    c1->AddDrawable(1, h2, "hist", "Poisson (#mu = 5)");
    c1->AddDrawable(1, h1, "hist", "Gaussian");

    c1->SetPage(1, "UD"); // for Up-Down pads layout
    c1->AddDrawable(0, h1, "hist");
    c1->AddDrawable(1, h3, "hist", "Landau");

    // Custom layout: one pad on the left, two stacked on the right
    c1->SetPage(2, {{0, 0, 0.5, 1}, {0.5, 0.5, 1, 1}, {0.5, 0, 1, 0.5}});
    c1->AddDrawable(0, h4, "COLZ");
    c1->AddDrawable(1, g1, "P", "Measured decay");
    c1->AddDrawable(2, h1, "E", "Gaussian");
    c1->GetPad(2, 2)->SetLogy();


    c1->DrawAndSave();
    delete c1;

    return 0;
}
