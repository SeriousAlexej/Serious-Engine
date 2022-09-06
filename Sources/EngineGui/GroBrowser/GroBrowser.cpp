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

#include "stdh.h"
#include "GroBrowser.h"
#include "GroBrowser.ui.h"
#include "GroBrowser.qrc.h"

#include <QPushButton>
#include <QKeyEvent>

#include <algorithm>
#include <string_view>
#include <map>

Q_DECLARE_METATYPE(GroBrowser::_FileNode*)

struct GroBrowser::_FileNode {
  QString name;
  _FileNode* parent = nullptr;
  std::map<QString, std::unique_ptr<_FileNode>> children;

  bool IsFile() const
  {
    return children.empty();
  }
};

std::unique_ptr<GroBrowser::_FileNode> GroBrowser::mp_root_node;

GroBrowser::GroBrowser(QWidget* parent)
  : QDialog(parent)
  , mp_ui(std::make_unique<Ui::GroBrowser>())
{
  _CacheFiles();
  mp_current_node = mp_root_node.get();

  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  mp_ui->setupUi(this);
  m_icon_provider.setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);

  mp_ui->listWidget->installEventFilter(this);
  connect(mp_ui->listWidget, &QListWidget::itemDoubleClicked, this, &GroBrowser::_OnDoubleClicked);
  connect(mp_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(mp_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  _RefillList();
}

GroBrowser::~GroBrowser()
{
}

bool GroBrowser::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == mp_ui->listWidget)
    if (event->type() == QEvent::KeyPress)
    {
      auto* key_event = static_cast<QKeyEvent*>(event);
      if (key_event->key() == Qt::Key_Enter || key_event->key() == Qt::Key_Return)
      {
        const auto selection = mp_ui->listWidget->selectedItems();
        if (selection.size() == 1)
          return _CD(selection[0]);
      }
    }
  return QDialog::eventFilter(watched, event);
}

bool GroBrowser::_CD(QListWidgetItem* item)
{
  auto* node = item->data(Qt::UserRole).value<_FileNode*>();
  if (!node->IsFile())
  {
    mp_current_node = node;
    _RefillList();
    return true;
  }
  return false;
}

void GroBrowser::_OnDoubleClicked(QListWidgetItem* item)
{
  _CD(item);
}

void GroBrowser::_RefillList()
{
  mp_ui->listWidget->clear();

  auto insert_node = [this](const std::unique_ptr<_FileNode>& node)
  {
    QIcon node_icon;
    if (node->IsFile())
      node_icon = m_icon_provider.icon(QFileInfo(node->name));
    else
      node_icon = m_icon_provider.icon(QFileIconProvider::Folder);
    auto* item = new QListWidgetItem(node_icon, node->name, mp_ui->listWidget);
    item->setData(Qt::UserRole, QVariant::fromValue(node.get()));
  };

  for (const auto& [dummy, node] : mp_current_node->children)
    if (!node->IsFile())
      insert_node(node);
  for (const auto& [dummy, node] : mp_current_node->children)
    if (node->IsFile())
      insert_node(node);
}

void GroBrowser::_CacheFiles()
{
  if (mp_root_node)
    return;

  mp_root_node = std::make_unique<_FileNode>();
  CDynamicStackArray<CTFileName> game_files;
  MakeDirList(game_files, CTString(""), "", DLI_RECURSIVE);
  for (INDEX i = 0; i < game_files.Count(); ++i)
  {
    CTFileName full_filename;
    const auto file_type = ExpandFilePath(EFP_READ, game_files[i], full_filename);
    if (file_type == EFP_BASEZIP || file_type == EFP_MODZIP)
    {
      _FileNode* current_parent = mp_root_node.get();
      const std::string_view str_view(full_filename.str_String, full_filename.Length());
      auto beg = std::begin(str_view);
      const auto end = std::end(str_view);
      while (true)
      {
        auto next_beg = std::find(beg, end, '\\');
        const auto name = QString::fromLocal8Bit(beg.operator->(), std::distance(beg, next_beg));
        auto new_node = std::make_unique<_FileNode>();
        new_node->name = name;
        new_node->parent = current_parent;
        auto [inserted_node, dummy] = current_parent->children.try_emplace(name.toLower(), std::move(new_node));
        current_parent = inserted_node->second.get();
        if (next_beg == end)
          break;
        beg = std::next(next_beg);
      }
    }
  }
}
