//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to class representing extensive form viewing window
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef EFGSHOW_H
#define EFGSHOW_H

#include "wx/listctrl.h"
#include "wx/sashwin.h"
#include "wx/printdlg.h"
#include "wx/notebook.h"

#include "gambit.h"
#include "efgconst.h"
#include "nash/behavsol.h"

class EfgProfileList;
class EfgNavigateWindow;
class EfgOutcomeWindow;
class EfgSupportWindow;
class TreeWindow;

const int idEFG_SOLUTION_LIST = 900;

class EfgShow : public wxFrame {
private:
  efgGame &m_efg;
  TreeWindow *m_treeWindow;
  gbtEfgNode m_cursor, m_copyNode, m_cutNode;

  int m_currentProfile;
  gList<BehavSolution> m_profiles;

  EFSupport *m_currentSupport;
  gList<EFSupport *> m_supports;

  EfgProfileList *m_profileTable;
  wxString m_filename;
  wxSashWindow *m_treeSashWindow, *m_nodeSashWindow, *m_toolSashWindow;
  wxSashWindow *m_solutionSashWindow;

  wxNotebook *m_infoNotebook;
  EfgNavigateWindow *m_navigateWindow;
  EfgOutcomeWindow *m_outcomeWindow;
  EfgSupportWindow *m_supportWindow;

  wxPageSetupData m_pageSetupData;
  wxPrintData m_printData;

  void MakeMenus(void);
  void MakeToolbar(void);
  void AdjustSizes(void);
  
  // Event handlers
  void OnCloseWindow(wxCloseEvent &);
  void OnFocus(wxFocusEvent &);
  void OnSize(wxSizeEvent &);
  void OnSashDrag(wxSashEvent &);

  // Menu event handlers
  void OnFileNew(wxCommandEvent &);
  void OnFileOpen(wxCommandEvent &);
  void OnFileSave(wxCommandEvent &);
  void OnFileImportComLab(wxCommandEvent &);
  void OnFileExportBMP(wxCommandEvent &);
  void OnFileExportJPEG(wxCommandEvent &);
  void OnFileExportPNG(wxCommandEvent &);
  void OnFileExportPS(wxCommandEvent &);
  void OnFilePageSetup(wxCommandEvent &);
  void OnFilePrintPreview(wxCommandEvent &);
  void OnFilePrint(wxCommandEvent &);
  void OnFileExit(wxCommandEvent &);
  void OnFileMRUFile(wxCommandEvent &);

  void OnEditCut(wxCommandEvent &);
  void OnEditCopy(wxCommandEvent &);
  void OnEditPaste(wxCommandEvent &);
  
  void OnEditInsert(wxCommandEvent &);
  void OnEditDelete(wxCommandEvent &);
  void OnEditReveal(wxCommandEvent &);

  void OnEditToggleSubgame(wxCommandEvent &);
  void OnEditMarkSubgameTree(wxCommandEvent &);
  void OnEditUnmarkSubgameTree(wxCommandEvent &);

  void OnEditMove(wxCommandEvent &);
  void OnEditGame(wxCommandEvent &);

  void OnViewProfiles(wxCommandEvent &);
  void OnViewCursor(wxCommandEvent &);
  void OnViewOutcomes(wxCommandEvent &);
  void OnViewSupports(wxCommandEvent &);
  void OnViewZoomIn(wxCommandEvent &);
  void OnViewZoomOut(wxCommandEvent &);
  void OnViewSupportReachable(wxCommandEvent &);

  void OnFormatDisplayLayout(wxCommandEvent &);
  void OnFormatDisplayLegend(wxCommandEvent &);
  void OnFormatDisplayColors(wxCommandEvent &);
  void OnFormatDisplayDecimals(wxCommandEvent &);
  void OnFormatFontsAboveNode(wxCommandEvent &);
  void OnFormatFontsBelowNode(wxCommandEvent &);
  void OnFormatFontsAfterNode(wxCommandEvent &);
  void OnFormatFontsAboveBranch(wxCommandEvent &);
  void OnFormatFontsBelowBranch(wxCommandEvent &);

  void OnToolsDominance(wxCommandEvent &);
  void OnToolsEquilibrium(wxCommandEvent &);
  void OnToolsQre(wxCommandEvent &);
  void OnToolsNormalReduced(wxCommandEvent &);
  void OnToolsNormalAgent(wxCommandEvent &);

  void OnHelpAbout(wxCommandEvent &);

  void OnSupportDuplicate(wxCommandEvent &);
  void OnSupportDelete(wxCommandEvent &);

  void OnProfilesNew(wxCommandEvent &);
  void OnProfilesDuplicate(wxCommandEvent &);
  void OnProfilesDelete(wxCommandEvent &);
  void OnProfilesProperties(wxCommandEvent &);
  void OnProfilesReport(wxCommandEvent &);
  void OnProfileSelected(wxListEvent &);

  void OnInfoNotebookPage(wxNotebookEvent &);

public:
  // CONSTRUCTOR AND DESTRUCTOR
  EfgShow(efgGame &p_efg, wxWindow *p_parent);
  virtual ~EfgShow();

  // PROFILE ACCESS AND MANIPULATION
  void AddProfile(const BehavSolution &, bool map);
  void RemoveProfile(int);
  void RemoveProfiles(void);
  void ChangeProfile(int);
  int CurrentProfile(void) const { return m_currentProfile; }
  const BehavSolution &GetCurrentProfile(void) const;
  const gList<BehavSolution> &Profiles(void) const { return m_profiles; }
  gText UniqueProfileName(void) const;

  // SUPPORT ACCESS AND MANIPULATION
  EFSupport *GetSupport(void);
  const gList<EFSupport *> &Supports(void) const { return m_supports; }
  void SetSupportNumber(int p_number);
  gText UniqueSupportName(void) const;
  void OnSupportsEdited(void);

  gText GetRealizProb(const gbtEfgNode &) const;
  gText GetBeliefProb(const gbtEfgNode &) const;
  gText GetNodeValue(const gbtEfgNode &) const;
  gText GetInfosetProb(const gbtEfgNode &) const;
  gText GetInfosetValue(const gbtEfgNode &) const;
  gText GetActionValue(const gbtEfgNode &, int act) const;
  gText GetActionProb(const gbtEfgNode &, int act) const;
  gNumber ActionProb(const gbtEfgNode &, int br) const;

  efgGame *Game(void) { return &m_efg; }

  void UpdateMenus(void);
  int NumDecimals(void) const;

  void OnOutcomesEdited(void);
  gText UniqueOutcomeName(void) const;

  void SetFilename(const wxString &s);
  const wxString &Filename(void) const { return m_filename; }

  void SetCursor(gbtEfgNode m_node);
  gbtEfgNode Cursor(void) const { return m_cursor; }
  gbtEfgNode CopyNode(void) const { return m_copyNode; }
  gbtEfgNode CutNode(void) const { return m_cutNode; }

  void OnEditNode(wxCommandEvent &);
  void OnTreeChanged(bool, bool);

  DECLARE_EVENT_TABLE()
};

#endif // EFGSHOW_H

