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
#include "pointer_widget.h"
#include "pointer_widget.h.moc"
#include "EventHub.h"

#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

namespace
{
  const char* g_button_style = R"(
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

PointerWidget::PointerWidget(CEntityPtr entity, QWidget* parent)
  : QWidget(parent)
{
  auto* layout = new QHBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);

  auto* label = new QLabel("(none)", this);
  if (entity)
    label->setText(QString("%1 (ID %2)").arg(QString::fromLocal8Bit(static_cast<const char*>(entity->GetName()))).arg(QString::number(entity->en_ulID)));
  layout->addWidget(label);

  if (entity)
  {
    auto* btn_locate = new QPushButton(QIcon(":/tree_icons/locate.png"), "", this);
    btn_locate->setStyleSheet(g_button_style);
    btn_locate->setFixedSize(12, 15);
    QObject::connect(btn_locate, &QPushButton::clicked, this, [this, entity]
      {
        CWorldEditorDoc* pDoc = theApp.GetActiveDocument();
        if (!pDoc)
          return;
        POSITION pos = pDoc->GetFirstViewPosition();
        CWorldEditorView* pWedView = (CWorldEditorView*)pDoc->GetNextView(pos);
        if (pWedView)
          pWedView->OnCenterEntity(entity);
      });
    layout->addWidget(btn_locate);
  }

  auto* btn_pick = new QPushButton(QIcon(":/tree_icons/pick_entity.png"), "", this);
  btn_pick->setStyleSheet(g_button_style);
  btn_pick->setFixedSize(12, 15);
  btn_pick->setCheckable(true);
  QObject::connect(btn_pick, &QPushButton::clicked, this, [this, btn_pick]
    {
      if (btn_pick->isChecked())
        pick();
      else
        theApp.InstallOneTimeSelectionStealer(nullptr, nullptr);
    });
  layout->addWidget(btn_pick);

  auto* btn_list = new QPushButton(QIcon(":/tree_icons/list_entities.png"), "", this);
  btn_list->setStyleSheet(g_button_style);
  btn_list->setFixedSize(12, 15);
  QObject::connect(btn_list, &QPushButton::clicked, this, [this] { selectFromList(); });
  layout->addWidget(btn_list);

  auto* btn_del = new QPushButton(QIcon(":/tree_icons/cross.png"), "", this);
  btn_del->setStyleSheet(g_button_style);
  btn_del->setFixedSize(12, 15);
  QObject::connect(btn_del, &QPushButton::clicked, this, [this] { clear(); });
  layout->addWidget(btn_del);

  QObject::connect(&EventHub::instance(), &EventHub::SelectionStealerInstalled, btn_pick, [this, btn_pick](void* source)
    {
      if (source != this)
        btn_pick->setChecked(false);
    });
}
