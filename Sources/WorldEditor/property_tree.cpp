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
#include "property_tree.h"
#include "Tree/property_tree_model.h"
#include "Tree/property_item_delegate.h"
#include "EventHub.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QHeaderView>

BEGIN_MESSAGE_MAP(PropertyTree_MFC_Host, CDialogBar)
  ON_WM_SIZE()
  ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
END_MESSAGE_MAP()

const char* tree_stylesheet =
R"(
QTreeView::branch:has-children:!has-siblings:closed,
QTreeView::branch:closed:has-children:has-siblings {
        border-image: none;
        image: url(:/tree_icons/tree_closed.png);
}

QTreeView::branch:open:has-children:!has-siblings,
QTreeView::branch:open:has-children:has-siblings  {
        border-image: none;
        image: url(:/tree_icons/tree_opened.png);
}
)";

class PropertyTree_MFC_Host::_Impl
{
public:
  _Impl()
  {
  }

  void CreateWidget(CWnd* parent)
  {
    if (mp_winWidget)
      return;

    mp_winWidget = std::make_unique<QWinWidget>(parent, nullptr, Qt::WindowFlags{});

    mp_tree_view = new QTreeView(mp_winWidget.get());
    mp_tree_model = new PropertyTreeModel(mp_winWidget.get());
    auto* item_delegate = new PropertyItemDelegate(mp_tree_view, mp_winWidget.get());
    mp_tree_view->setModel(mp_tree_model);
    mp_tree_view->setItemDelegate(item_delegate);
    mp_tree_view->setUniformRowHeights(true);
    mp_tree_view->setStyleSheet(tree_stylesheet);
    mp_tree_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mp_tree_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    mp_tree_view->setSelectionMode(QAbstractItemView::SingleSelection);

    auto* vertical_layout = new QVBoxLayout(mp_winWidget.get());
    vertical_layout->addWidget(mp_tree_view);
    vertical_layout->setContentsMargins(0, 0, 0, 0);
    mp_winWidget->move(8, 8);
    mp_winWidget->show();
    mp_winWidget->setEnabled(false);

    char* buffer = nullptr;
    UINT buffer_size = 0;
    BOOL result = AfxGetApp()->GetProfileBinary(_T("General"), _T("Property Tree Header"), (LPBYTE*)&buffer, &buffer_size);
    if (result != 0 && buffer_size > 0 && buffer)
    {
      QByteArray buffer_qt(buffer, buffer_size);
      delete[] buffer;
      mp_tree_view->header()->restoreState(buffer_qt);
    }

    QObject::connect(mp_tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, mp_tree_view, []
      {
        CWorldEditorDoc* pDoc = theApp.GetDocument();
        pDoc->UpdateAllViews(NULL);
      });

    QObject::connect(mp_tree_model, &PropertyTreeModel::dataChanged, mp_winWidget.get(), [this]
      (const QModelIndex& top_left, const QModelIndex& bottom_right, const QVector<int>&)
      {
        if (top_left.parent() == bottom_right.parent())
        {
          RemoveIndexWidgets(top_left.parent(), top_left.row(), bottom_right.row());
          CreateIndexWidgets(top_left.parent(), top_left.row(), bottom_right.row());
        } else {
          RemoveIndexWidgets(QModelIndex(), 0, mp_tree_model->rowCount() - 1);
          CreateIndexWidgets(QModelIndex(), 0, mp_tree_model->rowCount() - 1);
        }
        mp_tree_view->update();
      });

    QObject::connect(mp_tree_model, &PropertyTreeModel::rowsInserted, mp_winWidget.get(), [this]
      (const QModelIndex& parent, int row_start, int row_end)
      {
        CreateIndexWidgets(parent, row_start, row_end);
        mp_tree_view->update();
      });

    QObject::connect(mp_tree_model, &PropertyTreeModel::rowsRemoved, mp_winWidget.get(), [this]
      (const QModelIndex& parent, int row_start, int row_end)
      {
        RemoveIndexWidgets(parent, row_start, row_end);
        mp_tree_view->update();
      });

    QObject::connect(mp_tree_model, &PropertyTreeModel::modelReset, mp_winWidget.get(), [this]
      {
        RemoveIndexWidgets(QModelIndex(), 0, mp_tree_model->rowCount() - 1);
        CreateIndexWidgets(QModelIndex(), 0, mp_tree_model->rowCount() - 1);
        mp_tree_view->update();
      });

    QObject::connect(&EventHub::instance(), &EventHub::CurrentEntitySelectionChanged, mp_tree_view, [this]
      (const std::set<CEntity_*>& new_selection)
      {
        if (mp_tree_model->rowCount() > 0)
        {
          if (auto* prev_property = GetSelectedProperty())
            mp_last_selected_property = std::make_unique<CPropertyID>(prev_property->pid_strName, prev_property->pid_eptType, CEntityPropertyPtr{}, CAnimDataPtr{});
          else
            mp_last_selected_property.reset();
        }

        mp_tree_model->Fill(new_selection);
        mp_winWidget->setEnabled(!new_selection.empty());
        if (!new_selection.empty())
          mp_tree_view->setExpanded(mp_tree_model->index(0, 0), true);

        if (!mp_last_selected_property)
          return;
        auto index_of_same_property = mp_tree_model->FindProperty(*mp_last_selected_property);
        if (index_of_same_property.isValid())
          mp_tree_view->selectionModel()->select(index_of_same_property, QItemSelectionModel::Rows | QItemSelectionModel::Select);
      });

    QObject::connect(mp_tree_view, &QTreeView::expanded, mp_tree_model, [this]
      (const QModelIndex& index)
      {
        mp_tree_model->EnsureSubtreeIsFilled(index);
      });

    QObject::connect(&EventHub::instance(), &EventHub::EntityPicked, mp_tree_view, [this]
      (CEntity_* picked_entity)
      {
        mp_tree_model->OnEntityPicked(picked_entity, mp_tree_view->selectionModel()->selectedRows());
      });
  }

  void SaveState() const
  {
    if (!mp_winWidget)
      return;
    QByteArray buffer = mp_tree_view->header()->saveState();
    AfxGetApp()->WriteProfileBinary(_T("General"), _T("Property Tree Header"), (LPBYTE)buffer.data(), buffer.size());
  }

  QWinWidget* WinWidget()
  {
    return mp_winWidget.get();
  }

  CPropertyID* GetSelectedProperty() const
  {
    if (!mp_tree_view || !mp_tree_model)
      return nullptr;

    return mp_tree_model->GetSelectedProperty(mp_tree_view->selectionModel()->selectedIndexes());
  }

private:
  void CreateIndexWidgets(const QModelIndex& parent, int start_row, int end_row)
  {
    for (int row = start_row; row <= end_row; ++row)
    {
      if (QModelIndex index = mp_tree_model->index(row, 1, parent); index.isValid())
      {
        auto* index_widget = mp_tree_model->CreateEditor(index, mp_tree_view);
        if (index_widget)
          mp_tree_view->setIndexWidget(index, index_widget);
      }

      if (QModelIndex index = mp_tree_model->index(row, 0, parent); index.isValid())
        CreateIndexWidgets(index, 0, mp_tree_model->rowCount(index) - 1);
    }
  }

  void RemoveIndexWidgets(const QModelIndex& parent, int start_row, int end_row)
  {
    for (int row = start_row; row <= end_row; ++row)
    {
      if (QModelIndex index = mp_tree_model->index(row, 1, parent); index.isValid())
        mp_tree_view->setIndexWidget(index, nullptr);

      if (QModelIndex index = mp_tree_model->index(row, 0, parent); index.isValid())
        RemoveIndexWidgets(index, 0, mp_tree_model->rowCount(index) - 1);
    }
  }

private:
  std::unique_ptr<CPropertyID> mp_last_selected_property;
  std::unique_ptr<QWinWidget>  mp_winWidget;
  QTreeView*                   mp_tree_view = nullptr;
  PropertyTreeModel*           mp_tree_model = nullptr;
};

PropertyTree_MFC_Host::PropertyTree_MFC_Host()
  : mp_impl(std::make_unique<_Impl>())
{
}

PropertyTree_MFC_Host::~PropertyTree_MFC_Host()
{
}

BOOL PropertyTree_MFC_Host::Create(CWnd* pParentWnd, UINT nIDTemplate, UINT nStyle, UINT nID)
{
  auto result = CDialogBar::Create(pParentWnd, nIDTemplate, nStyle, nID);
  m_Size = m_sizeDefault;
  return result;
}

void PropertyTree_MFC_Host::OnSize(UINT, int cx, int cy)
{
  if (mp_impl && mp_impl->WinWidget())
    mp_impl->WinWidget()->resize(cx - 16, cy - 16);
}

LONG PropertyTree_MFC_Host::OnInitDialog(UINT wParam, LONG lParam)
{
  BOOL bRet = HandleInitDialog(wParam, lParam);

  if (!UpdateData(FALSE))
    TRACE0("Warning: UpdateData failed during dialog init.\n");

  mp_impl->CreateWidget(this);
  return bRet;
}

CSize PropertyTree_MFC_Host::CalcDynamicLayout(int nLength, DWORD dwMode)
{
  // Return default if it is being docked or floated
  if ((dwMode & LM_VERTDOCK) || (dwMode & LM_HORZDOCK))
  {
    if (dwMode & LM_STRETCH) // if not docked stretch to fit
      return CSize((dwMode & LM_HORZ) ? 32767 : m_Size.cx,
                   (dwMode & LM_HORZ) ? m_Size.cy : 32767);
    else
      return m_Size;
  }
  if (dwMode & LM_MRUWIDTH)
    return m_Size;
  // In all other cases, accept the dynamic length
  if (dwMode & LM_LENGTHY)
    return CSize(m_Size.cx, m_Size.cy = nLength);
  else
    return CSize(m_Size.cx = nLength, m_Size.cy);
}

bool PropertyTree_MFC_Host::IsUnderMouse() const
{
  if (!mp_impl || !mp_impl->WinWidget() || !mp_impl->WinWidget()->isEnabled())
    return false;

  return mp_impl->WinWidget()->underMouse();
}

CPropertyID* PropertyTree_MFC_Host::GetSelectedProperty() const
{
  if (!mp_impl)
    return nullptr;
  return mp_impl->GetSelectedProperty();
}

void PropertyTree_MFC_Host::SaveState() const
{
  if (mp_impl)
    mp_impl->SaveState();
}

void PropertyTree_MFC_Host::Reset()
{
  mp_impl.reset();
}
