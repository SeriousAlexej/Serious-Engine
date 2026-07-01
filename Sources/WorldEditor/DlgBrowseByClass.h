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

#if !defined(AFX_DLGBROWSEBYCLASS_H__A1C15CF4_D8A2_11D1_8270_000000000000__INCLUDED_)
#define AFX_DLGBROWSEBYCLASS_H__A1C15CF4_D8A2_11D1_8270_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgBrowseByClass.h : header file
//

#include <functional>

#include <QString>

/////////////////////////////////////////////////////////////////////////////
// CDlgBrowseByClass dialog

class CDlgBrowseByClass final : public CDialog 
{
// Construction
public:
	CDlgBrowseByClass(CWnd* pParent = NULL, bool for_picking = false, std::function<bool(CEntity*)>&& filter = nullptr);   // standard constructor
	~CDlgBrowseByClass();
  BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = nullptr) override;
  void AddEntity( CEntityPtr pen);
  void FillListWithEntities(void);
  void InitializePluggins(void);
  void InitializeListColumns(void);
  CTString GetItemValue(CEntityPtr pen, INDEX iColumn, BOOL &bNumber);
  CDynamicContainer_CEntity* GetCurrentContainer(void);

  BOOL m_bCenterSelected;
	const bool m_for_picking;
	std::function<bool(CEntity*)> m_filter;
	CEntityPtr m_selected_entity;

// Dialog Data
	//{{AFX_DATA(CDlgBrowseByClass)
	enum { IDD = IDD_BROWSE_BY_CLASS };
	CComboBox	m_ctrlPluggins;
	CListCtrl	m_listEntities;
	CString	m_strEntitiesInVolume;
	BOOL	m_bShowVolume;
	BOOL	m_bShowImportants;
  CString m_filter_string;
  QString m_filter_string_qstring;
  CCtrlEditString m_filter_edit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgBrowseByClass)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
private:

	// Generated message map functions
	//{{AFX_MSG(CDlgBrowseByClass)
	void OnSize(UINT nType, int w, int h);
	void AdjustSize();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkEntityList(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnColumnclickEntityList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickEntityList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickEntityList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRemove();
	afx_msg void OnLeave();
	afx_msg void OnSelectAll();
	afx_msg void OnFeedVolume();
	afx_msg void OnRevert();
	afx_msg void OnSelectSectors();
	afx_msg void OnDeleteBrowseByClass();
	afx_msg void OnDisplayVolume();
	afx_msg void OnSelendokPluggins();
	afx_msg void OnDisplayImportants();
	afx_msg void OnEnChangeEditFilterentity();
	afx_msg void OnTimer(UINT eventID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  bool _EntityMatchesStringFilter(const CEntity* entity) const;

private:
  UINT_PTR m_filter_timer = 0;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGBROWSEBYCLASS_H__A1C15CF4_D8A2_11D1_8270_000000000000__INCLUDED_)
