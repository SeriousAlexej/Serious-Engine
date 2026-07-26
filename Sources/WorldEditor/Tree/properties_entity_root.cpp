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
#include "properties_entity_root.h"
#include "EventHub.h"

EntityRootProperties::EntityRootProperties(BasePropertyTreeItem* parent, const std::set<CEntity_*>& entities)
  : BasePropertyTreeItem(parent)
  , m_entities(entities)
{
  QObject::connect(&EventHub::instance(), &EventHub::PropertyChanged, this,
    [this](const std::set<CEntity_*>& entities, CPropertyID* prop)
    {
      if (prop->pid_eptType == CEntityProperty::PropertyType::EPT_STRING ||
          prop->pid_eptType == CEntityProperty::PropertyType::EPT_STRINGTRANS)
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

QVariant EntityRootProperties::data(int column, int role) const
{
  if (column < 0 || column >= 3)
    return QVariant();

  QString common_value;
  if (column == 0 || column == 2)
  {
    auto it = m_entities.begin();
    CEntity entity(*it, false);
    common_value = entity.GetClass()->ec_pdecDLLClass->dec_strName;
    for (++it; it != m_entities.end(); ++it)
    {
      CEntity entity(*it, false);
      if (common_value != entity.GetClass()->ec_pdecDLLClass->dec_strName)
      {
        common_value = "(mixed selection)";
        break;
      }
    }
  }
  else if (column == 1)
  {
    auto it = m_entities.begin();
    if (m_entities.size() == 1)
    {
      CEntity entity(*it, false);
      auto entity_name = QString::fromLocal8Bit(static_cast<const char*>(entity.GetName()));
      entity_name.replace('\n', " ");
      common_value = QString("%1 (ID %2)").arg(entity_name).arg(QString::number(entity.en_ulID));
    } else {
      CEntity entity(*it, false);
      common_value = QString::fromLocal8Bit(static_cast<const char*>(entity.GetName()));
      for (++it; it != m_entities.end(); ++it)
      {
        CEntity entity(*it, false);
        if (common_value != QString::fromLocal8Bit(static_cast<const char*>(entity.GetName())))
        {
          common_value = "(mixed names)";
          break;
        }
      }
    }
  }
  return common_value;
}
