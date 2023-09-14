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
  return entity.IsSelected(ENF_SELECTED);
}

void NewEntitySelection::Select(const std::set<CEntity*>& entities)
{
  for (auto* entity : entities)
  {
    if (!entity->IsSelected(ENF_SELECTED))
    {
      entity->Select(ENF_SELECTED);
      m_entities.insert(entity);
    } else {
      ASSERTALWAYS("Object already selected!");
    }
  }
  Notify();
}

void NewEntitySelection::Select(CEntity& entity)
{
  if (!entity.IsSelected(ENF_SELECTED))
  {
    entity.Select(ENF_SELECTED);
    m_entities.insert(&entity);
    Notify();
  } else {
    ASSERTALWAYS("Object already selected!");
  }
}

void NewEntitySelection::Deselect(CEntity& entity)
{
  if (entity.IsSelected(ENF_SELECTED))
  {
    entity.Deselect(ENF_SELECTED);
    m_entities.erase(&entity);
    Notify();
  } else {
    ASSERTALWAYS("Object is not selected!");
  }
}

CEntity* NewEntitySelection::GetFirstInSelection() const
{
  if (m_entities.empty())
    return nullptr;
  return *m_entities.begin();
}

void NewEntitySelection::Clear()
{
  for (auto* entity : m_entities)
    entity->Deselect(ENF_SELECTED);
  m_entities.clear();
  Notify();
}

void NewEntitySelection::DestroyEntities(CWorld& world)
{
  // must be in 24bit mode when managing entities
  CSetFPUPrecision FPUPrecision(FPT_24BIT);
  for (auto* entity : m_entities)
  {
    if (entity->IsTargetable())
      world.UntargetEntity(entity);
    entity->Destroy();
  }
  m_entities.clear();
  Notify();
}

void NewEntitySelection::ConvertToCTContainer(CDynamicContainer<CEntity>& output_container) const
{
  output_container.Clear();
  for (auto* entity : m_entities)
    output_container.Add(entity);
}

void NewEntitySelection::ConvertFromCTSelection(CEntitySelection& input_selection)
{
  //do not clear old selection for compatibility
  //m_entities.clear();
  FOREACHINDYNAMICCONTAINER(input_selection, CEntity, iten)
  {
    m_entities.insert(iten);
  }
  input_selection.CDynamicContainer<CEntity>::Clear();
  Notify();
}

void NewEntitySelection::Notify() const
{
  EventHub::instance().CurrentEntitySelectionChanged(m_entities);
}

const std::set<CEntity*>& NewEntitySelection::Set() const
{
  return m_entities;
}

std::set<CEntity*>::iterator NewEntitySelection::begin()
{
  return m_entities.begin();
}

std::set<CEntity*>::iterator NewEntitySelection::end()
{
  return m_entities.end();
}

std::set<CEntity*>::const_iterator NewEntitySelection::cbegin() const
{
  return m_entities.cbegin();
}

std::set<CEntity*>::const_iterator NewEntitySelection::cend() const
{
  return m_entities.cend();
}
