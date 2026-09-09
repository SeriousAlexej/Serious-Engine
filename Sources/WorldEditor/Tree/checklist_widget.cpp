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
#include "checklist_widget.h"
#include "checklist_widget.h.moc"

#include <QLineEdit>
#include <QStyledItemDelegate>
#include <QListView>

namespace
{
  const char* g_combo_style = R"(
QComboBox {
  background-color: transparent;
  border: 0px;
}
)";
  class _NoDecorationsDelegate : public QStyledItemDelegate
  {
  public:
    explicit _NoDecorationsDelegate(QObject* parent = nullptr)
      : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& const_option, const QModelIndex& index) const override
    {
      auto& option = const_cast<QStyleOptionViewItem&>(const_option);
      option.showDecorationSelected = false;
      QStyledItemDelegate::paint(painter, option, index);
    }
  };
}

CheckListWidget::CheckListWidget(QWidget* parent)
  : QComboBox(parent)
{
  setModel(&m_model);
  setEditable(true);
  lineEdit()->setReadOnly(true);
  lineEdit()->installEventFilter(this);
  setItemDelegate(new _NoDecorationsDelegate(this));

  QObject::connect(lineEdit(), &QLineEdit::selectionChanged, lineEdit(), &QLineEdit::deselect);
  QObject::connect(view(), &QAbstractItemView::pressed, this, [this]
    (const QModelIndex& index)
    {
      QStandardItem* item = m_model.itemFromIndex(index);

      if (item->checkState() == Qt::Checked)
        item->setCheckState(Qt::Unchecked);
      else
        item->setCheckState(Qt::Checked);
    });
  QObject::connect(&m_model, &QStandardItemModel::dataChanged, this, [this]
    {
      _UpdateText();
      m_changed = true;
    });

  setStyleSheet(g_combo_style);
}

QStandardItem* CheckListWidget::AddItem(const QString& label, const Qt::CheckState check_state, QVariant data)
{
  QStandardItem* item = new QStandardItem(label);
  item->setCheckState(check_state);
  item->setData(data);
  item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

  m_model.appendRow(item);

  _UpdateText();

  setMinimumWidth(30);

  return item;
}

bool CheckListWidget::event(QEvent* event)
{
  if (event->type() == QEvent::Wheel)
  {
    event->ignore();
    return true;
  }
  return QComboBox::event(event);
}

bool CheckListWidget::eventFilter(QObject* object, QEvent* event)
{
  if (object == lineEdit() && event->type() == QEvent::MouseButtonPress)
  {
    showPopup();
    return true;
  }

  return false;
}

void CheckListWidget::showPopup()
{
  QComboBox::showPopup();
  m_changed = false;
}

void CheckListWidget::hidePopup()
{
  QComboBox::hidePopup();
  if (m_changed)
    Changed();
  m_changed = false;
}

std::optional<Qt::CheckState> CheckListWidget::_GlobalCheckState() const
{
  const int num_rows = m_model.rowCount();
  if (num_rows == 0)
    return std::nullopt;

  int num_checked = 0;
  int num_unchecked = 0;

  for (int i = 0; i < num_rows; i++)
  {
    if (m_model.item(i)->checkState() == Qt::Checked)
      num_checked++;
    else if (m_model.item(i)->checkState() == Qt::Unchecked)
      num_unchecked++;
  }

  if (num_checked == num_rows)
    return Qt::Checked;
  if (num_unchecked == num_rows)
    return Qt::Unchecked;
  return Qt::PartiallyChecked;
}

void CheckListWidget::_UpdateText()
{
  QString text = "<unknown>";

  auto check_state = _GlobalCheckState();
  if (check_state.has_value())
  {
    switch (*check_state)
    {
    case Qt::Checked:
      text = "(all)";
      break;

    case Qt::Unchecked:
      text = "(none)";
      break;

    case Qt::PartiallyChecked:
      text = "(mixed)";
      break;

    default:
      break;
    }
  }

  lineEdit()->setText(text);
}
