/* Copyright (c) 2026 SeriousAlexej.
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

#ifndef STRING_WIDGET_AND_DIALOG_H
#define STRING_WIDGET_AND_DIALOG_H

#include <QDialog>
#include <QWidget>
#include <QString>
#include <memory>

class QLineEdit;

class StringWidget : public QWidget
{
  Q_OBJECT
public:
  StringWidget(const QString& string, QWidget* parent);

  QString Text() const;

  Q_SIGNAL void editingFinished();

private:
  QLineEdit* mp_line = nullptr;
};

namespace Ui {
  class EditStringDialog;
}
class EditStringDialog final : public QDialog
{
  Q_OBJECT
public:
  EditStringDialog(const QString& string, QWidget* parent);

  QString Text() const;

  void accept() final;
  void reject() final;

private:
  void _LoadState();
  void _SaveState();

private:
  std::unique_ptr<Ui::EditStringDialog> mp_ui;
};

#endif
