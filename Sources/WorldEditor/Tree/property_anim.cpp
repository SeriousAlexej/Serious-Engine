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

#include <QComboBox>

namespace
{
  const char* g_combo_style = R"(
QComboBox {
  background-color: transparent;
  border: 0px;
}
)";
}

using ANIMATION = INDEX;

class Property_Anim : public BaseEntityPropertyTreeItem
{
public:
  Property_Anim(BasePropertyTreeItem* parent)
    : BaseEntityPropertyTreeItem(parent)
  {
    QObject::connect(&EventHub::instance(), &EventHub::PropertyChanged, this,
      [this](const std::set<CEntity_*>& entities, CPropertyID* prop, BasePropertyTreeItem* source)
      {
        if (source == this)
          return;

        if (prop->pid_eptType == CEntityProperty::EPT_ENTITYPTR ||
            prop->pid_eptType == CEntityProperty::EPT_FILENAME ||
            prop->pid_eptType == CEntityProperty::EPT_FILENAMENODEP)
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

  QWidget* CreateEditor(QWidget* parent) override
  {
    auto* editor = new QComboBox(parent);
    editor->setStyleSheet(g_combo_style);

    CEntity entity(*m_entities.begin(), false);
    CEntityPropertyPtr actual_property = entity.PropertyForName(mp_property->pid_strName);
    CAnimDataPtr anim_data = entity.GetAnimData(actual_property->ep_slOffset);

    if (anim_data)
    {
      for (INDEX i = 0; i < anim_data->GetAnimsCt(); ++i)
      {
        CAnimInfo anim_info;
        anim_data->GetAnimInfo(i, anim_info);
        editor->addItem(anim_info.ai_AnimName, i);
      }
      editor->setCurrentIndex(editor->findData(_CurrentPropValue()));
    }

    editor->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    editor->installEventFilter(this);
    editor->setMinimumWidth(30);

    QObject::connect(editor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, editor]
      (int index)
      {
        if (index != -1)
          _WriteProperty(editor->itemData(index).toInt());
      });

    return editor;
  }

  IMPL_GENERIC_PROPERTY_FUNCTIONS(ANIMATION)

private:
  bool eventFilter(QObject* object, QEvent* event) override
  {
    if (event->type() == QEvent::Wheel)
    {
      event->ignore();
      return true;
    }
    return BaseEntityPropertyTreeItem::eventFilter(object, event);
  }
};

/*******************************************************************************************/
static UIPropertyFactory::Registrar g_registrar(CEntityProperty::PropertyType::EPT_ANIMATION,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_Anim(parent);
  });
