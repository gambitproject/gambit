//
// FILE: algdlgs.h -- declaration of base classes for all of the algorithm
//                    parameter dialogs
//
// $Id$
//

#ifndef ALGDLGS_H
#define ALGDLGS_H

#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"

#include "gnumber.h"     // for gPrecision

#define PARAMS_SECTION  "Algorithm Params"      // section in .ini file

// This class just creates fields for error/output, trace level, and controls
// the max # of solutions generated.
// Also creates the option to use the current settings as default

#define     ERROR_FIELD         1
#define     OUTPUT_FIELD        2
#define     SPS_FIELD           4
#define     MAXSOLN_FIELD       8
#define     PRECISION_FIELD     16

//=========================================================================
//                OutputParamsSettings: Class declaration
//=========================================================================

class OutputParamsSettings {
protected:
  char *outname, *errname;
  gOutput *outfile, *errfile;
  wxStringList *trace_list, *m_precisionList;
  char *trace_str, *m_precisionStr;
 // max solutions_per_subgame, max total solutions
  int m_stopAfter, m_maxSolns;
  char *defaults_file;
  gPrecision m_precision;

  gOutput *MakeOutputFile(const char *s, gOutput *&outp) const;
  void SaveDefaults(void); // Called automatically in the destructor

public:
  OutputParamsSettings(void);
  virtual ~OutputParamsSettings(void);

  // Return the results...
  gOutput *OutFile(void) const;
  gOutput *ErrFile(void) const;
  int TraceLevel(void) const;
  bool Default(void) const { return true; }

  int StopAfter(void) const { return m_stopAfter; }
  int MaxSolns(void) const { return m_maxSolns; }

  gPrecision Precision(void) const;
};

//=========================================================================
//                 OutputParamsDialog: Class declaration
//=========================================================================

class OutputParamsDialog : public virtual OutputParamsSettings, 
			   public MyDialogBox {
public:
  // Constructor
  OutputParamsDialog(const char *label = 0, wxWindow *parent = 0, 
		     const char *help_str = 0);

  // Destructor
  virtual ~OutputParamsDialog(void);

  // Create the fields
  void MakeOutputFields(unsigned int fields = OUTPUT_FIELD);
};


//=========================================================================
//                 PxiParamsSettings: Class declaration
//=========================================================================

// This class is used for PXI type algorithms.  That is those that generate
// PXI compatible output files.  It creates the fields for output name,
// date type [Log/Lin], and allows one to run PXI right after the algorithm
// is done (call RunPxi).  Note that PxiParamsSettings will prompt for a
// new filename iff it is created by itself (not a part of PxiParamsDialog)
// and the naming option is set to "Prompt"

class PxiParamsSettings: public virtual OutputParamsSettings {
protected:
  char         *pxi_command, *pxiname, *algname, *filename;
  gOutput      *pxifile;
  Bool          run_pxi;
  wxStringList *type_list, *name_option_list;
  char         *type_str, *name_option_str;

  void SaveDefaults(void);
  static int naming_option; // 0-default, 1-saved, 2-query

public:
  PxiParamsSettings(const char *alg, const char *fn);
  virtual ~PxiParamsSettings();
  
  // Return the results...
  gOutput *PxiFile(void);
  int      PxiType(void);

  // Run pxi if necessary
  int RunPxi(void);

  // Check if I am a parent of a PxiParamsDialog derived class
  virtual bool FromDialog(void) { return false; }
};

//=========================================================================
//                 PxiParamsDialog: Class declaration
//=========================================================================

class PxiParamsDialog : public OutputParamsDialog,
			public virtual PxiParamsSettings {
public:
  // Constructor
  PxiParamsDialog(const char *alg = "Pxi", const char *label = 0, 
		  const char *filename = "pxi.out", wxWindow *parent = 0, 
		  const char *help_str = 0);

  // Destructor
  virtual ~PxiParamsDialog(void);

  // Create Fields
  void MakePxiFields(void);
  virtual bool FromDialog(void) { return true; }
};

#endif   // ALGDLGS_H

