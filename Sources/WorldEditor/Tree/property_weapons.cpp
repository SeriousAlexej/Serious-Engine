/* Copyright (c) 2022 SeriousAlexej (Oleksii Sierov).
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
#include "checklist_widget.h"

class Property_Weapons : public BaseEntityPropertyTreeItem
{
private:
  enum WeaponType {
    WEAPON_DOUBLECOLT = 2,
    WEAPON_SINGLESHOTGUN = 3,
    WEAPON_DOUBLESHOTGUN = 4,
    WEAPON_TOMMYGUN = 5,
    WEAPON_MINIGUN = 6,
    WEAPON_ROCKETLAUNCHER = 7,
    WEAPON_GRENADELAUNCHER = 8,
    WEAPON_CHAINSAW = 9,
    WEAPON_FLAMER = 10,
    WEAPON_LASER = 11,
    WEAPON_SNIPER = 12,
    WEAPON_IRONCANNON = 13
  };

public:
  Property_Weapons(BasePropertyTreeItem* parent)
    : BaseEntityPropertyTreeItem(parent)
  {
  }

  QWidget* CreateEditor(QWidget* parent) override final
  {
    m_flags.clear();
    auto* editor = new CheckListWidget(parent);
    _AddFlag(editor, "Chainsaw", 1 << WEAPON_CHAINSAW);
    _AddFlag(editor, "Double Colt", 1 << WEAPON_DOUBLECOLT);
    _AddFlag(editor, "Single Shotgun", 1 << WEAPON_SINGLESHOTGUN);
    _AddFlag(editor, "Double Shotgun", 1 << WEAPON_DOUBLESHOTGUN);
    _AddFlag(editor, "Tommygun", 1 << WEAPON_TOMMYGUN);
    _AddFlag(editor, "Minigun", 1 << WEAPON_MINIGUN);
    _AddFlag(editor, "Rocket Launcher", 1 << WEAPON_ROCKETLAUNCHER);
    _AddFlag(editor, "Grenade Launcher", 1 << WEAPON_GRENADELAUNCHER);
    _AddFlag(editor, "Flamethrower", 1 << WEAPON_FLAMER);
    _AddFlag(editor, "Laser", 1 << WEAPON_LASER);
    _AddFlag(editor, "Sniper Rifle", 1 << WEAPON_SNIPER);
    _AddFlag(editor, "Cannon", 1 << WEAPON_IRONCANNON);

    QObject::connect(editor, &CheckListWidget::Changed, this, [this]
      {
        INDEX bits_to_set = 0;

        for (auto* flag : m_flags)
        {
          if (flag->checkState() == Qt::Checked)
            bits_to_set |= flag->data().toInt();
        }

        _WritePropertyT<INDEX>(bits_to_set);
      });

    return editor;
  }

  bool ValueIsCommonForAllEntities() const override final
  {
    return true;
  }

  void SetFirstValueToAllEntities() override final
  {
  }

private:
  QString _GetTypeName() const override final
  {
    return "WEAPONS";
  }

  void _AddFlag(CheckListWidget* editor, const QString& label, INDEX flag)
  {
    m_flags.push_back(editor->AddItem(label, _GetFlagState(flag), flag));
  }

  Qt::CheckState _GetFlagState(INDEX flag) const
  {
    auto it = m_entities.begin();
    const bool flag_is_set = _GetWeapons(*it) & flag;
    for (++it; it != m_entities.end(); ++it)
    {
      const bool curr_flag = _GetWeapons(*it) & flag;
      if (curr_flag != flag_is_set)
        return Qt::PartiallyChecked;
    }
    if (flag_is_set)
      return Qt::Checked;
    return Qt::Unchecked;
  }

  INDEX _GetWeapons(CEntity_* entity_) const
  {
    CEntityPtr entity(entity_);
    CEntityPropertyPtr actual_property = entity->PropertyForName(mp_property->pid_strName);
    return *ENTITY_PROPERTY(entity, actual_property->ep_slOffset, INDEX);
  }

private:
  std::vector<QStandardItem*> m_flags;
};

/*******************************************************************************************/
static UIPropertyFactory::Registrar g_registrar_1(CEntityProperty::PropertyType::EPT_INDEX,
  {
  [](BasePropertyTreeItem* parent)
  {
    return new Property_Weapons(parent);
  },
  "Take Weapons"
  });

static UIPropertyFactory::Registrar g_registrar_2(CEntityProperty::PropertyType::EPT_INDEX,
  {
  [](BasePropertyTreeItem* parent)
  {
    return new Property_Weapons(parent);
  },
  "Give Weapons"
  });
