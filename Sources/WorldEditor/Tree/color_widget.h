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
#ifndef COLOR_WIDGET_H
#define COLOR_WIDGET_H

#include <QWidget>
#include <QColor>

class QEvent;
class QMouseEvent;
class QPaintEvent;

class ColorWidget : public QWidget
{
  Q_OBJECT
public:
  ColorWidget(int hue, int sat, int val, int alp, QWidget* parent);

  void SetColor(QColor col);
  int Hue() const;
  int Saturation() const;
  int Value() const;
  int Alpha() const;

  Q_SIGNAL void clicked();
  Q_SIGNAL void colorChanged();
  Q_SIGNAL void editingStarted();
  Q_SIGNAL void editingFinished();

private:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void paintEvent(QPaintEvent* event) override;
  bool event(QEvent* event) override;
  void showTooltip() const;

  QRect rectHue() const;
  QRect rectSaturation() const;
  QRect rectValue() const;
  QRect rectAlpha() const;
  QRect rectColor() const;

  enum class EMode
  {
    Idle,
    H,
    S,
    V,
    A
  };

private:
  int m_hue;
  int m_sat;
  int m_val;
  int m_alp;
  EMode  m_mode = EMode::Idle;
  QPoint m_edit_start_pos;
};

#endif
