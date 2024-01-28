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
      [this](const std::set<CEntity*>& entities, CPropertyID* prop, BasePropertyTreeItem* source)
      {
        if (source == this)
          return;

        if (prop->pid_eptType == CEntityProperty::EPT_STRING || prop->pid_eptType == CEntityProperty::EPT_STRINGTRANS)
        {
          std::set<CEntity*> pointers;
          for (auto* entity : m_entities)
          {
            CEntityProperty* actual_property = entity->PropertyForName(mp_property->pid_strName);
            pointers.insert(ENTITYPROPERTY(entity, actual_property->ep_slOffset, CEntityPointer).ep_pen);
          }

          std::vector<CEntity*> common_entities;
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
    auto* editor = new PointerWidget(_CurrentPropValue().ep_pen, parent);
    
    QObject::connect(editor, &PointerWidget::clear, this, [this]
      {
        _WriteProperty(nullptr);
      });

    QObject::connect(editor, &PointerWidget::pick, this, [this, pthis = this, editor]
      {
        theApp.InstallOneTimeSelectionStealer([qthis = QPointer(pthis)]
          (CEntity* entity)
          {
            if (qthis)
              qthis->OnEntityPicked(entity);
          },
          editor);
      });

    QObject::connect(editor, &PointerWidget::selectFromList, this, [this]
      {
        CDlgBrowseByClass select_entity_dialog(nullptr, true, [this](CEntity* entity) { return _CanTarget(entity); });
        if (select_entity_dialog.DoModal() == IDOK && select_entity_dialog.m_selected_entity)
          _WriteProperty(select_entity_dialog.m_selected_entity);
      });

    return editor;
  }

  bool _CanTarget(CEntity* target_entity) const
  {
    if (target_entity && target_entity->IsTargetable())
    {
      return std::all_of(m_entities.begin(), m_entities.end(),
        [this, target_entity](CEntity* p_entity)
        {
          auto* actual_property = p_entity->PropertyForName(mp_property->pid_strName);
          return p_entity->IsTargetValid(actual_property->ep_slOffset, target_entity);
        });
    }
    return false;
  }

  void OnEntityPicked(CEntity* picked_entity) override final
  {
    if (_CanTarget(picked_entity))
      _WriteProperty(picked_entity);
  }

  IMPL_GENERIC_PROPERTY_FUNCTIONS_IMPL(CEntityPointer, nullptr)
};

/*******************************************************************************************/
static UIPropertyFactory::Registrar g_registrar(CEntityProperty::PropertyType::EPT_ENTITYPTR,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_Pointer(parent);
  });
