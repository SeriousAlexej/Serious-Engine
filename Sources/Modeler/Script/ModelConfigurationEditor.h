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

#ifndef MODEL_CONFIGURATION_EDITOR_H
#define MODEL_CONFIGURATION_EDITOR_H
#include "Script.h"

#include <QDialog>
#include <QDoubleSpinBox>

#include <array>
#include <memory>

class QListWidgetItem;

namespace Ui
{
class ModelConfigurationEditor;
}

class ModelConfigurationEditor : public QDialog
{
Q_OBJECT

public:
  explicit ModelConfigurationEditor(ModelScript& script, QWidget* parent = nullptr);
  ~ModelConfigurationEditor();

private:
  using TFileAndAnims = std::pair<CTFileName, std::vector<std::string>>;

  void _SortFrames();
  void _FillFrames();
  void _FillAnims();
  void _FillMips();
  void _FillSkeleton();
  void _FillRefSkeleton();
  void _FillSkelAnimFile();
  void _FillSkelBones();
  void _FillAnimSourceNames(const std::vector<std::string>& animNames);
  void _AddAnim();
  void _OnFrameUp();
  void _OnFrameDown();
  void _OnFrameDuplicate();
  void _OnFrameDelete();
  void _OnAnimUp();
  void _OnAnimDown();
  void _OnAnimDelete();
  void _OnMipUp();
  void _OnMipDown();
  void _OnMipDelete();
  void _OnPickMips();
  void _OnPickFrames();
  void _OnMipSelected(QListWidgetItem* current, QListWidgetItem* prev);
  void _OnAnimSelected(QListWidgetItem* current, QListWidgetItem* prev);
  void _OnFrameSelected(QListWidgetItem* current, QListWidgetItem* prev);
  void _FillAnimWidgets(ModelScript::Animation& anim);
  void _OnPickSkeleton(int index);
  void _OnPickSkelAnimFile(int index);
  void _OnPickOriginBone(int index);
  void _OnPickInterpolation(int index);
  void _OnPickRefSkeleton(int index);
  void _OnPickTransform();
  void _AnalyzeTransform();
  TFileAndAnims _PickFileWithAnimation(const CTFileName& defaultDir);
  std::vector<TFileAndAnims> _PickFilesWithAnimations(const CTFileName& defaultDir);
  std::vector<CTFileName> _PickFrames(const CTFileName& defaultDir);

private:
  std::unique_ptr<Ui::ModelConfigurationEditor> mp_ui;
  ModelScript& m_script;
  std::array<std::array<QDoubleSpinBox*, 3>, 3> m_uiTransform;
  std::vector<QMetaObject::Connection> m_animConnections;
  bool m_reverseSort = false;
};

#endif
