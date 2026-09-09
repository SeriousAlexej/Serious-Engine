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

#ifndef CHECKLIST_WIDGET_H
#define CHECKLIST_WIDGET_H

#include <QComboBox>
#include <QStandardItemModel>
#include <QEvent>

#include <optional>

class CheckListWidget : public QComboBox
{
  Q_OBJECT
public:
  explicit CheckListWidget(QWidget* parent = nullptr);

  QStandardItem* AddItem(const QString& label, const Qt::CheckState check_state, QVariant data);

  Q_SIGNAL void Changed();

private:
  std::optional<Qt::CheckState> _GlobalCheckState() const;
  bool event(QEvent* event) override;
  bool eventFilter(QObject* object, QEvent* event) override;
  void showPopup() override;
  void hidePopup() override;
  void _UpdateText();

private:
  QStandardItemModel m_model;
  bool m_changed = false;
};

#endif
