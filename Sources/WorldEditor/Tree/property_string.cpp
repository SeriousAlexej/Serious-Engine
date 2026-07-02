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

#include <QLineEdit>

namespace
{
  const char* g_lineedit_style = R"(
QLineEdit {
  background-color: transparent;border:
  0px;
}
)";
}

class Property_String : public BaseEntityPropertyTreeItem
{
public:
  Property_String(BasePropertyTreeItem* parent)
    : BaseEntityPropertyTreeItem(parent)
  {
  }

  QWidget* CreateEditor(QWidget* parent) override
  {
    auto* editor = new QLineEdit(parent);
    editor->setStyleSheet(g_lineedit_style);
    editor->setText(QString::fromLocal8Bit(static_cast<const char*>(_CurrentPropValue())));

    QObject::connect(editor, &QLineEdit::editingFinished, this, [this, editor]
      {
        CTString new_value = editor->text().toLocal8Bit().data();
        _WriteProperty(new_value);
      });
    return editor;
  }

  IMPL_GENERIC_PROPERTY_FUNCTIONS(CTString)

protected:
  bool _ChangesDocument() const override
  {
    return true;
  }
};

/*******************************************************************************************/
static UIPropertyFactory::Registrar g_registrar(CEntityProperty::PropertyType::EPT_STRING,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_String(parent);
  });
static UIPropertyFactory::Registrar g_registrar_translatable(CEntityProperty::PropertyType::EPT_STRINGTRANS,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_String(parent);
  });
