// WinEditView.cpp : implementation of the CWinEditView class
//

#include "stdafx.h"
#include "WinEdit.h"

#include "WinEditDoc.h"
#include "WinEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinEditView

IMPLEMENT_DYNCREATE(CWinEditView, CEditView)

BEGIN_MESSAGE_MAP(CWinEditView, CEditView)
	//{{AFX_MSG_MAP(CWinEditView)
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinEditView construction/destruction

CWinEditView::CWinEditView()
  : m_CaretPos( 0 )
{
	// TODO: add construction code here

}

CWinEditView::~CWinEditView()
{
}

BOOL CWinEditView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	BOOL bPreCreated = CEditView::PreCreateWindow(cs);
	cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);	// Enable word-wrapping

	return bPreCreated;
}

/////////////////////////////////////////////////////////////////////////////
// CWinEditView drawing

void CWinEditView::OnDraw(CDC* pDC)
{
	CWinEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CWinEditView printing

BOOL CWinEditView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default CEditView preparation
	return CEditView::OnPreparePrinting(pInfo);
}

void CWinEditView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CEditView begin printing.
	CEditView::OnBeginPrinting(pDC, pInfo);
}

void CWinEditView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CEditView end printing
	CEditView::OnEndPrinting(pDC, pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CWinEditView diagnostics

#ifdef _DEBUG
void CWinEditView::AssertValid() const
{
	CEditView::AssertValid();
}

void CWinEditView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CWinEditDoc* CWinEditView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWinEditDoc)));
	return (CWinEditDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWinEditView message handlers

void CWinEditView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	// CEditView::OnKeyDown(nChar, nRepCnt, nFlags);

  CEdit& edit = GetEditCtrl();

  switch( nChar )
  {
  case VK_LEFT:
    OnChar( 27, 1, 0 );
    OnChar( '[', 1, 0 );
    OnChar( 68, 1, 0 );
    break;
  case VK_RIGHT:
    OnChar( 27, 1, 0 );
    OnChar( '[', 1, 0 );
    OnChar( 67, 1, 0 );
    break;
  case VK_UP:
    OnChar( 27, 1, 0 );
    OnChar( '[', 1, 0 );
    OnChar( 65, 1, 0 );
    break;
  case VK_DOWN:
    OnChar( 27, 1, 0 );
    OnChar( '[', 1, 0 );
    OnChar( 66, 1, 0 );
    break;
  case VK_DELETE:
    OnChar( 27, 1, 0 );
    OnChar( '[', 1, 0 );
    OnChar( 51, 1, 0 );
    OnChar( 126, 1, 0 );
    break;

  }
}

void CWinEditView::OnInitialUpdate()
{
	CEditView::OnInitialUpdate();

}

char CWinEditView::GetChar( void )
{
  char ch = 0;
  while( m_Buffer.IsEmpty() )
  {
    ((CWinEditApp*) AfxGetApp())->ProcessMessages();
  }

  ASSERT( !m_Buffer.IsEmpty() );

  do
  {
    ch = m_Buffer[0];
    m_Buffer = m_Buffer.Right( m_Buffer.GetLength() - 1 );
  }
  while( ch == '\n' );

  return ch;
}

void CWinEditView::PutChar(char ch)
{
  // CEditView::OnChar(ch, 1, 0);

  CEdit& edit = GetEditCtrl();
  char s[3] = { '\0', '\0', '\0' };
  s[0] = ch;
  if( s[0] == '\n' )
  {
    s[0] = '\r';
    s[1] = '\n';
    m_CaretPos = GetWindowTextLength();
    edit.SetSel( m_CaretPos, m_CaretPos + strlen( s ) );
    edit.ReplaceSel( s );
    m_CaretPos += strlen( s );
  }
  else if( s[0] == '\r' )
  {
    int lineindex = edit.LineIndex();
    edit.SetSel( lineindex, -1 );
    edit.Clear();
    m_CaretPos = GetWindowTextLength();
  }
  else if( s[0] == '\a' )
  {
    // beep here
  }
  else if( s[0] == '\b' )
  {
    m_CaretPos -= strlen( s );
    edit.SetSel( m_CaretPos, m_CaretPos );
  }
  else
  {
    edit.SetSel( m_CaretPos, m_CaretPos + strlen( s ) );
    edit.ReplaceSel( s );
    m_CaretPos += strlen( s );
  }
}

void CWinEditView::PutString(const char* str)
{
  // CEditView::OnChar(ch, 1, 0);

  CEdit& edit = GetEditCtrl();
  if( !strstr(str, "\r") &&
      !strstr(str, "\n") &&
      !strstr(str, "\a") &&
      !strstr(str, "\b") )
	{
    edit.SetSel( m_CaretPos, m_CaretPos + strlen( str ) );
    edit.ReplaceSel( str );
    m_CaretPos += strlen( str );
  }
  else
  {
    int length = strlen( str );
    int i = 0;
    for( i = 0; i < length; ++i )
      PutChar( str[i] );
  }
}


void CWinEditView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
  int i = 0;
  for( i = 0; i < nRepCnt; ++i )
    m_Buffer += nChar;
  // CEditView::OnChar(nChar, nRepCnt, nFlags);
}


void CWinEditView::OnEditPaste()
{
	// TODO: Add your command handler code here

  if( !OpenClipboard() )
    return;

  HANDLE hData = GetClipboardData( CF_TEXT ); 
  if( hData )
  {
    char* s = (char*) GlobalLock( hData );
    if( s )
      m_Buffer += s;
    GlobalUnlock( hData );
  }
  ::CloseClipboard();
}

void CWinEditView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	int nStart = 0;
  int nEnd = 0;
  GetEditCtrl().GetSel( nStart, nEnd );
  if( nStart != nEnd )
    GetSelectedText( m_SelectedText );
  m_Buffer += m_SelectedText;
}


