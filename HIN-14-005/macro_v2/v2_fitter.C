/*
 Macro that:
 a) reads the yield vs dphi histograms created by the readFitTable/makeHistos_v2.C
 b) fits the dphi distributions, to extract v2
 c) makes the v2 vs Npart, pt, and y (which you have to chose before running the macro)
 d) writes out all numbers and figures in a root file in directory rootFiles
 e) saves canvases as png and pdf in the figs/ directory

Note: binning, labels, etc, are defined in v2_dataNumbers_2015.h
 
 */
#if !defined(__CINT__) || defined(__MAKECINT__)
#include <iostream>
#include <fstream>
#include <string>

#include <Riostream.h>
#include <TSystem.h>
#include <TBrowser.h>
#include <TROOT.h>
#include <TGraph.h>
#include <TNtuple.h>
#include <TString.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TFile.h>
#include <TF1.h>
#include <TProfile.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TInterpreter.h>
#include <TGraphAsymmErrors.h>
#include <TGraphErrors.h>

#include "v2_dataNumbers_2015.h"
#include "../CMS_lumi.C"
#include "../tdrstyle.C"

#endif

void v2_fitter(int jpsiCategory      = -1, // -1 all; 1 : Prompt, 2 : Non-Prompt, 3: Bkg
               int nChoseCategory    = -1, // -1 = all; 0 integrated; 1 pt; 2: rapidity; 3: centrality;
               int nChoseSetting     = 1, // 0: nominal weighted; 1: systm. uncert (all fit settings in histYieldFile)
               int nDphiBins         = 4,
               const char* inputDir  = "../readFitTable", // the place where the input root files, with the histograms are
               const char* outputDir = "outputNumbers",
               bool bSavePlots       = true,
               bool doV2ZeroFit      = false)
{
  const char* signal[4]      = {"", "Prp","NPrp","Bkg"};
  // the order does not matter here
  string histYieldFile[] = {
  "histsV2Yields_20160304_v2noW_dPhiBins4",
  "histsV2Yields_20160304_v2W_dPhiBins4",
  "histsV2Yields_20160304_v2W_const_dPhiBins4",
  "histsV2Yields_20160304_v2W_4DEffOnly_dPhiBins4",
  "histsV2Yields_20160304_v2W_MLAR_dPhiBins4",
  "histsV2Yields_20160304_v2W_muIDTrig_dPhiBins4",
  "histsV2Yields_20160304_v2W_noTnPSF_dPhiBins4",
  "histsV2Yields_20160304_v2W_polFunct_dPhiBins4",
  "histsV2Yields_20160304_v2W_ctau2mm_dPhiBins4",
//  "histsV2Yields_20160304_v2W_resOpt2_dPhiBins4",
  "histsV2Yields_20160304_v2W_sigG1G2_dPhiBins4",
  };
  const int nConfigFilesIn = sizeof(histYieldFile)/sizeof(string);
  const char* histYieldFile_noWeight[1]     = {
    "histsV2Yields_20160304_v2noW_dPhiBins4",
  };

  int nFile_start       = 0;
  int nFile_end         = 1;
  if(nChoseSetting == 1) {
    cout <<"You are doing systematics too"<<endl;
    nFile_end  = nConfigFilesIn;
  }

  int varCategStart = 0; 
  int varCategEnd   = 4;
  if(nChoseCategory==0) varCategEnd=1;//mb only
  if(nChoseCategory==1) {varCategStart=1; varCategEnd=2;}//pt only
  if(nChoseCategory==2) {varCategStart=2; varCategEnd=3;}//y only
  if(nChoseCategory==3) {varCategStart=3;}//centrality only

  int categStart        = 1;
  int categEnd          = 4;
  if (jpsiCategory==1) categEnd=2;// pr
  if (jpsiCategory==2) {categStart=2; categEnd=3;}//npr
  if (jpsiCategory==3) {categStart=3;}//bkg

  gROOT->Macro("../logon.C");
  gStyle->SetOptFit(0);
  gStyle->SetOptStat(0);
  gStyle->SetEndErrorSize(5);

  gStyle->SetOptFit(0);
  gStyle->SetTitleFont(62,"xyz");
  gStyle->SetLabelFont(62,"xyz");
  gStyle->SetPadTopMargin(0.05);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadRightMargin(0.05);
  gStyle->SetMarkerSize(1.8);
  gStyle->SetErrorX(0); // disable if you want to draw horizontal error bars, e.g. when having variable bin size
  gStyle->SetEndErrorSize(3.5);
 
  for(int iCateg=categStart; iCateg<categEnd; iCateg++){
    for(int iVar=varCategStart; iVar<varCategEnd; iVar++) {// for each var: pt, y, cent,mb
      for (int iFile = nFile_start; iFile<nFile_end; iFile++) {
        std::string nameVarCateg   = outFilePlot[iVar];
        std::string nameSigCateg   = signal[iCateg];
        std::string nameOutDir     = outputDir;
        std::string inFile         = histYieldFile[iFile];
        gSystem->mkdir(Form("./%s/%s/root",outputDir,inFile.c_str()), kTRUE);// numbers
        gSystem->mkdir(Form("./%s/%s/data",outputDir,inFile.c_str()), kTRUE);// numbers
        gSystem->mkdir(Form("./%s/%s/figs/png",outputDir,inFile.c_str()), kTRUE);// figures
        gSystem->mkdir(Form("./%s/%s/figs/pdf",outputDir,inFile.c_str()), kTRUE);// figures
        std::string outputLocation = nameOutDir +"/"+ inFile;
        
        if(doV2ZeroFit)
        {
          gSystem->mkdir(Form("./%s/v2zeroFit/%s/root",outputDir,inFile.c_str()), kTRUE);// numbers
          gSystem->mkdir(Form("./%s/v2zeroFit/%s/data",outputDir,inFile.c_str()), kTRUE);// numbers
          gSystem->mkdir(Form("./%s/v2zeroFit/%s/figs/png",outputDir,inFile.c_str()), kTRUE);// figures
          gSystem->mkdir(Form("./%s/v2zeroFit/%s/figs/pdf",outputDir,inFile.c_str()), kTRUE);// figures
          outputLocation = nameOutDir +"/v2zeroFit/"+ inFile;
        }

        std::string outputFileName = Form("%s_%s_nphibin%d",nameVarCateg.c_str(),nameSigCateg.c_str(),nDphiBins);
        std::string outputDats     = outputLocation + "/data/";
        std::string outputRoots    = outputLocation + "/root/";
        std::string outputFigs     = outputLocation + "/figs/";
      
        cout<< "#### Input file is: "<< histYieldFile[iFile] <<endl;
        // ###################################  get the yields from input files
        TFile *fWeighFile   = new TFile(Form("%s/%s.root",inputDir,inFile.c_str()));
        TFile *fUnweighFile = new TFile(Form("%s/%s.root",inputDir,histYieldFile_noWeight[0]));

        TH1F *phPhi[20]; 
        TH1F *phPhi_noWeight[20]; 
        int nBins=0;
        int nBins_np=0;
        int nBins_pr=0;
        const char* rapidity = yBinsName[0];
        const char* ptbin    = ptBinsName[0];
        const char* centbin  = centBinsName[0];
        
        switch(iVar){
        case 1://pt
          cout<<"You are doing pt dependence!"<<endl;
          nBins = nPtBins; nBins_np = nPtBins_np;nBins_pr = nPtBins_pr;
          for(int iBin = 0; iBin < nBins; iBin++)
          {
            cout << "\t" << iBin << " " << nBins << " " << endl;
            if(iBin==1 || iBin==5) rapidity = yBinsName[3]; // low-pt bin 3->6.5 and fwd rapidity; we add it to the pT dependence 
            else rapidity = yBinsName[0];
            TString histInc(Form("Rap_%s_pT_%s_Cent_%s_%s",rapidity,ptBinsName[iBin],centbin,signal[iCateg]));
            cout<<"histogram input name: "<< histInc<<endl; 
            phPhi[iBin]          = (TH1F*)fWeighFile->Get(histInc);
            phPhi_noWeight[iBin] = (TH1F*)fUnweighFile->Get(histInc);
            cout <<  phPhi[iBin] << " " << phPhi_noWeight[iBin] << endl;
          }
          break;
          
        case 2://rapidity
          cout<<"You are doing rapidity dependence!"<<endl;
          nBins = nYBins; nBins_np = nYBins_np;nBins_pr = nYBins_pr;
          for(int iBin = 0; iBin < nBins; iBin++)
          {
            TString histInc(Form("Rap_%s_pT_%s_Cent_%s_%s",yBinsName[iBin],ptbin,centbin,signal[iCateg]));
            cout<<"histogram input name: "<< histInc<<endl; 
            phPhi[iBin]          = (TH1F*)fWeighFile->Get(histInc);
            phPhi_noWeight[iBin] = (TH1F*)fUnweighFile->Get(histInc);
            cout <<  phPhi[iBin] << " " << phPhi_noWeight[iBin] << endl;
          }
          break;
          
        case 3://centrality
          cout<<"You are doing centrality dependence!"<<endl;
          nBins = nCentBins; nBins_np = nCentBins_np; nBins_pr = nCentBins_pr;
          for(int iBin = 0; iBin < nBins; iBin++)
          {
            TString histInc(Form("Rap_%s_pT_%s_Cent_%s_%s",rapidity,ptbin,centBinsName[iBin],signal[iCateg]));
            cout<<"histogram input name: "<< histInc<<endl; 
            phPhi[iBin]          = (TH1F*)fWeighFile->Get(histInc);
            phPhi_noWeight[iBin] = (TH1F*)fUnweighFile->Get(histInc);
            cout <<  phPhi[iBin] << " " << phPhi_noWeight[iBin] << endl;
          }
          break;
          
        case 0:
        default:
          cout<<"You are calculating the integrated bin!"<<endl;
          nBins =1;
          TString histInc(Form("Rap_%s_pT_%s_Cent_%s_%s",rapidity,ptbin,centbin,signal[iCateg]));
          cout<<"histogram input name: "<< histInc<<endl; 
          phPhi[0]          = (TH1F*)fWeighFile->Get(histInc);
          phPhi_noWeight[0] = (TH1F*)fUnweighFile->Get(histInc);
          cout <<  phPhi[0] << " " << phPhi_noWeight[0] << endl;
          break; 
        }
        cout<< "Got the histograms!!!" <<endl;
    
        // ################################### create the (sum yield in all 4 dPhi bins) )
        int nDphi_yield[20][20]       = {{0}};
        double nDphi_yieldErr[20][20] = {{0}};
        int nSumDphiYield[20]         = {0};
        
        TH1F *phPhiNor[40];
        for(int iKin = 0; iKin < nBins; iKin++) // for all pt or y or cent  bins, including the minbias one (iKin==0)
        {
          cout << "\t" << iKin << " " << nBins << " " << endl;
          if (!phPhi[iKin] || !phPhi_noWeight[iKin]) {
            cout << "skip" << endl;
            continue;
          } else {
            cout << phPhi_noWeight[iKin]->GetName() << " " << phPhi[iKin]->GetName() << endl;
          }
          
          for(int ibin = 0; ibin < nDphiBins; ibin++) // add the yield in all dPhi bins
          {
            // get relative error from the 'unweighted case'
            double dRelErr_unweight    = phPhi_noWeight[iKin]->GetBinError(ibin+1)/phPhi_noWeight[iKin]->GetBinContent(ibin+1);
            nDphi_yieldErr[iKin][ibin] = dRelErr_unweight*phPhi[iKin]->GetBinContent(ibin+1);
            nDphi_yield[iKin][ibin]    = phPhi[iKin]->GetBinContent(ibin+1);
            nSumDphiYield[iKin]       += nDphi_yield[iKin][ibin];
            
            cout<<"dPhiBin= "<<ibin<< " yieldErr : "<<nDphi_yieldErr[iKin][ibin]<<"\t yield: "<< nDphi_yield[iKin][ibin] <<"\t Total: "<<nSumDphiYield[iKin]<<endl;
          }

          char tmp[512];
          // ###################################   create the normalized histograms
          double wbin = TMath::Pi()/(2*nDphiBins);
          sprintf(tmp,"phPhiNor_%d",iKin);
          phPhiNor[iKin] = new TH1F(tmp,tmp,nDphiBins,0,TMath::Pi()/2);
          phPhiNor[iKin]->Sumw2();
          
          for(int ibin = 0; ibin < nDphiBins; ibin++)
          {
            phPhiNor[iKin]->SetBinContent(ibin+1,(double)nDphi_yield[iKin][ibin]/((double)nSumDphiYield[iKin]*wbin));
            phPhiNor[iKin]->SetBinError(ibin+1,(double)nDphi_yieldErr[iKin][ibin]/((double)nSumDphiYield[iKin]*wbin));
            // cout<<ibin<<"\t content "<<phPhiNor[iKin]->GetBinContent(ibin+1)<<endl;
          }
          cout<< "iKin " << iKin << " nSumDphiYield : "<<nSumDphiYield[iKin]<<endl;
        }//ikin loop
        cout<<"Finished with the histograms!!!!!!"<<endl;
        
        //###################################  fit to get v2 , and store the fit parameters, again for all kinematic regions
        TF1 *v2Fit    = new TF1("v2Fit","[1]*(1+2*[0]*TMath::Cos(2.0*x))",0,TMath::PiOver2());
        //// fit parameter initialization     
        v2Fit->SetParameters(0.5,1);
        
        if(doV2ZeroFit)v2Fit    = new TF1("v2ZeroFit","[1]",0,TMath::PiOver2());
        v2Fit->SetLineColor(kMagenta+2);
        v2Fit->SetLineStyle(2);
        v2Fit->SetLineWidth(4);
        
        double dFit_v2[30]    = {0.0}; 
        double dFit_v2Err[30] = {0.0};
        double dFitProb[30]   = {0.0}; 
        
        for(int iKin = 0; iKin < nBins; iKin++)
        {
          cout<<"Bin "<<iKin<<" fitting ... "<<endl;
          phPhiNor[iKin]->Fit(v2Fit,"rqm"); 
          dFit_v2[iKin]    = v2Fit->GetParameter(0);
          dFit_v2Err[iKin] = v2Fit->GetParError(0);
          dFitProb[iKin]   = v2Fit->GetProb();
          cout<<"v2= "<<dFit_v2[iKin]<<"\t v2_err= "<<dFit_v2Err[iKin]<< "\t fit probability= " << dFitProb[iKin]
              << "\t Prob= " << TMath::Prob(v2Fit->GetChisquare(),v2Fit->GetNDF()) 
              << "\t NDF= " << v2Fit->GetNDF() << "\t Chi2= " << v2Fit->GetChisquare() << "\t Chi2/NDF= " << v2Fit->GetChisquare()/v2Fit->GetNDF() << endl;
        }
        cout<<"Finished fitting all bins!"<<endl;
        
        // ###################################  make EP resolution corrections (for all bins, used and not used)
        double dFit_v2Final[30]    = {0.0}; 
        double dFit_v2FinalErr[30] = {0.0};
        double resCorrection       = dEvPlResCorr[0];
        double resCorrectionErr    = dEvPlResCorrErr[0];
        for(int iBin = 0; iBin < nBins; iBin++)
        {
          cout<<"Bin "<<iBin<<" corrected for resolution!!"<<endl;
          if(iVar==3)
          {
            resCorrection    = dEvPlResCorr[iBin];
            resCorrectionErr = dEvPlResCorrErr[iBin];
          }
          dFit_v2Final[iBin]    = dFit_v2[iBin]/resCorrection;
          dFit_v2FinalErr[iBin] =  fabs( dFit_v2[iBin]/resCorrection * sqrt( TMath::Power(dFit_v2Err[iBin]/dFit_v2[iBin],2) + TMath::Power(resCorrectionErr/resCorrection,2) ) );
          cout<<"Bin = "<<iBin<<"\t raw_v2 = " << dFit_v2[iBin] << "\t corr_v2 = "<<dFit_v2Final[iBin]<<endl;
        }
        cout<<"Finished making the resolution corrections"<<endl;
        
        // ###################################  Setting v2 histograms, with different bins for prompt and non-prompt 
        TH1F *phV2Raw;  // raw v2
        TH1F *phV2;     // corrected for resolution
        
        switch(iVar){
        case 1://pt
          cout<<"You are doing pt dependence!"<<endl;
          phV2Raw  = new TH1F("phV2Raw",";p_{T} GeV/c;",nBins_pr,ptBins_pr); 
          phV2     = new TH1F("phV2",";p_{T} GeV/c;",nBins_pr,ptBins_pr);
          if(iCateg==2)
          {
            phV2Raw  = new TH1F("phV2Raw",";p_{T} GeV/c;",nBins_np,ptBins_np); 
            phV2     = new TH1F("phV2",";p_{T} GeV/c;",nBins_np,ptBins_np);
          }
          break;
          
        case 2:// rap
          cout<<"You are doing rapidity dependence!"<<endl;
          phV2Raw = new TH1F("phV2Raw",";|y|;",nBins_pr,yBins_pr);
          phV2    = new TH1F("phV2",";|y|;",nBins_pr,yBins_pr);
          if(iCateg==2)
          {
            phV2Raw = new TH1F("phV2Raw",";|y|;",nBins_np,yBins_np);
            phV2    = new TH1F("phV2",";|y|;",nBins_np,yBins_np);
          }
          break;
          
        case 3://cent
          cout<<"You are doing centrality dependence!"<<endl;
          phV2Raw = new TH1F("phV2Raw",";Centrality Bin;",nBins_pr,centBins_pr);
          phV2    = new TH1F("phV2",";Centrality Bin;",nBins_pr,centBins_pr);
          if(iCateg==2)
          {
            phV2Raw = new TH1F("phV2Raw",";Centrality Bin;",nBins_np,centBins_np);
            phV2    = new TH1F("phV2",";Centrality Bin;",nBins_np,centBins_np);
          }
          break;
          
        case 0:
        default://minbias
          cout<<"You are calculating the integrated bin!"<<endl;
          phV2Raw = new TH1F("phV2Raw",";Integrated Bin;",1,10,60);
          phV2    = new TH1F("phV2",";Integrated Bin;",1,10,60);
          break; 
        }
        
        phV2->SetDirectory(0);
        phV2Raw->SetDirectory(0);
        phV2->Sumw2();
        phV2Raw->Sumw2();
        
        // ################################### fill the results histograms; 
        //attention, the v2 vector contains, in order the bins, the bins in v2_dataNumbers_2015.h arrays:
        // ptBinsName[nPtBins], yBinsName[nYBins], centBinsName[nCentBins] 
        
        int binStart = 1;
        int binEnd = nBins-nBins_np;
        if(iCateg==2) 
        {
          binStart = nBins-nBins_np;
          binEnd   = nBins;
        }
        if(iVar==0)//integrated bin for all of them
        {
          binStart = 0;
          binEnd   = 1;
        }
        cout << "Starting bin: "<< binStart << "\t ending bins: " << binEnd << endl;
        
        // fill histograms, and write *.txt file with numbers
        string outDataName = outputDats + outputFileName + ".dat"; 
        ofstream outputData(outDataName.c_str());
        if (!outputData.good()) {cout << "######### Fail to open *.txt file.##################" << endl;}
        outputData << "pT " << " rapidity " << " cent " << " v2Raw " << " v2RawErr " << " v2 " << " v2Err " << " fitProb "<< "\n";
        int bin =1;
        for(int iBin = binStart; iBin < binEnd; iBin++)
        {
          phV2Raw->SetBinContent(bin, dFit_v2[iBin]);
          phV2Raw->SetBinError(bin,   dFit_v2Err[iBin]); 
          
          phV2->SetBinContent(bin, dFit_v2Final[iBin]);
          phV2->SetBinError(bin,   dFit_v2FinalErr[iBin]); 
          bin++;
          cout<<bin<<"\t v2_raw= "<<dFit_v2[iBin]<<"\t v2= "<<dFit_v2Final[iBin]<<endl;
          
          if(iVar==0) //integrated bin
            outputData << ptbin <<" " << rapidity << " " << centbin << " " <<dFit_v2[iBin] << " " <<dFit_v2Err[iBin] << " " <<dFit_v2Final[iBin] << " " <<dFit_v2FinalErr[iBin] << " "<< dFitProb[iBin] << "\n";
          if(iVar==1) //pt dependence
            outputData << ptBinsName[iBin] <<" " << rapidity << " " << centbin << " " <<dFit_v2[iBin] << " " <<dFit_v2Err[iBin] << " " <<dFit_v2Final[iBin] << " " <<dFit_v2FinalErr[iBin] << " "<< dFitProb[iBin] << "\n";
          if(iVar==2) //y dependence
            outputData << ptbin <<" " << yBinsName [iBin] << " " << centbin << " " <<dFit_v2[iBin] << " " <<dFit_v2Err[iBin] << " " <<dFit_v2Final[iBin] << " " <<dFit_v2FinalErr[iBin] << " "<< dFitProb[iBin] << "\n";
          if(iVar==3) //cent dependence
            outputData << ptbin <<" " << rapidity << " " << centBinsName[iBin] << " " <<dFit_v2[iBin] << " " <<dFit_v2Err[iBin] << " " <<dFit_v2Final[iBin] << " " <<dFit_v2FinalErr[iBin] << " "<< dFitProb[iBin] << "\n";
          
        }
        outputData.close();
        // ----------------------------------------------------------------- DRAWING
        TLatex *lt1 = new TLatex();
        lt1->SetNDC();
        lt1->SetTextSize(0.050);
    
        gROOT->Macro("~/JpsiStyle.C");
        // Draw the dPhi distributions
        TCanvas *pcDPhi = new TCanvas("pcDPhi","",900,900);
        pcDPhi->Divide(3,3);
        if(iVar==0) pcDPhi = new TCanvas("pcDPhi","pcDPhi");
        if (iVar == 0) { // mb pr & npr J/psi number is written here
          pcDPhi->SetTopMargin(0.08); // was 0.05
          pcDPhi->SetBottomMargin(0.12); // was 0.13
          pcDPhi->SetLeftMargin(0.18); // was 0.16
          pcDPhi->SetRightMargin(0.03); // was 0.02
          gStyle->SetLabelFont(42,"xyz");
          gStyle->SetTitleFont(42,"xyz");
        }
        for(int iBin = 0; iBin < nBins; iBin++)
        {
          pcDPhi->cd(iBin+1);
          phPhiNor[iBin]->SetTitle("");
          phPhiNor[iBin]->SetMaximum(1.0);
          phPhiNor[iBin]->SetMinimum(0.2);
          phPhiNor[iBin]->GetXaxis()->CenterTitle();
          phPhiNor[iBin]->GetYaxis()->CenterTitle();
          phPhiNor[iBin]->GetXaxis()->SetTitleOffset(0.95);
          phPhiNor[iBin]->GetYaxis()->SetTitleOffset(1.45);
          phPhiNor[iBin]->GetYaxis()->SetTitleSize(0.05);
          phPhiNor[iBin]->GetYaxis()->SetTitle("#frac{1}{N_{total}^{J/#psi}} #frac{dN}{d|#Delta#Phi|} (rad^{-1})");
          phPhiNor[iBin]->GetXaxis()->SetTitle("|#Delta#Phi| (rad)");
          phPhiNor[iBin]->SetMarkerStyle(20);
          phPhiNor[iBin]->SetMarkerColor(kBlack);
          phPhiNor[iBin]->SetMarkerSize(1.5);
          phPhiNor[iBin]->Draw("e1");
          lt1->SetTextSize(0.036);
          if (iVar == 0 && iCateg == 1) { // mb pr J/psi number is written here
            lt1->DrawLatex(0.22,0.34,Form("v_{2} = 0.07 #pm 0.01(stat.) #pm 0.01(syst.) #pm 0.03(glob.)"));
            lt1->DrawLatex(0.22,0.27,Form("v_{2}^{obs} = %.2f #pm %.2f",dFit_v2[iBin],dFit_v2Err[iBin]));
            lt1->DrawLatex(0.22,0.20,Form("R = 82%%"));
          } else if (iVar == 0 && iCateg == 2) { // mb npr J/psi number is written here
            lt1->DrawLatex(0.22,0.34,Form("v_{2} = 0.07 #pm 0.01(stat.) #pm 0.01(syst.) #pm 0.03(glob.)"));
            lt1->DrawLatex(0.22,0.27,Form("v_{2}^{obs} = %.2f #pm %.2f",dFit_v2[iBin],dFit_v2Err[iBin]));
            lt1->DrawLatex(0.22,0.20,Form("R = 82%%"));
          } else {
	    lt1->DrawLatex(0.22,0.34,Form("v_{2} = %.3f #pm %.3f",dFit_v2[iBin],dFit_v2Err[iBin]));
	    lt1->DrawLatex(0.22,0.27,Form("FitProb() = %.3f",dFitProb[iBin]));
          }
          if(iVar==1) lt1->DrawLatex(0.45,0.40,Form("%s",ptBinsLegend[iBin]));
          if(iVar==2) lt1->DrawLatex(0.60,0.40,Form("%s",yBinsLegend[iBin]));
          if(iVar==3) lt1->DrawLatex(0.60,0.40,Form("%s",centBinsLegend[iBin]));
        }
        
        pcDPhi->cd(1);
        lt1->SetTextSize(0.05);
        lt1->DrawLatex(0.22,0.84,legend[iCateg]);
        
        if(iVar==0) //mb
        {
          lt1->SetTextSize(0.04);
          lt1->DrawLatex(0.60,0.84,"|y| < 2.4");
          lt1->DrawLatex(0.60,0.79,"6.5 < p_{T} < 30 GeV/c");
          lt1->DrawLatex(0.60,0.73,"Cent. 10-60%");
        }
        if(iVar==1) //pt dependence
        {
          lt1->DrawLatex(0.20,0.24,"Cent. 10 - 60 %, |y| < 2.4");
          pcDPhi->cd(2);
          lt1->DrawLatex(0.20,0.24,"1.6 < |y| < 2.4");
          pcDPhi->cd(6);
          lt1->DrawLatex(0.20,0.24,"1.6 < |y| < 2.4");
        }
        if(iVar==2) lt1->DrawLatex(0.20,0.24,"Cent. 10 - 60 %, 6.5 < p_{T} < 30 GeV/c");//rapidity dependence
        if(iVar==3) lt1->DrawLatex(0.20,0.24,"6.5 < p_{T} < 30 GeV/c, |y| < 2.4"); // centrality dependence
        
        CMS_lumi(pcDPhi,12001000,0);
        
        if (bSavePlots)
        {
          pcDPhi->SaveAs(Form("./%s/png/phi_%s.png",outputFigs.c_str(),outputFileName.c_str()));
          pcDPhi->SaveAs(Form("./%s/pdf/phi_%s.pdf",outputFigs.c_str(),outputFileName.c_str()));
        }
        
        // -------------- make the results plots ---------------....
        
        TCanvas *pcResult = new TCanvas("pcResult","");
        
        TH1F *hPad = new TH1F("hPad1",";p_{T} GeV/c;v_{2}",10,0,30);
        if(iVar==2) hPad = new TH1F("hPad2",";|y|;v_{2}",10,0,2.4);
        if(iVar==3) hPad = new TH1F("hPad3",";Centrality Bin (%);v_{2}",10,0,100);
        
        hPad->GetXaxis()->CenterTitle();
        hPad->GetYaxis()->CenterTitle();
        hPad->SetMaximum(0.25);
        hPad->SetMinimum(-0.05);
        hPad->Draw();
        
        phV2Raw->SetMarkerSize(1.4);
        phV2->SetMarkerSize(1.4);
        
        if(iCateg == 1){// prompt
          phV2Raw->SetMarkerStyle(21);
          phV2Raw->SetMarkerColor(kRed+2);
          phV2Raw->SetLineColor(kRed+2);
          
          phV2->SetMarkerStyle(21);
          phV2->SetMarkerColor(kRed+2);
          phV2->SetLineColor(kRed+2);
        }
        if(iCateg == 2){// non-prompt
          phV2Raw->SetMarkerStyle(29);
          phV2Raw->SetMarkerColor(kOrange+2);
          phV2Raw->SetLineColor(kOrange+2);
          phV2Raw->SetMarkerSize(2.0);
          
          phV2->SetMarkerStyle(29);
          phV2->SetMarkerColor(kOrange+2);
          phV2->SetLineColor(kOrange+2);
          phV2->SetMarkerSize(2.0);
        }
        // ---------------- draw the v2 w/o the EP res correction
        phV2Raw->Draw("e1 same");
        
        lt1->SetTextSize(0.05);
        lt1->DrawLatex(0.48,0.88,"CMS Preliminary");
        lt1->DrawLatex(0.48,0.81,"PbPb  #sqrt{s_{NN}} = 2.76 TeV");
        lt1->DrawLatex(0.48,0.74,"L_{int} = 166 #mub^{-1}");
        lt1->SetTextSize(0.04);
        lt1->DrawLatex(0.18,0.88,legend[iCateg]);
        
        if(iVar==1)// pt
        {
          lt1->DrawLatex(0.48,0.66,"Cent. 10 - 60 %");
          lt1->DrawLatex(0.48,0.60,"|y| < 2.4, 1.6<|y| < 2.4");
        }
        if(iVar==2)//rap
        {
          lt1->DrawLatex(0.64,0.87,"Cent. 10 - 60 %");
          lt1->DrawLatex(0.21,0.28,"6.5 < p_{T} < 30 GeV/c");
        }
        if(iVar==3)// centrality
        {
          lt1->DrawLatex(0.48,0.66,"p_{T} > 6.5 GeV/c");
          lt1->DrawLatex(0.48,0.60,"|y| < 2.4");
        }
        // save the v2 w/o resolution correction
        if(bSavePlots)
        {
          pcResult->SaveAs(Form("./%s/png/v2Raw_%s.png",outputFigs.c_str(),outputFileName.c_str()));
          pcResult->SaveAs(Form("./%s/pdf/v2Raw_%s.pdf",outputFigs.c_str(),outputFileName.c_str()));
        }
        
        // ------------------  draw the EP -resolution corrected v2 
        hPad->Draw();
        phV2->Draw("e1 same");
        lt1->SetTextSize(0.05);
        lt1->DrawLatex(0.48,0.88,"CMS Preliminary");
        lt1->DrawLatex(0.48,0.81,"PbPb  #sqrt{s_{NN}} = 2.76 TeV");
        lt1->DrawLatex(0.48,0.74,"L_{int} = 166 #mub^{-1}");
        lt1->SetTextSize(0.04);
        lt1->DrawLatex(0.18,0.88,legend[iCateg]);
        
        if(iVar==1)// pt
        {
          lt1->DrawLatex(0.48,0.66,"Cent. 10 - 60 %");
          lt1->DrawLatex(0.48,0.60,"|y| < 2.4, 1.6<|y| < 2.4");
        }
        if(iVar==2)//rap
        {
          lt1->DrawLatex(0.64,0.66,"Cent. 10 - 60 %");
          lt1->DrawLatex(0.21,0.28,"6.5 < p_{T} < 30 GeV/c");
        }
        if(iVar==3)// centrality
        {
          lt1->DrawLatex(0.48,0.66,"p_{T} > 6.5 GeV/c");
          lt1->DrawLatex(0.48,0.60,"|y| < 2.4");
        }
        // save the v2 w/o resolution correction
        if(bSavePlots)
        {
          pcResult->SaveAs(Form("./%s/png/v2_%s.png",outputFigs.c_str(),outputFileName.c_str()));
          pcResult->SaveAs(Form("./%s/pdf/v2_%s.pdf",outputFigs.c_str(),outputFileName.c_str()));
        }
        
        // ---------------------------OUTPUT FILE------------------------------
        // output file with all numbers/histograms
        TFile *out = new TFile(Form("./%s/%s.root",outputRoots.c_str(),outputFileName.c_str()),"RECREATE");
        out->cd();
        
        rapidity = yBinsName[0];
        ptbin    = ptBinsName[0];
        centbin  = centBinsName[0];
        switch(iVar){
        case 1://pt
          cout<<"You are writting pt dependence!"<<endl;
          for(int iBin = 0; iBin < nBins; iBin++)
          {
            phPhiNor[iBin]->SetName(Form("Rap_%s_pt_%s_cent_%s_%s",rapidity,ptBinsName[iBin],centbin,signal[iCateg]));
            if(iBin==1 || iBin==5)  phPhiNor[iBin]->SetName(Form("Rap_%s_pt_%s_cent_%s_%s",yBinsName[3],ptBinsName[iBin],centbin,signal[iCateg]));
            phPhiNor[iBin]->Write();
          }
          break;
          
        case 2:
          cout<<"You are writting rapidity dependence!"<<endl;
          for(int iBin = 0; iBin < nBins; iBin++)
          {
            phPhiNor[iBin]->SetName(Form("Rap_%s_pt_%s_cent_%s_%s",yBinsName[iBin],ptbin,centbin,signal[iCateg]));
            phPhiNor[iBin]->Write();
          }
          break;
          
        case 3:
          cout<<"You are doing centrality dependence!"<<endl;
          for(int iBin = 0; iBin < nBins; iBin++)
          {
            phPhiNor[iBin]->SetName(Form("Rap_%s_pt_%s_cent_%s_%s",rapidity,ptbin,centBinsName[iBin],signal[iCateg]));
            phPhiNor[iBin]->Write();
          }
          break;  

        case 0:
        default:
          cout<<"You are writting the integrated bin!"<<endl;
          nBins =1;
          phPhiNor[0]->SetName(Form("Rap_%s_pt_%s_cent_%s_%s",rapidity,ptbin,centbin,signal[iCateg]));
          phPhiNor[0]->Write();
          break; 
        }
        
        phV2Raw->Write();
        phV2->Write();
        
        out->Write();
        out->Close();
    
      }// for each file
    }//for each category (mb, pt, y, cent)
  }// for each jpsicateg
}
