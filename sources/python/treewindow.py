#
# FILE: treewindow.py -- Display an extensive form as a tree
#

from wxPython.wx import *

class PyTreeLayout:
    def __init__(self, doc):
        self.doc = doc
        self.colors = [ wxLIGHT_GREY, wxRED, wxBLUE, wxGREEN ]
        self.nodes = { }

    def GetMaxX(self):  
        if hasattr(self, "maxX"): return self.maxX
        else:                     return 1
        
    def GetMinX(self):
        if hasattr(self, "minX"): return self.minX
        else:                     return 0
        
    def GetMaxY(self):
        if hasattr(self, "maxY"): return self.maxY
        else:                     return 1
        
    def GetMinY(self):
        if hasattr(self, "minY"): return self.minY
        else:                     return 0
            
    def LayoutSubtree(self, node, level):
        y1 = -1
        self.nodes[node] = { }
        
        if node.NumChildren() > 0:
            for i in xrange(1, node.NumChildren() + 1):
                yn = self.LayoutSubtree(node.GetChild(i), level + 1)
                if y1 == -1:  y1 = yn
            self.nodes[node]["x"] = 100 * level + 50
            self.nodes[node]["y"] = (y1 + yn) / 2
        else:
            self.nodes[node]["x"] = 100 * level + 50
            self.nodes[node]["y"] = self.ycoord
            self.ycoord += 100
        
        if not hasattr(self, "minX"):
            self.minX = self.nodes[node]["x"]
            self.maxX = self.nodes[node]["x"]
            self.minY = self.nodes[node]["y"]
            self.maxY = self.nodes[node]["y"]
        else:
            self.minX = min(self.minX, self.nodes[node]["x"])
            self.maxX = max(self.maxX, self.nodes[node]["x"])
            self.minY = min(self.minY, self.nodes[node]["y"])
            self.maxY = max(self.maxY, self.nodes[node]["y"])

        return self.nodes[node]["y"]
        
    def Layout(self):
        self.nodes = { }
        self.ycoord = 50
        if hasattr(self, "minX"):
            del self.minX
            del self.maxX
            del self.minY
            del self.maxY
        self.LayoutSubtree(self.doc.GetGame().GetRoot(), 0)

    def DrawSubtree(self, dc, node):
        x = self.nodes[node]["x"]
        y = self.nodes[node]["y"]
        radius = 10
        if node.NumChildren() > 0:
            dc.SetPen(wxPen(self.colors[node.GetPlayer().GetId()],
                            1, wxSOLID))
        else:
            dc.SetPen(wxPen(wxBLACK, 1, wxSOLID))
        dc.DrawCircle(x, y, radius)
            
        if node.NumChildren() > 0:
            for i in xrange(1, node.NumChildren() + 1):
                dc.SetPen(wxPen(self.colors[node.GetPlayer().GetId()],
                                1, wxSOLID))
                dc.DrawLine(x + radius, y,
                            self.nodes[node.GetChild(i)]["x"] - radius,
                            self.nodes[node.GetChild(i)]["y"])
                self.DrawSubtree(dc, node.GetChild(i))
        outcome = node.GetOutcome()
        textX = x + 25
        if outcome.IsNull():
            dc.SetFont(wxFont(8, wxSWISS, wxITALIC, wxNORMAL))
            dc.SetTextForeground(wxColour(180, 180, 180))
            string = "Null"
            width, height = dc.GetTextExtent(string)
            textY = y - height / 2
            dc.DrawText(string, textX, textY)
        else:
            dc.SetFont(wxFont(9, wxSWISS, wxNORMAL, wxBOLD))
            game = self.doc.GetGame()
            for j in xrange(1, game.NumPlayers() + 1):
                string = "%.2f" % outcome.GetPayoff(game.GetPlayer(j))
                width, height = dc.GetTextExtent(string)
                textY = y - height / 2
                dc.SetTextForeground(self.colors[j])
                dc.DrawText(string, textX, textY)
                self.nodes[node][j] = wxRect(textX, textY, width, height)
                textX += width + 10
       
                            
    def Draw(self, dc):
        self.Layout()
        dc.SetPen(wxBLACK_PEN)
        self.DrawSubtree(dc, self.doc.GetGame().GetRoot())
        
    def NodeHitTest(self, x, y):
        for node in self.nodes.keys():
            if (abs(self.nodes[node]["x"] - x) <= 5 and
                abs(self.nodes[node]["y"] - y) <= 5):
                return node
        return None
    
    def OutcomeHitTest(self, x, y):
        for node in self.nodes.keys():
            try:
                for j in xrange(1, self.doc.GetGame().NumPlayers() + 1):
                    if self.nodes[node][j].Inside(wxPoint(x, y)):
                        return (node, j)
            except KeyError: pass
        return (None, 0)
    
    def GetOutcomeRect(self, node, j):    return self.nodes[node][j]
            


class PyTreeWindow(wxScrolledWindow):
    def __init__(self, parent, doc):
        wxScrolledWindow.__init__(self, parent, -1)
        self.doc = doc
        doc.AddView(self)
        self.layout = PyTreeLayout(doc)
        self.SetBackgroundColour(wxColour(250, 250, 250))
                
        EVT_PAINT(self, self.OnPaint)
        EVT_LEFT_DOWN(self, self.OnLeftDown)
        EVT_LEFT_DCLICK(self, self.OnLeftDoubleClick)
        
        EVT_CHAR(self, self.OnChar)
        # TODO: Figure out kill focus logic (how to call FindFocus)
        #EVT_KILL_FOCUS(self, self.OnKillFocus)
        #EVT_MOTION(self, self.OnMouseMotion)

    def __del__(self):
        self.doc.RemoveView(self)
        wxScrolledWindow.__del__(self)

    def SetScrollbarSteps(self):
        width, height = self.GetClientSizeTuple()
        scrollX, scrollY = self.GetViewStart()
        
        self.SetScrollbars(50, 50,
                           self.layout.GetMaxX() / 50 + 1,
                           self.layout.GetMaxY() / 50 + 1,
                           scrollX, scrollY)
        
    def Draw(self, dc):
        dc.BeginDrawing()
        dc.Clear()
        self.layout.Draw(dc)
        dc.EndDrawing()
        self.SetScrollbarSteps()

    def OnLeftDown(self, event):
        if hasattr(self, "textctrl"):
            self.textctrl.Destroy()
            del self.textctrl
        
    def OnLeftDoubleClick(self, event):
        if hasattr(self, "textctrl"):  
            self.textctrl.Destroy()
            del self.textctrl
            
        node, player = self.layout.OutcomeHitTest(event.GetX(), event.GetY())
        if node is not None:
            rect = self.layout.GetOutcomeRect(node, player)
            self.textctrl = wxTextCtrl(self, -1, "",
                                       wxPoint(rect.GetX(), rect.GetY()),
                                       wxSize(rect.GetWidth(), rect.GetHeight()))
            self.textctrl.SetFocus()
        
    def OnChar(self, event):
        if (event.GetKeyCode() == WXK_ESCAPE and
            hasattr(self, "textctrl")):
            self.textctrl.Destroy()
            del self.textctrl
        else:
            event.Skip()
    
    def OnKillFocus(self, event):
        if hasattr(self, "textctrl") and FindFocus() != self.textctrl:  
            self.textctrl.Destroy()
            del self.textctrl
            
    def OnPaint(self, event):  
        dc = wxPaintDC(self)
        self.PrepareDC(dc)
        self.Draw(dc)
        
    def OnUpdate(self):
        dc = wxClientDC(self)
        self.PrepareDC(dc)
        self.Draw(dc)
    
