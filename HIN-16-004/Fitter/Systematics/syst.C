#ifndef syst_C
#define syst_C

#include "syst.h"
#include "../Macros/Utilities/monster.h"

vector<TString> fileList_syst(const char* token, const char* prependPath) {
   vector<TString> ans;

   TString basedir("Systematics/csv/");
   if ( strcmp(prependPath,"") ) basedir.Prepend(Form("%s/",prependPath));
   TSystemDirectory dir(token,basedir);

   TList *files = dir.GetListOfFiles();

   if (files) {
      TIter next(files);
      TSystemFile *file;
      TString fname;

      while ((file=(TSystemFile*)next())) {
         fname = file->GetName();
         if (fname.EndsWith(".csv") && (TString(token) == "" || fname.Index(token) != kNPOS)) {
            ans.push_back(basedir+fname);
         }
      }
   }

   return ans;
};


map<anabin, syst> readSyst(const char* systfile, const char* workDirName, const char* path2Fitter, int mode, const char* workDirNameFail) {
   map<anabin, syst> ans;

   ifstream file(systfile);
   if (!(file.good())) return ans;

   string systname; getline(file,systname);

   string line;
   double rapmin=0, rapmax=0, ptmin=0, ptmax=0, centmin=0, centmax=0, value=0;

   while (file.good()) {
      getline(file,line);
      if (line.size()==0) break;
      TString tline(line.c_str());
      TString t; Int_t from = 0, cnt=0;
      while (tline.Tokenize(t, from , ",")) {
         t.Strip(TString::kBoth,' ');
         value = atof(t.Data());
         if (cnt==0) rapmin = atof(t.Data());
         else if (cnt==1) rapmax = value;
         else if (cnt==2) ptmin = value;
         else if (cnt==3) ptmax = value;
         else if (cnt==4) centmin = value;
         else if (cnt==5) centmax = value;
         else if (cnt>6) {
            cout << "Warning, too many fields, I'll take the last one." << endl;
            continue;
         }
         cnt++;
      }
      anabin thebin(rapmin, rapmax, ptmin, ptmax, centmin, centmax);
      syst thesyst; thesyst.value = value; thesyst.name = systname;
     
      if ( strcmp(workDirName,"") ) {
         double doubleR;
         if (mode==0) doubleR = doubleratio_pass_nominal(workDirName, thebin, path2Fitter);
         else if (mode==1) doubleR = doubleratio_prompt_nominal(workDirName, workDirNameFail, thebin, path2Fitter);
         else doubleR = doubleratio_nonprompt_nominal(workDirName, workDirNameFail, thebin, path2Fitter);

         TString sfile(systfile), systtag;
         if (sfile.Contains("bhad_add")) systtag = "Bhad_add";
         else if (sfile.Contains("bhad") || sfile.Contains("eff")) systtag = "DR";
         else if (sfile.Contains("PbPb_fit")) systtag = "fitpbpb";
         else if (sfile.Contains("PP_fit")) systtag = "fitpp";

         if (mode==0) thesyst.value_dR = doubleratio_pass_syst(workDirName, thebin, path2Fitter, systtag.Data());
         else if (mode==1) thesyst.value_dR = doubleratio_prompt_syst(workDirName, workDirNameFail, thebin, path2Fitter, systtag.Data());
         else thesyst.value_dR = doubleratio_nonprompt_syst(workDirName, workDirNameFail, thebin, path2Fitter, systtag.Data());

         thesyst.value_dR_rel = thesyst.value_dR/doubleR;
      } else {
         thesyst.value_dR = -99.;
         thesyst.value_dR_rel = -99.;
      }

      ans[thebin] = thesyst;
   }

   file.close();
  
   return ans;
};

map<anabin, syst> combineSyst(vector< map<anabin, syst> > theSysts, string name, bool isdRSyst) {
   map<anabin, syst> ans;

   vector< map<anabin, syst> >::const_iterator it;
   for (it=theSysts.begin(); it!=theSysts.end(); it++) {
      map<anabin, syst>::const_iterator it2;
      for (it2=it->begin(); it2!=it->end(); it2++) {
         anabin thebin = it2->first;
         syst thesyst = it2->second;
         thesyst.name = name;

         // if we already have a syst for this bin, sum quadractically the existing syst and the new syst
         if (ans.find(thebin) != ans.end())
         {
           if (isdRSyst)
           {
           thesyst.value_dR = sqrt(pow(thesyst.value_dR,2) + pow(ans[thebin].value_dR,2));
           thesyst.value_dR_rel = sqrt(pow(thesyst.value_dR_rel,2) + pow(ans[thebin].value_dR_rel,2));
           }
           else thesyst.value = sqrt(pow(thesyst.value,2) + pow(ans[thebin].value,2));
         }
         ans[thebin] = thesyst;
      }
   }

   return ans;
};

map<anabin, syst> readSyst_all_pass(const char* token, const char* prependPath, const char* workDirName) {
   map<anabin,syst> ans;
   set<anabin> all = allbins();
   for (set<anabin>::const_iterator it = all.begin(); it!=all.end(); it++) {
      syst thesyst;
      thesyst.name = token;
      thesyst.value = doubleratio_pass_syst(workDirName, *it, prependPath, token);
      thesyst.value_dR = thesyst.value;
      thesyst.value_dR_rel = thesyst.value / doubleratio_pass_nominal(workDirName, *it, prependPath);
      ans[*it] = thesyst;
   }
   return ans;
}

map<anabin, syst> readSyst_all_prompt(const char* token, const char* prependPath, const char* workDirName, const char* workDirNameFail) {
   map<anabin,syst> ans;
   set<anabin> all = allbins();
   for (set<anabin>::const_iterator it = all.begin(); it!=all.end(); it++) {
      syst thesyst;
      thesyst.name = token;
      thesyst.value = doubleratio_prompt_syst(workDirName, workDirNameFail, *it, prependPath, token);
      thesyst.value_dR = thesyst.value;
      thesyst.value_dR_rel = thesyst.value / doubleratio_prompt_nominal(workDirName, workDirNameFail, *it, prependPath);
      ans[*it] = thesyst;
   }
   return ans;
}

map<anabin, syst> readSyst_all_nonprompt(const char* token, const char* prependPath, const char* workDirName, const char* workDirNameFail) {
   map<anabin,syst> ans;
   set<anabin> all = allbins();
   for (set<anabin>::const_iterator it = all.begin(); it!=all.end(); it++) {
      syst thesyst;
      thesyst.name = token;
      thesyst.value = doubleratio_nonprompt_syst(workDirName, workDirNameFail, *it, prependPath, token);
      thesyst.value_dR = thesyst.value;
      thesyst.value_dR_rel = thesyst.value / doubleratio_nonprompt_nominal(workDirName, workDirNameFail, *it, prependPath);
      ans[*it] = thesyst;
   }
   return ans;
}

map<anabin, syst> readSyst_all(const char* prependPath, const char* workDirName, bool doPrintTex, const char* texName, int mode, const char* workDirNameFail) {
   vector<TString> tags;
   tags.push_back("fitpp");
   tags.push_back("fitpbpb");
   tags.push_back("DR");
   tags.push_back("Bhad_add");
  
   vector< map<anabin, syst> > systmap_all;

   for (vector<TString>::const_iterator it=tags.begin(); it!=tags.end(); it++) {
      map<anabin,syst> systmap;
      if (mode==0) systmap = readSyst_all_pass(it->Data(),prependPath,workDirName);
      else if (mode==1) systmap = readSyst_all_prompt(it->Data(),prependPath,workDirName,workDirNameFail);
      else systmap = readSyst_all_nonprompt(it->Data(),prependPath,workDirName,workDirNameFail);
      systmap_all.push_back(systmap);
   }

   map<anabin,syst> ans = combineSyst(systmap_all, "Total" ,true);
   systmap_all.push_back(ans);

   if (doPrintTex) printTex(systmap_all, texName, true);

   return ans;
};

void printTex(vector< map<anabin, syst> > theSysts, const char* texName, bool isdRSyst) {
   unsigned int nsyst = theSysts.size();

   ofstream file(texName);
   file << "\\begin{tabular}{|ccc|"; 
   for (unsigned int i=0; i<nsyst; i++) {
      if (i==nsyst-1) file << "|";
      file << "c|";
   }
   file << "}" << endl;
   file << "\\hline" << endl;
   file << "$|y|$ & \\pt & Centrality";
   for (unsigned int i=0; i<nsyst; i++) file << " & " << theSysts[i].begin()->second.name;
   file<< "\\\\" << endl;
   file << "\\hline" << endl;

   anabin oldbin(0,0,0,0,0,0);
   map<anabin, vector<syst> > themap = vm2mv(theSysts);
   map<anabin, vector<syst> >::const_iterator itm;
   for (itm=themap.begin(); itm!=themap.end(); itm++) {
      vector<syst> v = itm->second;
      if (v.size() != nsyst) {
         cout << "Error, not all systematics have the same size. Aborting." << endl;
         file.close();
         return;
      }
      anabin thebin = itm->first;
      if (thebin.rapbin() == oldbin.rapbin()) {
         file << " - ";
      } else {
         if (itm != themap.begin()) file << "\\hline" << endl;
         file.precision(1); file.setf(ios::fixed);
         file << thebin.rapbin().low() << " $< |y| < $ " << thebin.rapbin().high();
      }
      file << " & ";
      if (thebin.ptbin() == oldbin.ptbin()) {
         file << " - ";
      } else {
         file.precision(1); file.setf(ios::fixed);
         file << thebin.ptbin().low() << " $< \\pt < $ " << thebin.ptbin().high();
      }
      file << " & ";
      file.unsetf(ios::fixed);
      file << thebin.centbin().low()/2 << "-" << thebin.centbin().high()/2 << "\\% ";
      if (isdRSyst) file.precision(3);
      else file.precision(1);
      file.setf(ios::fixed);

      for (unsigned int i=0; i<nsyst; i++) {
         if (isdRSyst) file << " & " << v[i].value_dR;
         else file << " & " << 100.*v[i].value;
      }
      file << " \\\\" << endl;

      oldbin = thebin;
   }

   file << "\\hline" << endl;
   file << "\\end{tabular}" << endl;
   file.close();
   cout << "Closed " << texName << endl;
}

map<anabin, vector<syst> > vm2mv(vector< map<anabin,syst> > v) {
   map<anabin, vector<syst> > ans;

   vector<map<anabin,syst> >::const_iterator itv;
   map<anabin,syst>::const_iterator itm;
   for (itv=v.begin(); itv!=v.end(); itv++) {
      for (itm=itv->begin(); itm!=itv->end(); itm++) {
         anabin thebin = itm->first;
         if (ans.find(thebin)==ans.end()) ans[thebin] = vector<syst>();
         ans[thebin].push_back(itm->second);
      }
   }

   return ans;
}


RooWorkspace* getWorkspaceFromBin(anabin thebin, const char* workDirName, const char* token, const char* path2Fitter){
  
  // List of files in workDirName
  vector<TString> theFiles = fileList(workDirName,token,"DATA",path2Fitter);
  if ( (theFiles.size() < 1) )
  {
    cout << "#[Error]: No files found in " << workDirName << endl;
    return 0x0;
  }
  
  if (!strcmp(token,"PP"))
  {
    bin<int> bincent(0,200);
    thebin.setcentbin(bincent);
  }
  
  // Find the name of the file containing the workspace
  int cnt=1;
  bool wsFound(false);
  vector<TString>::const_iterator it;
  for (it=theFiles.begin(); it!=theFiles.end(); it++)
  {
    anabin thebinTest = binFromFile(*it);
    if (thebinTest==thebin)
    {
      wsFound = true;
      break;
    }
  }
  if (!wsFound)
  {
    cout << "No workspace root file found in " << workDirName << endl;
    return 0x0;
  }
  
  // Read file
  TFile *f = TFile::Open(*it,"READ");
  if (!f)
  {
    cout << "#[Error]: Unable to read file " << *it << endl;
    return 0x0;
  }
  
  // Retrieve workspace from file
  RooWorkspace* ws = (RooWorkspace*) f->Get("workspace");
  if (!ws)
  {
    cout << "#[Error]: Unable to retrieve workspace" << endl;
    return 0x0;
  }
  
  RooWorkspace* wsClone = (RooWorkspace*)(ws->Clone());
  f->Close(); delete f;
  return wsClone;
}

#endif // ifndef syst_C
