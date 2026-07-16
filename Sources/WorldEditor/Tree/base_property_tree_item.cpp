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
#include "base_property_tree_item.h"
#include "base_property_tree_item.h.moc"

BasePropertyTreeItem::BasePropertyTreeItem(BasePropertyTreeItem* parent)
  : QObject(nullptr)
  , m_parentItem(parent)
{}

BasePropertyTreeItem::~BasePropertyTreeItem()
{
  Clear();
}

void BasePropertyTreeItem::Clear()
{
  m_childItems.clear();
}

bool BasePropertyTreeItem::EntityPresentInHierarchy(CEntity_* entity) const
{
  if (m_parentItem)
    return m_parentItem->EntityPresentInHierarchy(entity);
  return false;
}

bool BasePropertyTreeItem::IsVolatile() const
{
  return true;
}

void BasePropertyTreeItem::appendChild(std::unique_ptr<BasePropertyTreeItem>&& item)
{
  m_childItems.push_back(std::move(item));
}

BasePropertyTreeItem* BasePropertyTreeItem::child(int row)
{
  if (row < 0 || row >= static_cast<int>(m_childItems.size()))
    return nullptr;
  return m_childItems.at(row).get();
}

int BasePropertyTreeItem::childCount() const
{
  return m_childItems.size();
}

BasePropertyTreeItem* BasePropertyTreeItem::parentItem()
{
  return m_parentItem;
}

int BasePropertyTreeItem::row() const
{
  if (m_parentItem)
  {
    auto found_pos = std::find_if(
      m_parentItem->m_childItems.begin(),
      m_parentItem->m_childItems.end(),
      [this](const std::unique_ptr<BasePropertyTreeItem>& item) { return item.get() == this; });
    if (found_pos != m_parentItem->m_childItems.end())
      return std::distance(
        m_parentItem->m_childItems.begin(),
        found_pos);

    return -1;
  }

  return 0;
}
