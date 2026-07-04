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

// EditMipSwitchDistance.cpp : implementation file
//

#include "stdafx.h"
#include "EditMipSwitchDistance.h"

#ifdef _DEBUG
#undef new
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditMipSwitchDistance

CEditMipSwitchDistance::CEditMipSwitchDistance()
{
  m_fLastValue = -1;
}

CEditMipSwitchDistance::~CEditMipSwitchDistance()
{
}


BEGIN_MESSAGE_MAP(CEditMipSwitchDistance, CEdit)
	//{{AFX_MSG_MAP(CEditMipSwitchDistance)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditMipSwitchDistance message handlers

CBrushMipPtr GetMipBrush(void)
{
  CMainFrame* pMainFrame = STATIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
  CWorldEditorView *pWedView = theApp.GetActiveView();

  if( pWedView != NULL)
  {
    return pWedView->GetCurrentBrushMip();
  }

  return {};
}

BOOL IsEditingEnabled(void)
{
  CWorldEditorView *pWedView = theApp.GetActiveView();
  if( pWedView != NULL)
  {
    CChildFrame *pWedChild = pWedView->GetChildFrame();
    if( pWedChild != NULL)
    {
      CBrushMipPtr pbrm = GetMipBrush();
      CWorldEditorDoc *pDoc = pWedView->GetDocument();
      if( (pDoc != NULL) && (pDoc->GetEditingMode() == ENTITY_MODE) && (pbrm))
      {
        return !pWedChild->m_bAutoMipBrushingOn;
      }
    }
  }
  return FALSE;
}

BOOL CEditMipSwitchDistance::PreTranslateMessage(MSG* pMsg) 
{
	// if we caught key down message
  if( pMsg->message==WM_KEYDOWN)
  {
    if( ((int)pMsg->wParam==VK_RETURN) && IsEditingEnabled() )
    {
      // set new mip switch distance
      CString strWindowText;
      GetWindowText( strWindowText);
      CTString strValue = static_cast<const char*>(CStringA(strWindowText));
      FLOAT fValue = 100.0f;
      CBrushMipPtr pbrm = GetMipBrush();
      // if value is valid and brush exists
      if( (strValue.ScanF( "%g", &fValue) == 1) && (pbrm) )
      {
        pbrm->SetMipDistance( fValue);
        m_fLastValue = fValue;
        CWorldEditorView *pWedView = theApp.GetActiveView();
        CWorldEditorDoc *pDoc = pWedView->GetDocument();
        pDoc->SetModifiedFlag( TRUE);
        pDoc->m_chSelections.MarkChanged();
        pDoc->UpdateAllViews( NULL);
      }
    }
    else
    {
      TranslateMessage(pMsg);
      SendMessage( WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
    }

    return TRUE;
  }
	
	return CEdit::PreTranslateMessage(pMsg);
}

BOOL CEditMipSwitchDistance::OnIdle(LONG lCount)
{
  CBrushMipPtr pbrmip = GetMipBrush();
  // if editing is disabled
  if( !IsEditingEnabled() )
  {
    EnableWindow( FALSE);
  }
  // if we should update value
  else
  {
    EnableWindow( TRUE);
    FLOAT fValue = pbrmip->GetMipDistance();
    if( (pbrmip != m_pbrmBrushMipSelected) || (fValue != m_fLastValue) )
    {
      CTString strValue;
      m_fLastValue = fValue;
      strValue.PrintF( "%g", fValue);
      SetWindowText( CString(static_cast<const char*>(strValue)));
    }
  }
  m_pbrmBrushMipSelected = pbrmip;
  return TRUE;
}
