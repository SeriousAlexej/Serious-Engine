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

// DlgPgPosition.cpp : implementation file
//

#include "stdafx.h"
#include "DlgPgPosition.h"

#include <QPointer>

#ifdef _DEBUG
#undef new
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgPgPosition property page

IMPLEMENT_DYNCREATE(CDlgPgPosition, CPropertyPage)

CDlgPgPosition::CDlgPgPosition() : CPropertyPage(CDlgPgPosition::IDD)
{
	//{{AFX_DATA_INIT(CDlgPgPosition)
	m_fBanking = 0.0f;
	m_fHeading = 0.0f;
	m_fPitch = 0.0f;
	m_fX = 0.0f;
	m_fY = 0.0f;
	m_fZ = 0.0f;
	//}}AFX_DATA_INIT
}

CDlgPgPosition::~CDlgPgPosition()
{
}

BOOL CDlgPgPosition::OnInitDialog()
{
  BOOL res = CPropertyPage::OnInitDialog();
  HICON pick_icon = static_cast<HICON>(::LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_ICON_PICK), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
  if (pick_icon)
  {
    auto* pick_button = static_cast<CButton*>(GetDlgItem(IDC_PICK_ROTATION));
    pick_button->SetIcon(pick_icon);
  }
  mp_context = std::make_unique<QObject>();
  return res;
}

void CDlgPgPosition::DoDataExchange(CDataExchange* pDX)
{
  if( theApp.m_bDisableDataExchange) return;

  CPropertyPage::DoDataExchange(pDX);

  SetModified( TRUE);

  // obtain document
  CWorldEditorDoc* pDoc = theApp.GetDocument();
  // if document doesn't exist, return
  if( pDoc == NULL)  return;
  // get active view 
  CWorldEditorView *pWorldEditorView = theApp.GetActiveView();

  // if dialog is recieving data
  if( pDX->m_bSaveAndValidate == FALSE)
  {
	  // is CSG on?
    if( pDoc->m_pwoSecondLayer != NULL)
    {
      // yes, pick up coordinates for editting from second layer
      m_fHeading = DegAngle( pDoc->m_plSecondLayer.pl_OrientationAngle(1));
	    m_fPitch   = DegAngle( pDoc->m_plSecondLayer.pl_OrientationAngle(2));
	    m_fBanking = DegAngle( pDoc->m_plSecondLayer.pl_OrientationAngle(3));

      m_fX = pDoc->m_plSecondLayer.pl_PositionVector(1);
      m_fY = pDoc->m_plSecondLayer.pl_PositionVector(2);
      m_fZ = pDoc->m_plSecondLayer.pl_PositionVector(3);
    }
    // otherwise if we are in entity mode and there is only one entity selected
    else if (pDoc->m_iMode == ENTITY_MODE && pDoc->m_selEntitySelection.Count() == 1)
    {
      // get first entity
      CEntity *penEntityOne = pDoc->m_selEntitySelection.GetFirstInSelection();

      // get placement of first entity
      CPlacement3D plEntityOnePlacement = penEntityOne->GetPlacement();
      m_fHeading = DegAngle( plEntityOnePlacement.pl_OrientationAngle(1));
	    m_fPitch   = DegAngle( plEntityOnePlacement.pl_OrientationAngle(2));
	    m_fBanking = DegAngle( plEntityOnePlacement.pl_OrientationAngle(3));

      m_fX = plEntityOnePlacement.pl_PositionVector(1);
      m_fY = plEntityOnePlacement.pl_PositionVector(2);
      m_fZ = plEntityOnePlacement.pl_PositionVector(3);
    }
    m_udSelection.MarkUpdated();
  }

	//{{AFX_DATA_MAP(CDlgPgPosition)
  const BOOL single_selection = pDoc->m_selEntitySelection.Count() == 1;
  GetDlgItem(IDC_EDIT_BANKING)->EnableWindow(single_selection);
  GetDlgItem(IDC_EDIT_HEADING)->EnableWindow(single_selection);
  GetDlgItem(IDC_EDIT_PITCH)->EnableWindow(single_selection);
  GetDlgItem(IDC_EDIT_X)->EnableWindow(single_selection);
  GetDlgItem(IDC_EDIT_Y)->EnableWindow(single_selection);
  GetDlgItem(IDC_EDIT_Z)->EnableWindow(single_selection);

	DDX_Text(pDX, IDC_EDIT_BANKING, m_fBanking);
	DDX_Text(pDX, IDC_EDIT_HEADING, m_fHeading);
	DDX_Text(pDX, IDC_EDIT_PITCH, m_fPitch);
	DDX_Text(pDX, IDC_EDIT_X, m_fX);
	DDX_Text(pDX, IDC_EDIT_Y, m_fY);
  DDX_Text(pDX, IDC_EDIT_Z, m_fZ);
  DDX_Check(pDX, IDC_ABSOLUTE_ROTATION, pDoc->m_absoluteRotation);
  DDX_Check(pDX, IDC_SNAP_TO_GRID, pDoc->m_bAutoSnap);
	//}}AFX_DATA_MAP

  // if dialog is giving data
  if( pDX->m_bSaveAndValidate != FALSE)
  {
	  // is CSG on?
    if( pDoc->m_pwoSecondLayer != NULL)
    {
      // yes, copy coordinates from editting controls into second layer
      pDoc->m_plSecondLayer.pl_OrientationAngle(1) = AngleDeg( m_fHeading);
	    pDoc->m_plSecondLayer.pl_OrientationAngle(2) = AngleDeg( m_fPitch);
	    pDoc->m_plSecondLayer.pl_OrientationAngle(3) = AngleDeg( m_fBanking);
      pDoc->m_plSecondLayer.pl_PositionVector(1) = m_fX;
      pDoc->m_plSecondLayer.pl_PositionVector(2) = m_fY;
      pDoc->m_plSecondLayer.pl_PositionVector(3) = m_fZ;

      // snap values to grid
      pDoc->SnapToGrid( pDoc->m_plSecondLayer, SNAP_FLOAT_12);
      theApp.m_vfpCurrent.vfp_plPrimitive = pDoc->m_plSecondLayer;

      // update all document's views
      pDoc->UpdateAllViews( NULL);
    }
    // otherwise if we are in entity mode
    else if( pDoc->m_iMode == ENTITY_MODE)
    {
      if (pDoc->m_selEntitySelection.Count() == 1)
      {
        // get first entity
        CEntity* penEntityOne = pDoc->m_selEntitySelection.GetFirstInSelection();

        // get placement of first entity
        CPlacement3D plEntityOnePlacement = penEntityOne->GetPlacement();
        plEntityOnePlacement.pl_OrientationAngle(1) = AngleDeg(m_fHeading);
        plEntityOnePlacement.pl_OrientationAngle(2) = AngleDeg(m_fPitch);
        plEntityOnePlacement.pl_OrientationAngle(3) = AngleDeg(m_fBanking);

        plEntityOnePlacement.pl_PositionVector(1) = m_fX;
        plEntityOnePlacement.pl_PositionVector(2) = m_fY;
        plEntityOnePlacement.pl_PositionVector(3) = m_fZ;

        // snap entity's placement
        pDoc->SnapToGrid(plEntityOnePlacement, SNAP_FLOAT_12);

        // set placement back to entity
        penEntityOne->SetPlacement(plEntityOnePlacement);

        pDoc->SetModifiedFlag(TRUE);
        pDoc->UpdateAllViews(NULL);
        m_udSelection.MarkUpdated();

        // update all document's views
        pDoc->UpdateAllViews(NULL);
      }
      else
      {
        m_udSelection.MarkUpdated();
      }
    }
  }
}

BEGIN_MESSAGE_MAP(CDlgPgPosition, CPropertyPage)
  ON_COMMAND(IDC_PICK_ROTATION, OnPickRotation)
  ON_UPDATE_COMMAND_UI(IDC_PICK_ROTATION, OnUpdatePickRotation)
  ON_COMMAND(IDC_ABSOLUTE_ROTATION, OnAbsoluteRotation)
  ON_UPDATE_COMMAND_UI(IDC_ABSOLUTE_ROTATION, OnUpdateAbsoluteRotation)
  ON_COMMAND(IDC_SNAP_TO_GRID, OnSnapToGrid)
  ON_UPDATE_COMMAND_UI(IDC_SNAP_TO_GRID, OnUpdateSnapToGrid)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPgPosition message handlers

BOOL CDlgPgPosition::OnIdle(LONG lCount)
{
  // obtain document
  CWorldEditorDoc* pDoc = theApp.GetDocument();
  if( (pDoc == NULL) || !IsWindow(m_hWnd)) return FALSE;

  UpdateDialogControls(GetDlgItem(IDC_PICK_ROTATION), FALSE);
  UpdateDialogControls(GetDlgItem(IDC_ABSOLUTE_ROTATION), FALSE);
  UpdateDialogControls(GetDlgItem(IDC_SNAP_TO_GRID), FALSE);

  // if selections have been changed (they are not up to date)
  if( !pDoc->m_chSelections.IsUpToDate( m_udSelection))
  {
    // update dialog data
    UpdateData(FALSE);
  }

  return TRUE;
}

BOOL CDlgPgPosition::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN)
  {
    // move coordinates from page to entity and snap them
    UpdateData( TRUE);
    // place snapped coordinates back to dialog
    UpdateData( FALSE);
    // the message is handled
    return TRUE;
  }
	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CDlgPgPosition::OnPickRotation()
{
  auto* curr_doc = theApp.GetDocument();
  if (!mp_context || !curr_doc)
    return;
  theApp.InstallOneTimeSelectionStealer([this, curr_doc, qContext = QPointer { mp_context.get() }]
  (CEntity* entity)
  {
    if (!qContext)
      return;
    CWorldEditorDoc* pDoc = theApp.GetDocument();
    if (pDoc == curr_doc && !pDoc->m_absoluteRotation)
    {
      pDoc->m_plMouseMove.pl_OrientationAngle = entity->GetPlacement().pl_OrientationAngle;
      pDoc->UpdateSelectionCommonPos();
    }
  },
    this);
}

void CDlgPgPosition::OnUpdatePickRotation(CCmdUI* pCmdUI)
{
  CWorldEditorDoc* pDoc = theApp.GetDocument();
  pCmdUI->Enable(!pDoc->m_absoluteRotation && !theApp.GetSelectionStealer());
}

void CDlgPgPosition::OnAbsoluteRotation()
{
  CWorldEditorDoc* pDoc = theApp.GetDocument();
  pDoc->FlipRotationMode();
}

void CDlgPgPosition::OnUpdateAbsoluteRotation(CCmdUI* pCmdUI)
{
  CWorldEditorDoc* pDoc = theApp.GetDocument();
  pCmdUI->SetCheck(pDoc->m_absoluteRotation);
}

void CDlgPgPosition::OnSnapToGrid()
{
  CWorldEditorDoc* pDoc = theApp.GetDocument();
  pDoc->m_bAutoSnap = !pDoc->m_bAutoSnap;
}

void CDlgPgPosition::OnUpdateSnapToGrid(CCmdUI* pCmdUI)
{
  CWorldEditorDoc* pDoc = theApp.GetDocument();
  pCmdUI->SetCheck(pDoc->m_bAutoSnap);
}
