
from wxPython.wx import *
from wxPython.grid import *
import gambit




class DialogAbout(wxDialog):
    def __init__(self, parent, title, programName, versionString):
        wxDialog.__init__(self, parent, -1, title)

        topSizer = wxBoxSizer(wxVERTICAL)

        # Add bitmap here

        strings = [ programName, versionString,
                    "Part of the Gambit Project",
                    "(http://econweb.tamu.edu/gambit)",
                    "gambit@econmail.tamu.edu",
                    "Built with wxPython",
                    "Copyright (C) 2004",
                    "The Gambit Project",
                    "Theodore Turocy, Maintainer",
                    "This program is free software",
                    "distributed under the terms of",
                    "the GNU General Public License" ]
        for string in strings:
            topSizer.Add(wxStaticText(self, wxID_STATIC, string),
                         0, wxTOP | wxALIGN_CENTER, 5)

        okButton = wxButton(self, wxID_OK, "OK")
        okButton.SetDefault()
        topSizer.Add(okButton, 0, wxALL | wxALIGN_CENTER, 10)

        self.SetSizer(topSizer)
        topSizer.Fit(self)
        topSizer.SetSizeHints(self)
        self.Layout()
        self.CenterOnParent()


#
# Thread for long computations
#

import thread
from wxPython.lib import newevent     # to define the custom event

ComputeDoneEvent, EVT_COMPUTE_DONE = newevent.NewEvent()
ComputeUpdateEvent, EVT_COMPUTE_UPDATE = newevent.NewEvent()

class ComputeThread:
    def __init__(self, name, window):
        self.window = window
        self.name = name
        self.progress = 0

    def IsRunning(self): return self.running
    def Start(self):  thread.start_new_thread(self.Run, ())
    def GetName(self):  return self.name
    def GetProgress(self):  return self.progress
    def GetFound(self):    return 0
    
    def Run(self):
        self.running = True
        wxPostEvent(self.window, ComputeUpdateEvent())
        for j in xrange(0, 100):
            for i in xrange(0, 1000000):  pass
            self.progress = j + 1
            wxPostEvent(self.window, ComputeUpdateEvent())
        self.running = False
        
class LiapThread:
    def __init__(self, window, game):
        self.window = window
        self.progress = 0
        self.game = game
        self.solns = [ ]

    def IsRunning(self):  return self.running
    def Start(self):      thread.start_new_thread(self.Run, ())
    def GetName(self):    return "Liap"
    def GetProgress(self): return self.progress
    def GetFound(self):    return len(self.solns)
    
    def Run(self):
        self.running = True
        self.solns = gambit.LiapEfg(gambit.gbtEfgSupport(self.game))
        self.progress = 100
        wxPostEvent(self.window, ComputeDoneEvent())
 
class LcpThread:
    def __init__(self, window, game):
        self.window = window
        self.progress = 0
        self.game = game
        self.solns = [ ]

    def IsRunning(self):  return self.running
    def Start(self):      thread.start_new_thread(self.Run, ())
    def GetName(self):    return "Lcp"
    def GetProgress(self): return self.progress
    def GetFound(self):    return len(self.solns)
    
    def Run(self):
        self.running = True
        self.solns = gambit.LcpEfg(gambit.gbtEfgSupport(self.game))
        self.progress = 100
        wxPostEvent(self.window, ComputeDoneEvent())
       

import treewindow

class PyComputeView(wxGrid):
    def __init__(self, parent):
        wxGrid.__init__(self, parent, -1)
        self.CreateGrid(0, 3)
        self.SetRowLabelSize(0)
        self.SetColLabelValue(0, "Algorithm")
        self.SetColLabelValue(1, "%")
        self.SetColLabelValue(2, "Found")
        
        self.SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER)
        self.SetDefaultCellFont(wxFont(9, wxSWISS, wxNORMAL, wxBOLD))
        self.SetLabelFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD))
        
        self.AutoSizeRows()
        self.AutoSizeColumns()

    def UpdateThreads(self, threads):
        if self.GetNumberRows() < len(threads):
            self.AppendRows(len(threads) - self.GetNumberRows())
            
        for row in xrange(0, self.GetNumberRows()):
            self.SetCellValue(row, 0, threads[row].GetName())
            self.SetCellValue(row, 1, "%d%%" % threads[row].GetProgress())
            self.SetCellValue(row, 2, "%d" % threads[row].GetFound())
            
        self.AutoSizeRows()
        self.AutoSizeColumns()    
 
 
class GameDocument:
    def __init__(self, efg):
        self.efg = efg
        self.views = [ ]
        
    def AddView(self, view):  self.views.append(view)
    def RemoveView(self, view):  self.views.remove(self.views.index(view))
    def UpdateViews(self):  map(lambda x: x.OnUpdate(), self.views)
    
    def GetGame(self):    return self.efg

class PyGamePanel(wxPanel):
    def __init__(self, parent, doc):
        wxPanel.__init__(self, parent, -1)
        self.doc = doc
        doc.AddView(self)

        topSizer = wxBoxSizer(wxHORIZONTAL)
        self.computeView = PyComputeView(self)
        topSizer.Add(self.computeView,
                     1, wxALL | wxEXPAND, 0)
        topSizer.Add(treewindow.PyTreeWindow(self, doc),
                     3, wxALL | wxEXPAND, 0)
        self.SetSizer(topSizer)
        self.Layout()
        
    def __del__(self):
        self.doc.RemoveView(self)
        wxPanel.__del__(self)
        
    def UpdateThreads(self, threads):
        self.computeView.UpdateThreads(threads)

    def OnUpdate(self):  pass

#
# Menu constants
#
GBT_MENU_FILE_EXPORT = 100
GBT_MENU_FILE_EXPORT_HTML = 103

GBT_MENU_TOOLS_DOMINANCE = 501
GBT_MENU_TOOLS_LCP = 502
GBT_MENU_TOOLS_LIAP = 503
GBT_MENU_TOOLS_QRE = 506

class PyGambitFrame(wxFrame):
    def __init__(self, parent):
        wxFrame.__init__(self, parent, -1, "Gambit")
        self.computeThreads = [ ]
        self.SetSizer(wxBoxSizer(wxVERTICAL))

        EVT_MENU(self, wxID_NEW, self.OnFileNew)
        EVT_MENU(self, wxID_OPEN, self.OnFileOpen)
        EVT_MENU(self, wxID_EXIT, self.OnFileExit)
        
        EVT_MENU(self, GBT_MENU_TOOLS_LCP, self.OnToolsLcp)
        EVT_MENU(self, GBT_MENU_TOOLS_LIAP, self.OnToolsLiap)
        EVT_MENU(self, wxID_ABOUT, self.OnHelpAbout)
        EVT_COMPUTE_DONE(self, self.OnComputeDone)
        EVT_COMPUTE_UPDATE(self, self.OnComputeUpdate)

        self.CreateStatusBar(2)
        self.MakeMenus()

    def OnComputeUpdate(self, event):
        self.panel.UpdateThreads(self.computeThreads)
        
    def OnComputeDone(self, event):
        self.panel.UpdateThreads(self.computeThreads)

    def ShowGame(self, window):
        if hasattr(self, "panel"):
            self.GetSizer().Remove(self.panel)
            del self.panel

        self.panel = window
        self.GetSizer().Add(self.panel, 1, wxALL | wxEXPAND, 0)
        self.GetSizer().Layout()

    def OnFileNew(self, event):
        doc = GameDocument(gambit.NewEfg())
        self.ShowGame(PyGamePanel(self, doc))

    def OnFileOpen(self, event):
        dialog = wxFileDialog(self, "Open game", "", "", "*.efg",
                              wxOPEN)
        if dialog.ShowModal() == wxID_OK:
            efg = gambit.ReadEfg(dialog.GetPath())
            doc = GameDocument(efg)
            self.ShowGame(PyGamePanel(self, doc))
            
    def OnFileExit(self, event):
        wxGetApp().GetTopWindow().Destroy()

    def OnToolsLcp(self, event):
        thread = LcpThread(self, self.panel.GetGame())
        self.computeThreads.append(thread)
        thread.Start()
        self.panel.UpdateThreads(self.computeThreads)
        self.SetStatusText("%d thread(s) running" % len(self.computeThreads))

    def OnToolsLiap(self, event):
        thread = LiapThread(self, self.panel.GetGame())
        self.computeThreads.append(thread)
        thread.Start()
        self.panel.UpdateThreads(self.computeThreads)
        self.SetStatusText("%d thread(s) running" % len(self.computeThreads))
        

    def OnHelpAbout(self, event):
        DialogAbout(self, "About Gambit...",
                    "Gambit Graphical User Interface",
                    "PyGambit Testing Version").ShowModal()

    def MakeMenus(self):
        menuBar = wxMenuBar()

        fileMenu = wxMenu()
        fileMenu.Append(wxID_NEW, "&New", "Create a new game")
        fileMenu.Append(wxID_OPEN, "&Open", "Open a saved game")
        fileMenu.Append(wxID_CLOSE, "&Close", "Close this window")
        fileMenu.AppendSeparator()
        fileMenu.Append(wxID_SAVE, "&Save", "Save this game")
        fileMenu.Append(wxID_SAVEAS, "Save &as",
                        "Save game to a different file")
        fileMenu.AppendSeparator()
        fileExportMenu = wxMenu()
        fileExportMenu.Append(GBT_MENU_FILE_EXPORT_HTML, "&HTML",
                              "Save this game in HTML format")
        fileMenu.AppendMenu(GBT_MENU_FILE_EXPORT, "&Export", fileExportMenu,
                            "Export the game in various formats")
        fileMenu.AppendSeparator()
        fileMenu.Append(wxID_PRINT_SETUP, "Page Se&tup",
                        "Set up preferences for printing")
        fileMenu.Append(wxID_PREVIEW, "Print pre&view",
                        "View a preview of the game printout")
        fileMenu.Append(wxID_PRINT, "&Print", "Print this game")
        fileMenu.AppendSeparator()
        fileMenu.Append(wxID_EXIT, "E&xit", "Exit Gambit")
        menuBar.Append(fileMenu, "&File")

        toolsMenu = wxMenu()
        toolsMenu.Append(GBT_MENU_TOOLS_DOMINANCE, "&Dominance",
                         "Find undominated strategies")
        toolsMenu.Append(GBT_MENU_TOOLS_LCP, "&Lcp",
                         "Compute Nash equilibria (and refinements)")
        toolsMenu.Append(GBT_MENU_TOOLS_LIAP, "&Liap",
                         "Compute Nash equilibria (and refinements)")
        toolsMenu.Append(GBT_MENU_TOOLS_QRE, "&Qre",
                         "Compute quantal response equilibria")
        menuBar.Append(toolsMenu, "&Tools")

        helpMenu = wxMenu()
        helpMenu.Append(wxID_ABOUT, "&About", "About Gambit")
        menuBar.Append(helpMenu, "&Help")

        self.SetMenuBar(menuBar)

class PyGambitApp(wxApp):
    def OnInit(self):
        frame = PyGambitFrame(NULL)
        self.SetTopWindow(frame)
        frame.Show(true)
        return True

app = PyGambitApp()
app.MainLoop()
