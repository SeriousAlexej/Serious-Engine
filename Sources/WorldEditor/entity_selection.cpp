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

#include "StdAfx.h"
#include "entity_selection.h"
#include "EventHub.h"

NewEntitySelection::NewEntitySelection()
{
}

NewEntitySelection::~NewEntitySelection()
{
}

size_t NewEntitySelection::Count() const
{
  return m_entities.size();
}

BOOL NewEntitySelection::IsSelected(const CEntity& entity) const
{
  return entity.IsSelected();
}

void NewEntitySelection::Select(const std::set<CEntity_*>& entities)
{
  for (auto* entity_ : entities)
  {
    CEntityPtr entity = entity_;
    if (!entity->IsSelected())
    {
      entity->Select();
      m_entities.insert(entity);
    } else {
      ASSERTALWAYS("Object already selected!");
    }
  }
  Notify();
}

void NewEntitySelection::Select(CEntity& entity)
{
  if (!entity.IsSelected())
  {
    entity.Select();
    m_entities.insert(entity);
    Notify();
  } else {
    ASSERTALWAYS("Object already selected!");
  }
}

void NewEntitySelection::Deselect(CEntity& entity)
{
  if (entity.IsSelected())
  {
    entity.Deselect();
    m_entities.erase(entity);
    Notify();
  } else {
    ASSERTALWAYS("Object is not selected!");
  }
}

CEntityPtr NewEntitySelection::GetFirstInSelection() const
{
  if (m_entities.empty())
    return {};
  return *m_entities.begin();
}

void NewEntitySelection::Clear()
{
  for (auto* entity_ : m_entities)
  {
    CEntityPtr entity = entity_;
    entity->Deselect();
  }
  m_entities.clear();
  Notify();
}

void NewEntitySelection::DestroyEntities(CWorld& world)
{
  // must be in 24bit mode when managing entities
  CSetFPUPrecision FPUPrecision(FPT_24BIT);
  for (auto* entity_ : m_entities)
  {
    CEntityPtr entity = entity_;
    if (entity->IsTargetable())
      world.UntargetEntity(entity);
    entity->Destroy();
  }
  m_entities.clear();
  Notify();
}

void NewEntitySelection::ConvertToCTContainer(CDynamicContainer_CEntity& output_container) const
{
  output_container.Clear();
  for (auto* entity_ : m_entities)
    output_container.Add(entity_);
}

void NewEntitySelection::ConvertFromCTSelection(CEntitySelection& input_selection)
{
  //do not clear old selection for compatibility
  //m_entities.clear();
  FOREACHINDYNAMICCONTAINER(input_selection, CEntity, iten)
  {
    m_entities.insert(iten.Current());
  }
  input_selection.CDynamicContainer_CEntity::Clear();
  Notify();
}

void NewEntitySelection::Notify() const
{
  EventHub::instance().CurrentEntitySelectionChanged(m_entities);
}

const std::set<CEntity_*>& NewEntitySelection::Set() const
{
  return m_entities;
}

std::set<CEntity_*>::iterator NewEntitySelection::begin()
{
  return m_entities.begin();
}

std::set<CEntity_*>::iterator NewEntitySelection::end()
{
  return m_entities.end();
}

std::set<CEntity_*>::const_iterator NewEntitySelection::cbegin() const
{
  return m_entities.cbegin();
}

std::set<CEntity_*>::const_iterator NewEntitySelection::cend() const
{
  return m_entities.cend();
}
