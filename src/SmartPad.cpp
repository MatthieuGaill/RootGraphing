
/*

███████╗███╗   ███╗ █████╗ ██████╗ ████████╗    ██████╗  █████╗ ██████╗ 
██╔════╝████╗ ████║██╔══██╗██╔══██╗╚══██╔══╝    ██╔══██╗██╔══██╗██╔══██╗
███████╗██╔████╔██║███████║██████╔╝   ██║       ██████╔╝███████║██║  ██║
╚════██║██║╚██╔╝██║██╔══██║██╔══██╗   ██║       ██╔═══╝ ██╔══██║██║  ██║
███████║██║ ╚═╝ ██║██║  ██║██║  ██║   ██║       ██║     ██║  ██║██████╔╝
╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝       ╚═╝     ╚═╝  ╚═╝╚═════╝ 
                                                                        

*/

#include "SmartPad.hh"

#include <cmath>
#include <cstring>
#include <limits>
#include "TError.h"
#include "TStyle.h"

namespace ROOTEnhancedGraphing {

SmartPad::SmartPad(const char* name, const char* title, Double_t xlow_, Double_t ylow_, Double_t xup_, Double_t yup_)
    : TPad(name, title, xlow_, ylow_, xup_, yup_),
      fxlow(xlow_), fylow(ylow_), fxup(xup_), fyup(yup_)
{
    // TPad sets kCanDelete, so clearing the canvas between pages would delete the pad.
    // The pad is owned by its PadContainer instead.
    ResetBit(kCanDelete);
}

///////////////////////////////////////
// DRAWABLES: TH1, TGraph, TH2, etc. //
///////////////////////////////////////

Bool_t SmartPad::AddDrawable(TObject* obj, Option_t* option, const TString& legEntry) {
    if (obj == nullptr) {
        ::Error("SmartPad::AddDrawable", "Null object provided to pad \"%s\".", GetName());
        return kFALSE;
    }

    Drawable d;
    d.obj = obj;
    d.option = option;
    d.legEntry = legEntry;

    // TH2 inherits from TH1: the dimension decides
    if (TH1* h = dynamic_cast<TH1*>(obj)) {
        if (h->GetDimension() == 1) {
            if (!CheckDims(k1D)) return kFALSE;
            d.kind = Kind::kHist1D;
            if (d.option.IsNull()) d.option = "HIST";
        } else if (h->GetDimension() == 2) {
            if (!CheckDims(k2D)) return kFALSE;
            d.kind = Kind::kHist2D;
            if (d.option.IsNull()) d.option = "COLZ";
        } else {
            ::Error("SmartPad::AddDrawable", "\"%s\": 3D histograms are not supported.", obj->GetName());
            return kFALSE;
        }
    } else if (dynamic_cast<TGraph*>(obj)) {
        if (!CheckDims(k1D)) return kFALSE;
        d.kind = Kind::kGraph;
        if (d.option.IsNull()) d.option = "LP";
    } else {
        ::Error("SmartPad::AddDrawable", "\"%s\" (%s): must inherit from TH1 or TGraph (for now).",
                obj->GetName(), obj->ClassName());
        return kFALSE;
    }

    fDrawables.push_back(d);
    return kTRUE;
}

Bool_t SmartPad::CheckDims(GraphDimension dim){
    if (dimension == kUndefined) dimension = dim;
    else if (dimension != dim){
        ::Error("SmartPad::AddDrawable", "Mixed 1D and 2D objects in the same pad \"%s\".", GetName());
        return kFALSE;
    }
    return kTRUE;
}

Bool_t SmartPad::HasZPalette() const {
    for (const auto& d : fDrawables) {
        if (d.kind == Kind::kHist2D && d.option.Contains("Z", TString::kIgnoreCase)) return kTRUE;
    }
    return kFALSE;
}

TAxis* SmartPad::GetFirstXaxis() const {
    if (fDrawables.empty()) return nullptr;
    const Drawable& d0 = fDrawables[0];
    return d0.kind == Kind::kGraph ? d0.Graph()->GetXaxis() : d0.Hist()->GetXaxis();
}

TAxis* SmartPad::GetFirstYaxis() const {
    if (fDrawables.empty()) return nullptr;
    const Drawable& d0 = fDrawables[0];
    return d0.kind == Kind::kGraph ? d0.Graph()->GetYaxis() : d0.Hist()->GetYaxis();
}

void SmartPad::SetRange(Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax) {
    SetXMin(xmin);
    SetXMax(xmax);
    SetYMin(ymin);
    SetYMax(ymax);
}

///////////////////////////
// FRAME RANGES & LAYOUT //
///////////////////////////

void SmartPad::ComputeFrame() {
    // Read-only on the drawables: the frame carries the ranges, not the user objects
    constexpr Double_t inf = std::numeric_limits<Double_t>::infinity();
    const Bool_t logx = GetLogx();
    const Bool_t logy = GetLogy();

    Double_t xmin = inf, xmax = -inf;
    Double_t ymin = inf, ymax = -inf, yminPos = inf;
    Double_t zmin = inf, zmax = -inf;
    Double_t forcedYmin = inf, forcedYmax = -inf; // from TH1::SetMinimum/SetMaximum
    Bool_t hasHist = kFALSE;

    auto addY = [&](Double_t lo, Double_t hi) {
        ymin = std::min(ymin, lo);
        ymax = std::max(ymax, hi);
        if (lo > 0.) yminPos = std::min(yminPos, lo);
        else if (hi > 0.) yminPos = std::min(yminPos, hi);
    };

    for (const auto& d : fDrawables) {
        if (d.kind == Kind::kHist1D || d.kind == Kind::kHist2D) {
            const TH1* h = d.Hist();
            const TAxis* ax = h->GetXaxis();
            xmin = std::min(xmin, ax->GetBinLowEdge(ax->GetFirst()));
            xmax = std::max(xmax, ax->GetBinUpEdge(ax->GetLast()));
            if (h->GetMinimumStored() != -1111) forcedYmin = std::min(forcedYmin, h->GetMinimumStored());
            if (h->GetMaximumStored() != -1111) forcedYmax = std::max(forcedYmax, h->GetMaximumStored());
        }

        if (d.kind == Kind::kHist1D) {
            const TH1* h = d.Hist();
            const TAxis* ax = h->GetXaxis();
            hasHist = kTRUE;
            TString opt = d.option;
            opt.ToUpper();
            opt.ReplaceAll("SAME", "");
            opt.ReplaceAll("TEXT", "");
            const Bool_t withErrors = opt.Contains("E") && !opt.Contains("HIST");
            for (Int_t i = ax->GetFirst(); i <= ax->GetLast(); ++i) {
                const Double_t c = h->GetBinContent(i);
                const Double_t e = withErrors ? h->GetBinError(i) : 0.;
                addY(c - e, c + e);
            }
        } else if (d.kind == Kind::kHist2D) {
            const TH1* h = d.Hist();
            const TAxis* ay = h->GetYaxis();
            ymin = std::min(ymin, ay->GetBinLowEdge(ay->GetFirst()));
            ymax = std::max(ymax, ay->GetBinUpEdge(ay->GetLast()));
            zmin = std::min(zmin, h->GetMinimum());
            zmax = std::max(zmax, h->GetMaximum());
        } else {
            const TGraph* gr = d.Graph();
            const Int_t n = gr->GetN();
            if (n < 1) continue;
            Double_t gxmin = inf, gxmax = -inf;
            for (Int_t i = 0; i < n; ++i) {
                const Double_t x = gr->GetX()[i];
                const Double_t y = gr->GetY()[i];
                gxmin = std::min(gxmin, x - std::max(0., gr->GetErrorXlow(i)));
                gxmax = std::max(gxmax, x + std::max(0., gr->GetErrorXhigh(i)));
                addY(y - std::max(0., gr->GetErrorYlow(i)), y + std::max(0., gr->GetErrorYhigh(i)));
            }
            // Leave some room around graph points (histograms fill the frame)
            if (logx && gxmin > 0.) {
                const Double_t f = std::pow(gxmax / gxmin, 0.05);
                gxmin /= f;
                gxmax *= f;
            } else {
                const Double_t pad = (gxmax > gxmin) ? 0.05 * (gxmax - gxmin) : 1.;
                gxmin -= pad;
                gxmax += pad;
            }
            xmin = std::min(xmin, gxmin);
            xmax = std::max(xmax, gxmax);
        }
    }

    // X range
    if (!(xmin < xmax)) {
        if (std::isfinite(xmin)) { xmin -= 1.; xmax += 1.; }
        else { xmin = 0.; xmax = 1.; }
    }
    if (logx && xmin <= 0.) xmin = (xmax > 0.) ? 1e-3 * xmax : 1e-3;

    // Y range
    if (dimension == k1D) {
        if (!std::isfinite(ymin)) { ymin = 0.; ymax = 1.; }
        if (logy) {
            const Double_t lo = std::isfinite(yminPos) ? yminPos : 0.1;
            const Double_t hi = (ymax > lo) ? ymax : 10. * lo;
            const Double_t decades = std::max(1., std::log10(hi / lo));
            ymin = lo / std::pow(10., 0.05 * decades);
            ymax = hi * std::pow(10., 0.1 * decades);
        } else {
            // Histograms of positive contents start at 0
            const Double_t lo = (hasHist && ymin >= 0.) ? 0. : ymin - 0.05 * (ymax - ymin);
            ymax = ymax + 0.05 * (ymax - lo);
            ymin = lo;
            if (!(ymin < ymax)) { // flat data
                if (ymin == 0.) {
                    ymax = 1.;
                } else {
                    const Double_t pad = 0.1 * std::abs(ymin);
                    ymin -= pad;
                    ymax += pad;
                }
            }
        }
        if (std::isfinite(forcedYmin)) ymin = forcedYmin;
        if (std::isfinite(forcedYmax)) ymax = forcedYmax;
    }

    fFrame = {xmin, xmax, ymin, ymax};

    // User ranges win
    for (Int_t i = 0; i < 4; ++i) {
        if (fSetRanges & (1 << i)) fFrame[i] = fRanges[i];
    }
    fYmaxFixed = (fSetRanges & 0x8) || std::isfinite(forcedYmax);
    if (!(fFrame[0] < fFrame[1]) || !(fFrame[2] < fFrame[3])) {
        ::Warning("SmartPad::ComputeFrame", "Pad \"%s\": invalid user range, using the automatic one.", GetName());
        fFrame = {xmin, xmax, ymin, ymax};
        fYmaxFixed = std::isfinite(forcedYmax);
    }
    if ((logx && fFrame[0] <= 0.) || (logy && fFrame[2] <= 0.)) {
        ::Warning("SmartPad::ComputeFrame", "Pad \"%s\": log scale with a non-positive range.", GetName());
    }

    fXLabels = Layout::PredictLabels(fFrame[0], fFrame[1], logx);
    fYLabels = Layout::PredictLabels(fFrame[2], fFrame[3], logy);
    fZLabels = (dimension == k2D && zmin < zmax) ? Layout::PredictLabels(zmin, zmax, GetLogz())
                                                 : Layout::AxisLabels{};
}

void SmartPad::Prepare(Double_t canvasWpx, Double_t canvasHpx, const Layout::Sizes& sizes, Bool_t gridDefault) {
    Clear(); // remove the previous drawing: the frame is deleted, user objects are only detached

    // Pad look from the current (SmartCanvas) style. Not UseCurrentStyle(): it would reset log scales.
    SetFillColor(gStyle->GetPadColor());
    SetBorderMode(gStyle->GetPadBorderMode());
    SetBorderSize(gStyle->GetPadBorderSize());
    SetFrameFillColor(gStyle->GetFrameFillColor());
    SetFrameLineColor(gStyle->GetFrameLineColor());
    SetFrameLineWidth(gStyle->GetFrameLineWidth());
    SetFrameBorderMode(gStyle->GetFrameBorderMode());
    SetTicks(gStyle->GetPadTickX(), gStyle->GetPadTickY());
    const Bool_t grid = (dimension == k1D) && (fShowGrid < 0 ? gridDefault : fShowGrid > 0);
    SetGrid(grid, grid); // drawn with the frame, hence below the data

    fSizes = sizes;
    fPadWpx = canvasWpx * GetXWidth();
    fPadHpx = canvasHpx * GetYWidth();
    if (HasDrawables()) ComputeFrame();
}

SmartPad::Margins SmartPad::RequiredMargins() const {
    const auto& s = fSizes;
    Margins m;
    m.top = std::max(s.minTop, 0.6 * s.label);
    m.right = s.minRight;
    m.left = m.bottom = s.outer;
    if (!HasDrawables()) return m;

    const TAxis* xAxis = GetFirstXaxis();
    const TAxis* yAxis = GetFirstYaxis();
    const Bool_t hasXTitle = xAxis && std::strlen(xAxis->GetTitle()) > 0;
    const Bool_t hasYTitle = yAxis && std::strlen(yAxis->GetTitle()) > 0;

    m.left = s.labelOffset + fYLabels.maxWidthEm * s.label + s.outer;
    if (hasYTitle) m.left += s.titleGap + 0.95 * s.title;

    m.bottom = s.labelOffset + 0.9 * s.label + s.outer;
    if (hasXTitle) m.bottom += s.titleGap + 0.95 * s.title;

    // The last x label is centred on the axis end and sticks out to the right
    m.right = std::max(m.right, 0.5 * fXLabels.lastWidthEm * s.label + 0.3 * s.label);
    if (fXLabels.useExponent) m.right = std::max(m.right, 3. * s.label); // "x10^{n}" after the axis end
    if (fYLabels.useExponent) m.top += 1.2 * s.label;                    // "x10^{n}" above the axis

    if (HasZPalette()) {
        // ROOT draws the palette between 0.5% and 5% of the pad width right of the frame
        m.right = 0.05 * fPadWpx + s.labelOffset + fZLabels.maxWidthEm * s.label + s.outer;
        for (const auto& d : fDrawables) {
            if (d.kind == Kind::kHist2D && std::strlen(d.Hist()->GetZaxis()->GetTitle()) > 0) {
                m.right += s.titleGap + 0.95 * s.title;
                break;
            }
        }
    }
    return m;
}

void SmartPad::ApplyMargins(const Margins& marginsPx) {
    fMarginsPx = marginsPx;
    // Keep at least 10% of the pad for the frame
    const Double_t sx = std::min(1., 0.9 * fPadWpx / (marginsPx.left + marginsPx.right));
    const Double_t sy = std::min(1., 0.9 * fPadHpx / (marginsPx.bottom + marginsPx.top));
    if (sx < 1. || sy < 1.) {
        ::Warning("SmartPad::ApplyMargins", "Pad \"%s\" is too small for its labels.", GetName());
        fMarginsPx.left *= sx;
        fMarginsPx.right *= sx;
        fMarginsPx.bottom *= sy;
        fMarginsPx.top *= sy;
    }
    SetLeftMargin(fMarginsPx.left / fPadWpx);
    SetRightMargin(fMarginsPx.right / fPadWpx);
    SetBottomMargin(fMarginsPx.bottom / fPadHpx);
    SetTopMargin(fMarginsPx.top / fPadHpx);
}

void SmartPad::StyleAxis(TAxis* axis, char which) const {
    const auto& s = fSizes;
    const Double_t frameWpx = fPadWpx - fMarginsPx.left - fMarginsPx.right;
    const Double_t frameHpx = fPadHpx - fMarginsPx.bottom - fMarginsPx.top;
    const Layout::AxisLabels& labels = (which == 'x') ? fXLabels : (which == 'y') ? fYLabels : fZLabels;

    axis->SetLabelFont(Layout::kFont);
    axis->SetTitleFont(Layout::kFont);
    axis->SetLabelSize(s.label);
    axis->SetTitleSize(s.title);
    axis->SetLabelColor(1);
    axis->SetTitleColor(1);
    axis->SetAxisColor(1);
    axis->SetNdivisions(Layout::kNdivisions);
    axis->CenterTitle();
    axis->SetNoExponent(!labels.useExponent);

    // Offsets: label offsets are fractions of the pad size, tick lengths of the frame size
    if (which == 'x') {
        axis->SetLabelOffset(s.labelOffset / fPadHpx);
        axis->SetTickLength(s.tick / frameHpx);
        const Double_t titleCenter = s.labelOffset + 0.9 * s.label + s.titleGap + 0.45 * s.title;
        axis->SetTitleOffset(Layout::XTitleOffset(titleCenter, s.title));
    } else {
        axis->SetLabelOffset(s.labelOffset / fPadWpx);
        if (which == 'y') axis->SetTickLength(s.tick / frameWpx);
        const Double_t titleCenter = s.labelOffset + labels.maxWidthEm * s.label + s.titleGap + 0.45 * s.title;
        axis->SetTitleOffset(Layout::YTitleOffset(titleCenter, s.title));
    }
}

/////////////
// DRAWING //
/////////////

void SmartPad::DrawAll() {
    if (!HasDrawables()) return;
    cd();

    // An empty frame carries the axes; every object is then drawn on top of it
    const TAxis* xAxis = GetFirstXaxis();
    const TAxis* yAxis = GetFirstYaxis();
    const TString titles = TString::Format(";%s;%s", xAxis->GetTitle(), yAxis->GetTitle());
    TH1F* frame = DrawFrame(fFrame[0], fFrame[2], fFrame[1], fFrame[3], titles);
    if (!frame) {
        ::Error("SmartPad::DrawAll", "Could not draw the frame of pad \"%s\".", GetName());
        return;
    }
    StyleAxis(frame->GetXaxis(), 'x');
    StyleAxis(frame->GetYaxis(), 'y');

    for (auto& d : fDrawables) {
        TString opt = d.option;
        if (d.kind == Kind::kGraph) {
            // Graph options are single letters: drop the axis ("A") and "SAME"
            opt.ToUpper();
            opt.ReplaceAll("SAME", "");
            opt.ReplaceAll("A", "");
            opt = opt.Strip(TString::kBoth);
            if (opt.IsNull()) opt = "LP";
            d.Graph()->Draw(opt);
            continue;
        }
        if (!opt.Contains("SAME", TString::kIgnoreCase)) opt += " SAME";
        if (d.kind == Kind::kHist2D && HasZPalette()) StyleAxis(d.Hist()->GetZaxis(), 'z');
        d.Hist()->Draw(opt);
    }

    DrawLegend();
    Modified();
}

Double_t SmartPad::ToFracX(Double_t x) const {
    if (GetLogx()) return (std::log10(x) - std::log10(fFrame[0])) / (std::log10(fFrame[1]) - std::log10(fFrame[0]));
    return (x - fFrame[0]) / (fFrame[1] - fFrame[0]);
}

Double_t SmartPad::ToFracY(Double_t y) const {
    if (GetLogy()) return (std::log10(y) - std::log10(fFrame[2])) / (std::log10(fFrame[3]) - std::log10(fFrame[2]));
    return (y - fFrame[2]) / (fFrame[3] - fFrame[2]);
}

void SmartPad::FillGrid() {
    // Highest drawn point of each column (lines, error bars and markers included), in frame fraction.
    // Everything below it counts as occupied.
    ClearGrid();
    fTop.fill(-1.);
    if (dimension != k1D) return;

    const Double_t pxToFx = 1. / FrameWpx();
    const Double_t pxToFy = 1. / FrameHpx();
    const Double_t cell = 1. / fNbinX;

    // Mark the columns overlapping [fx0, fx1] as occupied up to fy
    auto mark = [&](Double_t fx0, Double_t fx1, Double_t fy) {
        if (std::isnan(fx0) || std::isnan(fx1) || !(fy >= 0.)) return; // NaN, or below the frame
        if (fx1 < 0. || fx0 > 1.) return;
        const Int_t i0 = std::max(0, static_cast<Int_t>(std::floor(fx0 * fNbinX)));
        const Int_t i1 = std::min(fNbinX - 1, static_cast<Int_t>(std::floor(fx1 * fNbinX)));
        for (Int_t i = i0; i <= i1; ++i) fTop[i] = std::max(fTop[i], std::min(fy, 1.));
    };

    for (const auto& d : fDrawables) {
        TString opt = d.option;
        opt.ToUpper();
        for (const char* s : {"SAME", "TEXT", "PLC", "PMC", "PFC"}) opt.ReplaceAll(s, "");

        if (d.kind == Kind::kHist1D) {
            const TH1* h = d.Hist();
            const TAxis* ax = h->GetXaxis();
            const Bool_t withErrors = opt.Contains("E") && !opt.Contains("HIST");
            const Bool_t withMarkers = (opt.Contains("P") || withErrors) && h->GetMarkerStyle() > 1;
            const Double_t padPx = 0.5 * h->GetLineWidth() + (withMarkers ? 4. * h->GetMarkerSize() : 0.) + 1.;
            for (Int_t i = ax->GetFirst(); i <= ax->GetLast(); ++i) {
                const Double_t top = h->GetBinContent(i) + (withErrors ? h->GetBinError(i) : 0.);
                mark(ToFracX(ax->GetBinLowEdge(i)), ToFracX(ax->GetBinUpEdge(i)), ToFracY(top) + padPx * pxToFy);
            }
        } else if (d.kind == Kind::kGraph) {
            const TGraph* gr = d.Graph();
            opt.ReplaceAll("A", "");
            const Bool_t withMarkers = opt.Contains("P") || opt.Contains("*") || opt.IsNull();
            const Bool_t withLine = opt.Contains("L") || opt.Contains("C") || opt.Contains("F") || opt.IsNull();
            const Double_t markerPx = withMarkers ? 4. * gr->GetMarkerSize() + 1. : 0.;
            const Double_t linePx = 0.5 * gr->GetLineWidth() + 1.;
            const Int_t n = gr->GetN();
            for (Int_t i = 0; i < n; ++i) {
                const Double_t x = gr->GetX()[i];
                const Double_t y = gr->GetY()[i];
                const Double_t fy = ToFracY(y + std::max(0., gr->GetErrorYhigh(i)));
                mark(ToFracX(x - std::max(0., gr->GetErrorXlow(i))) - markerPx * pxToFx,
                     ToFracX(x + std::max(0., gr->GetErrorXhigh(i))) + markerPx * pxToFx,
                     fy + std::max(markerPx, linePx) * pxToFy);
            }
            if (!withLine) continue;
            // Straight segments in display coordinates: highest point of each column piece
            for (Int_t i = 0; i + 1 < n; ++i) {
                Double_t fxa = ToFracX(gr->GetX()[i]), fya = ToFracY(gr->GetY()[i]);
                Double_t fxb = ToFracX(gr->GetX()[i + 1]), fyb = ToFracY(gr->GetY()[i + 1]);
                if (fxa > fxb) { std::swap(fxa, fxb); std::swap(fya, fyb); }
                if (!(fxb > fxa)) continue;
                auto yAt = [&](Double_t fx) { return fya + (fyb - fya) * (fx - fxa) / (fxb - fxa); };
                const Int_t i0 = std::max(0, static_cast<Int_t>(std::floor(fxa * fNbinX)));
                const Int_t i1 = std::min(fNbinX - 1, static_cast<Int_t>(std::floor(fxb * fNbinX)));
                for (Int_t ic = i0; ic <= i1; ++ic) {
                    const Double_t lo = std::max(fxa, ic * cell), hi = std::min(fxb, (ic + 1) * cell);
                    if (lo > hi) continue;
                    mark(lo, hi, std::max(yAt(lo), yAt(hi)) + linePx * pxToFy);
                }
            }
        }
    }

    for (Int_t ix = 0; ix < fNbinX; ++ix) {
        const Int_t rows = static_cast<Int_t>(std::ceil(fTop[ix] * fNbinY));
        for (Int_t iy = 0; iy < std::min(rows, fNbinY); ++iy) fGrid[ix] |= (1ULL << iy);
    }
}

////////////
// LEGEND //
////////////

Bool_t SmartPad::HasLegend() const {
    if (dimension != k1D) return kFALSE;
    for (const auto& d : fDrawables) {
        if (!d.legEntry.IsNull()) return kTRUE;
    }
    return kFALSE;
}

Double_t SmartPad::LegendTextSizePx() const {
    return (fLegendTextSize > 0.) ? fLegendTextSize * fSizes.scale : fSizes.legend;
}

void SmartPad::LegendSizePx(Double_t text, Double_t& widthPx, Double_t& heightPx, Double_t& symbolPx) const {
    Double_t textWidthEm = 0.;
    Int_t nEntries = 0;
    for (const auto& d : fDrawables) {
        if (d.legEntry.IsNull()) continue;
        textWidthEm = std::max(textWidthEm, Layout::TextWidthEm(d.legEntry.Data()));
        ++nEntries;
    }
    symbolPx = 2.4 * text;                                // line/marker sample, then a gap
    widthPx = symbolPx + textWidthEm * text + 0.4 * text;
    heightPx = nEntries * 1.4 * text;
}

TString SmartPad::LegendOption(const Drawable& d) {
    TString opt = d.option;
    opt.ToUpper();
    for (const char* s : {"SAME", "TEXT", "PLC", "PMC", "PFC"}) opt.ReplaceAll(s, "");

    if (d.kind == Kind::kHist1D) {
        const TH1* h = d.Hist();
        const Bool_t withErrors = opt.Contains("E") && !opt.Contains("HIST");
        if (withErrors) return "lep";
        if (opt.Contains("P")) return "p";
        return (h->GetFillStyle() != 0 && h->GetFillColor() != 0) ? "f" : "l";
    }

    // TGraph: one letter per drawn part
    const TGraph* gr = d.Graph();
    opt.ReplaceAll("A", "");
    TString legOpt;
    if (opt.Contains("L") || opt.Contains("C")) legOpt += "l";
    if (opt.Contains("F")) legOpt += "f";
    if (opt.Contains("P") || opt.Contains("*")) legOpt += "p";
    if ((gr->GetEY() || gr->GetEYhigh()) && !opt.Contains("X")) legOpt += "e";
    return legOpt.IsNull() ? TString("lp") : legOpt;
}

Bool_t SmartPad::PlaceLegend() {
    FillGrid();
    fLegendTextPx = LegendTextSizePx();
    fLegendOverlaps = kFALSE;
    if (!HasLegend() || fLegendUserPos) return kFALSE;

    // The legend width is proportional to its text size: reduce it if the legend is wider than the frame
    Double_t wPx = 0., hPx = 0., symbolPx = 0.;
    LegendSizePx(1., wPx, hPx, symbolPx);
    const Double_t maxText = (FrameWpx() - 2. * fSizes.tick) / (wPx + 0.6);
    fLegendTextPx = std::min(fLegendTextPx, maxText);

    const Double_t text = fLegendTextPx;
    LegendSizePx(text, wPx, hPx, symbolPx);
    const Double_t insetPx = fSizes.tick + 0.3 * text; // keep clear of the ticks
    const Double_t gapPx = 0.4 * text;                  // between the data and the legend

    const Double_t wF = wPx / FrameWpx();
    const Double_t hF = hPx / FrameHpx();
    const Double_t insetX = insetPx / FrameWpx();
    const Double_t y1 = 1. - insetPx / FrameHpx();
    const Double_t y0 = y1 - hF;
    const Double_t gapY = gapPx / FrameHpx();

    // Candidates along the top of the frame: right corner, left corner, then sliding right to left
    std::vector<Double_t> candidates;
    const Double_t xRight = 1. - insetX - wF;
    candidates.push_back(std::max(insetX, xRight));
    if (xRight > insetX) {
        candidates.push_back(insetX);
        for (Int_t k = 1; xRight - k * (1. / fNbinX) > insetX; ++k) candidates.push_back(xRight - k * (1. / fNbinX));
    }

    // Highest data under each candidate
    auto topUnder = [&](Double_t x0) {
        const Int_t i0 = std::max(0, static_cast<Int_t>(std::floor(x0 * fNbinX)));
        const Int_t i1 = std::min(fNbinX - 1, static_cast<Int_t>(std::ceil((x0 + wF) * fNbinX)) - 1);
        Double_t top = -1.;
        for (Int_t i = i0; i <= i1; ++i) top = std::max(top, fTop[i]);
        return top;
    };

    // First free spot; if there is none, the corner needing the least headroom
    Double_t bestX = candidates.front();
    Double_t bestTop = topUnder(bestX);
    for (size_t i = 0; i < candidates.size(); ++i) {
        const Double_t top = topUnder(candidates[i]);
        if (top <= y0 - gapY) {
            bestX = candidates[i];
            bestTop = top;
            break;
        }
        if (i < 2 && top < bestTop - 1e-9) {
            bestX = candidates[i];
            bestTop = top;
        }
    }
    fLegendFrame = {bestX, y0, bestX + wF, y1};
    if (bestTop <= y0 - gapY) return kFALSE;

    // No free spot: raise the top of the frame so that the data passes under the legend
    const Double_t limit = y0 - gapY;
    if (fYmaxFixed || limit < 0.2) {
        fLegendOverlaps = kTRUE;
        return kFALSE;
    }
    const Bool_t logy = GetLogy();
    const Double_t lo = logy ? std::log10(fFrame[2]) : fFrame[2];
    const Double_t hi = logy ? std::log10(fFrame[3]) : fFrame[3];
    // (3% of safety: the pixel padding of lines and markers does not scale with the range)
    const Double_t newHi = lo + bestTop * (hi - lo) / (limit - 0.03);
    fFrame[3] = logy ? std::pow(10., newHi) : newHi;
    fYLabels = Layout::PredictLabels(fFrame[2], fFrame[3], logy);
    return kTRUE;
}

void SmartPad::DrawLegend() {
    if (!HasLegend()) return;

    if (fLegendTextPx < LegendTextSizePx() - 1e-6) {
        ::Warning("SmartPad::DrawLegend", "Pad \"%s\": legend text reduced to %.1f px to fit in the frame.", GetName(), fLegendTextPx);
    }
    if (fLegendOverlaps) {
        ::Warning("SmartPad::DrawLegend", "Pad \"%s\": no room for the legend (fixed y maximum or too many entries), it overlaps the data.", GetName());
    }

    Double_t wPx = 0., hPx = 0., symbolPx = 0.;
    LegendSizePx(fLegendTextPx, wPx, hPx, symbolPx);
    std::array<Double_t, 4> ndc = fLegendNDC;
    if (!fLegendUserPos) {
        // Frame fraction to pad NDC
        const Double_t fl = GetLeftMargin(), fw = 1. - GetLeftMargin() - GetRightMargin();
        const Double_t fb = GetBottomMargin(), fh = 1. - GetBottomMargin() - GetTopMargin();
        ndc = {fl + fLegendFrame[0] * fw, fb + fLegendFrame[1] * fh, fl + fLegendFrame[2] * fw, fb + fLegendFrame[3] * fh};
    }

    TLegend* leg = new TLegend(ndc[0], ndc[1], ndc[2], ndc[3]);
    leg->SetBit(kCanDelete); // deleted with the pad drawing
    leg->SetTextFont(Layout::kFont);
    leg->SetTextSize(fLegendTextPx);
    leg->SetBorderSize(0);
    if (GetGridx() || GetGridy()) {
        leg->SetFillStyle(1001); // hide the grid lines behind the text
        leg->SetFillColor(GetFillColor());
    } else {
        leg->SetFillStyle(0);
    }
    leg->SetMargin(symbolPx / ((ndc[2] - ndc[0]) * fPadWpx));
    for (const auto& d : fDrawables) {
        if (!d.legEntry.IsNull()) leg->AddEntry(d.obj, d.legEntry, LegendOption(d));
    }
    leg->Draw();
}

void SmartPad::PrintInfo() const {
    std::cout << "   --- Pad \"" << GetName() << "\" (" << fxlow << "," << fylow << ") to (" << fxup << "," << fyup << ") ---" << std::endl;
    for (size_t idraw=0; idraw < fDrawables.size(); idraw++){
        const TObject* obj = fDrawables[idraw].obj;
        std::cout << "           - Drawable " << idraw << ": " << obj->GetName() << " of type " << obj->ClassName()
                  << " (option \"" << fDrawables[idraw].option << "\")" << std::endl;
    }
}

void SmartPad::PrintGrid() const {
    std::cout << "   Grid for pad \"" << GetName() << "\" (" << fNbinX << "x" << fNbinY << "):" << std::endl;
    // Print from top (y=63) to bottom (y=0) so it looks like a normal plot
    for (Int_t iy = fNbinY - 1; iy >= 0; --iy) {
        std::cout << "   ";
        for (Int_t ix = 0; ix < fNbinX; ++ix) {
            const bool bitSet = (fGrid[ix] >> iy) & 1ULL;
            std::cout << (bitSet ? "🮆" : "🮐");
        }
        std::cout << std::endl;
    }
}

}
