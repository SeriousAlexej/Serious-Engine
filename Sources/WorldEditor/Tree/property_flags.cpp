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

class Property_Flags : public BaseEntityPropertyTreeItem
{
public:
  Property_Flags(BasePropertyTreeItem* parent)
    : BaseEntityPropertyTreeItem(parent)
  {
  }

  QWidget* CreateEditor(QWidget* parent) override final
  {
    m_flags.clear();
    CEntity entity(*m_entities.begin(), false);
    CEntityPropertyPtr actual_property = entity.PropertyForName(mp_property->pid_strName);
    CEntityPropertyEnumTypePtr enum_type = actual_property->ep_pepetEnumType;

    auto* editor = new CheckListWidget(parent);
    for (int i = 0; i < enum_type->epet_ctValues; ++i)
    {
      const auto enum_value = enum_type->epet_aepevValues(i);
      if (enum_value.epev_strName != "")
      {
        ULONG bit_value = (1UL) << enum_value.epev_iValue;
        _AddFlag(editor, QString::fromLocal8Bit(enum_value.epev_strName), bit_value);
      }
    }

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
          entity->End();
          CEntityPropertyPtr actual_property = entity->PropertyForName(mp_property->pid_strName);
          auto& flags = *ENTITY_PROPERTY(entity, actual_property->ep_slOffset, ULONG);
          flags &= bits_to_clear;
          flags |= bits_to_set;
          entity->Initialize();
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
    return "FLAGS";
  }

  void _AddFlag(CheckListWidget* editor, const QString& label, ULONG flag)
  {
    static_assert(sizeof(ULONG) == sizeof(unsigned int));
    m_flags.push_back(editor->AddItem(label, _GetFlagState(flag), static_cast<unsigned int>(flag)));
  }

  Qt::CheckState _GetFlagState(ULONG flag) const
  {
    auto it = m_entities.begin();
    CEntityPtr entity(*it);
    CEntityPropertyPtr actual_property = entity->PropertyForName(mp_property->pid_strName);
    const bool flag_is_set = (*ENTITY_PROPERTY(entity, actual_property->ep_slOffset, ULONG)) & flag;
    for (++it; it != m_entities.end(); ++it)
    {
      CEntityPtr entity(*it);
      CEntityPropertyPtr actual_property = entity->PropertyForName(mp_property->pid_strName);
      const bool curr_flag = (*ENTITY_PROPERTY(entity, actual_property->ep_slOffset, ULONG)) & flag;
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
static UIPropertyFactory::Registrar g_registrar(CEntityProperty::PropertyType::EPT_FLAGS,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_Flags(parent);
  });
