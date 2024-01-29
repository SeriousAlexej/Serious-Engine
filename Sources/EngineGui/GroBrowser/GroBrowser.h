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

#ifndef GRO_BROWSER_H
#define GRO_BROWSER_H

#include <QDialog>
#include <QString>
#include <QThread>
#include <QPointer>
#include <QFileIconProvider>

#include <deque>
#include <vector>
#include <memory>

namespace Ui {
class GroBrowser;
}

class QListWidgetItem;

class GroBrowser : public QDialog {
public:
  GroBrowser(const char* filter, bool multiselection, const CTString& default_selection, QWidget* parent = nullptr);
  ~GroBrowser();

  std::vector<QString> SelectedFiles() const;

  struct _FileNode;

private:
  enum class HistoryDirection
  {
    Back,
    Forward,
    New
  };

  bool eventFilter(QObject* watched, QEvent* event) override;
  bool _CD(_FileNode* node, HistoryDirection history_direction);
  bool _CD(QListWidgetItem* item);
  void _OnCDUp();
  void _OnCDBack();
  void _OnCDForward();
  void _OnDoubleClicked(QListWidgetItem* item);
  void _OnSelectionChanged();
  void _FillFilter(const char* filter);
  void _RefillList();
  void _UpdateLinePath();
  void _OnCacheReady();

private:
  std::unique_ptr<Ui::GroBrowser> mp_ui;
  QFileIconProvider m_icon_provider;
  _FileNode* mp_current_node = nullptr;
  std::deque<_FileNode*> m_history;
  std::vector<_FileNode*> m_forward_history;
  QPointer<QThread> mp_cache_builder;
  const QString m_default_selection;
  static std::unique_ptr<_FileNode> mp_root_node;
};

class CacheBuilder final : public QThread {
  Q_OBJECT
public:
  explicit CacheBuilder(std::unique_ptr<GroBrowser::_FileNode>& root_node, QObject* parent = nullptr);

  Q_SIGNAL void Calculated();
  Q_SIGNAL void Max(int maximum);
  Q_SIGNAL void FileDone(int done);
  Q_SIGNAL void Progress(QString currentFile);

private:
  void run() override;

private:
  std::unique_ptr<GroBrowser::_FileNode>& mp_root_node;
};

#endif
