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
#include "property_tree_model.h"
#include "property_tree_model.h.moc"
#include "base_property_tree_item.h"
#include "properties_entity_root.h"
#include "ui_property_factory.h"
#include "clickable_label.h"

#include <QTimer>
#include <QPointer>

#include <algorithm>

namespace
{
  void _JoinProperties(std::list<std::unique_ptr<CPropertyID>>& properties, CEntity_* penEntity_, bool intersect)
  {
    CEntityPtr penEntity(penEntity_);
    // if we should add all of this entity's properties (if this is first entity)
    if (!intersect)
    {
      CDLLEntityClassPtr pdecDLLClass = penEntity->GetClass()->ec_pdecDLLClass;
      for (; pdecDLLClass; pdecDLLClass = pdecDLLClass->dec_pdecBase())
      {
        for (INDEX iProperty = 0; iProperty < pdecDLLClass->dec_ctProperties; iProperty++)
        {
          CEntityPropertyPtr epProperty = pdecDLLClass->dec_aepProperties(iProperty);
          // dont add properties with no name
          if (epProperty->ep_strName != CTString(""))
          {
            CAnimDataPtr pAD;
            if (epProperty->ep_eptType == CEntityProperty::EPT_ANIMATION)
              pAD = penEntity->GetAnimData(epProperty->ep_slOffset);

            properties.push_back(std::make_unique<CPropertyID>(epProperty->ep_strName, epProperty->ep_eptType, epProperty, pAD));
          }
        }
      }
    }
    // in case of intersecting properties we should take one of existing properties in list
    // and see if investigating entity has property with same descriptive name
    // If not, remove it that existing property.
    else
    {
      for (auto itProp = properties.begin(); itProp != properties.end();)
      {
        CTString strCurrentName = (*itProp)->pid_strName;
        CEntityProperty::PropertyType eptCurrentType = (*itProp)->pid_eptType;
        bool same_found = false;
        bool keep_looking = true;

        CDLLEntityClassPtr pdecDLLClass = penEntity->GetClass()->ec_pdecDLLClass;
        for (; keep_looking && pdecDLLClass; pdecDLLClass = pdecDLLClass->dec_pdecBase())
        {
          for (INDEX iProperty = 0; iProperty < pdecDLLClass->dec_ctProperties; iProperty++)
          {
            CEntityPropertyPtr epProperty = pdecDLLClass->dec_aepProperties(iProperty);
            CAnimDataPtr pAD;
            if (epProperty->ep_eptType == CEntityProperty::EPT_ANIMATION)
              pAD = penEntity->GetAnimData(epProperty->ep_slOffset);

            CPropertyID PropertyID(epProperty->ep_strName, epProperty->ep_eptType, epProperty, pAD);

            // is this property same as one we are investigating
            if ((strCurrentName == PropertyID.pid_strName) &&
                (eptCurrentType == PropertyID.pid_eptType))
            {
              // if propperty is enum, enum ptr must also be the same
              if ((*itProp)->pid_eptType == CEntityProperty::EPT_ENUM)
              {
                if ((*itProp)->pid_penpProperty->ep_pepetEnumType == PropertyID.pid_penpProperty->ep_pepetEnumType)
                  same_found = true;
              }
              // if propperty is animation, anim data ptr must be the same
              else if ((*itProp)->pid_eptType == CEntityProperty::EPT_ANIMATION)
              {
                if ((*itProp)->pid_padAnimData == PropertyID.pid_padAnimData)
                  same_found = true;
              }
              else
              {
                same_found = true;
              }
              keep_looking = false;
              break;
            }
          }
        }

        // if property with same name is not found - remove our investigating property from list
        if (!same_found)
          itProp = properties.erase(itProp);
        else
          ++itProp;
      }
    }
  }

  std::list<std::unique_ptr<CPropertyID>> _CollectProperties(const std::set<CEntity_*>& entities)
  {
    std::list<std::unique_ptr<CPropertyID>> properties;
    if (!entities.empty())
    {
      auto beg_it = entities.begin();
      auto cur_it = beg_it;
      _JoinProperties(properties, *beg_it, false);
      for (++cur_it; cur_it != entities.end(); ++cur_it)
        _JoinProperties(properties, *cur_it, true);

      properties.push_back(std::make_unique<CPropertyID>("Spawn flags", CEntityProperty::EPT_SPAWNFLAGS, CEntityPropertyPtr{}, CAnimDataPtr{}));
      properties.push_back(std::make_unique<CPropertyID>("Parent", CEntityProperty::EPT_PARENT, CEntityPropertyPtr{}, CAnimDataPtr{}));
    }
    properties.sort([](const std::unique_ptr<CPropertyID>& lhs, const std::unique_ptr<CPropertyID>& rhs)
      {
        return std::lexicographical_compare(
          static_cast<const char*>(lhs->pid_strName), static_cast<const char*>(lhs->pid_strName) + lhs->pid_strName.Length(),
          static_cast<const char*>(rhs->pid_strName), static_cast<const char*>(rhs->pid_strName) + rhs->pid_strName.Length());
      });
    return properties;
  }

  class _HeaderTreeItem : public BasePropertyTreeItem
  {
  public:
    _HeaderTreeItem()
      : BasePropertyTreeItem(nullptr)
    {
    }

    QVariant data(int column, int role) const override
    {
      if (column < 0 || column > 2)
        return QVariant();

      if (column == 0)
        return "Name";
      if (column == 1)
        return "Value";
      return "Type";
    }
  };

  class _DummyTreeItem : public BasePropertyTreeItem
  {
  public:
    explicit _DummyTreeItem(BasePropertyTreeItem* parent)
      : BasePropertyTreeItem(parent)
    {
    }

    QVariant data(int, int) const override
    {
      return QVariant();
    }
  };
} // anonymous namespace


PropertyTreeModel::PropertyTreeModel(QObject* parent)
  : QAbstractItemModel(parent)
{
  mp_header_item = std::make_unique<_HeaderTreeItem>();
}

int PropertyTreeModel::columnCount(const QModelIndex&) const
{
  return 3;
}

int PropertyTreeModel::rowCount(const QModelIndex& parent) const
{
  BasePropertyTreeItem* parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = mp_header_item.get();
  else
    parentItem = static_cast<BasePropertyTreeItem*>(parent.internalPointer());

  return parentItem->childCount();
}

QWidget* PropertyTreeModel::CreateEditor(const QModelIndex& index, QWidget* parent)
{
  if (!index.isValid() || index.column() != 1)
    return nullptr;

  auto* item = static_cast<BasePropertyTreeItem*>(index.internalPointer());
  auto* entity_item = dynamic_cast<BaseEntityPropertyTreeItem*>(item);
  if (!entity_item)
    return nullptr;

  if (entity_item->ValueIsCommonForAllEntities())
  {
    return entity_item->CreateEditor(parent);
  } else {
    auto* mixed_editor = new ClickableLabel("(mixed)", parent);
    QObject::connect(mixed_editor, &ClickableLabel::clicked, entity_item,
      [entity_item]
      {
        entity_item->SetFirstValueToAllEntities();
      });
    return mixed_editor;
  }
}

CPropertyID* PropertyTreeModel::GetSelectedProperty(const QModelIndexList& model_indices) const
{
  for (QModelIndex index : model_indices)
  {
    auto* item = static_cast<BasePropertyTreeItem*>(index.internalPointer());
    auto* entity_item = dynamic_cast<BaseEntityPropertyTreeItem*>(item);
    if (!entity_item)
      continue;
    return entity_item->_GetProperty();
  }
  return nullptr;
}

void PropertyTreeModel::OnEntityPicked(CEntity_* picked_entity, const QModelIndexList& model_indices)
{
  for (QModelIndex index : model_indices)
  {
    auto* item = static_cast<BasePropertyTreeItem*>(index.internalPointer());
    auto* entity_item = dynamic_cast<BaseEntityPropertyTreeItem*>(item);
    if (!entity_item)
      continue;
    entity_item->OnEntityPicked(picked_entity);
  }
}

void PropertyTreeModel::EnsureSubtreeIsFilled(const QModelIndex& index)
{
  auto* item = static_cast<BasePropertyTreeItem*>(index.internalPointer());
  auto* entity_item = dynamic_cast<BaseEntityPropertyTreeItem*>(item);
  if (!entity_item)
    return;

  if (dynamic_cast<_DummyTreeItem*>(entity_item->child(0)))
    _FillSubTree(entity_item);
}

void PropertyTreeModel::Clear()
{
  beginResetModel();
  mp_header_item->Clear();
  endResetModel();
}

void PropertyTreeModel::_AppendItem(std::unique_ptr<BasePropertyTreeItem>&& item, BasePropertyTreeItem& parent)
{
  auto* item_raw = item.get();
  QObject::connect(item_raw, &BasePropertyTreeItem::Changed,
    [this, item_raw]
    {
      int row = item_raw->row();
      dataChanged(createIndex(row, 0, item_raw), createIndex(row, 2, item_raw));
    });
  parent.appendChild(std::move(item));
}

void PropertyTreeModel::Fill(const std::set<CEntity_*>& curr_selection)
{
  Clear();
  if (curr_selection.empty())
    return;

  QModelIndex root_index;
  _AddEntityProperties(root_index, curr_selection);
}

void PropertyTreeModel::_AddEntityProperties(const QModelIndex& parent, const std::set<CEntity_*>& entities)
{
  BasePropertyTreeItem* parent_item = mp_header_item.get();
  if (parent.isValid())
    parent_item = static_cast<BasePropertyTreeItem*>(parent.internalPointer());

  int starting_row = parent_item->childCount();
  _AppendItem(std::make_unique<EntityRootProperties>(parent_item, entities), *parent_item);
  beginInsertRows(parent, starting_row, starting_row);
  endInsertRows();

  _FillSubProperties(index(starting_row, 0, parent), entities);
}

void PropertyTreeModel::_FillSubProperties(const QModelIndex& parent, const std::set<CEntity_*>& entities)
{
  BasePropertyTreeItem* parent_item = static_cast<BasePropertyTreeItem*>(parent.internalPointer());

  int starting_row = parent_item->childCount();
  int inserted_rows = 0;

  auto common_properties = _CollectProperties(entities);
  std::vector<BaseEntityPropertyTreeItem*> pointer_properties;

  for (auto it = common_properties.begin(); it != common_properties.end(); ++it)
  {
    std::unique_ptr<CPropertyID>& property = *it;
    if (UIPropertyFactory::Instance().HasFactoryFor(property->pid_eptType))
    {
      std::unique_ptr<BaseEntityPropertyTreeItem> new_item(UIPropertyFactory::Instance().GetFactoryFor(property->pid_eptType, static_cast<const char*>(property->pid_strName))(parent_item));
      if (property->pid_eptType == CEntityProperty::EPT_PARENT || property->pid_eptType == CEntityProperty::EPT_ENTITYPTR)
        pointer_properties.push_back(new_item.get());

      new_item->_SetEntitiesAndProperty(entities, std::move(property));
      _AppendItem(std::move(new_item), *parent_item);
      ++inserted_rows;
    }
  }
  if (inserted_rows > 0)
  {
    beginInsertRows(parent, starting_row, starting_row + inserted_rows - 1);
    endInsertRows();
    for (int i = 0; i < parent_item->childCount(); ++i)
    {
      auto* inserted_item = parent_item->child(i);
      if (inserted_item->childCount() <= 0)
        continue;
      beginInsertRows(createIndex(inserted_item->row(), 0, inserted_item), 0, inserted_item->childCount());
      endInsertRows();
    }
  }

  for (auto* pointer_prop : pointer_properties)
  {
    QObject::connect(pointer_prop, &BasePropertyTreeItem::Changed, [this, pointer_prop]
      {
        _FillSubTree(pointer_prop);
      });

    if (pointer_prop->ValueIsCommonForAllEntities() && _GetPointerEntity(pointer_prop))
    {
      pointer_prop->appendChild(std::make_unique<_DummyTreeItem>(pointer_prop));

      QTimer::singleShot(0, this, [this, qpointer_prop = QPointer{ pointer_prop }]
        {
          if (qpointer_prop && dynamic_cast<_DummyTreeItem*>(qpointer_prop->child(0)))
          {
            beginInsertRows(createIndex(qpointer_prop->row(), 0, qpointer_prop), 0, 1);
            endInsertRows();
          }
        });
    }
  }
}

CEntity_* PropertyTreeModel::_GetPointerEntity(BaseEntityPropertyTreeItem* entity_item) const
{
  CEntity_* pointed_entity = nullptr;
  CEntityPtr first_entity(*entity_item->m_entities.begin());
  if (entity_item->mp_property->pid_eptType == CEntityProperty::EPT_PARENT)
  {
    pointed_entity = first_entity->GetParent();
  } else {
    CEntityPropertyPtr actual_property = first_entity->PropertyForName(entity_item->mp_property->pid_strName);
    if (actual_property)
    {
      CEntityPointer pointer(ENTITY_PROPERTY(first_entity, actual_property->ep_slOffset, CEntityPointer_), false);
      pointed_entity = pointer.ep_pen();
    }
  }
  return pointed_entity;
}

void PropertyTreeModel::_FillSubTree(BaseEntityPropertyTreeItem* entity_item)
{
  QModelIndex prop_index = createIndex(entity_item->row(), 0, entity_item);
  if (entity_item->childCount() > 0)
  {
    beginRemoveRows(prop_index, 0, entity_item->childCount() - 1);
    endRemoveRows();
    entity_item->Clear();
  }

  CEntity_* entity_to_fill = _GetPointerEntity(entity_item);
  if (entity_to_fill && !entity_item->EntityPresentInHierarchy(entity_to_fill))
    _FillSubProperties(prop_index, { entity_to_fill });
}

QVariant PropertyTreeModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (role != Qt::DisplayRole && role != Qt::EditRole)
    return QVariant();

  BasePropertyTreeItem* item = static_cast<BasePropertyTreeItem*>(index.internalPointer());
  return item->data(index.column(), role);
}

Qt::ItemFlags PropertyTreeModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;

  return QAbstractItemModel::flags(index);
}

QVariant PropertyTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return mp_header_item->data(section, role);

  return QVariant();
}

QModelIndex PropertyTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  BasePropertyTreeItem *parentItem;

  if (!parent.isValid())
    parentItem = mp_header_item.get();
  else
    parentItem = static_cast<BasePropertyTreeItem*>(parent.internalPointer());

  BasePropertyTreeItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  return QModelIndex();
}

QModelIndex PropertyTreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  BasePropertyTreeItem *childItem = static_cast<BasePropertyTreeItem*>(index.internalPointer());
  BasePropertyTreeItem *parentItem = childItem->parentItem();

  if (!parentItem || parentItem == mp_header_item.get())
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}
