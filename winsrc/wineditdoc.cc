// WinEditDoc.cpp : implementation of the CWinEditDoc class
//

#include "stdafx.h"
#include "WinEdit.h"

#include "WinEditDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinEditDoc

IMPLEMENT_DYNCREATE(CWinEditDoc, CDocument)

BEGIN_MESSAGE_MAP(CWinEditDoc, CDocument)
	//{{AFX_MSG_MAP(CWinEditDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinEditDoc construction/destruction

CWinEditDoc::CWinEditDoc()
{
	// TODO: add one-time construction code here

}

CWinEditDoc::~CWinEditDoc()
{
}

BOOL CWinEditDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	((CEditView*)m_viewList.GetHead())->SetWindowText(NULL);

  CFont* font = new CFont;
  font->CreateFont( -10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Courier" );

  ((CEditView*)m_viewList.GetHead())->GetEditCtrl().SetFont( font );

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CWinEditDoc serialization

void CWinEditDoc::Serialize(CArchive& ar)
{
	// CEditView contains an edit control which handles all serialization
	((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CWinEditDoc diagnostics

#ifdef _DEBUG
void CWinEditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWinEditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWinEditDoc commands
