/* Copyright (c) 2026 SeriousAlexej (Oleksii Sierov).
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
#include "string_widget.h"
#include "string_widget.ui.h"
#include "string_widget.h.moc"

#include <QIcon>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDialogButtonBox>

namespace
{
  const char* g_string_widget_style = R"(
QLineEdit {
  background-color: transparent;border:
  0px;
}
QPushButton {
  background-color: transparent;
  border-style: solid;
  border-width: 0px;
  border-top-color: #EAEAE4;
  border-right-color: #404040;
  border-bottom-color: #404040;
  border-left-color: #EAEAE4;
}
QPushButton:hover {
  background-color: white;
  border-style: solid;
  border-width: 1px;
  border-top-color: #EAEAE4;
  border-right-color: #404040;
  border-bottom-color: #404040;
  border-left-color: #EAEAE4;
}
QPushButton:pressed, QPushButton:checked {
  background-color: #BCBCB8;
  border-style: solid;
  border-width: 1px;
  border-top-color: #404040;
  border-right-color: #EAEAE4;
  border-bottom-color: #EAEAE4;
  border-left-color: #404040;
}
)";
}

StringWidget::StringWidget(const QString& string, QWidget* parent)
  : QWidget(parent)
{
  auto* layout = new QHBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);

  mp_line = new QLineEdit(this);
  mp_line->setText(string);
  layout->addWidget(mp_line);

  QObject::connect(mp_line, &QLineEdit::editingFinished, this, &StringWidget::editingFinished);

  auto* btn_edit = new QPushButton(QIcon(":/tree_icons/list_entities.png"), "", this);
  btn_edit->setFixedSize(12, 15);
  connect(btn_edit, &QPushButton::clicked, this, [this]
    {
      CWorldEditorApp::ModalGuard guard;
      QWinWidget modal_widget(AfxGetApp()->m_pMainWnd->GetSafeHwnd(), nullptr, Qt::WindowFlags{});
      EditStringDialog dialog(mp_line->text(), &modal_widget);
      if (dialog.exec() == QDialog::Accepted)
      {
        mp_line->setText(dialog.Text());
        editingFinished();
      }
    });
  layout->addWidget(btn_edit);

  setStyleSheet(g_string_widget_style);
}

QString StringWidget::Text() const
{
  return mp_line->text();
}

EditStringDialog::EditStringDialog(const QString& string, QWidget* parent)
  : QDialog(parent)
  , mp_ui(std::make_unique<Ui::EditStringDialog>())
{
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  mp_ui->setupUi(this);
  mp_ui->plainTextEdit->setPlainText(string);
  connect(mp_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(mp_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString EditStringDialog::Text() const
{
  return mp_ui->plainTextEdit->toPlainText();
}
