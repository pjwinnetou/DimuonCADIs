/*
Macro to calculate the systm. uncertainty for v2
Input:
 *.dat produced by v2_fitter.C macro
Output:
 *.dat files, containing the total uncert, as well as individual contributions

 */

#if !defined(__CINT__) || defined(__MAKECINT__)
#include <iostream>
#include <fstream>
#include <string>

#include <Riostream.h>
#include <TSystem.h>
#include <TProfile.h>
#include <TBrowser.h>
#include <TROOT.h>
#include <TGraph.h>
#include <TNtuple.h>
#include <TString.h>
#include <TH1D.h>
#include <TFile.h>
#include <TH1D.h>
#include <TF1.h>
#include <TMath.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TInterpreter.h>
#include <TGraphAsymmErrors.h>
#include <TGraphErrors.h>
#include "../CMS_lumi.C"
#include "../tdrstyle.C"
#include "../macro_v2/v2_dataNumbers_2015.h"
#endif

void makeSyst_all(
    int jpsiCategory      = -1, // -1: all, 1 : Prompt, 2 : Non-Prompt, 3: Bkg
    int nChoseVariable    = -1, // -1 = all; 0 integrated; 1 pt; 2: rapidity; 3: centrality;
    string nDphiBins      = "4",
    int method            = 2, // For fit variations, 0: nominal (rms of same category variations)&&added in quadrature with non-correlated sourcesvariations; 1: max of each variation type, added in quadrature, 2: same as 0, but quadrature sum for fit variations
    const char* inputDir  = "../macro_v2/outputNumbers", // the place where the input root files, with the histograms are
    const char* outputDir = "histSyst",// where the output figures will be
    bool bDoDebug         = false,
    bool bSavePlots       = true
    ) {
  gSystem->mkdir(Form("./%s/png",outputDir), kTRUE);
  gSystem->mkdir(Form("./%s/pdf",outputDir), kTRUE);
  
  // input files: prompt and non-prompt ones
  const int nFiles = 8;
  // the position in the array where a specific type of set of variations starts;
  // it's used later; make sure it corresponds to what's in array
  int iVar_tnp = 1;
  int iVar_4d  = 2;
  int iVar_3d  = 3;
  int iVar_fit = 4;
  const char* v2InFileDirs[nFiles] = {
              "histsV2Yields_20160304_v2W_dPhiBins4",  //0
              "histsV2Yields_20160304_v2W_noTnPSF_dPhiBins4",//1
              "histsV2Yields_20160304_v2W_ctau2mm_dPhiBins4",//2
              "histsV2Yields_20160304_v2W_4DEffOnly_dPhiBins4", //3
	      "histsV2Yields_20160304_v2W_const_dPhiBins4",//4
              "histsV2Yields_20160304_v2W_MLAR_dPhiBins4",//5
              "histsV2Yields_20160304_v2W_polFunct_dPhiBins4",//6
//              "histsV2Yields_20160304_v2W_resOpt2_dPhiBins4",//7
              "histsV2Yields_20160304_v2W_sigG1G2_dPhiBins4"//8
  };
  const char* signal[4]       = {"", "Prp","NPrp","Bkg"};
 
  // Reminder for TGraphAssymError: gr = new TGraphAsymmErrors(n,x,y,exl,exh,eyl,eyh);// n,x,y,err_x, err_y

  // Check if categEnd is small or equal to the array size of sample list 
  int categStart        = 1;
  int categEnd          = 4;
  if (jpsiCategory==1) categEnd=2;
  if (jpsiCategory==2) {categStart=2; categEnd=3;}
  if (jpsiCategory==3) {categStart=3;}

  int varStart = 0; 
  int varEnd   = 4;
  if(nChoseVariable==0) varEnd=1;//mb only
  if(nChoseVariable==1) {varStart=1; varEnd=2;}//pt only
  if(nChoseVariable==2) {varStart=2; varEnd=3;}//y only
  if(nChoseVariable==3) {varStart=3;}//centrality only

  for(int iCateg=categStart; iCateg<categEnd; iCateg++)
  {
    cout<<"====================== Doing "<< signal[iCateg] << "==========================="<<endl;
    for ( int iVar=varStart; iVar<varEnd; iVar++)
    {  
      int nBins           =  1; // mb, 1 bin, is the default
      if(iCateg==1 || iCateg==3)
      {
        if (iVar == 1)  nBins =  nPtBins_pr;
        if (iVar == 2)  nBins =  nYBins_pr;
        if (iVar == 3)  nBins =  nCentBins_pr;
      }
      if(iCateg ==2)
      {
        if (iVar == 1)  nBins =  nPtBins_np; // for non-prompt Jpsi, take as default 2 pt bin
        if (iVar == 2)  nBins =  nYBins_np;
        if (iVar == 3)  nBins =  nCentBins_np;
      }
      cout<<" !!!!! Number of bins: "<< nBins<<endl;


      cout<<"######## Variable: "<< outFilePlot[iVar] << "  ########" << endl;
      double adV2[nFiles][nBins]; // v2 values
    
      std::vector<std::vector<std::string>> str_kin;// pt, rapidity, centrlaity, to store for output dat files
      str_kin.resize( nBins );
      for( auto &i : str_kin )
        i.resize( 3 );

      // open the files with v2, and store them in arrays
      for(int iFile = 0; iFile<nFiles; iFile++)
      {
        ifstream in;
        std::string nameVar   = outFilePlot[iVar]; //rap
        std::string nameSig   = signal[iCateg];// pr, npr, bkg
        std::string nameDir   = v2InFileDirs[iFile];// 
        string inputFile      = nameVar + "_"+ nameSig + "_nphibin" + nDphiBins + ".dat";
        cout << "!!!!!! Input file name: "<< nameDir <<"/\t"<<inputFile <<endl;
        // read the v2 nominal value from input file
        
        string whatBin[3];
        double x[4] = {0};
	double fitprob = 0;
        int   iline = 0;
        string tmpstring;
        in.open(Form("%s/%s/data/%s",inputDir,nameDir.c_str(),inputFile.c_str()));
        
        getline(in,tmpstring);
        
        while ( in.good() && iline<nBins ) {
          in >> whatBin[0] >> whatBin[1] >> whatBin[2] >> x[0] >> x[1] >> x[2] >> x[3] >> fitprob;
          str_kin[iline][0] = whatBin[0];
          str_kin[iline][1] = whatBin[1];
          str_kin[iline][2] = whatBin[2];
            
          adV2[iFile][iline]  = x[2];
            
          cout<< "Bin " << whatBin[0] << "\t"<< whatBin[1] << "\t" << whatBin[2]<<"\t";
          cout <<"v2= "<< x[2] <<endl;
          iline++;
        }
        in.close();
      }// iFile
      
      if(bDoDebug) {
        for(int ib=0; ib<nBins; ib++) {
          cout <<"!!!! Looking at: pt = "<< str_kin[ib][0]<<"\t y: "<<str_kin[ib][1]<<"\t cent: "<<str_kin[ib][2]<<endl;
        }
      }
      // at this point have in the 2D arrays all variations; nominal value is stored in [0][0] 
      //-------------------------------------------
      // calculate the systm. uncertainty: 
      // A) for TnP: full difference between with and without TnP -- variation #1
      // B) for 4D efficiency: -- variation #2
      // C) for 3D efficiency:  -- #3 3/4D efficiency only from the non-prompt samples, to test the 3D efficiency binning, fitting, uncert.
      // D) for fit(sgn+bkg) systm: RMS of all variations -- #4--#8
      // finaly uncert: sqrt of sum in quadrature of A->D
      
      // we'll calculate each individually, and store them too (for future plotting if needed), and then calculate the final uncertainty
      
      double v2[nBins];
      double contrib_tnp[nBins];
      double contrib_4d[nBins];
      double contrib_3d[nBins];
      double contrib_fit[nBins];
      
      double v2Syst[nBins];
      
      for(int ib=0; ib<nBins; ib++) {
        double v2 = adV2[0][ib];// nominal value for bin ib
    
        // calculate individual contributions
        double relContrib_tnp = (v2-adV2[iVar_tnp][ib])/v2;
        contrib_tnp[ib]       = TMath::Power(relContrib_tnp,2);
        
        double relContrib_4d = (v2-adV2[iVar_4d][ib])/v2;
        contrib_4d[ib]       = TMath::Power(relContrib_4d,2);
    
        double relContrib_3d = (v2-adV2[iVar_3d][ib])/v2;
        contrib_3d[ib]       = TMath::Power(relContrib_3d,2); 
        
        for (int iFit=iVar_fit; iFit<nFiles; iFit++)
        {
          double iFitContrib = (v2-adV2[iFit][ib])/v2;
          if (method==0 || method==2) { // RMS
            contrib_fit[ib]+=TMath::Power(iFitContrib,2);
            if(bDoDebug)
            {
              cout<<"Fit contribution "<<iFit<<"\t = "<< iFitContrib<<endl;
            }
          } else if (method==1) { // Maximum
            if (contrib_fit[ib] < TMath::Power(iFitContrib,2))
            contrib_fit[ib] = TMath::Power(iFitContrib,2);
          }
        }
        // calculate total contribution: 
        if (method==0) v2Syst[ib] = v2 * TMath::Sqrt( contrib_tnp[ib] + contrib_4d[ib] + contrib_3d[ib] + contrib_fit[ib]/(nFiles-iVar_fit) );
        else if (method==1 || method==2) v2Syst[ib] = v2 * TMath::Sqrt( contrib_tnp[ib] + contrib_4d[ib] + contrib_3d[ib] + contrib_fit[ib] );
        if(bDoDebug)
        {
          cout<<"Bin "<<ib <<" v2 = "<< v2<<"\t Total_systm= " << v2Syst[ib] <<endl;
          cout<<"Relative contributions: \t TnP: "<<relContrib_tnp<<"\t 4D= "<<relContrib_4d<<"\t 3D "<<relContrib_3d<<endl;
        }
      }//for each bin
        
      // -----------------------------------------------------------------------------
      // write out the output
      std::string nameVar   = outFilePlot[iVar]; //pt
      std::string nameSig   = signal[iCateg];// pr, npr, bkg
      string outputFileName     = "syst_"+nameVar + "_"+ nameSig + "_nphibin" + nDphiBins + ".dat";
        
      std::string nameOutDir     = outputDir;
      std::string outputDats     = nameOutDir + "/data/";
      gSystem->mkdir(Form("./%s",outputDats.c_str()), kTRUE);// numbers   
        
      string outDataName = outputDats + outputFileName; 
      ofstream outputData(outDataName.c_str());
      if (!outputData.good()) {cout << "######### Fail to open *.txt file.##################" << endl;}
        
      outputData << "pT " << " rapidity " << " cent " << " v2_nominal " << " v2Syst_tot " << " contrib_tnp " << " contrib_4d " << " contrib_3d "<< "contrib_fit "<<"\n";
      for(int iBin=0; iBin<nBins; iBin++)
      {
        outputData << str_kin[iBin][0] <<" " << str_kin[iBin][1] << " " << str_kin[iBin][2] << " " << adV2[0][iBin] << " " << v2Syst[iBin] << " " <<contrib_tnp[iBin] << " " <<contrib_4d[iBin] << " " <<contrib_3d[iBin] << " "<< contrib_fit[iBin]<<"\n";

      }
      outputData.close();
    }// for each variable
  }// for each category
}






