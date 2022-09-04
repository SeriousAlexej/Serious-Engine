/* Copyright (c) 2021 SeriousAlexej (Oleksii Sierov).
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
#ifndef PROPERTY_TREE_H
#define PROPERTY_TREE_H

#include <memory>

class PropertyTree_MFC_Host : public CDialogBar
{
public:
  PropertyTree_MFC_Host();
  ~PropertyTree_MFC_Host();

  BOOL Create(CWnd* pParentWnd, UINT nIDTemplate, UINT nStyle, UINT nID);
  CSize CalcDynamicLayout(int nLength, DWORD dwMode) override;
  bool IsUnderMouse() const;
  CPropertyID* GetSelectedProperty() const;
  void SaveState() const;
  void Reset();

public:
  CSize m_Size;

private:
  afx_msg LONG OnInitDialog(UINT wParam, LONG lParam);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  DECLARE_MESSAGE_MAP()

private:
  class _Impl;
  std::unique_ptr<_Impl> mp_impl;
};

#endif
