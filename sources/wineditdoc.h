// WinEditDoc.h : interface of the CWinEditDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINEDITDOC_H__51F1F61C_9928_11D1_BCE8_006008049675__INCLUDED_)
#define AFX_WINEDITDOC_H__51F1F61C_9928_11D1_BCE8_006008049675__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CWinEditDoc : public CDocument
{
protected: // create from serialization only
	CWinEditDoc();
	DECLARE_DYNCREATE(CWinEditDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinEditDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWinEditDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWinEditDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINEDITDOC_H__51F1F61C_9928_11D1_BCE8_006008049675__INCLUDED_)
