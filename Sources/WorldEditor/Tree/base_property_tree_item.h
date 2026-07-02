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

#ifndef BASE_PROPERTY_TREE_ITEM_H
#define BASE_PROPERTY_TREE_ITEM_H

#include <QVariant>

#include <vector>
#include <memory>

class BasePropertyTreeItem : public QObject
{
  Q_OBJECT
public:
    explicit BasePropertyTreeItem(BasePropertyTreeItem* parentItem = nullptr);
    virtual ~BasePropertyTreeItem();

    virtual QVariant      data(int column, int role) const = 0;

    void                  Clear();
    virtual bool          EntityPresentInHierarchy(CEntity_* entity) const;

    void                  appendChild(std::unique_ptr<BasePropertyTreeItem>&& child);
    BasePropertyTreeItem* parentItem();
    BasePropertyTreeItem* child(int row);
    int                   childCount() const;
    int                   row() const;

    Q_SIGNAL void         Changed();

private:
    std::vector<std::unique_ptr<BasePropertyTreeItem>> m_childItems;
    BasePropertyTreeItem* m_parentItem;
};

Q_DECLARE_METATYPE(BasePropertyTreeItem*);

#endif
