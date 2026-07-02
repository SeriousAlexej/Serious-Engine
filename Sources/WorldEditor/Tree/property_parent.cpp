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

class Property_Parent : public BaseEntityPropertyTreeItem
{
public:
  Property_Parent(BasePropertyTreeItem* parent)
    : BaseEntityPropertyTreeItem(parent)
  {
    QObject::connect(&EventHub::instance(), &EventHub::PropertyChanged, this,
      [this](const std::set<CEntity_*>& entities, CPropertyID* prop, BasePropertyTreeItem* source)
      {
        if (source == this)
          return;

        if (prop->pid_eptType == CEntityProperty::EPT_STRING || prop->pid_eptType == CEntityProperty::EPT_STRINGTRANS)
        {
          std::set<CEntity_*> parents;
          for (auto* entity_ : m_entities)
          {
            CEntityPtr entity(entity_);
            parents.insert(entity->GetParent());
          }

          std::vector<CEntity_*> common_entities;
          std::set_intersection(entities.begin(), entities.end(),
            parents.begin(), parents.end(),
            std::back_inserter(common_entities));
          if (!common_entities.empty())
            Changed();
        }
      });
  }

  QWidget* CreateEditor(QWidget* parent) override final
  {
    CEntity entity(*m_entities.begin(), false);
    auto* editor = new PointerWidget(entity.GetParent(), parent);

    QObject::connect(editor, &PointerWidget::clear, this, [this]
      {
        _SetParent(nullptr);
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
        CDlgBrowseByClass select_entity_dialog(nullptr, true, [this](CEntity_* entity) { return m_entities.find(entity) == m_entities.end(); });
        if (select_entity_dialog.DoModal() == IDOK && select_entity_dialog.m_selected_entity)
          _SetParent(select_entity_dialog.m_selected_entity);
      });

    return editor;
  }

  void OnEntityPicked(CEntity_* picked_entity) override final
  {
    _SetParent(picked_entity);
  }

  bool ValueIsCommonForAllEntities() const override final
  {
    CEntity entity(*m_entities.begin(), false);
    const CEntityPtr first_parent = entity.GetParent();
    auto it = m_entities.begin();
    for (++it; it != m_entities.end(); ++it)
    {
      CEntity entity(*it, false);
      if (entity.GetParent() != first_parent)
        return false;
    }
    return true;
  }

  void SetFirstValueToAllEntities() override final
  {
    // use nullptr just in case
    _SetParent(nullptr);
  }

private:
  void _SetParent(CEntity_* new_parent)
  {
    for (auto* entity_ : m_entities)
    {
      CEntityPtr entity(entity_);
      entity->SetParent(new_parent);
    }

    CWorldEditorDoc* pDoc = theApp.GetDocument();
    pDoc->SetModifiedFlag(TRUE);
    pDoc->UpdateAllViews(NULL);

    auto property_copy = *mp_property;
    auto entities_copy = m_entities;
    Changed();
    EventHub::instance().PropertyChanged(entities_copy, &property_copy, this);
  }

  QString _GetTypeName() const override final
  {
    return "CEntityPointer";
  }
};

/*******************************************************************************************/
static UIPropertyFactory::Registrar g_registrar(CEntityProperty::PropertyType::EPT_PARENT,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_Parent(parent);
  });
