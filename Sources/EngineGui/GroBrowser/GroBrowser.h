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
#include <QFileIconProvider>

#include <memory>

namespace Ui {
class GroBrowser;
}

class QListWidgetItem;

class GroBrowser : public QDialog {
public:
  explicit GroBrowser(QWidget* parent = nullptr);
  ~GroBrowser();

  struct _FileNode;

private:
  bool eventFilter(QObject* watched, QEvent* event) override;
  bool _CD(QListWidgetItem* item);
  void _OnDoubleClicked(QListWidgetItem* item);
  void _RefillList();
  void _CacheFiles();

private:
  std::unique_ptr<Ui::GroBrowser> mp_ui;
  QFileIconProvider m_icon_provider;
  _FileNode* mp_current_node = nullptr;
  static std::unique_ptr<_FileNode> mp_root_node;
};

#endif
