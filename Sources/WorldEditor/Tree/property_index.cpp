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

#include <QSpinBox>

namespace
{
  const char* g_spin_style = R"(
QSpinBox {
  background-color: transparent;
  border: 0px;
}
)";
}

class Property_Index : public BaseEntityPropertyTreeItem
{
public:
  Property_Index(BasePropertyTreeItem* parent)
    : BaseEntityPropertyTreeItem(parent)
  {
  }

  QWidget* CreateEditor(QWidget* parent) override
  {
    auto* editor = new QSpinBox(parent);
    editor->setStyleSheet(g_spin_style);
    editor->setRange(INT_MIN, INT_MAX);
    editor->setValue(_CurrentPropValue());
    editor->setFocusPolicy(Qt::StrongFocus);
    editor->installEventFilter(this);

    QObject::connect(editor, &QDoubleSpinBox::editingFinished, this, [this, editor]
      {
        _WriteProperty(editor->value());
      });

    return editor;
  }

  IMPL_GENERIC_PROPERTY_FUNCTIONS(INDEX)

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
static UIPropertyFactory::Registrar g_registrar(CEntityProperty::PropertyType::EPT_INDEX,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_Index(parent);
  });
