//
//  dt1toefg, .dt1 to .efg converter front end
//     converts old gambit save files to the newer format
// 
// $Id$
//

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "gstring.h"

#ifndef __GNUG__
#include <dir.h>
#include <dos.h>
#endif // !__GNUG__

extern void dt1toefg(FILE *, FILE *);

output cout(stdout);

void doConversion(gString fIn, gString fOut)
{
  char *inName, *outName;
  FILE *fileIn, *fileOut;

  fileIn = fopen(fIn, "r");
  cout << "Input:  " << fIn;
  if (fileIn == NULL)
    {
      cout << "\nError#1.  Could not open input file " << fIn << "\n";
      exit(1);
    }

  fileOut = fopen(fOut, "w");
  cout << "\n  Output:  " << fOut << "\n";
  if (fileOut == NULL)
    {
      cout << "Error#2.  Could not open output file " << fOut << "\n";
      exit(2);
    }

  dt1toefg(fileIn, fileOut);

  // fclose(fileIn); , done already
  fclose(fileOut);
}

gString suffixCheck(gString pathnam, gString suffix)
{
  gString temp, posSuf;

  temp = pathnam;
  posSuf = temp.right(4);    // 4 because of '.ccc', use less for shorter suffixes

  if (posSuf == suffix)
    { int lengthLeft = temp.length() - 4;
      return temp.left(lengthLeft);}
  else
    { return temp;}
}

gString replaceSuffix(gString fName, gString newSuffix)
{
  gString result;

  int sufBegin;
  sufBegin = fName.lastOccur('.');
  assert(sufBegin != 0);
  result = fName.left(sufBegin - 1);
  result = result + newSuffix;
  return result;
}

void helpInst(const gString &program_name)
{
    cout << program_name << " help \n";
    cout << "Syntax:  " << program_name << " [infile] [infile] [infile] ... \n";
    cout << "  -or-   " << program_name << " [infile] [-o outfile] \n";
    cout << "\n       " << program_name << " will expect the suffixes .dt1 for input files and\n";
    cout << "       .efg for output files.  It will supply them if they are not\n";
    cout << "       present.\n";
    cout << "Switch: \n";
    cout << "   -o  This will specify an output file.  If this option is not\n";
    cout << "       used, input files will be translated to files of the same\n";
    cout << "       name, with the appropriate suffix.  If this option is used,\n";
    cout << "       only one input file may be specified.  Any other number\n";
    cout << "       will cause an error.\n\n";
}

void main(int argc, char *argv[])
{
  if (argc == 1)
    {
      //check name and do help screen;
      helpInst(argv[0]);
    }
  if (argc > 1)
    {
      gString fIn;
      gString fOut;
      gString fRoot;
      gString temp;
      gString inSuffix = ".dt1";
      gString outSuffix = ".efg";
      #ifdef __BORLANDC__
	inSuffix = inSuffix.upcase();
	outSuffix = outSuffix.upcase();
      #endif //__BORLANDC
      int outFlag = 0;
      for (int i=1; i < argc; i++)   //Checking for -o flag
	{
	  temp = argv[i];
	  if ( temp == "-o" )
	    {
	      if (i == argc-1)
		{cout << "Error#3, no output file specified\n";
		 exit(3);}
	      else
		{ outFlag = i;}
	      if (argc != 4)
		{cout << "Error#4, incorrect number of input files\n";
		 exit(4);}
	    }
	}
      if (outFlag)         //-o flag was found
	{
	    if (outFlag == 1)
	      {
		temp = suffixCheck(argv[3], inSuffix.dncase());
		fIn = suffixCheck(temp, inSuffix.upcase()) + inSuffix;
		temp = suffixCheck(argv[2], outSuffix.dncase());
		fOut = suffixCheck(temp, outSuffix.upcase()) + outSuffix;
	      }
	    else
	      {
		temp = suffixCheck(argv[1], inSuffix.dncase());
		fIn = suffixCheck(temp, inSuffix.upcase()) + inSuffix;
		temp = suffixCheck(argv[3], outSuffix.dncase());
		fOut = suffixCheck(temp, outSuffix.upcase()) + outSuffix;
	      }
	  doConversion(fIn, fOut);
	  cout << "Done.  1 file converted.\n";
	}
      else   //-o flag not found
	{
	  int converCount = 0;
	  for (int j = 1; j < argc; j++)
	    {
	      #ifdef __BORLANDC__
		struct ffblk fileBlock;
		int done;

		temp = argv[j];
		int sufBegin;
		sufBegin = temp.lastOccur('.');
		if (sufBegin == 0) {temp = temp + inSuffix;}
		else {temp = replaceSuffix(temp,inSuffix);};

		done = findfirst(temp, &fileBlock, 0);
		while (!done) {
		  cout << fileBlock.ff_name << "\n";
		  temp = suffixCheck(fileBlock.ff_name, inSuffix.dncase());
	      #endif //__BORLANDC__
	      #ifdef __GNUG__
		  temp = suffixCheck(argv[j], inSuffix.dncase());
	      #endif //__GNUG__
		fRoot = suffixCheck(temp, inSuffix.upcase());
		fIn = fRoot + inSuffix;
		fOut = fRoot + outSuffix;
		doConversion(fIn, fOut);
		converCount++;
	      #ifdef __BORLANDC__
		  done = findnext(&fileBlock);
		  }
	      #endif //__BORLANDC__
	    }
	  cout << "Done.  " << converCount << " files converted.\n";
	}
    }
}
