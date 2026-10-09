


void test(){
    gROOT->SetBatch(kTRUE); 
    TH1D* h1 = new TH1D("h1", "Gaussian Histogram;X-axis;Y-axis", 100, -5, 5);
    for (int i = 0; i < 10000; ++i) {
        h1->Fill(gRandom->Gaus());
    }
    // another with poisson data
    TH1D* h2 = new TH1D("h2", "Poisson Histogram;X-axis;Y-axis", 100, 0, 20);
    for (int i = 0; i < 10000; ++i) {
        h2->Fill(gRandom->Poisson(5));
    }

    TCanvas* c1 = new TCanvas("c1", "Test Canvas", 1000, 600);
    // TCanvas* c2 = new TCanvas("c2", "Test Canvas 2", 600, 1000);

    Double_t Xwidth = 1;
    Double_t Ywidth = 0.5;
    TPad* lpad = new TPad("lpad", "Left Pad", 0.0, 0.0, Xwidth, Ywidth);
    // TPad* rpad = new TPad("rpad", "Right Pad", 0.5, 0, 1, 1);
    lpad->Draw();
    // rpad->Draw();

    Double_t CoeffPadY = 1;//std::abs(sqrt(Xwidth * Ywidth))/sqrt(0.5*0.5);

    Double_t YMargin = 0;

    // Title size
    Double_t TitleSize = 0.04* CoeffPadY;
    YMargin += TitleSize;

    // Title offset
    Double_t TitleOffset = 1.2 / CoeffPadY;
    YMargin += TitleOffset;

    // Label size
    Int_t Ncharacters = 3;
    Double_t LabelSize = 0.033 * CoeffPadY;
    YMargin += LabelSize * CoeffPadY * Ncharacters;

    // Extra space
    // YMargin += 1000;

    h1->GetYaxis()->SetTitleSize(TitleSize);
    h1->GetYaxis()->SetTitleOffset(TitleOffset + LabelSize * Ncharacters);
    h1->GetYaxis()->SetLabelSize(LabelSize);

    lpad->SetLeftMargin(YMargin / CoeffPadY);
    lpad->SetRightMargin(0.03);



    // c1->Print("outputtest.pdf[");
    lpad->cd();
    h1->Draw("hist");
    c1->SaveAs("outputtest.pdf");
    // rpad->cd();
    // h2->Draw("hist");
    // c1->Print("outputtest.pdf");
    // c2->Print("outputtest.pdf[");

    // h2->Draw("hist");
    // c2->Print("outputtest.pdf");
    // c1->Print("outputtest.pdf]");


}