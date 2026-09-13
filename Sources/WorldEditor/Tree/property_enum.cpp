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

using ENUM = INDEX;

class Property_Enum : public BaseEntityPropertyTreeItem
{
public:
  Property_Enum(BasePropertyTreeItem* parent)
    : BaseEntityPropertyTreeItem(parent)
  {
  }

  QWidget* CreateEditor(QWidget* parent) override
  {
    auto* editor = new QComboBox(parent);
    editor->setStyleSheet(g_combo_style);

    CEntity entity(*m_entities.begin(), false);
    CEntityPropertyPtr actual_property = entity.PropertyForName(mp_property->pid_strName);
    CEntityPropertyEnumTypePtr enum_type = actual_property->ep_pepetEnumType;

    for (INDEX i = 0; i < enum_type->epet_ctValues; ++i)
    {
      const auto enum_value = enum_type->epet_aepevValues(i);
      editor->addItem(enum_value.epev_strName, enum_value.epev_iValue);
    }
    editor->setCurrentIndex(editor->findData(_CurrentPropValue()));

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

  IMPL_GENERIC_PROPERTY_FUNCTIONS(ENUM)

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
static UIPropertyFactory::Registrar g_registrar(CEntityProperty::PropertyType::EPT_ENUM,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_Enum(parent);
  });
