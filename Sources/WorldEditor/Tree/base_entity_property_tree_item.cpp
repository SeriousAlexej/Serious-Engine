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
#include "base_entity_property_tree_item.h"
#include "EventHub.h"

#include <stdexcept>

BaseEntityPropertyTreeItem::BaseEntityPropertyTreeItem(BasePropertyTreeItem* parent)
  : BasePropertyTreeItem(parent)
{
  QObject::connect(&EventHub::instance(), &EventHub::PropertyChanged, this,
    [this](const std::set<CEntity_*>& entities, CPropertyID* prop, BasePropertyTreeItem* source)
    {
      if (source == this)
        return;

      if (mp_property && prop && prop->pid_eptType == mp_property->pid_eptType)
      {
        std::vector<CEntity_*> common_entities;
        std::set_intersection(entities.begin(), entities.end(),
          m_entities.begin(), m_entities.end(),
          std::back_inserter(common_entities));
        if (!common_entities.empty())
          Changed();
      }
    });
}

QVariant BaseEntityPropertyTreeItem::data(int column, int role) const
{
  if (role != Qt::DisplayRole || (column != 0 && column != 2))
    return QVariant();

  if (column == 0)
    return QString(mp_property->pid_strName);

  return _GetTypeName();
}

void BaseEntityPropertyTreeItem::OnEntityPicked(CEntity_* picked_entity)
{
  (void)picked_entity;
}

bool BaseEntityPropertyTreeItem::EntityPresentInHierarchy(CEntity_* entity) const
{
  if (m_entities.find(entity) != m_entities.end())
    return true;
  return BasePropertyTreeItem::EntityPresentInHierarchy(entity);
}

bool BaseEntityPropertyTreeItem::_ChangesDocument() const
{
  return false;
}

CPropertyID* BaseEntityPropertyTreeItem::_GetProperty() const
{
  return mp_property.get();
}

void BaseEntityPropertyTreeItem::_SetEntitiesAndProperty(const std::set<CEntity_*>& entities, std::unique_ptr<CPropertyID>&& prop)
{
  if (!m_entities.empty() || mp_property)
    throw std::runtime_error("Entities were already set to this item! Examine callstack to fix this");
  m_entities = entities;
  mp_property = std::move(prop);
}
