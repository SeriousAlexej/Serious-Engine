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

#ifndef BASE_ENTITY_PROPERTY_TREE_ITEM_H
#define BASE_ENTITY_PROPERTY_TREE_ITEM_H

#include "base_property_tree_item.h"
#include "EventHub.h"

#include <set>

#define IMPL_GENERIC_PROPERTY_FUNCTIONS_IMPL(TPropType, FirstValue)\
bool ValueIsCommonForAllEntities() const override final\
{\
  return _ValueIsCommonForAllEntities<TPropType>();\
}\
void SetFirstValueToAllEntities() override final\
{\
  _WritePropertyT<TPropType>(FirstValue);\
}\
protected:\
QString _GetTypeName() const override final\
{\
  return #TPropType ;\
}\
private:\
const TPropType& _CurrentPropValue() const\
{\
  return _CurrentPropValueT<TPropType>();\
}\
void _WriteProperty(const TPropType& prop_value)\
{\
  _WritePropertyT<TPropType>(prop_value);\
}

#define IMPL_GENERIC_PROPERTY_FUNCTIONS(TPropType)\
IMPL_GENERIC_PROPERTY_FUNCTIONS_IMPL(TPropType, _CurrentPropValueT<TPropType>())

class BaseEntityPropertyTreeItem : public BasePropertyTreeItem
{
public:
  BaseEntityPropertyTreeItem(BasePropertyTreeItem* parent);

  QVariant         data(int column, int role) const override;
  virtual bool     ValueIsCommonForAllEntities() const = 0;
  virtual void     SetFirstValueToAllEntities() = 0;
  virtual QWidget* CreateEditor(QWidget* parent) = 0;
  virtual void     OnEntityPicked(CEntity* picked_entity);
  bool             EntityPresentInHierarchy(CEntity* entity) const override final;

protected:
  virtual QString _GetTypeName() const = 0;
  virtual bool    _ChangesDocument() const;
  virtual CPropertyID* _GetProperty() const;

  template<typename TPropType>
  bool _ValueIsCommonForAllEntities() const
  {
    auto beg_it = m_entities.begin();
    auto cur_it = beg_it;
    CEntityProperty* beg_actual_property = (*beg_it)->PropertyForName(mp_property->pid_strName);
    for (++cur_it; cur_it != m_entities.end(); ++cur_it)
    {
      CEntityProperty* cur_actual_property = (*cur_it)->PropertyForName(mp_property->pid_strName);
      if (ENTITYPROPERTY((*cur_it), cur_actual_property->ep_slOffset, TPropType) !=
          ENTITYPROPERTY((*beg_it), beg_actual_property->ep_slOffset, TPropType))
        return false;
    }
    return true;
  }

  template<typename TPropType>
  const TPropType& _CurrentPropValueT() const
  {
    CEntityProperty* actual_property = (*m_entities.begin())->PropertyForName(mp_property->pid_strName);
    return ENTITYPROPERTY((*m_entities.begin()), actual_property->ep_slOffset, TPropType);
  }

  template<typename TPropType>
  void _WritePropertyT(const TPropType& prop_value)
  {
    for (auto* entity : m_entities)
    {
      entity->End();
      CEntityProperty* actual_property = entity->PropertyForName(mp_property->pid_strName);
      ENTITYPROPERTY(entity, actual_property->ep_slOffset, TPropType) = prop_value;
      entity->Initialize();
    }

    CWorldEditorDoc* pDoc = theApp.GetDocument();
    pDoc->SetModifiedFlag(TRUE);
    pDoc->UpdateAllViews(NULL);

    if (_ChangesDocument())
      pDoc->m_chDocument.MarkChanged();

    auto property_copy = *mp_property;
    auto entities_copy = m_entities;
    Changed();
    if (!signalsBlocked())
      EventHub::instance().PropertyChanged(entities_copy, &property_copy, this);
  }

private:
  friend class PropertyTreeModel;
  void _SetEntitiesAndProperty(const std::set<CEntity*>& entities, std::unique_ptr<CPropertyID>&& prop);

protected:
  std::set<CEntity*>           m_entities;
  std::unique_ptr<CPropertyID> mp_property;
};

#endif
