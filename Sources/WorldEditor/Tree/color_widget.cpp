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
#include "color_widget.h"
#include "color_widget.h.moc"

#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

ColorWidget::ColorWidget(int hue, int sat, int val, int alp, QWidget* parent)
  : QWidget(parent)
  , m_hue(hue)
  , m_sat(sat)
  , m_val(val)
  , m_alp(alp)
{
  setAttribute(Qt::WA_Hover);
  setMouseTracking(true);
}

void ColorWidget::SetColor(QColor col)
{
  if (QColor::fromHsv(m_hue, m_sat, m_val, m_alp) != col)
  {
    int hue = -1;
    col.getHsv(&hue, &m_sat, &m_val, &m_alp);
    if (hue != -1)
      m_hue = qBound(0, hue, 359);
    update();
  }
}

int ColorWidget::Hue() const
{
  return m_hue;
}

int ColorWidget::Saturation() const
{
  return m_sat;
}

int ColorWidget::Value() const
{
  return m_val;
}

int ColorWidget::Alpha() const
{
  return m_alp;
}

void ColorWidget::mousePressEvent(QMouseEvent* event)
{
  if (event->buttons() == Qt::LeftButton)
  {
    if (rectColor().contains(event->pos()))
    {
      clicked();
    }
    else
    {
      m_edit_start_pos = QCursor::pos();
      if (rectHue().contains(event->pos()))
        m_mode = EMode::H;
      else if (rectSaturation().contains(event->pos()))
        m_mode = EMode::S;
      else if (rectValue().contains(event->pos()))
        m_mode = EMode::V;
      else
        m_mode = EMode::A;
      setCursor(Qt::BlankCursor);
      editingStarted();
    }
  }
}

void ColorWidget::mouseReleaseEvent(QMouseEvent* event)
{
  m_mode = EMode::Idle;
  setCursor(Qt::ArrowCursor);
  editingFinished();
}

void ColorWidget::mouseMoveEvent(QMouseEvent* event)
{
  if (m_mode == EMode::Idle)
    return;

  const auto delta = QCursor::pos().x() - m_edit_start_pos.x();
  QCursor::setPos(m_edit_start_pos);

  if (delta != 0)
  {
    switch (m_mode)
    {
    case EMode::H:
      m_hue += delta;
      break;
    case EMode::S:
      m_sat += delta;
      break;
    case EMode::V:
      m_val += delta;
      break;
    case EMode::A:
      m_alp += delta;
      break;
    default:
      break;
    }

    while (m_hue < 0)
      m_hue += 360;
    m_hue = m_hue % 360;
    m_sat = qBound(0, m_sat, 255);
    m_val = qBound(0, m_val, 255);
    m_alp = qBound(0, m_alp, 255);
    colorChanged();

    update();
  }
  showTooltip();
}

void ColorWidget::paintEvent(QPaintEvent*)
{
  QPainter painter(this);

  painter.setPen(Qt::black);
  for (const auto& [rect, col] : {
    std::make_pair(rectHue(), QColor::fromHsv(m_hue, 255, 255)),
    std::make_pair(rectSaturation(), QColor::fromHsv(m_hue, m_sat, 255)),
    std::make_pair(rectValue(), QColor::fromHsv(0, 0, m_val)),
    std::make_pair(rectAlpha(), QColor::fromHsv(0, 0, m_alp)),
    std::make_pair(rectColor(), QColor::fromHsv(m_hue, m_sat, m_val)) })
  {
    painter.fillRect(rect, col);
    painter.drawRect(rect);
  }
}

bool ColorWidget::event(QEvent* event)
{
  if (event->type() == QEvent::ToolTip)
  {
    showTooltip();
    return true;
  }
  return QWidget::event(event);
}

void ColorWidget::showTooltip() const
{
  const QColor col = QColor::fromHsv(m_hue, m_sat, m_val, m_alp);
  QString color_string = QString("RGBA: %1,%2,%3,%4   ").arg(col.red()).arg(col.green()).arg(col.blue()).arg(m_alp);
  color_string += QString("HSV: %1,%2,%3").arg(m_hue).arg(m_sat).arg(m_val);
  QToolTip::showText(QCursor::pos(), color_string);
}

QRect ColorWidget::rectHue() const
{
  return QRect(0, 0, width() / 6, height()-1);
}

QRect ColorWidget::rectSaturation() const
{
  auto rect_base = rectHue();
  return rect_base.translated(rect_base.width(), 0);
}

QRect ColorWidget::rectValue() const
{
  auto rect_base = rectSaturation();
  return rect_base.translated(rect_base.width(), 0);
}

QRect ColorWidget::rectAlpha() const
{
  auto rect_base = rectValue();
  return rect_base.translated(rect_base.width(), 0);
}

QRect ColorWidget::rectColor() const
{
  auto rect_base = rectAlpha();
  rect_base.translate(rect_base.width(), 0);
  rect_base.setRight(rect().right()-1);
  return rect_base;
}
