/* Copyright (c) 2002-2012 Croteam Ltd. 
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

// ScriptView.cpp : implementation file
//

#include "stdafx.h"
#include <vector>

#ifdef _DEBUG
#undef new
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScriptView

IMPLEMENT_DYNCREATE(CScriptView, CEditView)

CScriptView::CScriptView()
{
}

CScriptView::~CScriptView()
{
}


BEGIN_MESSAGE_MAP(CScriptView, CEditView)
	//{{AFX_MSG_MAP(CScriptView)
	ON_COMMAND(ID_SCRIPT_MAKE_MODEL, OnScriptMakeModel)
	ON_COMMAND(ID_SCRIPT_UPDATE_ANIMATIONS, OnScriptUpdateAnimations)
	ON_COMMAND(ID_SCRIPT_UPDATE_MIPMODELS, OnScriptUpdateMipmodels)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
void CScriptView::Serialize(CArchive& ar)
{
  ASSERT_VALID(this);
  if (ar.IsStoring())
  {
    WriteToFile(ar);
  }
  else
  {
    CFile* pFile = ar.GetFile();
    ASSERT(pFile->GetPosition() == 0);
    if (pFile->GetLength() > GetEditCtrl().GetLimitText())
    {
      // Larger than edit control limit. Call SetLimitText() to set your own max size.
      // Refer to documentation for EM_LIMITTEXT for max sizes for your target OS.
      AfxMessageBox(AFX_IDP_FILE_TOO_LARGE);
      AfxThrowUserException();
    }
    // ReadFromArchive takes the number of characters as argument
    ReadFromFile(ar);
  }
  ASSERT_VALID(this);
}

/////////////////////////////////////////////////////////////////////////////
void CScriptView::WriteToFile(CArchive& ar)
{
  ASSERT_VALID(this);
  LPCTSTR lpszText = LockBuffer();
  ASSERT(lpszText != NULL);
  UINT nLen = GetBufferLength();
  TRY
  {
    CW2A textA(lpszText);
    ar.Write(textA.m_psz, nLen);
  }
    CATCH_ALL(e)
  {
    UnlockBuffer();
    THROW_LAST();
  }
  END_CATCH_ALL
    UnlockBuffer();
  ASSERT_VALID(this);
}

/////////////////////////////////////////////////////////////////////////////
void CScriptView::ReadFromFile(CArchive& ar)
{
  ASSERT_VALID(this);

  UINT nLen = ar.GetFile()->GetLength();
  std::vector<char> content(nLen + 1, '\0');
  ar.Read(content.data(), nLen);

  for (size_t i = 0; i < content.size(); ++i)
    if (content[i] == '\n' && (i == 0 || content[i - 1] != '\r'))
    {
      content.insert(content.begin() + i++, '\r');
      nLen++;
    }

  LPVOID hText = LocalAlloc(LMEM_MOVEABLE, static_cast<UINT>(::ATL::AtlMultiplyThrow(static_cast<UINT>(nLen + 1), static_cast<UINT>(sizeof(TCHAR)))));
  if (hText == NULL)
    AfxThrowMemoryException();

  LPTSTR lpszText = (LPTSTR)LocalLock(hText);
  ASSERT(lpszText != NULL);
  CA2W contentW(content.data());
  if (lstrcpynW(lpszText, contentW.m_psz, nLen + 1) == NULL)
  {
    LocalUnlock(hText);
    LocalFree(hText);
    AfxThrowArchiveException(CArchiveException::endOfFile);
  }

#ifndef _UNICODE
  if (_AfxGetComCtlVersion() >= VERSION_6)
  {
    // set the text with SetWindowText, then free
    BOOL bResult = ::SetWindowText(m_hWnd, lpszText);
    LocalUnlock(hText);
    LocalFree(hText);

    // make sure that SetWindowText was successful
    if (!bResult || ::GetWindowTextLength(m_hWnd) < (int)nLen)
      AfxThrowMemoryException();

    // remove old shadow buffer
    delete[] m_pShadowBuffer;
    m_pShadowBuffer = NULL;
    m_nShadowSize = 0;

    ASSERT_VALID(this);
    return;
  }
#endif

  LocalUnlock(hText);
  HLOCAL hOldText = GetEditCtrl().GetHandle();
  ASSERT(hOldText != NULL);
  LocalFree(hOldText);
  GetEditCtrl().SetHandle((HLOCAL)hText);
  Invalidate();
  ASSERT_VALID(this);
}

/////////////////////////////////////////////////////////////////////////////
// CScriptView drawing

void CScriptView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CScriptView diagnostics

#ifdef _DEBUG
void CScriptView::AssertValid() const
{
	CEditView::AssertValid();
}

void CScriptView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CScriptView message handlers

void CScriptView::OnScriptMakeModel() 
{
	// First we save script file
  CModelerDoc *pDoc = (CModelerDoc *) GetDocument();
  CTFileName fnScriptName = CTString(CStringA(pDoc->GetPathName()));

  CTFileName fnModelName = fnScriptName.FileDir() + fnScriptName.FileName() + ".mdl";
  try
  {
    fnScriptName.RemoveApplicationPath_t();
  }
  catch( char *err_str)
  {
    AfxMessageBox( CString(err_str));
    return;
  }
  pDoc->OnSaveDocument( pDoc->GetPathName());

	// close mdl document with same name
  POSITION pos = theApp.m_pdtModelDocTemplate->GetFirstDocPosition();
  while (pos!=NULL)
  {
    CModelerDoc *pmdCurrent = (CModelerDoc *)theApp.m_pdtModelDocTemplate->GetNextDoc(pos);
    if( CTFileName( CTString(CStringA(pmdCurrent->GetPathName()))) == fnModelName)
    {
      pmdCurrent->OnCloseDocument();
      break;
    }
  }

	// Now we will create one instance of new document of type CModelerDoc
  CDocument* pDocument = theApp.m_pdtModelDocTemplate->CreateNewDocument();
 	if (pDocument == NULL)
	{
		TRACE0("CDocTemplate::CreateNewDocument returned NULL.\n");
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		return;
	}
	ASSERT_VALID(pDocument);
	
  BOOL bAutoDelete = pDocument->m_bAutoDelete;
	pDocument->m_bAutoDelete = FALSE;   // don't destroy if something goes wrong
	CFrameWnd* pFrame = theApp.m_pdtModelDocTemplate->CreateNewFrame(pDocument, NULL);
	pDocument->m_bAutoDelete = bAutoDelete;
	if (pFrame == NULL)
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		delete pDocument;       // explicit delete on error
		return;
	}
	ASSERT_VALID(pFrame);

  pDocument->SetPathName( CString(fnModelName), FALSE);
  pDocument->SetTitle( CString(fnModelName.FileName() + fnModelName.FileExt()));
  
  char strError[ 256];
  if( !((CModelerDoc *)pDocument)->CreateModelFromScriptFile( fnScriptName, strError))
  {
    pDocument->OnCloseDocument();
    AfxMessageBox( CString(strError));
    return;
  }
	theApp.m_pdtModelDocTemplate->InitialUpdateFrame(pFrame, pDocument, TRUE);
  ((CModelerDoc *)pDocument)->m_emEditModel.edm_md.md_bPreparedForRendering = FALSE;
  pDocument->SetModifiedFlag();

  // add textures from .ini file
  CTFileName fnIniFileName = fnScriptName.NoExt() + ".ini";
  try
  {
    ((CModelerDoc *)pDocument)->m_emEditModel.CSerial::Load_t(fnIniFileName);
  }
  catch( char *strError)
  {
    // ignore errors
    (void) strError;
  }
}


void CScriptView::OnScriptUpdateAnimations() 
{
	// find document with same name
  CModelerDoc *pDoc = (CModelerDoc *) GetDocument();
  CTFileName fnScriptName = CTString(CStringA(pDoc->GetPathName()));
  CTFileName fnModelName = fnScriptName.FileDir() + fnScriptName.FileName() + ".mdl";

	POSITION pos = theApp.m_pdtModelDocTemplate->GetFirstDocPosition();
  while (pos!=NULL)
  {
    CModelerDoc *pmdCurrent = (CModelerDoc *)theApp.m_pdtModelDocTemplate->GetNextDoc(pos);
    if( CTFileName( CTString(CStringA(pmdCurrent->GetPathName()))) == fnModelName)
    {
      POSITION pos = pmdCurrent->GetFirstViewPosition();
      CView *pView = pmdCurrent->GetNextView( pos);
      if( DYNAMIC_DOWNCAST(CModelerView, pView) != NULL)
      {
        CModelerView* pModelerView = (CModelerView *) pView;
        if(pModelerView != NULL)
        {
          // if updating was successful
          if( pModelerView->UpdateAnimations())
          {
            pModelerView->SetActiveWindow();
            pModelerView->SetFocus();
            CMainFrame* pMainFrame = STATIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
            pMainFrame->MDIActivate(pModelerView->GetParentFrame());
          }
        }
      }
      break;
    }
  }
}

void CScriptView::OnScriptUpdateMipmodels() 
{
	// find document with same name
  CModelerDoc *pDoc = (CModelerDoc *) GetDocument();
  CTFileName fnScriptName = CTString(CStringA(pDoc->GetPathName()));
  CTFileName fnModelName = fnScriptName.FileDir() + fnScriptName.FileName() + ".mdl";

	POSITION pos = theApp.m_pdtModelDocTemplate->GetFirstDocPosition();
  while (pos!=NULL)
  {
    CModelerDoc *pmdCurrent = (CModelerDoc *)theApp.m_pdtModelDocTemplate->GetNextDoc(pos);
    if( CTFileName( CTString(CStringA(pmdCurrent->GetPathName()))) == fnModelName)
    {
      POSITION pos = pmdCurrent->GetFirstViewPosition();
      CView *pView = pmdCurrent->GetNextView( pos);
      if( DYNAMIC_DOWNCAST(CModelerView, pView) != NULL)
      {
        CModelerView* pModelerView = (CModelerView *) pView;
        if(pModelerView != NULL)
        {
          pModelerView->OnScriptUpdateMipmodels();
          pModelerView->SetActiveWindow();
          pModelerView->SetFocus();
          CMainFrame* pMainFrame = STATIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
          pMainFrame->MDIActivate(pModelerView->GetParentFrame());
        }
      }
      break;
    }
  }
}
