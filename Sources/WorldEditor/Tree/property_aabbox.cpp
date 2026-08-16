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
#include "ui_property_factory.h"
#include "base_entity_property_tree_item.h"
#include "spinbox_no_trailing.h"
#include "EventHub.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>

class Property_AABBox : public BaseEntityPropertyTreeItem
{
  class AABBox_SubItem : public BaseEntityPropertyTreeItem
  {
    public:
    AABBox_SubItem(Property_AABBox* parent, const QString& label)
      : BaseEntityPropertyTreeItem(parent)
      , m_label(label)
      , mp_parent(parent)
    {
    }

    QWidget* CreateEditor(QWidget* parent) override
    {
      if (mp_parent)
      {
        auto* editor = new QWidget(parent);
        auto* layout = new QHBoxLayout(editor);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);

        layout->addWidget(new QLabel("Min: ", editor));
        auto* min_spinbox = mp_parent->_CreateSubSpinbox(parent);
        layout->addWidget(min_spinbox);

        layout->addWidget(new QLabel("Max: ", editor));
        auto* max_spinbox = mp_parent->_CreateSubSpinbox(parent);
        layout->addWidget(max_spinbox);

        mp_parent->_SetupAxisEditors(this, min_spinbox, max_spinbox);

        return editor;
      }
      return nullptr;
    }

    QVariant data(int column, int role) const override
    {
      if (role != Qt::DisplayRole || column != 0)
        return QVariant();

      return m_label;
    }

    bool ValueIsCommonForAllEntities() const override
    {
      return true;
    }

    void SetFirstValueToAllEntities() override
    {
    }

  private:
    QString _GetTypeName() const override
    {
      return {};
    }

    CPropertyID* _GetProperty() const override
    {
      if (mp_parent)
        return mp_parent->_GetProperty();
      return nullptr;
    }

  private:
    const QString m_label;
    QPointer<Property_AABBox> mp_parent;
  };

public:
  Property_AABBox(BasePropertyTreeItem* parent)
    : BaseEntityPropertyTreeItem(parent)
    , m_enabled(false)
  {
    appendChild(std::make_unique<AABBox_SubItem>(this, "X"));
    appendChild(std::make_unique<AABBox_SubItem>(this, "Y"));
    appendChild(std::make_unique<AABBox_SubItem>(this, "Z"));

    QObject::connect(&EventHub::instance(), &EventHub::PropertyChanged, this,
      [this](const std::set<CEntity_*>&, CPropertyID*, BasePropertyTreeItem* source)
      {
        if (source == this && !ValueIsCommonForAllEntities())
        {
          m_enabled = false;
          if (m_x_min)
            m_x_min->setEnabled(false);
          if (m_x_max)
            m_x_max->setEnabled(false);
          if (m_y_min)
            m_y_min->setEnabled(false);
          if (m_y_max)
            m_y_max->setEnabled(false);
          if (m_z_min)
            m_z_min->setEnabled(false);
          if (m_z_max)
            m_z_max->setEnabled(false);
        }
      });
  }

  QWidget* CreateEditor(QWidget* parent) override
  {
    m_enabled = true;
    if (m_x_min)
      m_x_min->setEnabled(true);
    if (m_x_max)
      m_x_max->setEnabled(true);
    if (m_y_min)
      m_y_min->setEnabled(true);
    if (m_y_max)
      m_y_max->setEnabled(true);
    if (m_z_min)
      m_z_min->setEnabled(true);
    if (m_z_max)
      m_z_max->setEnabled(true);

    if (_AllEditorsCreated())
      _Setup();
    return nullptr;
  }

  IMPL_GENERIC_PROPERTY_FUNCTIONS(FLOATaabbox3D)

protected:
  using BaseEntityPropertyTreeItem::_GetProperty;

private:
  SpinBoxNoTrailing* _CreateSubSpinbox(QWidget* parent)
  {
    auto* spinbox = new SpinBoxNoTrailing(parent);
    spinbox->setRange(-FLT_MAX, FLT_MAX);
    spinbox->setDecimals(4);
    spinbox->setSingleStep(0.25);
    spinbox->setSuffix(" m");
    spinbox->setSingleStep(1.0);
    return spinbox;
  }

  void _SetupAxisEditors(AABBox_SubItem* source, SpinBoxNoTrailing* min_spin, SpinBoxNoTrailing* max_spin)
  {
    if (!m_enabled)
    {
      min_spin->setEnabled(false);
      max_spin->setEnabled(false);
    }
    if (source == child(0))
    {
      m_x_min = min_spin;
      m_x_max = max_spin;
    } else if (source == child(1)) {
      m_y_min = min_spin;
      m_y_max = max_spin;
    } else {
      m_z_min = min_spin;
      m_z_max = max_spin;
    }

    if (m_enabled && _AllEditorsCreated())
      _Setup();
  }

  bool _AllEditorsCreated() const
  {
    return m_x_min && m_x_max && m_y_min && m_y_max && m_z_min && m_z_max;
  }

  void _Setup()
  {
    QObject::disconnect(m_x_min_connection);
    QObject::disconnect(m_x_max_connection);
    QObject::disconnect(m_y_min_connection);
    QObject::disconnect(m_y_max_connection);
    QObject::disconnect(m_z_min_connection);
    QObject::disconnect(m_z_max_connection);

    const FLOATaabbox3D& bbox = _CurrentPropValue();
    m_x_min->setValue(bbox.Min()(1));
    m_x_max->setValue(bbox.Max()(1));
    m_y_min->setValue(bbox.Min()(2));
    m_y_max->setValue(bbox.Max()(2));
    m_z_min->setValue(bbox.Min()(3));
    m_z_max->setValue(bbox.Max()(3));

    auto on_edited = [this]
    {
      FLOAT3D new_min(min(m_x_min->value(), m_x_max->value()), min(m_y_min->value(), m_y_max->value()), min(m_z_min->value(), m_z_max->value()));
      FLOAT3D new_max(max(m_x_min->value(), m_x_max->value()), max(m_y_min->value(), m_y_max->value()), max(m_z_min->value(), m_z_max->value()));
      _WriteProperty(FLOATaabbox3D(new_min, new_max));
    };

    m_x_min_connection = QObject::connect(m_x_min.data(), &SpinBoxNoTrailing::editingFinished, this, on_edited);
    m_x_max_connection = QObject::connect(m_x_max.data(), &SpinBoxNoTrailing::editingFinished, this, on_edited);
    m_y_min_connection = QObject::connect(m_y_min.data(), &SpinBoxNoTrailing::editingFinished, this, on_edited);
    m_y_max_connection = QObject::connect(m_y_max.data(), &SpinBoxNoTrailing::editingFinished, this, on_edited);
    m_z_min_connection = QObject::connect(m_z_min.data(), &SpinBoxNoTrailing::editingFinished, this, on_edited);
    m_z_max_connection = QObject::connect(m_z_max.data(), &SpinBoxNoTrailing::editingFinished, this, on_edited);
  }

  private:
    bool m_enabled;
    QPointer<SpinBoxNoTrailing> m_x_min;
    QPointer<SpinBoxNoTrailing> m_x_max;
    QPointer<SpinBoxNoTrailing> m_y_min;
    QPointer<SpinBoxNoTrailing> m_y_max;
    QPointer<SpinBoxNoTrailing> m_z_min;
    QPointer<SpinBoxNoTrailing> m_z_max;
    QMetaObject::Connection m_x_min_connection;
    QMetaObject::Connection m_x_max_connection;
    QMetaObject::Connection m_y_min_connection;
    QMetaObject::Connection m_y_max_connection;
    QMetaObject::Connection m_z_min_connection;
    QMetaObject::Connection m_z_max_connection;
};

/*******************************************************************************************/
static UIPropertyFactory::Registrar g_registrar(CEntityProperty::PropertyType::EPT_FLOATAABBOX3D,
  [](BasePropertyTreeItem* parent)
  {
    return new Property_AABBox(parent);
  });
