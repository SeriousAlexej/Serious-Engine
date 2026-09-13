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

class Property_Ammo : public BaseEntityPropertyTreeItem
{
private:
  enum AmmoType {
    AMMO_BULLETS = 0,
    AMMO_SHELLS = 1,
    AMMO_ROCKETS = 2,
    AMMO_GRENADES = 3,
    AMMO_NAPALM = 4,
    AMMO_ELECTRICITY = 5,
    AMMO_IRONBALLS = 7,
    AMMO_SNIPERBULLETS = 8,
    AMMO_9 = 9,
    AMMO_10 = 10,
    AMMO_11 = 11,
    AMMO_12 = 12,
    AMMO_13 = 13,
    AMMO_14 = 14,
    AMMO_15 = 15,
    AMMO_16 = 16,
    AMMO_17 = 17,
    AMMO_18 = 18,
    AMMO_19 = 19,
    AMMO_20 = 20,
    AMMO_21 = 21,
    AMMO_22 = 22,
    AMMO_23 = 23,
    AMMO_24 = 24,
    AMMO_25 = 25,
    AMMO_26 = 26,
    AMMO_27 = 27,
    AMMO_28 = 28,
    AMMO_29 = 29,
    AMMO_30 = 30,
    AMMO_31 = 31,
  };

public:
  Property_Ammo(BasePropertyTreeItem* parent)
    : BaseEntityPropertyTreeItem(parent)
  {
  }

  QWidget* CreateEditor(QWidget* parent) override final
  {
    m_flags.clear();
    auto* editor = new CheckListWidget(parent);
    _AddFlag(editor, "Bullets (bit 0)", 1 << AMMO_BULLETS);
    _AddFlag(editor, "Shells (bit 1)", 1 << AMMO_SHELLS);
    _AddFlag(editor, "Rockets (bit 2)", 1 << AMMO_ROCKETS);
    _AddFlag(editor, "Grenades (bit 3)", 1 << AMMO_GRENADES);
    _AddFlag(editor, "Napalm (bit 4)", 1 << AMMO_NAPALM);
    _AddFlag(editor, "Electricity (bit 5)", 1 << AMMO_ELECTRICITY);
    _AddFlag(editor, "Cannon Balls (bit 7)", 1 << AMMO_IRONBALLS);
    _AddFlag(editor, "Sniper Bullets (bit 8)", 1 << AMMO_SNIPERBULLETS);
    _AddFlag(editor, "(bit 9)", 1 << AMMO_9);
    _AddFlag(editor, "(bit 10)", 1 << AMMO_10);
    _AddFlag(editor, "(bit 11)", 1 << AMMO_11);
    _AddFlag(editor, "(bit 12)", 1 << AMMO_12);
    _AddFlag(editor, "(bit 13)", 1 << AMMO_13);
    _AddFlag(editor, "(bit 14)", 1 << AMMO_14);
    _AddFlag(editor, "(bit 15)", 1 << AMMO_15);
    _AddFlag(editor, "(bit 16)", 1 << AMMO_16);
    _AddFlag(editor, "(bit 17)", 1 << AMMO_17);
    _AddFlag(editor, "(bit 18)", 1 << AMMO_18);
    _AddFlag(editor, "(bit 19)", 1 << AMMO_19);
    _AddFlag(editor, "(bit 20)", 1 << AMMO_20);
    _AddFlag(editor, "(bit 21)", 1 << AMMO_21);
    _AddFlag(editor, "(bit 22)", 1 << AMMO_22);
    _AddFlag(editor, "(bit 23)", 1 << AMMO_23);
    _AddFlag(editor, "(bit 24)", 1 << AMMO_24);
    _AddFlag(editor, "(bit 25)", 1 << AMMO_25);
    _AddFlag(editor, "(bit 26)", 1 << AMMO_26);
    _AddFlag(editor, "(bit 27)", 1 << AMMO_27);
    _AddFlag(editor, "(bit 28)", 1 << AMMO_28);
    _AddFlag(editor, "(bit 29)", 1 << AMMO_29);
    _AddFlag(editor, "(bit 30)", 1 << AMMO_30);
    _AddFlag(editor, "(bit 31)", 1 << AMMO_31);

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
    return "AMMO";
  }

  void _AddFlag(CheckListWidget* editor, const QString& label, INDEX flag)
  {
    m_flags.push_back(editor->AddItem(label, _GetFlagState(flag), flag));
  }

  Qt::CheckState _GetFlagState(INDEX flag) const
  {
    auto it = m_entities.begin();
    const bool flag_is_set = _GetAmmo(*it) & flag;
    for (++it; it != m_entities.end(); ++it)
    {
      const bool curr_flag = _GetAmmo(*it) & flag;
      if (curr_flag != flag_is_set)
        return Qt::PartiallyChecked;
    }
    if (flag_is_set)
      return Qt::Checked;
    return Qt::Unchecked;
  }

  INDEX _GetAmmo(CEntity_* entity_) const
  {
    CEntityPtr entity(entity_);
    CEntityPropertyPtr actual_property = entity->PropertyForName(mp_property->pid_strName);
    return *ENTITY_PROPERTY(entity, actual_property->ep_slOffset, INDEX);
  }

private:
  std::vector<QStandardItem*> m_flags;
};

/*******************************************************************************************/
static UIPropertyFactory::Registrar g_registrar(CEntityProperty::PropertyType::EPT_INDEX,
  {
  [](BasePropertyTreeItem* parent)
  {
    return new Property_Ammo(parent);
  },
  "Take Ammo"
  });
