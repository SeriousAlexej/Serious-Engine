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
#include "GroBrowser.h.moc"
#include "GroBrowser.ui.h"
#include "GroBrowser.qrc.h"

#include <SeriousEngineCppAPI/Templates/Stock_CTextureData.h>

#include <QPushButton>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QTimer>

#include <algorithm>
#include <string_view>
#include <map>

Q_DECLARE_OPAQUE_POINTER(GroBrowser::_FileNode*)

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

CacheBuilder::CacheBuilder(std::unique_ptr<GroBrowser::_FileNode>& root_node, QObject* parent)
  : QThread(parent)
  , mp_root_node(root_node)
{
}

void CacheBuilder::run()
{
  mp_root_node = std::make_unique<GroBrowser::_FileNode>();
  CDynamicStackArray_CTFileName game_files;
  MakeDirList(game_files, CTString(""), "", DLI_RECURSIVE);
  Max(game_files.Count());
  for (INDEX i = 0; i < game_files.Count() && !isInterruptionRequested(); ++i)
  {
    CTFileName full_filename;
    CTFileNamePtr game_file = game_files[i];
    const auto file_type = ExpandFilePath(EFP_READ, *game_file, full_filename);
    if (file_type == EFP_BASEZIP || file_type == EFP_MODZIP)
    {
      GroBrowser::_FileNode* current_parent = mp_root_node.get();
      const std::string_view str_view(static_cast<const char*>(full_filename), full_filename.Length());
      auto beg = std::begin(str_view);
      const auto end = std::end(str_view);
      while (true)
      {
        auto next_beg = std::find(beg, end, '\\');
        const auto name = QString::fromLocal8Bit(beg.operator->(), std::distance(beg, next_beg));
        auto new_node = std::make_unique<GroBrowser::_FileNode>();
        new_node->name = name;
        new_node->parent = current_parent;
        auto [inserted_node, dummy] = current_parent->children.try_emplace(name.toLower(), std::move(new_node));
        current_parent = inserted_node->second.get();
        if (next_beg == end)
          break;
        beg = std::next(next_beg);
      }
      Progress(current_parent->name);
    }
    FileDone(i + 1);
  }
  Calculated();
}

std::unique_ptr<GroBrowser::_FileNode> GroBrowser::mp_root_node;

GroBrowser::GroBrowser(const char* filter, bool multiselection, const CTString& default_selection, QWidget* parent)
  : QDialog(parent)
  , mp_ui(std::make_unique<Ui::GroBrowser>())
  , m_default_selection(QString::fromLocal8Bit(static_cast<const char*>(default_selection)).toLower())
{
  m_forward_history.reserve(g_maxBackSteps);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  m_icon_provider.setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);
  mp_ui->setupUi(this);

  _FillFilter(filter);

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
  connect(mp_ui->buttonCancelCaching, &QPushButton::clicked, this, [this]
    {
      mp_ui->buttonCancelCaching->setEnabled(false);
      if (mp_cache_builder && mp_cache_builder->isRunning())
        mp_cache_builder->requestInterruption();
    });

  if (!mp_root_node)
  {
    mp_ui->stackedWidget->setCurrentWidget(mp_ui->pageProgress);
    auto* cache_builder = new CacheBuilder(mp_root_node, this);
    connect(cache_builder, &CacheBuilder::Calculated, this, &GroBrowser::_OnCacheReady);
    connect(cache_builder, &CacheBuilder::finished, cache_builder, &QObject::deleteLater);
    connect(cache_builder, &CacheBuilder::Max, this, [this](int maximum) { mp_ui->progressBar->setMaximum(maximum); });
    connect(cache_builder, &CacheBuilder::FileDone, this, [this](int done) { mp_ui->progressBar->setValue(done); });
    connect(cache_builder, &CacheBuilder::Progress, this, [this](QString current_file) { mp_ui->labelCurrentFile->setText(current_file); });
    cache_builder->start();
    mp_cache_builder = cache_builder;
  }
  else
  {
    QTimer::singleShot(0, this, &GroBrowser::_OnCacheReady);
  }
}

GroBrowser::~GroBrowser()
{
  if (mp_cache_builder && mp_cache_builder->isRunning())
  {
    mp_cache_builder->disconnect(this);
    mp_cache_builder->requestInterruption();
    mp_cache_builder->wait();
    mp_root_node.release();
  }
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
  const auto* node = item->data(Qt::UserRole).value<_FileNode*>();
  if (node && node->IsFile())
  {
    mp_ui->listWidget->clearSelection();
    item->setSelected(true);
    accept();
  } else {
    _CD(item);
  }
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
  _UpdateLinePath();
  _UpdatePreview();
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

  std::vector<QRegularExpression> filters;
  if (auto filter_wildcard = mp_ui->comboFilter->currentData(); filter_wildcard.isValid())
  {
    const auto wildcards = filter_wildcard.toString().split(';', Qt::SkipEmptyParts);
    for (const auto& wildcard : wildcards)
      filters.push_back(QRegularExpression::fromWildcard(wildcard));
  }

  auto insert_node = [this, &filters](const std::unique_ptr<_FileNode>& node)
  {
    if (node->IsFile() && !filters.empty())
    {
      if (std::none_of(filters.begin(), filters.end(),
        [&](const QRegularExpression& filter)
        {
        const auto match = filter.match(node->name);
        return match.isValid() && match.hasMatch() && !match.hasPartialMatch();
        }))
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

  mp_ui->buttonUp->setEnabled(mp_current_node->parent);
  mp_ui->buttonBack->setEnabled(!m_history.empty());
  mp_ui->buttonForward->setEnabled(!m_forward_history.empty());
  _UpdateLinePath();
  _UpdatePreview();
}

void GroBrowser::_UpdateLinePath()
{
  const auto* current_node = mp_current_node;
  const auto selection = mp_ui->listWidget->selectedItems();
  if (selection.size() == 1)
  {
    const auto* node = selection.front()->data(Qt::UserRole).value<_FileNode*>();
    if (node && node->IsFile())
      current_node = node;
  }
  mp_ui->linePath->setText(current_node->Path());
}

void GroBrowser::_UpdatePreview()
{
  const auto selection = mp_ui->listWidget->selectedItems();
  if (selection.size() == 1)
  {
    const auto* node = selection.front()->data(Qt::UserRole).value<_FileNode*>();
    if (node && node->IsFile())
    {
      CTextureDataPtr textureData;

      try
      {
        CTFileName nodePath = CTString(node->Path().toLocal8Bit().data());
        const auto nodeExtension = nodePath.FileExt();
        CTFileName thumbnailFile = CTString("");

        if (nodeExtension == ".wld" || nodeExtension == ".mdl")
        {
          thumbnailFile = nodePath.FileDir() + nodePath.FileName() + ".tbn";
        }
        else if (nodeExtension == ".tex" || nodeExtension == ".tbn")
        {
          thumbnailFile = nodePath;
        }
        else
        {
          const auto& supported_image_formats = _EngineGUI.GetSupportedImportFormats();
          if (std::any_of(supported_image_formats.begin(), supported_image_formats.end(),
            [&](const std::string& extension)
            {
              return nodeExtension == extension.c_str();
            }))
          {
            CImageInfo iiImageInfo;
            _EngineGUI.LoadAnyGfxFormat_t(iiImageInfo, nodePath);
            if ((iiImageInfo.ii_Width == 1 << ((int)Log2(iiImageInfo.ii_Width))) &&
                (iiImageInfo.ii_Height == 1 << ((int)Log2(iiImageInfo.ii_Height))))
            {
              thumbnailFile = CTString("Temp\\Temp.tex");
              CTextureData tdForPictureConverting;
              tdForPictureConverting.Create_t(iiImageInfo, iiImageInfo.ii_Width, 1, FALSE);
              tdForPictureConverting.Save_t(thumbnailFile);
            }
          }
        }

        if (thumbnailFile != "")
        {
          CUpdateable updatable;
          updatable.MarkUpdated();
          textureData = _pTextureStock_Obtain_t(thumbnailFile);
          textureData->Force(TEX_CONSTANT | TEX_STATIC | TEX_KEEPCOLOR);
          // if texture data is up to date to recently changed updatable
          // then it means it was not reloaded
          if (textureData->IsUpToDate(updatable))
            textureData->Reload();
        }
      }
      catch (const char*)
      {
        textureData.Reset();
      }

      if (textureData)
      {
        if (true)
        {
          const QImage image(
            reinterpret_cast<const unsigned char*>(textureData->td_pulFrames),
            textureData->GetPixWidth(),
            textureData->GetPixHeight(),
            textureData->GetPixWidth() * BYTES_PER_TEXEL,
            QImage::Format_RGBA8888);

          const auto frameSize = mp_ui->previewThumbnail->frameWidth();
          const auto width = mp_ui->previewThumbnail->width();
          const auto height = mp_ui->previewThumbnail->height();
          mp_ui->previewThumbnail->setPixmap(
            QPixmap::fromImage(image).scaled(
              width - frameSize*2,
              height - frameSize*2,
              Qt::IgnoreAspectRatio,
              Qt::SmoothTransformation));
        }
        const auto previewDescription = textureData->GetDescription();
        mp_ui->previewInfo->setText(QString::fromLocal8Bit(static_cast<const char*>(previewDescription)));
        _pTextureStock_Release(*textureData);
        return;
      }
    }
  }

  mp_ui->previewThumbnail->clear();
  mp_ui->previewInfo->setText("No thumbnail");
}

void GroBrowser::_OnCacheReady()
{
  if (mp_cache_builder && mp_cache_builder->isInterruptionRequested())
  {
    mp_root_node.reset();
    reject();
    return;
  }
  mp_ui->stackedWidget->setCurrentWidget(mp_ui->pageMain);
  mp_current_node = mp_root_node.get();

  const auto selection_path = m_default_selection.split('\\', Qt::SkipEmptyParts);
  int i = 0;
  while (i < selection_path.size())
  {
    auto next_node = mp_current_node->children.find(selection_path.at(i++));
    if (next_node == mp_current_node->children.end() || next_node->second->IsFile())
      break;
    mp_current_node = next_node->second.get();
  }

  _RefillList();

  if (i == selection_path.size() && !selection_path.isEmpty())
  {
    const auto found_leaf = mp_ui->listWidget->findItems(selection_path.back(), Qt::MatchFixedString);
    if (!found_leaf.isEmpty())
    {
      found_leaf.front()->setSelected(true);
      mp_ui->listWidget->scrollToItem(found_leaf.front(), QAbstractItemView::PositionAtCenter);
    }
  }

  mp_ui->listWidget->setFocus();
}
