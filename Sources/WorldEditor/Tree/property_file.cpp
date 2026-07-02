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
#include <QSignalBlocker>

namespace
{
  const char* g_combo_style = R"(
QComboBox {
  background-color: transparent;
  border: 0px;
}
)";
}

class Property_Filename : public BaseEntityPropertyTreeItem
{
public:
  Property_Filename(BasePropertyTreeItem* parent, bool dependencies)
    : BaseEntityPropertyTreeItem(parent)
    , m_dependencies(dependencies)
  {
  }

  QWidget* CreateEditor(QWidget* parent) override
  {
    auto* editor = new QComboBox(parent);
    editor->setStyleSheet(g_combo_style);

    CTFileName curr_value = (m_dependencies ? _CurrentPropValueT<CTFileName>() : _CurrentPropValueT<CTFileNameNoDep>());
    if (curr_value != "")
    {
      editor->addItem(QString(static_cast<const char*>(curr_value.FileName())) + QString(static_cast<const char*>(curr_value.FileExt())), 1);
      editor->setToolTip(static_cast<const char*>(curr_value));
    }
    editor->addItem("(browse)", 2);
    editor->addItem("(none)", 3);

    if (curr_value != "")
      editor->setCurrentIndex(editor->findData(1));
    else
      editor->setCurrentIndex(editor->findData(3));

    editor->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    editor->installEventFilter(this);
    editor->setMinimumWidth(30);

    QObject::connect(editor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, editor]
      (int index)
      {
        if (index != -1 && editor->itemData(index).toInt() == 3)
        {
          if (m_dependencies)
            _WritePropertyT<CTFileName>(CTFileName(CTString("")));
          else
            _WritePropertyT<CTFileNameNoDep>(CTFileNameNoDep(""));
        } else if (index != -1 && editor->itemData(index).toInt() == 2)
        {
          CTFileName curr_value = (m_dependencies ? _CurrentPropValueT<CTFileName>() : _CurrentPropValueT<CTFileNameNoDep>());

          CTFileName chosen_file;
          if( curr_value.FileExt() == ".mdl")
          {
            chosen_file = _EngineGUI.FileRequester( "Choose file",
            FILTER_MDL FILTER_ALL FILTER_END,
            KEY_NAME_REQUEST_FILE_DIR, curr_value.FileDir(),
            curr_value.FileName()+curr_value.FileExt());
          }
          else if( curr_value.FileExt() == ".tex")
          {
            chosen_file = _EngineGUI.FileRequester( "Choose file",
            FILTER_TEX FILTER_ALL FILTER_END,
            KEY_NAME_REQUEST_FILE_DIR, curr_value.FileDir(),
            curr_value.FileName()+curr_value.FileExt());
          }
          else if( curr_value.FileExt() == ".wav")
          {
            chosen_file = _EngineGUI.FileRequester( "Choose file",
            FILTER_WAV FILTER_ALL FILTER_END,
            KEY_NAME_REQUEST_FILE_DIR, curr_value.FileDir(),
            curr_value.FileName()+curr_value.FileExt());
          }
          else if( curr_value.FileExt() == ".smc")
          {
            chosen_file = _EngineGUI.FileRequester( "Choose file",
            FILTER_SMC FILTER_ALL FILTER_END,
            KEY_NAME_REQUEST_FILE_DIR, curr_value.FileDir(),
            curr_value.FileName()+curr_value.FileExt());
          }
          else
          {
            chosen_file = _EngineGUI.FileRequester( "Choose file",
            FILTER_ALL FILTER_MDL FILTER_TEX FILTER_WAV FILTER_END,
            KEY_NAME_REQUEST_FILE_DIR, curr_value.FileDir(),
            curr_value.FileName()+curr_value.FileExt());
          }

          if (chosen_file != "")
          {
            if (m_dependencies)
              _WritePropertyT<CTFileName>(chosen_file);
            else
              _WritePropertyT<CTFileNameNoDep>((CTFileNameNoDep)chosen_file);
          } else {
            QSignalBlocker block(editor);
            if (curr_value != "")
              editor->setCurrentIndex(editor->findData(1));
            else
              editor->setCurrentIndex(editor->findData(3));
          }
        }
      });

    return editor;
  }

  bool ValueIsCommonForAllEntities() const override final
  {
    if (m_dependencies)
      return _ValueIsCommonForAllEntities<CTFileName>();
    return _ValueIsCommonForAllEntities<CTFileNameNoDep>();
  }

  void SetFirstValueToAllEntities() override final
  {
    if (m_dependencies)
      _WritePropertyT<CTFileName>(_CurrentPropValueT<CTFileName>());
    else
      _WritePropertyT<CTFileNameNoDep>(_CurrentPropValueT<CTFileNameNoDep>());
  }

protected:
  QString _GetTypeName() const override final
  {
    return "CTFileName";
  }

  bool _ChangesDocument() const
  {
    return true;
  }

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

private:
  const bool m_dependencies;
};

/*******************************************************************************************/
static UIPropertyFactory::Registrar g_registrar_1(CEntityProperty::PropertyType::EPT_FILENAME,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_Filename(parent, true);
  });
static UIPropertyFactory::Registrar g_registrar_2(CEntityProperty::PropertyType::EPT_FILENAMENODEP,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_Filename(parent, false);
  });
