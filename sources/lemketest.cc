//
// FILE: lemketest.cc -- Lemke test program
//
// $Id$
//

#include "nfg.h"
#include "lemke.h"
#include "gopts.cc"

main(int argc, char *argv[])
{
  Nfg<double> *N = 0;
  
  gString FInput = "", FOutput = "", FParam = "";
  gopt(argc, argv, FInput, FOutput, FParam);

  gFileInput inputfile(FInput.stradr());
  if (!inputfile.IsValid()) gout << "File not found\n";
  gFileInput paramfile(FParam.stradr());
  gFileOutput outputfile;
  if (FOutput == "") outputfile = stdout;
  else outputfile = FOutput.stradr();

  LemkeParams P;

  ReadNfgFile( (gInput &) inputfile, N);
  if (paramfile.IsValid()) {

    gArray<gString> p(4);
    p[1] = "trace";
    p[2] = "stopAfter";
    p[3] = "maxDepth";
    p[4] = "output";

    gArray<gString> pstrings = ReadParams(paramfile, p);

    if (pstrings[1] != "" ) P.trace = atoi(pstrings[1]);
    if (pstrings[2] != "" ) P.stopAfter = atoi(pstrings[2]);
    if (pstrings[3] != "" ) P.maxDepth = atoi(pstrings[3]);
    if (pstrings[4] != "" ) {  
      P.output = (gOutput *) new gFileOutput(pstrings[4].stradr());
    }
  }

  NFSupport S(*N);

  LemkeModule<double> M(*N, P,S);

  M.Lemke();


  M.GetSolutions().Dump(outputfile);
  outputfile << "\nNum Pivots = " << M.NumPivots();
  outputfile << ", Time = " << M.Time() << "\n";

  delete P.output;

  return 1;
}

