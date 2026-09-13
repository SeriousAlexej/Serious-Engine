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
    WEAPON_KNIFE = 0,
    WEAPON_COLT = 1,
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
    WEAPON_IRONCANNON = 13,
    WEAPON_14 = 14,
    WEAPON_15 = 15,
    WEAPON_16 = 16,
    WEAPON_17 = 17,
    WEAPON_18 = 18,
    WEAPON_19 = 19,
    WEAPON_20 = 20,
    WEAPON_21 = 21,
    WEAPON_22 = 22,
    WEAPON_23 = 23,
    WEAPON_24 = 24,
    WEAPON_25 = 25,
    WEAPON_26 = 26,
    WEAPON_27 = 27,
    WEAPON_28 = 28,
    WEAPON_29 = 29,
    WEAPON_30 = 30,
    WEAPON_31 = 31,
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
    _AddFlag(editor, "Chainsaw (bit 9)", 1 << WEAPON_CHAINSAW);
    _AddFlag(editor, "Double Colt (bit 2)", 1 << WEAPON_DOUBLECOLT);
    _AddFlag(editor, "Single Shotgun (bit 3)", 1 << WEAPON_SINGLESHOTGUN);
    _AddFlag(editor, "Double Shotgun (bit 4)", 1 << WEAPON_DOUBLESHOTGUN);
    _AddFlag(editor, "Tommygun (bit 5)", 1 << WEAPON_TOMMYGUN);
    _AddFlag(editor, "Minigun (bit 6)", 1 << WEAPON_MINIGUN);
    _AddFlag(editor, "Rocket Launcher (bit 7)", 1 << WEAPON_ROCKETLAUNCHER);
    _AddFlag(editor, "Grenade Launcher (bit 8)", 1 << WEAPON_GRENADELAUNCHER);
    _AddFlag(editor, "Flamethrower (bit 10)", 1 << WEAPON_FLAMER);
    _AddFlag(editor, "Laser (bit 11)", 1 << WEAPON_LASER);
    _AddFlag(editor, "Sniper Rifle (bit 12)", 1 << WEAPON_SNIPER);
    _AddFlag(editor, "Cannon (bit 13)", 1 << WEAPON_IRONCANNON);
    _AddFlag(editor, "(bit 0)", 1 << WEAPON_KNIFE);
    _AddFlag(editor, "(bit 1)", 1 << WEAPON_COLT);
    _AddFlag(editor, "(bit 14)", 1 << WEAPON_14);
    _AddFlag(editor, "(bit 15)", 1 << WEAPON_15);
    _AddFlag(editor, "(bit 16)", 1 << WEAPON_16);
    _AddFlag(editor, "(bit 17)", 1 << WEAPON_17);
    _AddFlag(editor, "(bit 18)", 1 << WEAPON_18);
    _AddFlag(editor, "(bit 19)", 1 << WEAPON_19);
    _AddFlag(editor, "(bit 20)", 1 << WEAPON_20);
    _AddFlag(editor, "(bit 21)", 1 << WEAPON_21);
    _AddFlag(editor, "(bit 22)", 1 << WEAPON_22);
    _AddFlag(editor, "(bit 23)", 1 << WEAPON_23);
    _AddFlag(editor, "(bit 24)", 1 << WEAPON_24);
    _AddFlag(editor, "(bit 25)", 1 << WEAPON_25);
    _AddFlag(editor, "(bit 26)", 1 << WEAPON_26);
    _AddFlag(editor, "(bit 27)", 1 << WEAPON_27);
    _AddFlag(editor, "(bit 28)", 1 << WEAPON_28);
    _AddFlag(editor, "(bit 29)", 1 << WEAPON_29);
    _AddFlag(editor, "(bit 30)", 1 << WEAPON_30);
    _AddFlag(editor, "(bit 31)", 1 << WEAPON_31);

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
