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
  void Select(const std::set<CEntity_*>& entities);
  void Select(CEntity& entity);
  void Deselect(CEntity& entity);
  CEntityPtr GetFirstInSelection() const;
  void Clear();
  void DestroyEntities(CWorld& world);
  void ConvertToCTContainer(CDynamicContainer_CEntity& output_container) const;
  void ConvertFromCTSelection(CEntitySelection& input_selection);
  void Notify() const;
  const std::set<CEntity_*>& Set() const;

  std::set<CEntity_*>::iterator begin();
  std::set<CEntity_*>::iterator end();
  std::set<CEntity_*>::const_iterator cbegin() const;
  std::set<CEntity_*>::const_iterator cend() const;

private:
  std::set<CEntity_*> m_entities;
};

#endif
