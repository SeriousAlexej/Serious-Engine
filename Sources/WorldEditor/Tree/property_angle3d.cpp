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
#include "spinbox_no_trailing.h"

#include <QHBoxLayout>
#include <QLabel>

class Property_Angle3D : public BaseEntityPropertyTreeItem
{
public:
  Property_Angle3D(BasePropertyTreeItem* parent)
    : BaseEntityPropertyTreeItem(parent)
  {
  }

  QWidget* CreateEditor(QWidget* parent) override
  {
    auto* editor = new QWidget(parent);
    auto* layout = new QHBoxLayout(editor);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(new QLabel("HPB: ", editor));

    SpinBoxNoTrailing* spinboxes[3] = { _CreateSubSpinbox(editor), _CreateSubSpinbox(editor), _CreateSubSpinbox(editor) };
    for (int i = 0; i < 3; ++i)
    {
      spinboxes[i]->setValue(DegAngle(_CurrentPropValue()(i + 1)));
      layout->addWidget(spinboxes[i]);
      QObject::connect(spinboxes[i], &SpinBoxNoTrailing::editingFinished, this, [this, spin_H = spinboxes[0], spin_P = spinboxes[1], spin_B = spinboxes[2]]
        {
          _WriteProperty(ANGLE3D(spin_H->value(), spin_P->value(), spin_B->value()));
        });
    }

    return editor;
  }

  IMPL_GENERIC_PROPERTY_FUNCTIONS(ANGLE3D)

private:
  SpinBoxNoTrailing* _CreateSubSpinbox(QWidget* parent)
  {
    auto* spinbox = new SpinBoxNoTrailing(parent);
    spinbox->setRange(-FLT_MAX, FLT_MAX);
    spinbox->setDecimals(4);
    spinbox->setSingleStep(0.25);
    spinbox->setSuffix(QStringLiteral("°"));
    spinbox->setSingleStep(1.0);
    return spinbox;
  }
};



/*******************************************************************************************/
static UIPropertyFactory::Registrar g_registrar(CEntityProperty::PropertyType::EPT_ANGLE3D,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_Angle3D(parent);
  });
