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
#include "ui_property_factory.h"
#include "base_entity_property_tree_item.h"
#include "pointer_widget.h"

#include <QPointer>

class Property_Pointer : public BaseEntityPropertyTreeItem
{
public:
  Property_Pointer(BasePropertyTreeItem* parent)
    : BaseEntityPropertyTreeItem(parent)
  {
    QObject::connect(&EventHub::instance(), &EventHub::PropertyChanged, this,
      [this](const std::set<CEntity_*>& entities, CPropertyID* prop, BasePropertyTreeItem* source)
      {
        if (source == this)
          return;

        if (prop->pid_eptType == CEntityProperty::EPT_STRING || prop->pid_eptType == CEntityProperty::EPT_STRINGTRANS)
        {
          std::set<CEntity_*> pointers;
          for (auto* entity_ : m_entities)
          {
            CEntityPtr entity(entity_);
            CEntityPropertyPtr actual_property = entity->PropertyForName(mp_property->pid_strName);
            CEntityPointer property(ENTITY_PROPERTY(entity, actual_property->ep_slOffset, CEntityPointer_), false);
            pointers.insert(property.ep_pen());
          }

          std::vector<CEntity_*> common_entities;
          std::set_intersection(entities.begin(), entities.end(),
            pointers.begin(), pointers.end(),
            std::back_inserter(common_entities));
          if (!common_entities.empty())
            Changed();
        }
      });
  }

  QWidget* CreateEditor(QWidget* parent) override final
  {
    auto* editor = new PointerWidget(_CurrentPropValue().ep_pen(), parent);
    
    QObject::connect(editor, &PointerWidget::clear, this, [this]
      {
        _WriteProperty({});
      });

    QObject::connect(editor, &PointerWidget::pick, this, [this, pthis = this, editor]
      {
        theApp.InstallOneTimeSelectionStealer([qthis = QPointer(pthis)]
          (CEntity_* entity)
          {
            if (qthis)
              qthis->OnEntityPicked(entity);
          },
          editor);
      });

    QObject::connect(editor, &PointerWidget::selectFromList, this, [this]
      {
        CDlgBrowseByClass select_entity_dialog(nullptr, true, [this](CEntity_* entity) { return _CanTarget(entity); });
        if (select_entity_dialog.DoModal() == IDOK && select_entity_dialog.m_selected_entity)
          _WriteProperty(CEntityPointer(select_entity_dialog.m_selected_entity.get_handle()));
      });

    return editor;
  }

  bool _CanTarget(CEntity_* target_entity_) const
  {
    if (target_entity_)
    {
      CEntityPtr target_entity(target_entity_);
      if (target_entity->IsTargetable())
      {
        return std::all_of(m_entities.begin(), m_entities.end(),
          [this, &target_entity](CEntity_* entity_)
          {
            CEntityPtr entity(entity_);
            CEntityPropertyPtr actual_property = entity->PropertyForName(mp_property->pid_strName);
            return entity->IsTargetValid(actual_property->ep_slOffset, target_entity);
          });
      }
    }
    return false;
  }

  void OnEntityPicked(CEntity_* picked_entity) override final
  {
    if (_CanTarget(picked_entity))
      _WriteProperty(CEntityPointer(picked_entity));
  }

  IMPL_GENERIC_PROPERTY_FUNCTIONS_IMPL(CEntityPointer, {})
};

/*******************************************************************************************/
static UIPropertyFactory::Registrar g_registrar(CEntityProperty::PropertyType::EPT_ENTITYPTR,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_Pointer(parent);
  });
