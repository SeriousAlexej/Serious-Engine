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
#include "checklist_widget.h"

class Property_SpawnFlags : public BaseEntityPropertyTreeItem
{
public:
  Property_SpawnFlags(BasePropertyTreeItem* parent)
    : BaseEntityPropertyTreeItem(parent)
  {
  }

  QWidget* CreateEditor(QWidget* parent) override final
  {
    m_flags.clear();
    auto* editor = new CheckListWidget(parent);
    _AddFlag(editor, "Easy", SPF_EASY);
    _AddFlag(editor, "Normal", SPF_NORMAL);
    _AddFlag(editor, "Hard", SPF_HARD);
    _AddFlag(editor, "Extreme", SPF_EXTREME);
    _AddFlag(editor, "Singleplayer", SPF_SINGLEPLAYER);
    _AddFlag(editor, "Cooperative", SPF_COOPERATIVE);
    _AddFlag(editor, "Deathmatch", SPF_DEATHMATCH);
    _AddFlag(editor, "Difficulty 1", SPF_EXTREME << 1);
    _AddFlag(editor, "Difficulty 2", SPF_EXTREME << 2);
    _AddFlag(editor, "Difficulty 3", SPF_EXTREME << 3);
    _AddFlag(editor, "Difficulty 4", SPF_EXTREME << 4);
    _AddFlag(editor, "Difficulty 5", SPF_EXTREME << 5);
    _AddFlag(editor, "Game mode 1", SPF_COOPERATIVE << 1);
    _AddFlag(editor, "Game mode 2", SPF_COOPERATIVE << 2);
    _AddFlag(editor, "Game mode 3", SPF_COOPERATIVE << 3);
    _AddFlag(editor, "Game mode 4", SPF_COOPERATIVE << 4);
    _AddFlag(editor, "Game mode 5", SPF_COOPERATIVE << 5);
    _AddFlag(editor, "Game mode 6", SPF_COOPERATIVE << 6);

    QObject::connect(editor, &CheckListWidget::Changed, this, [this]
      {
        ULONG bits_to_clear = MAX_ULONG;
        ULONG bits_to_set = 0;

        for (auto* flag : m_flags)
        {
          if (flag->checkState() == Qt::Unchecked)
            bits_to_clear &= ~static_cast<ULONG>(flag->data().toUInt());
          else if (flag->checkState() == Qt::Checked)
            bits_to_set |= static_cast<ULONG>(flag->data().toUInt());
        }

        for (auto* entity_ : m_entities)
        {
          CEntityPtr entity(entity_);
          entity->SetSpawnFlags(entity->GetSpawnFlags() & bits_to_clear);
          entity->SetSpawnFlags(entity->GetSpawnFlags() | bits_to_set);
        }

        CWorldEditorDoc* pDoc = theApp.GetDocument();
        pDoc->SetModifiedFlag(TRUE);
        pDoc->UpdateAllViews(NULL);
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
    return "SPAWNFLAGS";
  }

  void _AddFlag(CheckListWidget* editor, const QString& label, ULONG flag)
  {
    static_assert(sizeof(ULONG) == sizeof(unsigned int));
    m_flags.push_back(editor->AddItem(label, _GetFlagState(flag), static_cast<unsigned int>(flag)));
  }

  Qt::CheckState _GetFlagState(ULONG flag) const
  {
    auto it = m_entities.begin();
    CEntity entity(*it, false);
    const bool flag_is_set = entity.GetSpawnFlags() & flag;
    for (++it; it != m_entities.end(); ++it)
    {
      CEntity entity(*it, false);
      const bool curr_flag = entity.GetSpawnFlags() & flag;
      if (curr_flag != flag_is_set)
        return Qt::PartiallyChecked;
    }
    if (flag_is_set)
      return Qt::Checked;
    return Qt::Unchecked;
  }

private:
  std::vector<QStandardItem*> m_flags;
};

/*******************************************************************************************/
static UIPropertyFactory::Registrar g_registrar(CEntityProperty::PropertyType::EPT_SPAWNFLAGS,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_SpawnFlags(parent);
  });
