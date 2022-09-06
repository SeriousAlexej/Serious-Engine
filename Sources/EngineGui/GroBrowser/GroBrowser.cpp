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
#include <QRegExp>

#include <algorithm>
#include <string_view>
#include <map>

Q_DECLARE_METATYPE(GroBrowser::_FileNode*)

static constexpr size_t g_maxBackSteps = 20;

struct GroBrowser::_FileNode {
  QString name;
  _FileNode* parent = nullptr;
  std::map<QString, std::unique_ptr<_FileNode>> children;

  bool IsFile() const
  {
    return children.empty();
  }

  QString Path() const
  {
    if (parent && !parent->name.isEmpty())
      return parent->Path() + '\\' + name;
    return name;
  }
};

std::unique_ptr<GroBrowser::_FileNode> GroBrowser::mp_root_node;

GroBrowser::GroBrowser(const char* filter, bool multiselection, QWidget* parent)
  : QDialog(parent)
  , mp_ui(std::make_unique<Ui::GroBrowser>())
{
  m_forward_history.reserve(g_maxBackSteps);
  _CacheFiles();
  mp_current_node = mp_root_node.get();

  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  mp_ui->setupUi(this);
  m_icon_provider.setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);

  _FillFilter(filter);
  _RefillList();

  mp_ui->listWidget->installEventFilter(this);
  mp_ui->listWidget->setSelectionMode(multiselection ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
  mp_ui->buttonBox->button(QDialogButtonBox::Open)->setEnabled(false);
  connect(mp_ui->buttonUp, &QPushButton::clicked, this, &GroBrowser::_OnCDUp);
  connect(mp_ui->buttonBack, &QPushButton::clicked, this, &GroBrowser::_OnCDBack);
  connect(mp_ui->buttonForward, &QPushButton::clicked, this, &GroBrowser::_OnCDForward);
  connect(mp_ui->listWidget, &QListWidget::itemDoubleClicked, this, &GroBrowser::_OnDoubleClicked);
  connect(mp_ui->listWidget, &QListWidget::itemSelectionChanged, this, &GroBrowser::_OnSelectionChanged);
  connect(mp_ui->comboFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GroBrowser::_RefillList);
  connect(mp_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(mp_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  mp_ui->listWidget->setFocus();
}

GroBrowser::~GroBrowser()
{
}

std::vector<QString> GroBrowser::SelectedFiles() const
{
  std::vector<QString> files;

  if (result() == QDialog::Accepted)
    for (const auto* item : mp_ui->listWidget->selectedItems())
    {
      auto* node = item->data(Qt::UserRole).value<_FileNode*>();
      if (node && node->IsFile())
        files.push_back(node->Path());
    }

  return files;
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

bool GroBrowser::_CD(_FileNode* node, HistoryDirection history_direction)
{
  switch (history_direction)
  {
  case HistoryDirection::New:
    if (!node || node->IsFile())
      return false;
    m_forward_history.clear();
    m_history.push_back(mp_current_node);
    if (m_history.size() == g_maxBackSteps)
      m_history.pop_front();
    break;

  case HistoryDirection::Back:
    if (m_history.empty())
      return false;

    node = m_history.back();
    m_history.pop_back();
    m_forward_history.push_back(mp_current_node);
    break;

  case HistoryDirection::Forward:
    if (m_forward_history.empty())
      return false;

    node = m_forward_history.back();
    m_forward_history.pop_back();
    m_history.push_back(mp_current_node);
    if (m_history.size() == g_maxBackSteps)
      m_history.pop_front();
    break;

  default:
    return false;
  }

  mp_current_node = node;
  _RefillList();
  mp_ui->buttonUp->setEnabled(mp_current_node->parent);
  mp_ui->buttonBack->setEnabled(!m_history.empty());
  mp_ui->buttonForward->setEnabled(!m_forward_history.empty());
  mp_ui->linePath->setText(mp_current_node->Path());
  return true;
}

bool GroBrowser::_CD(QListWidgetItem* item)
{
  auto* node = item->data(Qt::UserRole).value<_FileNode*>();
  return _CD(node, HistoryDirection::New);
}

void GroBrowser::_OnCDUp()
{
  if (mp_current_node->parent)
    _CD(mp_current_node->parent, HistoryDirection::New);
}

void GroBrowser::_OnCDBack()
{
  _CD(nullptr, HistoryDirection::Back);
}

void GroBrowser::_OnCDForward()
{
  _CD(nullptr, HistoryDirection::Forward);
}

void GroBrowser::_OnDoubleClicked(QListWidgetItem* item)
{
  _CD(item);
}

void GroBrowser::_OnSelectionChanged()
{
  const auto selection = mp_ui->listWidget->selectedItems();
  const bool has_selected_file = std::any_of(selection.begin(), selection.end(), [](const QListWidgetItem* item)
    {
      auto* node = item->data(Qt::UserRole).value<_FileNode*>();
      return node && node->IsFile();
    });
  mp_ui->buttonBox->button(QDialogButtonBox::Open)->setEnabled(has_selected_file);
}

void GroBrowser::_FillFilter(const char* filter)
{
  while (filter && *filter)
  {
    QString name = filter;
    filter += name.length() + 1;
    QString extension = filter;
    filter += extension.length() + 1;
    mp_ui->comboFilter->addItem(name, extension);
  }

  if (mp_ui->comboFilter->count() > 0)
    mp_ui->comboFilter->setCurrentIndex(0);
  else
    mp_ui->comboFilter->setVisible(false);
}

void GroBrowser::_RefillList()
{
  mp_ui->listWidget->clear();

  std::vector<QRegExp> filters;
  if (auto filter_wildcard = mp_ui->comboFilter->currentData(); filter_wildcard.isValid())
  {
    const auto wildcards = filter_wildcard.toString().split(';', QString::SkipEmptyParts);
    for (const auto& wildcard : wildcards)
    {
      QRegExp filter;
      filter.setPattern(wildcard);
      filter.setPatternSyntax(QRegExp::Wildcard);
      filter.setCaseSensitivity(Qt::CaseInsensitive);
      filters.push_back(filter);
    }
  }

  auto insert_node = [this, &filters](const std::unique_ptr<_FileNode>& node)
  {
    if (node->IsFile() && !filters.empty())
    {
      if (std::none_of(filters.begin(), filters.end(), [&](const QRegExp& filter) { return filter.exactMatch(node->name); }))
        return;
    }

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
