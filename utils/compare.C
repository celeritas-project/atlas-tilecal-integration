#include <TBranch.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TH1D.h>
#include <TLeaf.h>
#include <TLegend.h>
#include <TObjArray.h>
#include <TPaveText.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TTree.h>

#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <string>

//---------------------------------------------------------------------------//
/*!
 * Compare Geant4 vs. Celeritas root outputs.
 *
 * Usage:
 * $ root compare.C
 */
void compare()
{
  // Load tfiles and ttrees
  auto const g4_file = new TFile("ATLTileCalTBout_Run0.root", "read");
  auto const cel_file = new TFile("../g4-10gev-nomsc/ATLTileCalTBout_Run0.root", "read");

  auto const g4_tree = (TTree*)g4_file->Get("ATLTileCalTBout");
  auto const cel_tree = (TTree*)cel_file->Get("ATLTileCalTBout");

  // Get list of branches
  auto branches = g4_tree->GetListOfBranches();
  auto const num_branches = branches->GetEntries();
  if (num_branches != cel_tree->GetListOfBranches()->GetEntries()) {
    // Lazy assertion
    std::cerr << "ERROR: Files have different number of branches" << std::endl;
    return;
  }

  // Store branch names and their x-axis limits to correctly init histograms
  struct LeafLimits
  {
    double min{std::numeric_limits<double>::max()};
    double max{std::numeric_limits<double>::lowest()};
  };

  std::map<std::string, LeafLimits> map_branches_limits;

  for (auto i = 0; i < num_branches; i++) {
    auto const branch_name = branches->At(i)->GetName();
    map_branches_limits.insert({branch_name, LeafLimits{}});

    for (auto j = 0; j < g4_tree->GetEntries(); j++) {
      g4_tree->GetEntry(j);
      cel_tree->GetEntry(j);
      auto cel_val = cel_tree->GetLeaf(branch_name)->GetValue();
      auto g4_val = g4_tree->GetLeaf(branch_name)->GetValue();

      // Find min/max values between G4 and Celeritas
      auto min_val = std::min(cel_val, g4_val);
      auto max_val = std::max(cel_val, g4_val);
      // Store global min/max values
      map_branches_limits[branch_name].min =
        std::min(map_branches_limits[branch_name].min, min_val);
      map_branches_limits[branch_name].max =
        std::max(map_branches_limits[branch_name].max, max_val);
    }
  }

  // Create histograms
  TH1D* histograms_g4[num_branches];
  TH1D* histograms_cel[num_branches];

  for (int i = 0; i < num_branches; i++) {
    std::string branch_name = branches->At(i)->GetName();
    auto const& key = map_branches_limits.find(branch_name);

    // Lazily set up a reasonable number of bins
    int num_bins = key->second.max - key->second.min;
    if (! num_bins) {
      num_bins = 100;
    }
    num_bins = std::min(num_bins, 100);

    // Initialize histograms
    if (key->first == "EdepSum") {
      // Manually update x-axis limits for EdepSum only
      histograms_g4[i] = new TH1D("", branch_name.c_str(), num_bins, 0, 2.5e3);
      histograms_cel[i] = new TH1D("", branch_name.c_str(), num_bins, 0, 2.5e3);
    }
    else {
      histograms_g4[i] =
        new TH1D("", branch_name.c_str(), num_bins, key->second.min, key->second.max);
      histograms_cel[i] =
        new TH1D("", branch_name.c_str(), num_bins, key->second.min, key->second.max);
    }
  }

  // Fill histograms
  for (int i = 0; i < num_branches; i++) {
    auto const name = branches->At(i)->GetName();

    for (int j = 0; j < g4_tree->GetEntries(); j++) {
      g4_tree->GetEntry(j);
      histograms_g4[i]->Fill(g4_tree->GetLeaf(name)->GetValue());

      cel_tree->GetEntry(j);
      histograms_cel[i]->Fill(cel_tree->GetLeaf(name)->GetValue());
    }
  }

  // Create one legend for all plots
  auto const transparency = 0.75;
  auto const legend = new TLegend(0.35, 0.67, 0.55, 0.85);
  legend->AddEntry(histograms_g4[0], "Geant4", "l");
  legend->AddEntry(histograms_cel[0], "Celeritas", "l");
  legend->AddEntry(new TF1("fit", "gaus", 0, 1), "Best fit", "l");
  legend->SetLineColor(kGray);
  legend->SetFillColorAlpha(kWhite, transparency);

  // Store list of pdf filenames
  std::string pdf_list;

  // Create canvases and draw histograms
  TCanvas* canvases[num_branches];
  for (int i = 0; i < num_branches; i++) {
    // Initialize canvases
    auto const& key = map_branches_limits.find(branches->At(i)->GetName());
    std::string name = "c_" + key->first;
    canvases[i] = new TCanvas(name.c_str(), name.c_str(), 700, 500);

    // Set up plot attributes
    histograms_g4[i]->SetLineColor(kBlack);
    histograms_cel[i]->SetLineColorAlpha(kAzure + 1, 0.75);
    histograms_g4[i]->SetLineWidth(2);
    histograms_cel[i]->SetLineWidth(3);

    histograms_g4[i]->Draw();
    histograms_cel[i]->Draw("sames");

    auto fit = new TF1("fit", "gaus", key->second.min, key->second.max);
    histograms_g4[i]->Fit("fit", "R");
    histograms_cel[i]->Fit("fit", "R");

    // canvases[i]->SetObjectStat(true);
    histograms_g4[i]->SetStats(true);
    histograms_cel[i]->SetStats(true);
    gStyle->SetOptFit(true);
    gStyle->SetOptStat(1111);

    auto fit_stats_g4 = (TPaveText*)histograms_g4[i]->FindObject("stats");
    histograms_g4[i]->SetName("Geant4");
    fit_stats_g4->SetFillColorAlpha(kWhite, transparency);
    fit_stats_g4->SetLineColor(kGray);
    fit_stats_g4->SetShadowColor(kWhite);
    fit_stats_g4->SetX1NDC(.57);
    fit_stats_g4->SetY1NDC(.55);
    fit_stats_g4->SetX2NDC(.86);
    fit_stats_g4->SetY2NDC(.85);
    fit_stats_g4->Draw();

    auto fit_stats_cel = (TPaveText*)histograms_cel[i]->FindObject("stats");
    histograms_cel[i]->SetName("Celeritas");
    fit_stats_cel->SetFillColorAlpha(kWhite, transparency);
    fit_stats_cel->SetLineColor(kGray);
    fit_stats_cel->SetShadowColor(kWhite);
    fit_stats_cel->SetX1NDC(.57);
    fit_stats_cel->SetY1NDC(.23);
    fit_stats_cel->SetX2NDC(.86);
    fit_stats_cel->SetY2NDC(.53);

    // Draw legend
    legend->Draw();

    // Save each canvas into a pdf file
    std::string pdf_filename = key->first + ".pdf";
    canvases[i]->SaveAs(pdf_filename.c_str());
    pdf_list += pdf_filename + " ";
  }

#if 0
  // Generate single pdf and clean up (requires `brew install poppler`)
  std::string cmd_merge_pdfs = "pdfunite " + pdf_list + "hgcal-all.pdf";
  std::string cmd_rm_pdfs = "rm " + pdf_list;
  gSystem->Exec(cmd_merge_pdfs.c_str());
  gSystem->Exec(cmd_rm_pdfs.c_str());
#endif
}
