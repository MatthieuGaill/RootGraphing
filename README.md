# SmartCanvas: consistent ROOT plots with little code

SmartCanvas is a small C++ library on top of [ROOT](https://root.cern) that lays out and styles plots automatically and saves them to a multi-page PDF.
You give it histograms and graphs; it handles the rest:

- **Layouts**: single pad, left/right, up/down, or any custom set of pads, one layout per PDF page.
- **Consistent style**: fonts, margins and ticks are computed in pixels, so text looks identical in every pad and layout. Axis labels never get clipped, and frames of stacked pads line up.
- **Automatic ranges**: the axes cover all the objects of a pad, with error bars included. Histograms start at 0, and log scales are supported.
- **Automatic legend**: it is placed where it does not hide the data, with extra headroom when needed.
- **Grid and ticks**: soft gray grid, matplotlib-like outside ticks (both can be switched off).
- **Non-intrusive**: your histograms and graphs are never modified (bins, ranges and colors are kept), and `gStyle` is left untouched.

Supported objects: `TH1` (1D), `TH2` (drawn as a color map), and `TGraph` with all its subclasses (`TGraphErrors`, ...).

```cpp
#include "SmartCanvas/SmartCanvas.hh"

ROOTEnhancedGraphing::SmartCanvas c("c", "My plots", "plots", 800, 600); // -> plots.pdf
c.SetPage(0, "LR");                                   // page 0: two pads side by side
c.AddDrawable(0, hData, "E", "Data");                 // pad 0, draw option, legend entry
c.AddDrawable(0, hSim, "hist", "Simulation");
c.AddDrawable(1, gEff, "P", "Efficiency");            // pad 1
c.SetPage(1, "single");                               // page 1: one pad
c.AddDrawable(0, hMap, "COLZ");
c.DrawAndSave();
```

---

## 1. Build the library

You need ROOT 6 (built with C++17 or later), `g++` and `make`.
Build SmartCanvas **on the machine where it will be used**, with **the same ROOT and compiler as your program**.
A library built against another ROOT version will not link, or will crash.

On himster3, for example:

```bash
# 1. Set up the same ROOT environment as your project (same thisroot.sh / module as P2Sim-VMM)
source /path/to/root/bin/thisroot.sh
root-config --version          # check it is the expected version

# 2. Get the sources (anywhere, e.g. ~/src)
git clone https://github.com/MatthieuGaill/RootGraphing.git ~/src/RootGraphing
cd ~/src/RootGraphing

# 3. Build and install the headers and libraries in ~/SmartCanvas
make lib
make install                   # or: make install PREFIX=/some/other/path
```

This gives:

```
~/SmartCanvas/
├── include/SmartCanvas/   SmartCanvas.hh, SmartPad.hh, PadContainer.hh, LayoutTools.hh
└── lib/                   libSmartCanvas.a  (static)   libSmartCanvas.so  (shared)
```

The sources are only needed again to update or rebuild the library, for example after a ROOT update.

Other targets: `make` builds the libraries and the demo `build/smart_canvas`, `make run` runs the demo (writes `output.pdf`), and `make clean` removes `build/`.

---

## 2. Use it in an existing makefile project (e.g. `~/P2/P2Sim-VMM`)

Your project does **not** compile any SmartCanvas source. It only links the prebuilt library, which takes two lines in its makefile.

### Option A: static library (recommended)

The code SmartCanvas needs is copied into your executable at link time.
There is no extra runtime dependency, nothing to set in `LD_LIBRARY_PATH`, and the executable still works when you move it or run it on batch nodes.

Add to the makefile of your project, after `CXXFLAGS` and `LDLIBS` (or `LIBS`, `LDFLAGS`, depending on what your makefile uses) are defined:

```make
SMARTCANVAS = $(HOME)/SmartCanvas
CXXFLAGS += -I$(SMARTCANVAS)/include
LDLIBS   := $(SMARTCANVAS)/lib/libSmartCanvas.a $(LDLIBS)
```

The library must come **before** the ROOT libraries on the link line, hence the `:=` that puts it first.
Your link rule must use that variable, as in `$(CXX) -o $@ $^ $(LDLIBS)`.

### Option B: shared library

This keeps your executable smaller and picks up a rebuilt library without relinking.
The executable then needs the `.so` at run time; the rpath below records its location:

```make
SMARTCANVAS = $(HOME)/SmartCanvas
CXXFLAGS += -I$(SMARTCANVAS)/include
LDLIBS   := -L$(SMARTCANVAS)/lib -lSmartCanvas -Wl,-rpath,$(SMARTCANVAS)/lib $(LDLIBS)
```

### In your code

```cpp
#include "SmartCanvas/SmartCanvas.hh"
```

SmartCanvas only uses ROOT libraries that any ROOT program already links (`root-config --libs`).

### From a ROOT macro

```cpp
// in the macro, or in your rootlogon.C
gSystem->Load("~/SmartCanvas/lib/libSmartCanvas.so");
gInterpreter->AddIncludePath("~/SmartCanvas/include");
```

Then `#include "SmartCanvas/SmartCanvas.hh"` and use it as in C++.

---

## 3. Usage

Everything lives in the namespace `ROOTEnhancedGraphing`.

### Canvas and output

```cpp
SmartCanvas c(name, title, savePath, width, height, verbose = false);
...
c.DrawAndSave();   // draws every page and writes <savePath>.pdf (one page per SetPage)
```

- The canvas size sets the pixel scale. Text sizes are given for an 800×600 canvas and scale with it, so a 1600×1200 canvas gives the same picture at twice the resolution.
- `DrawAndSave()` can be called several times, for example after adding more objects.

### Pages and layouts

```cpp
c.SetPage(0, "single");                         // one pad
c.SetPage(1, "LR");                             // left / right, 50% each
c.SetPage(2, "UD");                             // up / down, 50% each
c.SetPage(3, 0.7, "LR");                        // left pad 70% of the width
c.SetPage(4, 0.3, "UD");                        // bottom pad 30% of the height (ratio plot)
c.SetPage(5, {{0, 0, 0.5, 1},                   // custom pads {xlow, ylow, xup, yup} in [0, 1]
              {0.5, 0.5, 1, 1},
              {0.5, 0, 1, 0.5}});
c.SetPage(1);                                   // select an existing page again
```

- Pads are numbered in the order above: LR is left then right, UD is top then bottom, custom pads follow your list.
- `AddDrawable` always adds to the **active** page, which is the last one created or selected.
- If `SetPage` fails (for example the index already exists), no page is active, so later `AddDrawable` calls report an error instead of filling the wrong page.

### Adding objects

```cpp
c.AddDrawable(padIndex, object, option, legendEntry = "");
```

| Object | Default option | Notes |
|---|---|---|
| `TH1` | `"HIST"` | Any THistPainter option: `"E"`, `"P"`, `"BAR"`, ... (`SAME` is added for you) |
| `TGraph` and subclasses | `"LP"` | Graph letters `L`, `P`, `C`, `F`, `*` (no `A` needed) |
| `TH2` | `"COLZ"` | A 2D pad cannot also hold 1D objects |

- Several objects in a pad are overlaid; the first one gives the axis titles.
- Colors, line widths and markers are yours: set them on the objects (`SetLineColor`, ...).
- The same object can appear in several pads and pages.

### Per-pad settings

```cpp
SmartPad* p = c.GetPad(page, padIndex);

p->SetLogy();                     // log scales (also SetLogx, SetLogz)
p->SetRange(xmin, xmax, ymin, ymax);   // fixed ranges...
p->SetXMin(0); p->SetYMax(1e3);        // ...or only some bounds, the others stay automatic
p->ResetRanges();

p->SetLegendTextSize(22);         // legend text size for this pad (px for 800x600)
p->SetLegendPosition(0.6, 0.6, 0.9, 0.85);  // fixed legend position (pad NDC) instead of automatic
p->SetAutoLegendPosition();

p->ShowGrid(false);               // grid off (or on) for this pad only
p->UseDefaultGrid();              // back to the canvas setting
```

`SetMinimum` and `SetMaximum` on a histogram are also respected.

### Canvas-wide settings

| Call | Default | Effect |
|---|---|---|
| `c.SetLegendTextSize(px)` | 18 | Legend text size of every pad (px for an 800×600 canvas) |
| `c.ShowGrid(bool)` | on | Soft gray grid on 1D pads |
| `c.SetTicksOutside(bool)` | on | Ticks pointing out of the frame |
| `c.SetMirrorTicks(bool)` | off | Ticks also on the top and right sides |
| `c.GetStyle()` | | The canvas `TStyle`, e.g. `GetStyle()->SetGridColor(...)`, `SetPalette(...)` |

The defaults of all sizes (labels, titles, ticks, legend) and options are in [include/LayoutTools.hh](include/LayoutTools.hh) (`Layout::Sizes`, `Layout::Options`), and the style in `SmartCanvas::ApplyCustomStyle()` ([src/SmartCanvas.cpp](src/SmartCanvas.cpp)).

### Legend

- A legend is drawn in a 1D pad as soon as one of its objects has a legend entry. TLatex syntax is supported: `"#mu = 5"`, `"#sigma_{tot}"`.
- The symbol follows the draw option: line, error bars, markers, or fill.
- It is placed in the top-right corner, else the top-left one, else in between, without covering any data, error bars or markers.
- If there is no room, the y axis is extended upwards to make some. With a fixed y maximum, the legend overlaps the data and a warning is printed.
- If the legend is wider than the frame, its text is reduced to fit, with a warning.

### Debugging

```cpp
c.PrintInfo();                        // pages, pads and their objects
c.GetPad(0, 1)->PrintGrid();          // occupancy map used for the legend placement (after DrawAndSave)
```

Errors and warnings use ROOT's messaging (`Error in <SmartCanvas::SetPage>: ...`).
Pass `verbose = true` to the constructor to also see ROOT's info messages while saving.

---

## 4. Things to know

- **Batch mode**: creating a `SmartCanvas` switches ROOT to batch mode (`gROOT->SetBatch(kTRUE)`) for the whole program, so no window is opened.
- **Object lifetime**: SmartCanvas does not copy or own your objects. Keep them alive until `DrawAndSave()` has run. A histogram created while a `TFile` is open belongs to that file and is deleted when the file is closed. Call `DrawAndSave()` before `file->Close()`, or detach the histogram with `h->SetDirectory(nullptr)`.
- **TH2**: its Z axis gets the canvas fonts so the color scale matches; this is the only change made to a user object.
- **Output**: PDF only, one page per `SetPage` index, in increasing index order.

---

## 5. Project layout

```
include/   public headers (SmartCanvas.hh is the one to include)
src/       library sources
main.cpp   demo / test program (make run -> output.pdf)
makefile   builds build/lib/libSmartCanvas.{a,so} and the demo
```
