#ifndef NUMBERSRAASYST_2015
#define NUMBERSRAASYST_2015


// ###########################################################################################
// systematics fits
const int nFitVariations   = 4;
const int nEff4DVariations = 1;
const int nEffTnPVariation = 5; 
//pp
const char* yieldHistFile_pp_systSgnBkg[nFitVariations] = {
  "histsRaaYields_20160304_pp_sigG1G2.root",
  "histsRaaYields_20160304_pp_polFunct.root",
//  "histsRaaYields_20160304_pp_resOpt2.root",
  "histsRaaYields_20160304_pp_MLAR.root", 
  "histsRaaYields_20160304_pp_const.root" 
};
const char* yieldHistFile_pp_syst4DCorr[nEff4DVariations] = {
  "histsRaaYields_20160304_pp_ctau2mm.root"
};

const char* yieldHistFile_pp_systTnP[nEffTnPVariation] = {
  "histEff_effSyst_201602_pp_3dEff.root",
  "histEff_effSyst_20160622_pp_IdTrg_stat.root",
  "histEff_effSyst_20160622_pp_IdTrg_syst.root",
  "histEff_effSyst_20160622_pp_Sta_stat.root",
  "histEff_effSyst_20160622_pp_Sta_syst.root"
};

//-----------------
// pbpb
const char* yieldHistFile_aa_systSgnBkg[nFitVariations]   = {
  "histsRaaYields_20160304_PbPb_sigG1G2.root",
  "histsRaaYields_20160304_PbPb_polFunct.root",
//  "histsRaaYields_20160304_PbPb_resOpt2.root", 
  "histsRaaYields_20160304_PbPb_MLAR.root", 
  "histsRaaYields_20160304_PbPb_const.root"
};
const char* yieldHistFile_aa_syst4DCorr[nEff4DVariations] = {
  "histsRaaYields_20160304_PbPb_ctau2mm.root"
};

const char* yieldHistFile_aa_systTnP[nEffTnPVariation] = {
  "histEff_effSyst_201602_pbpb_3dEff.root",
  "histEff_effSyst_20160622_pbpb_IdTrg_stat.root",
  "histEff_effSyst_20160622_pbpb_IdTrg_syst.root",
  "histEff_effSyst_20160622_pbpb_Sta_stat.root",
  "histEff_effSyst_20160622_pbpb_Sta_syst.root"
};

//------------------
// legend or systm variation
const char* legendSyst[] = {
  "signal_2Gaus",
  "bkg_polFunct",
//  "signal_resOpt2", 
  "bFrac_MLAR", 
  "constrained", 
//  "4DEff_profile", 
  "4DEff", 
  "3DEff_toy",
  "TnP_trgMuID_stat", 
  "TnP_trgMuID_syst", 
  "TnP_sta_stat",
  "TnP_sta_syst" 
};

// ######################################
// global uncertainties

  double systEventSelection[2] = {0.0, 0.0}; // HF gating and good event selection ... this should be different in pp and pbpb ...
  double systLumis[2]          = {0.037, 0.03}; // first for pp lumi, 2nd for counting the pbpb MB events
  double systTracking[2]       = {0.006, 0.012}; // first for pp tracking eff, 2nd for PbPb tracking eff

#endif

