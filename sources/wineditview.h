// WinEditView.h : interface of the CWinEditView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINEDITVIEW_H__51F1F61E_9928_11D1_BCE8_006008049675__INCLUDED_)
#define AFX_WINEDITVIEW_H__51F1F61E_9928_11D1_BCE8_006008049675__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CWinEditView : public CEditView
{
private:
	CString m_SelectedText;
  int m_CaretPos;
  
protected: // create from serialization only
	CWinEditView();
	DECLARE_DYNCREATE(CWinEditView)

// Attributes
public:
	CWinEditDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinEditView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	CString m_Buffer;
	void PutChar( char ch );
	char GetChar( void );
	virtual ~CWinEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWinEditView)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEditPaste();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in WinEditView.cpp
inline CWinEditDoc* CWinEditView::GetDocument()
   { return (CWinEditDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINEDITVIEW_H__51F1F61E_9928_11D1_BCE8_006008049675__INCLUDED_)
