//
// FILE: gambit.h
//
// $Id$
//

//=====================================================================
//                   CLASS GAMBITFRAME DECLARATION
//=====================================================================
class GambitApp: public wxApp
{
	public:
		wxFrame *OnInit(void);
};

class GambitCanvas;

class GambitFrame : public wxFrame
	{
	public:
  	GambitCanvas *canvas;
		GambitFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int type);
    Bool OnClose(void);
    void OnMenuCommand(int id);
		void Draw(wxDC& dc);
		void file_load(void);
};


