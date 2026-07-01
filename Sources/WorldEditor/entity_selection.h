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
#ifndef NEW_ENTITY_SELECTION_H
#define NEW_ENTITY_SELECTION_H

#include <set>

class NewEntitySelection
{
public:
  NewEntitySelection();
  ~NewEntitySelection();

  size_t Count() const;
  BOOL IsSelected(const CEntity& entity) const;
  void Select(const std::set<CEntityPtr>& entities);
  void Select(CEntity& entity);
  void Deselect(CEntity& entity);
  CEntityPtr GetFirstInSelection() const;
  void Clear();
  void DestroyEntities(CWorld& world);
  void ConvertToCTContainer(CDynamicContainer_CEntity& output_container) const;
  void ConvertFromCTSelection(CEntitySelection& input_selection);
  void Notify() const;
  const std::set<CEntityPtr>& Set() const;

  std::set<CEntityPtr>::iterator begin();
  std::set<CEntityPtr>::iterator end();
  std::set<CEntityPtr>::const_iterator cbegin() const;
  std::set<CEntityPtr>::const_iterator cend() const;

private:
  std::set<CEntityPtr> m_entities;
};

#endif
