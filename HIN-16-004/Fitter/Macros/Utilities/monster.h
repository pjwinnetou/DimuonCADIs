#ifndef monster_h
#define monster_h

// In this file we define the "monster" function linking all 
//   the ingredients to the prompt-only double ratio, including
//   systematics

#include <vector>
#include "RooWorkspace.h"
#include "RooArgList.h"
#include "bin.h"
#include "../../Systematics/syst.h"

using namespace std;

const int npar_pass = 8;
const int npar_prompt = 18;

double quadratic_sum(vector<double> v);
double uncert(double (*f)(const double *xx), const double *val, const double *err, const int npar);

double doubleratio_pass(const double *xx);
double doubleratio_monster(const double *xx, bool isprompt);
double doubleratio_prompt(const double *xx);
double doubleratio_nonprompt(const double *xx);
double npsip_pbpb_pass_from_doubleratio_prompt(RooWorkspace *w, RooArgList vars);

double doubleratio_pass_generic(const char* workDir, anabin thebin, const char* prependPath="", bool dostat=false, bool dosyst=false, const char* systtag="");
double doubleratio_pass_nominal(const char* workDir, anabin thebin, const char* prependPath="");
double doubleratio_pass_stat(const char* workDir, anabin thebin, const char* prependPath="");
double doubleratio_pass_syst(const char* workDir, anabin thebin, const char* prependPath="", const char* systtag="");
double doubleratio_monster_generic(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath="", bool isprompt=true, bool dostat=false, bool dosyst=false, const char* systtag="");
double doubleratio_prompt_nominal(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath="");
double doubleratio_prompt_stat(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath="");
double doubleratio_prompt_syst(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath="", const char* systtag="");
double doubleratio_nonprompt_nominal(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath="");
double doubleratio_nonprompt_stat(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath="");
double doubleratio_nonprompt_syst(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath="", const char* systtag="");

#include <math.h>
#include <iostream>
#include "RooRealVar.h"
#include "RooFormulaVar.h"
#include "resultUtils.h"
#include "../../../Efficiency/eff.h"
#include "../../Systematics/syst.C"

// simple function for quadratic sum
double quadratic_sum(vector<double> v) {
   double val=0;
   for (vector<double>::const_iterator it=v.begin(); it!=v.end(); it++) val += pow(*it,2);
   return sqrt(val);
}

// compute uncertainties
double uncert(double (*f)(const double *xx), const double *val, const double *err, const int npar) {
   double nominal = (*f)(val);
   vector<double> vars1;
   vector<double> vars2;
   for (int i=0; i<npar; i++) {
      double *valvar1 = new double[npar];
      double *valvar2 = new double[npar];
      for (int j=0; j<npar; j++) 
      {
         valvar1[j] = (i==j) ? val[j] + err[j] : val[j];
         valvar2[j] = (i==j) ? val[j] - err[j] : val[j];
      }
      vars1.push_back((*f)(valvar1) - nominal);
      vars2.push_back((*f)(valvar2) - nominal);
      delete[] valvar1; delete[] valvar2;
   }
   double uncert1 = quadratic_sum(vars1);
   double uncert2 = quadratic_sum(vars2);
   return max(uncert1, uncert2);
}

// double ratio
double doubleratio_pass(const double *xx) {
   // if one the inputs is zero (except the alphas), exit immediately
   for (int i=0; i<4; i++) if (fabs(xx[i])<1e-9) return -99;

   int i=0;
   // yields from data
   double njpsi_pbpb_pass = xx[i]; i++;
   double rfrac_pbpb_pass = xx[i]; i++;
   double njpsi_pp_pass = xx[i]; i++;
   double rfrac_pp_pass = xx[i]; i++;
   // systematics
   double alpha_DR = 1. + xx[i]; i++;
   double alpha_fitpp = xx[i]; i++;
   double alpha_fitpbpb = xx[i]; i++;
   double alpha_Bhad_add = xx[i]; i++;

   double rfrac_pbpb_pass_syst = rfrac_pbpb_pass + alpha_fitpbpb;
   double rfrac_pp_pass_syst = rfrac_pp_pass + alpha_fitpp;

   double npsip_pbpb_pass = rfrac_pbpb_pass_syst * njpsi_pbpb_pass;
   double npsip_pp_pass = rfrac_pp_pass_syst * njpsi_pp_pass;

   double dr_pass = (npsip_pbpb_pass / njpsi_pbpb_pass) / (npsip_pp_pass / njpsi_pp_pass);
   double dr_pass_syst = alpha_DR * dr_pass + alpha_Bhad_add;
   return dr_pass_syst;
}

// T H E   M O N S T E R   F U N C T I O N 
double doubleratio_monster(const double *xx, bool isprompt) {
   // if one the inputs is zero (except the alphas), exit immediately
   for (int i=0; i<11; i++) if (fabs(xx[i])<1e-9) return -99;

   int i=0;
   // yields from data
   double njpsi_pbpb_pass = xx[i]; i++;
   double rfrac_pbpb_pass = xx[i]; i++;
   double njpsi_pp_pass = xx[i]; i++;
   double rfrac_pp_pass = xx[i]; i++;
   double njpsi_pbpb_fail = xx[i]; i++;
   double rfrac_pbpb_fail = xx[i]; i++;
   double njpsi_pp_fail = xx[i]; i++;
   double rfrac_pp_fail = xx[i]; i++;
   // efficiencies
   double effjpsi_pp_P = xx[i]; i++;
   double effpsip_pp_P = xx[i]; i++;
   double effjpsi_pp_NP = xx[i]; i++;
   // systematics
   double alpha_effDR = 1. + xx[i]; i++;
   double alpha_fitpp = xx[i]; i++;
   double alpha_fitpbpb = xx[i]; i++;
   double alpha_effjpsi_pp_P = 1. + xx[i]; i++;
   double alpha_effpsip_pp_P = 1. + xx[i]; i++;
   double alpha_effjpsi_pp_NP = 1. + xx[i]; i++;
   double alpha_effpppbpb = 1. + xx[i]; i++;

   // DR_I
   double njpsi_pbpb = njpsi_pbpb_pass + njpsi_pbpb_fail;
   double npsip_pbpb_pass = njpsi_pbpb_pass*rfrac_pbpb_pass + alpha_fitpbpb;
   double npsip_pbpb_fail = njpsi_pbpb_fail*rfrac_pbpb_fail + alpha_fitpbpb;
   double npsip_pbpb = npsip_pbpb_pass + npsip_pbpb_fail;
   
   double njpsi_pp = njpsi_pp_pass + njpsi_pp_fail;
   double npsip_pp_pass = njpsi_pp_pass*rfrac_pp_pass + alpha_fitpp;
   double npsip_pp_fail = njpsi_pp_fail*rfrac_pp_fail + alpha_fitpp;
   double npsip_pp = npsip_pp_pass + npsip_pp_fail;

   double DR_I = (npsip_pbpb/njpsi_pbpb) / (npsip_pp/njpsi_pp);

   // DR_fP
   double effjpsi_pp_P_syst = effjpsi_pp_P * alpha_effjpsi_pp_P;
   double effpsip_pp_P_syst = effpsip_pp_P * alpha_effpsip_pp_P;
   double effjpsi_pp_NP_syst = effjpsi_pp_NP * alpha_effjpsi_pp_NP;
   double effpsip_pp_NP_syst = effjpsi_pp_NP_syst;

   double effjpsi_pbpb_P_syst = effjpsi_pp_P * alpha_effjpsi_pp_P * alpha_effpppbpb;
   double effpsip_pbpb_P_syst = effpsip_pp_P * alpha_effpsip_pp_P * alpha_effpppbpb;
   double effjpsi_pbpb_NP_syst = effjpsi_pp_NP * alpha_effjpsi_pp_NP * alpha_effpppbpb;
   double effpsip_pbpb_NP_syst = effjpsi_pbpb_NP_syst;
   
   if (isprompt) {
      double fpass_jpsi_pbpb = njpsi_pbpb_pass/(njpsi_pbpb_pass+njpsi_pbpb_fail);
      double fpass_psip_pbpb = npsip_pbpb_pass/(npsip_pbpb_pass+npsip_pbpb_fail);
      double fpass_jpsi_pp = njpsi_pp_pass/(njpsi_pp_pass+njpsi_pp_fail);
      double fpass_psip_pp = npsip_pp_pass/(npsip_pp_pass+npsip_pp_fail);

      double fP_jpsi_pbpb = (fpass_jpsi_pbpb - effjpsi_pbpb_NP_syst) / (effjpsi_pbpb_P_syst - effjpsi_pbpb_NP_syst);
      double fP_psip_pbpb = (fpass_psip_pbpb - effpsip_pbpb_NP_syst) / (effpsip_pbpb_P_syst - effpsip_pbpb_NP_syst);
      double fP_jpsi_pp = (fpass_jpsi_pp - effjpsi_pp_NP_syst) / (effjpsi_pp_P_syst - effjpsi_pp_NP_syst);
      double fP_psip_pp = (fpass_psip_pp - effpsip_pp_NP_syst) / (effpsip_pp_P_syst - effpsip_pp_NP_syst);

      double DR_fP = (fP_psip_pbpb / fP_jpsi_pbpb) / (fP_psip_pp / fP_jpsi_pp);

      double DR_P = DR_I * DR_fP * alpha_effDR;
      return DR_P;
   } else {
      double ffail_jpsi_pbpb = njpsi_pbpb_fail/(njpsi_pbpb_fail+njpsi_pbpb_pass);
      double ffail_psip_pbpb = npsip_pbpb_fail/(npsip_pbpb_fail+npsip_pbpb_pass);
      double ffail_jpsi_pp = njpsi_pp_fail/(njpsi_pp_fail+njpsi_pp_pass);
      double ffail_psip_pp = npsip_pp_fail/(npsip_pp_fail+npsip_pp_pass);

      double fNP_jpsi_pbpb = (ffail_jpsi_pbpb - effjpsi_pbpb_P_syst) / (effjpsi_pbpb_NP_syst - effjpsi_pbpb_P_syst);
      double fNP_psip_pbpb = (ffail_psip_pbpb - effpsip_pbpb_P_syst) / (effpsip_pbpb_NP_syst - effpsip_pbpb_P_syst);
      double fNP_jpsi_pp = (ffail_jpsi_pp - effjpsi_pp_P_syst) / (effjpsi_pp_NP_syst - effjpsi_pp_P_syst);
      double fNP_psip_pp = (ffail_psip_pp - effpsip_pp_P_syst) / (effpsip_pp_NP_syst - effpsip_pp_P_syst);

      double DR_fNP = (fNP_psip_pbpb / fNP_jpsi_pbpb) / (fNP_psip_pp / fNP_jpsi_pp);

      double DR_NP = DR_I * DR_fNP * alpha_effDR;
      return DR_NP;
   }
}

double doubleratio_prompt(const double *xx) {
   return doubleratio_monster(xx, true);
}

double doubleratio_nonprompt(const double *xx) {
   return doubleratio_monster(xx, false);
}

// T H E   M O N S T E R   F U N C T I O N,   R O O F I T   V E R S I O N
double npsip_pbpb_pass_from_doubleratio_prompt(RooWorkspace *w, RooArgList vars) {
   int i=0;
   // yields from data
   RooRealVar* njpsi_pbpb_pass = (RooRealVar*) &(vars[i]); i++;
   // RooRealVar* rfrac_pbpb_pass = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* DR_P = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* njpsi_pp_pass = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* rfrac_pp_pass = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* njpsi_pbpb_fail = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* rfrac_pbpb_fail = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* njpsi_pp_fail = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* rfrac_pp_fail = (RooRealVar*) &(vars[i]); i++;
   // efficiencies
   RooRealVar* effjpsi_pp_P = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* effpsip_pp_P = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* effjpsi_pp_NP = (RooRealVar*) &(vars[i]); i++;
   // systematics
   RooRealVar* alpha_effDR = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* alpha_fitpp = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* alpha_fitpbpb = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* alpha_effjpsi_pp_P = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* alpha_effpsip_pp_P = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* alpha_effjpsi_pp_NP = (RooRealVar*) &(vars[i]); i++;
   RooRealVar* alpha_effpppbpb = (RooRealVar*) &(vars[i]); i++;

   // DR_I
   RooFormulaVar *njpsi_pbpb = new RooFormulaVar("njpsi_pbpb","njpsi_pbpb","@0+@1",RooArgList(*njpsi_pbpb_pass,*njpsi_pbpb_fail));
   // RooFormulaVar *npsip_pbpb_pass = new RooFormulaVar("npsip_pbpb_pass","npsip_pbpb_pass","@0*@1+@2",RooArgList(*njpsi_pbpb_pass,*rfrac_pbpb_pass,*alpha_fitpbpb));
   RooFormulaVar *npsip_pbpb_fail = new RooFormulaVar("npsip_pbpb_fail","npsip_pbpb_fail","@0*@1+@2",RooArgList(*njpsi_pbpb_fail,*rfrac_pbpb_fail,*alpha_fitpbpb));
   // RooFormulaVar *npsip_pbpb = new RooFormulaVar("npsip_pbpb","npsip_pbpb","@0+@1",RooArgList(*npsip_pbpb_pass,*npsip_pbpb_fail));

   RooFormulaVar *njpsi_pp = new RooFormulaVar("njpsi_pp","njpsi_pp","@0+@1",RooArgList(*njpsi_pp_pass,*njpsi_pp_fail));
   RooFormulaVar *npsip_pp_pass = new RooFormulaVar("npsip_pp_pass","npsip_pp_pass","@0*@1+@2",RooArgList(*njpsi_pp_pass,*rfrac_pp_pass,*alpha_fitpp));
   RooFormulaVar *npsip_pp_fail = new RooFormulaVar("npsip_pp_fail","npsip_pp_fail","@0*@1+@2",RooArgList(*njpsi_pp_fail,*rfrac_pp_fail,*alpha_fitpp));
   RooFormulaVar *npsip_pp = new RooFormulaVar("npsip_pp","npsip_pp","@0+@1",RooArgList(*npsip_pp_pass,*npsip_pp_fail));

   // RooFormulaVar *DR_I = new RooFormulaVar("DR_I","DR_I","(@0/@1)/(@2/@3)",RooArgList(*npsip_pbpb,*njpsi_pbpb,*npsip_pp,*njpsi_pp));

   // DR_fP
   RooFormulaVar *effjpsi_pp_P_syst = new RooFormulaVar("effjpsi_pp_P_syst","effjpsi_pp_P_syst","@0*@1",RooArgList(*effjpsi_pp_P,*alpha_effjpsi_pp_P));
   RooFormulaVar *effpsip_pp_P_syst = new RooFormulaVar("effpsip_pp_P_syst","effpsip_pp_P_syst","@0*@1",RooArgList(*effpsip_pp_P,*alpha_effpsip_pp_P));
   RooFormulaVar *effjpsi_pp_NP_syst = new RooFormulaVar("effjpsi_pp_NP_syst","effjpsi_pp_NP_syst","@0*@1",RooArgList(*effjpsi_pp_NP,*alpha_effjpsi_pp_NP));
   RooFormulaVar *effpsip_pp_NP_syst = new RooFormulaVar("effpsip_pp_NP_syst","effpsip_pp_NP_syst","@0",RooArgList(*effjpsi_pp_NP_syst));

   RooFormulaVar *effjpsi_pbpb_P_syst = new RooFormulaVar("effjpsi_pbpb_P_syst","effjpsi_pbpb_P_syst","@0*@1*@2",RooArgList(*effjpsi_pp_P,*alpha_effjpsi_pp_P,*alpha_effpppbpb));
   RooFormulaVar *effpsip_pbpb_P_syst = new RooFormulaVar("effpsip_pbpb_P_syst","effpsip_pbpb_P_syst","@0*@1*@2",RooArgList(*effpsip_pp_P,*alpha_effpsip_pp_P,*alpha_effpppbpb));
   RooFormulaVar *effjpsi_pbpb_NP_syst = new RooFormulaVar("effjpsi_pbpb_NP_syst","effjpsi_pbpb_NP_syst","@0*@1*@2",RooArgList(*effjpsi_pp_NP,*alpha_effjpsi_pp_NP,*alpha_effpppbpb));
   RooFormulaVar *effpsip_pbpb_NP_syst = new RooFormulaVar("effpsip_pbpb_NP_syst","effpsip_pbpb_NP_syst","@0",RooArgList(*effjpsi_pp_NP_syst));

   RooFormulaVar *fpass_jpsi_pbpb = new RooFormulaVar("fpass_jpsi_pbpb","fpass_jpsi_pbpb","@0/(@0+@1)",RooArgList(*njpsi_pbpb_pass,*njpsi_pbpb_fail));
   // RooFormulaVar *fpass_psip_pbpb = new RooFormulaVar("fpass_psip_pbpb","fpass_psip_pbpb","@0/(@0+@1)",RooArgList(*npsip_pbpb_pass,*npsip_pbpb_fail));
   RooFormulaVar *fpass_jpsi_pp = new RooFormulaVar("fpass_jpsi_pp","fpass_jpsi_pp","@0/(@0+@1)",RooArgList(*njpsi_pp_pass,*njpsi_pp_fail));
   RooFormulaVar *fpass_psip_pp = new RooFormulaVar("fpass_psip_pp","fpass_psip_pp","@0/(@0+@1)",RooArgList(*npsip_pp_pass,*npsip_pp_fail));

   RooFormulaVar *fP_jpsi_pbpb = new RooFormulaVar("fP_jpsi_pbpb","fP_jpsi_pbpb","(@1-@2)/(@0-@2)",RooArgList(*effjpsi_pbpb_P_syst,*fpass_jpsi_pbpb,*effjpsi_pbpb_NP_syst));
   // RooFormulaVar *fP_psip_pbpb = new RooFormulaVar("fP_psip_pbpb","fP_psip_pbpb","(@0-@1)/(@0-@2)",RooArgList(*effpsip_pbpb_P_syst,*fpass_psip_pbpb,*effpsip_pbpb_NP_syst));
   RooFormulaVar *fP_jpsi_pp = new RooFormulaVar("fP_jpsi_pp","fP_jpsi_pp","(@1-@2)/(@0-@2)",RooArgList(*effjpsi_pp_P_syst,*fpass_jpsi_pp,*effjpsi_pp_NP_syst));
   RooFormulaVar *fP_psip_pp = new RooFormulaVar("fP_psip_pp","fP_psip_pp","(@1-@2)/(@0-@2)",RooArgList(*effpsip_pp_P_syst,*fpass_psip_pp,*effpsip_pp_NP_syst));

   // RooFormulaVar *DR_fP = new RooFormulaVar("DR_fP","DR_fP","(@0/@1)/(@2/@3)",RooArgList(*fP_psip_pbpb,*fP_jpsi_pbpb,*fP_psip_pp,*fP_jpsi_pp));

   // RooFormulaVar *DR_P = new RooFormulaVar("DR_P","DR_P","@0*@1*@2",RooArgList(*DR_I,*DR_fP,*alpha_effDR));
   // w->import(*DR_P);
   // return DR_P->getVal();

   // now that we have all the ingredients, let's get to what we want: npsip_pbpb_pass.
   RooFormulaVar *npsip_pbpb_P = new RooFormulaVar("npsip_pbpb_P","npsip_pbpb_P","@0*((@1*@2)/(@3*@4))*@5*@6",
         RooArgList(*DR_P, *fP_psip_pp,*npsip_pp, *fP_jpsi_pp,*njpsi_pp, *fP_jpsi_pbpb,*njpsi_pbpb));
   RooFormulaVar *npsip_pbpb_pass = new RooFormulaVar("npsip_pbpb_pass","npsip_pbpb_pass","(@0+(@1/(@2-@1))*@3)*(@2-@1)/(1-@1)",
         RooArgList(*npsip_pbpb_P, *effpsip_pbpb_NP_syst, *effpsip_pbpb_P_syst, *npsip_pbpb_fail));
   w->import(*npsip_pbpb_pass);
   return npsip_pbpb_pass->getVal();
}

double doubleratio_pass_generic(const char* workDir, anabin thebin, const char* prependPath, bool dostat, bool dosyst, const char* systtag) {
   double njpsi_pbpb_pass = poiFromBin(workDir, "PbPb", "N_Jpsi", thebin, "DATA", prependPath);
   double rfrac_pbpb_pass = poiFromBin(workDir, "PbPb", "RFrac2Svs1S", thebin, "DATA", prependPath);
   double njpsi_pp_pass = poiFromBin(workDir, "PP", "N_Jpsi", thebin, "DATA", prependPath);
   double rfrac_pp_pass = poiFromBin(workDir, "PP", "RFrac2Svs1S", thebin, "DATA", prependPath);

   double njpsi_pbpb_pass_err = poiErrFromBin(workDir, "PbPb", "N_Jpsi", thebin, "DATA", prependPath);
   double rfrac_pbpb_pass_err = poiErrFromBin(workDir, "PbPb", "RFrac2Svs1S", thebin, "DATA", prependPath);
   double njpsi_pp_pass_err = poiErrFromBin(workDir, "PP", "N_Jpsi", thebin, "DATA", prependPath);
   double rfrac_pp_pass_err = poiErrFromBin(workDir, "PP", "RFrac2Svs1S", thebin, "DATA", prependPath);

   TString TSprependPath(prependPath); if (TSprependPath != "") TSprependPath += "/";

   map<anabin,syst> alpha_DR = readSyst(TSprependPath + "Systematics/csv/syst_PbPb_eff_MCstat.csv");
   map<anabin,syst> alpha_fitpp_bkg = readSyst(TSprependPath + "Systematics/csv/syst_PP_fit_bkg.csv");
   map<anabin,syst> alpha_fitpp_sig = readSyst(TSprependPath + "Systematics/csv/syst_PP_fit_sig.csv");
   map<anabin,syst> alpha_fitpbpb_bkg = readSyst(TSprependPath + "Systematics/csv/syst_PbPb_fit_bkg.csv");
   map<anabin,syst> alpha_fitpbpb_sig = readSyst(TSprependPath + "Systematics/csv/syst_PbPb_fit_sig.csv");
   map<anabin,syst> alpha_Bhad_add = readSyst(TSprependPath + "Systematics/csv/syst_PbPb_bhad_add.csv");

   double *xx = new double[npar_pass];
   double *err = new double[npar_pass];
   int i=0;
   xx[i] = njpsi_pbpb_pass; err[i] = dostat ? njpsi_pbpb_pass_err : 0; i++;
   xx[i] = rfrac_pbpb_pass; err[i] = dostat ? rfrac_pbpb_pass_err : 0; i++;
   xx[i] = njpsi_pp_pass; err[i] = dostat ? njpsi_pp_pass_err : 0; i++;
   xx[i] = rfrac_pp_pass; err[i] = dostat ? rfrac_pp_pass_err : 0; i++;
   xx[i] = 0; err[i] = dosyst ? alpha_DR[thebin].value : 0; i++;
   xx[i] = 0; err[i] = dosyst ? sqrt(pow(alpha_fitpp_bkg[thebin].value,2) + pow(alpha_fitpp_sig[thebin].value,2)) : 0; i++;
   xx[i] = 0; err[i] = dosyst ? sqrt(pow(alpha_fitpbpb_bkg[thebin].value,2) + pow(alpha_fitpbpb_sig[thebin].value,2)) : 0; i++;
   xx[i] = 0; err[i] = dosyst ? alpha_Bhad_add[thebin].value : 0; i++;

   // set all systs but one to zero, if applicable
   TString TSsysttag(systtag);
   if (dosyst && TSsysttag != "") for (int j=4; j<i; j++) {
      if (j==4 && TSsysttag.Contains("DR")) continue;
      if (j==5 && TSsysttag.Contains("fitpp")) continue;
      if (j==6 && TSsysttag.Contains("fitpbpb")) continue;
      if (j==7 && TSsysttag.Contains("Bhad_add")) continue;
      err[j] = 0;
   }

   double ans = -99;
   if (!dostat && !dosyst) ans = doubleratio_pass(xx);
   else ans = uncert(&doubleratio_pass, xx, err, npar_pass);
   delete[] xx; delete[] err;
   return ans;
}

double doubleratio_pass_nominal(const char* workDir, anabin thebin, const char* prependPath="") {
   return doubleratio_pass_generic(workDir, thebin, prependPath, false, false);
}
double doubleratio_pass_stat(const char* workDir, anabin thebin, const char* prependPath="") {
   return doubleratio_pass_generic(workDir, thebin, prependPath, true, false);
}
double doubleratio_pass_syst(const char* workDir, anabin thebin, const char* prependPath="", const char* systtag) {
   return doubleratio_pass_generic(workDir, thebin, prependPath, false, true, systtag);
}

double doubleratio_monster_generic(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath, bool isprompt, bool dostat, bool dosyst, const char* systtag) {
   double njpsi_pbpb_pass = poiFromBin(workDir_pass, "PbPb", "N_Jpsi", thebin, "DATA", prependPath);
   double rfrac_pbpb_pass = poiFromBin(workDir_pass, "PbPb", "RFrac2Svs1S", thebin, "DATA", prependPath);
   double njpsi_pp_pass = poiFromBin(workDir_pass, "PP", "N_Jpsi", thebin, "DATA", prependPath);
   double rfrac_pp_pass = poiFromBin(workDir_pass, "PP", "RFrac2Svs1S", thebin, "DATA", prependPath);
   double njpsi_pbpb_fail = poiFromBin(workDir_fail, "PbPb", "N_Jpsi", thebin, "DATA", prependPath);
   double rfrac_pbpb_fail = poiFromBin(workDir_fail, "PbPb", "RFrac2Svs1S", thebin, "DATA", prependPath);
   double njpsi_pp_fail = poiFromBin(workDir_fail, "PP", "N_Jpsi", thebin, "DATA", prependPath);
   double rfrac_pp_fail = poiFromBin(workDir_fail, "PP", "RFrac2Svs1S", thebin, "DATA", prependPath);

   double njpsi_pbpb_pass_err = poiErrFromBin(workDir_pass, "PbPb", "N_Jpsi", thebin, "DATA", prependPath);
   double rfrac_pbpb_pass_err = poiErrFromBin(workDir_pass, "PbPb", "RFrac2Svs1S", thebin, "DATA", prependPath);
   double njpsi_pp_pass_err = poiErrFromBin(workDir_pass, "PP", "N_Jpsi", thebin, "DATA", prependPath);
   double rfrac_pp_pass_err = poiErrFromBin(workDir_pass, "PP", "RFrac2Svs1S", thebin, "DATA", prependPath);
   double njpsi_pbpb_fail_err = poiErrFromBin(workDir_fail, "PbPb", "N_Jpsi", thebin, "DATA", prependPath);
   double rfrac_pbpb_fail_err = poiErrFromBin(workDir_fail, "PbPb", "RFrac2Svs1S", thebin, "DATA", prependPath);
   double njpsi_pp_fail_err = poiErrFromBin(workDir_fail, "PP", "N_Jpsi", thebin, "DATA", prependPath);
   double rfrac_pp_fail_err = poiErrFromBin(workDir_fail, "PP", "RFrac2Svs1S", thebin, "DATA", prependPath);

   TString prependPathForEff("../Efficiency"); if (TString(prependPath) != "") prependPathForEff = TString(prependPath) + "/" + prependPathForEff;
   double effjpsi_pp_P = ljpsieff("jpsi","pp",thebin,prependPathForEff); 
   double effpsip_pp_P = ljpsieff("psi2s","pp",thebin,prependPathForEff);
   double effjpsi_pp_NP = ljpsieff("npjpsi","pp",thebin,prependPathForEff); 

   TString TSprependPath(prependPath); if (TSprependPath != "") TSprependPath += "/";

   map<anabin,syst> alpha_DR = readSyst(TSprependPath + "Systematics/csv/syst_PbPb_eff_MCstat.csv");
   map<anabin,syst> alpha_fitpp_bkg = readSyst(TSprependPath + "Systematics/csv/syst_PP_fit_bkg.csv");
   map<anabin,syst> alpha_fitpp_sig = readSyst(TSprependPath + "Systematics/csv/syst_PP_fit_sig.csv");
   map<anabin,syst> alpha_fitpbpb_bkg = readSyst(TSprependPath + "Systematics/csv/syst_PbPb_fit_bkg.csv");
   map<anabin,syst> alpha_fitpbpb_sig = readSyst(TSprependPath + "Systematics/csv/syst_PbPb_fit_sig.csv");

   double *xx = new double[npar_prompt];
   double *err = new double[npar_prompt];
   int i=0;
   xx[i] = njpsi_pbpb_pass; err[i] = dostat ? njpsi_pbpb_pass_err : 0; i++;
   xx[i] = rfrac_pbpb_pass; err[i] = dostat ? rfrac_pbpb_pass_err : 0; i++;
   xx[i] = njpsi_pp_pass; err[i] = dostat ? njpsi_pp_pass_err : 0; i++;
   xx[i] = rfrac_pp_pass; err[i] = dostat ? rfrac_pp_pass_err : 0; i++;
   xx[i] = njpsi_pbpb_fail; err[i] = dostat ? njpsi_pbpb_fail_err : 0; i++;
   xx[i] = rfrac_pbpb_fail; err[i] = dostat ? rfrac_pbpb_fail_err : 0; i++;
   xx[i] = njpsi_pp_fail; err[i] = dostat ? njpsi_pp_fail_err : 0; i++;
   xx[i] = rfrac_pp_fail; err[i] = dostat ? rfrac_pp_fail_err : 0; i++;
   xx[i] = effjpsi_pp_P; err[i] = 0; i++;
   xx[i] = effpsip_pp_P; err[i] = 0; i++;
   xx[i] = effjpsi_pp_NP; err[i] = 0; i++;
   xx[i] = 0; err[i] = dosyst ? alpha_DR[thebin].value : 0; i++;
   xx[i] = 0; err[i] = dosyst ? sqrt(pow(alpha_fitpp_bkg[thebin].value,2) + pow(alpha_fitpp_sig[thebin].value,2)) : 0; i++;
   xx[i] = 0; err[i] = dosyst ? sqrt(pow(alpha_fitpbpb_bkg[thebin].value,2) + pow(alpha_fitpbpb_sig[thebin].value,2)) : 0; i++;
   // unimplemented systs
   xx[i] = 0; err[i] = 0; i++; // alpha_effjpsi_pp_pr
   xx[i] = 0; err[i] = 0; i++; // alpha_effpsip_pp_pr
   xx[i] = 0; err[i] = 0; i++; // alpha_effjpsi_pp_npr
   xx[i] = 0; err[i] = 0; i++; // alpha_effpppbpb

   // set all systs but one to zero, if applicable
   TString TSsysttag(systtag);
   if (dosyst && TSsysttag != "") for (int j=11; j<i; j++) {
      if (j==11 && TSsysttag.Contains("DR")) continue;
      if (j==12 && TSsysttag.Contains("fitpp")) continue;
      if (j==13 && TSsysttag.Contains("fitpbpb")) continue;
      if (j==14 && TSsysttag.Contains("effjpsi_pp_pr")) continue;
      if (j==15 && TSsysttag.Contains("effpsip_pp_pr")) continue;
      if (j==16 && TSsysttag.Contains("effjpsi_pp_npr")) continue;
      if (j==17 && TSsysttag.Contains("effpppbpb")) continue;
      err[j] = 0;
   }

   double ans = -99;
   if (isprompt) {
      if (!dostat && !dosyst) ans = doubleratio_prompt(xx);
      else ans = uncert(&doubleratio_prompt, xx, err, npar_prompt);
   } else {
      if (!dostat && !dosyst) ans = doubleratio_nonprompt(xx);
      else ans = uncert(&doubleratio_nonprompt, xx, err, npar_prompt);
   }
   delete[] xx; delete[] err;
   return ans;
}

double doubleratio_prompt_nominal(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath="") {
   return doubleratio_monster_generic(workDir_pass, workDir_fail, thebin, prependPath, true, false, false);
}
double doubleratio_prompt_stat(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath="") {
   return doubleratio_monster_generic(workDir_pass, workDir_fail, thebin, prependPath, true, true, false);
}
double doubleratio_prompt_syst(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath="", const char* systtag) {
   return doubleratio_monster_generic(workDir_pass, workDir_fail, thebin, prependPath, true, false, true, systtag);
}
double doubleratio_nonprompt_nominal(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath="") {
   return doubleratio_monster_generic(workDir_pass, workDir_fail, thebin, prependPath, false, false, false);
}
double doubleratio_nonprompt_stat(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath="") {
   return doubleratio_monster_generic(workDir_pass, workDir_fail, thebin, prependPath, false, true, false);
}
double doubleratio_nonprompt_syst(const char* workDir_pass, const char* workDir_fail, anabin thebin, const char* prependPath="", const char* systtag) {
   return doubleratio_monster_generic(workDir_pass, workDir_fail, thebin, prependPath, false, false, true, systtag);
}

#endif // ifndef monster_h
