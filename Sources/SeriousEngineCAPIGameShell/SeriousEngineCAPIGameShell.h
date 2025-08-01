// SeriousEngineCAPIGameShell.h : main header file for the SERIOUSENGINECAPIGAMESHELL application
//

#if !defined(AFX_SERIOUSENGINECAPIGAMESHELL_H__7B95FCF0_0F75_4B9C_8396_CDE4E7012987__INCLUDED_)
#define AFX_SERIOUSENGINECAPIGAMESHELL_H__7B95FCF0_0F75_4B9C_8396_CDE4E7012987__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSeriousEngineCAPIGameShellApp:
// See SeriousEngineCAPIGameShell.cpp for the implementation of this class
//

class CSeriousEngineCAPIGameShellApp : public CWinApp
{
public:
	CSeriousEngineCAPIGameShellApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSeriousEngineCAPIGameShellApp)
	public:
	virtual BOOL InitInstance();
  void SubInitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSeriousEngineCAPIGameShellApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERIOUSENGINECAPIGAMESHELL_H__7B95FCF0_0F75_4B9C_8396_CDE4E7012987__INCLUDED_)
