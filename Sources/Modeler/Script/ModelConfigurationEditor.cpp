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

#include "StdAfx.h"
#include "ModelConfigurationEditor.h"
#include "ModelConfigurationEditor.ui.h"
#include "ModelConfigurationEditor.h.moc"
#include "AnglePicker.h"

#include <EngineGUI/ImportedMesh.h>
#include <EngineGUI/ImportedSkeleton.h>
#include <EngineGUI/ImportedSkeletalAnimation.h>

#include <QMessageBox>
#include <QMenu>

static constexpr int g_label = 8800;
static constexpr int g_none = 42;
static constexpr int g_browse = 1337;

ModelConfigurationEditor::ModelConfigurationEditor(ModelScript& script, QWidget* parent)
  : QDialog(parent)
  , mp_ui(std::make_unique<Ui::ModelConfigurationEditor>())
  , m_script(script)
{
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  mp_ui->setupUi(this);
  mp_ui->stackedWidget->setCurrentWidget(mp_ui->pageEmpty);

  m_uiTransform = {
    std::array<QDoubleSpinBox*,3>{ mp_ui->t11, mp_ui->t12, mp_ui->t13 },
    std::array<QDoubleSpinBox*,3>{ mp_ui->t21, mp_ui->t22, mp_ui->t23 },
    std::array<QDoubleSpinBox*,3>{ mp_ui->t31, mp_ui->t32, mp_ui->t33 }
  };

  _FillAnims();
  _FillSkeleton();
  _FillMips();
  connect(mp_ui->comboUVMap, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) { m_script.m_defaultUVChannel = i; });
  connect(mp_ui->comboSkeleton, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModelConfigurationEditor::_OnPickSkeleton);
  connect(mp_ui->listFrames, &QListWidget::currentItemChanged, this, &ModelConfigurationEditor::_OnFrameSelected);
  connect(mp_ui->listAnims, &QListWidget::currentItemChanged, this, &ModelConfigurationEditor::_OnAnimSelected);
  connect(mp_ui->listMips, &QListWidget::currentItemChanged, this, &ModelConfigurationEditor::_OnMipSelected);
  connect(mp_ui->buttonPickMips, &QPushButton::clicked, this, &ModelConfigurationEditor::_OnPickMips);
  connect(mp_ui->buttonPickFrames, &QPushButton::clicked, this, &ModelConfigurationEditor::_OnPickFrames);
  connect(mp_ui->buttonMipUp, &QPushButton::clicked, this, &ModelConfigurationEditor::_OnMipUp);
  connect(mp_ui->buttonMipDown, &QPushButton::clicked, this, &ModelConfigurationEditor::_OnMipDown);
  connect(mp_ui->buttonMipDelete, &QPushButton::clicked, this, &ModelConfigurationEditor::_OnMipDelete);
  connect(mp_ui->buttonPickTransform, &QPushButton::clicked, this, &ModelConfigurationEditor::_OnPickTransform);
  connect(mp_ui->buttonAnimAdd, &QPushButton::clicked, this, &ModelConfigurationEditor::_AddAnim);
  connect(mp_ui->buttonAnimUp, &QPushButton::clicked, this, &ModelConfigurationEditor::_OnAnimUp);
  connect(mp_ui->buttonAnimDown, &QPushButton::clicked, this, &ModelConfigurationEditor::_OnAnimDown);
  connect(mp_ui->buttonAnimDelete, &QPushButton::clicked, this, &ModelConfigurationEditor::_OnAnimDelete);
  connect(mp_ui->buttonFrameUp, &QPushButton::clicked, this, &ModelConfigurationEditor::_OnFrameUp);
  connect(mp_ui->buttonFrameDown, &QPushButton::clicked, this, &ModelConfigurationEditor::_OnFrameDown);
  connect(mp_ui->buttonFrameDuplicate, &QPushButton::clicked, this, &ModelConfigurationEditor::_OnFrameDuplicate);
  connect(mp_ui->buttonFrameDelete, &QPushButton::clicked, this, &ModelConfigurationEditor::_OnFrameDelete);
  connect(mp_ui->buttonSortFrames, &QPushButton::clicked, this, &ModelConfigurationEditor::_SortFrames);
  connect(mp_ui->confirmationButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(mp_ui->confirmationButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  for (auto* p_combo : { mp_ui->comboBoneTriangles, mp_ui->comboHighQuality, mp_ui->comboStretchDetail })
  {
    p_combo->addItem("Yes", true);
    p_combo->addItem("No", false);
  }
  mp_ui->comboFlat->addItem("No", static_cast<int>(ModelScript::Flat::No));
  mp_ui->comboFlat->addItem("Half", static_cast<int>(ModelScript::Flat::Half));
  mp_ui->comboFlat->addItem("Full", static_cast<int>(ModelScript::Flat::Full));

  mp_ui->comboBoneTriangles->setCurrentIndex(mp_ui->comboBoneTriangles->findData(m_script.m_boneTriangles));
  mp_ui->comboHighQuality->setCurrentIndex(mp_ui->comboHighQuality->findData(m_script.m_highQuality));
  mp_ui->comboStretchDetail->setCurrentIndex(mp_ui->comboStretchDetail->findData(m_script.m_stretchDetail));
  mp_ui->comboFlat->setCurrentIndex(mp_ui->comboFlat->findData(static_cast<int>(m_script.m_flat)));
  mp_ui->spinboxSize->setValue(m_script.m_scale);
  mp_ui->spinboxTextureSize1->setValue(m_script.m_textureScale(1));
  mp_ui->spinboxTextureSize2->setValue(m_script.m_textureScale(2));
  mp_ui->spinboxMaxShadow->setValue(m_script.m_maxShadow);
  for (int row = 1; row <= 3; ++row)
    for (int col = 1; col <= 3; ++col)
    {
      m_uiTransform[row - 1][col - 1]->setValue(m_script.m_transformation(row, col));
      connect(m_uiTransform[row - 1][col - 1], QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ModelConfigurationEditor::_AnalyzeTransform);
    }
  _AnalyzeTransform();

  connect(mp_ui->comboFlat, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
    [this] { m_script.m_flat = static_cast<ModelScript::Flat>(mp_ui->comboFlat->currentData().toInt()); });
  connect(mp_ui->comboBoneTriangles, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
    [this] { m_script.m_boneTriangles = mp_ui->comboBoneTriangles->currentData().toBool(); });
  connect(mp_ui->comboHighQuality, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
    [this] { m_script.m_highQuality = mp_ui->comboHighQuality->currentData().toBool(); });
  connect(mp_ui->comboStretchDetail, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
    [this] { m_script.m_stretchDetail = mp_ui->comboStretchDetail->currentData().toBool(); });
  connect(mp_ui->spinboxSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
    [this](double val) { m_script.m_scale = val; });
  connect(mp_ui->spinboxTextureSize1, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
    [this](double val) { m_script.m_textureScale(1) = val; });
  connect(mp_ui->spinboxTextureSize2, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
    [this](double val) { m_script.m_textureScale(2) = val; });
  connect(mp_ui->spinboxMaxShadow, QOverload<int>::of(&QSpinBox::valueChanged), this,
    [this](int val) { m_script.m_maxShadow = val; });
}

ModelConfigurationEditor::~ModelConfigurationEditor()
{
}

void ModelConfigurationEditor::_SortFrames()
{
  auto& currAnim = m_script.m_animations[mp_ui->listAnims->currentRow()];

  std::sort(currAnim.m_frames.begin(), currAnim.m_frames.end(), [this](const CTFileName& lhs, const CTFileName& rhs)
    {
      std::string lhsFile = static_cast<const char*>(lhs.FileName());
      std::string rhsFile = static_cast<const char*>(rhs.FileName());
      for (auto* p_str : { &lhsFile, &rhsFile })
        std::transform(p_str->begin(), p_str->end(), p_str->begin(), [](unsigned char c) { return std::tolower(c); });
      if (m_reverseSort)
        std::swap(lhsFile, rhsFile);
      return std::lexicographical_compare(lhsFile.begin(), lhsFile.end(), rhsFile.begin(), rhsFile.end());
    });

  m_reverseSort = !m_reverseSort;
  _FillFrames();
}

void ModelConfigurationEditor::_FillFrames()
{
  auto& currAnim = m_script.m_animations[mp_ui->listAnims->currentRow()];
  mp_ui->listFrames->clear();
  for (const auto& frame : currAnim.m_frames)
    mp_ui->listFrames->addItem(static_cast<const char*>(frame));
}

void ModelConfigurationEditor::_FillAnims()
{
  mp_ui->listAnims->clear();
  for (const auto& anim : m_script.m_animations)
    mp_ui->listAnims->addItem(QString::fromStdString(anim.m_name));
}

void ModelConfigurationEditor::_FillMips()
{
  mp_ui->listMips->clear();
  for (const auto& mip : m_script.m_mipModels)
    mp_ui->listMips->addItem(static_cast<const char*>(mip));

  if (!m_script.m_skeleton.has_value())
    _FillSkeleton();

  const size_t numUVs = ImportedMesh::GetUVChannelCount(m_script.m_mipModels.front());
  QSignalBlocker block(mp_ui->comboUVMap);
  mp_ui->comboUVMap->clear();
  for (size_t i = 0; i < numUVs; ++i)
    mp_ui->comboUVMap->addItem(QString::number(i));
  if (m_script.m_defaultUVChannel >= numUVs)
    m_script.m_defaultUVChannel = 0;
  mp_ui->comboUVMap->setCurrentIndex(m_script.m_defaultUVChannel);
}

void ModelConfigurationEditor::_FillSkeleton()
{
  const CTFileName* p_skeleton = &m_script.m_mipModels.front();
  if (m_script.m_skeleton.has_value())
    p_skeleton = &(*m_script.m_skeleton);
  QString label = QString(static_cast<const char*>(p_skeleton->FileName())) + static_cast<const char*>(p_skeleton->FileExt());
  if (!m_script.m_skeleton.has_value())
    label = "(from mip 0, " + label + ')';
  QSignalBlocker block(mp_ui->comboSkeleton);
  mp_ui->comboSkeleton->clear();
  mp_ui->comboSkeleton->addItem(label, g_label);
  mp_ui->comboSkeleton->setToolTip(static_cast<const char*>(*p_skeleton));
  mp_ui->comboSkeleton->addItem("(browse)", g_browse);
  mp_ui->comboSkeleton->addItem("(none)", g_none);
  mp_ui->comboSkeleton->setCurrentIndex(mp_ui->comboSkeleton->findData(g_label));
}

void ModelConfigurationEditor::_FillRefSkeleton()
{
  const auto& currAnim = m_script.m_animations[mp_ui->listAnims->currentRow()];
  QSignalBlocker block(mp_ui->comboReferenceSkeleton);
  mp_ui->comboReferenceSkeleton->clear();
  if (currAnim.m_optRefSkeleton.has_value())
  {
    const auto& skel = *currAnim.m_optRefSkeleton;
    mp_ui->comboReferenceSkeleton->addItem(QString(static_cast<const char*>(skel.FileName())) + static_cast<const char*>(skel.FileExt()), g_label);
    mp_ui->comboReferenceSkeleton->setToolTip(static_cast<const char*>(skel));
  }
  mp_ui->comboReferenceSkeleton->addItem("(browse)", g_browse);
  mp_ui->comboReferenceSkeleton->addItem("(none)", g_none);
  mp_ui->comboReferenceSkeleton->setCurrentIndex(mp_ui->comboReferenceSkeleton->findData(currAnim.m_optRefSkeleton.has_value() ? g_label : g_none));
}

void ModelConfigurationEditor::_FillSkelAnimFile()
{
  const auto& currAnim = m_script.m_animations[mp_ui->listAnims->currentRow()];
  const auto& src = currAnim.m_frames.front();
  QSignalBlocker block(mp_ui->comboSourceFile);
  mp_ui->comboSourceFile->clear();
  mp_ui->comboSourceFile->addItem(QString(static_cast<const char*>(src.FileName())) + static_cast<const char*>(src.FileExt()), g_label);
  mp_ui->comboSourceFile->setToolTip(static_cast<const char*>(src));
  mp_ui->comboSourceFile->addItem("(browse)", g_browse);
  mp_ui->comboSourceFile->setCurrentIndex(mp_ui->comboSourceFile->findData(g_label));
}

void ModelConfigurationEditor::_FillAnimSourceNames(const std::vector<std::string>& animNames)
{
  QSignalBlocker block(mp_ui->comboAnimSourceName);
  mp_ui->comboAnimSourceName->clear();
  for (const auto& name : animNames)
    mp_ui->comboAnimSourceName->addItem(QString::fromStdString(name), QString::fromStdString(name).toUpper());
  if (!animNames.empty())
    mp_ui->comboAnimSourceName->setCurrentIndex(0);
  mp_ui->comboAnimSourceName->setEnabled(animNames.size() > 1);
}

void ModelConfigurationEditor::_AddAnim()
{
  QMenu menu;
  const auto* p_vertex = menu.addAction("Vertex");
  const auto* p_skeletal = menu.addAction("Skeletal");
  auto* p_action = menu.exec(mp_ui->buttonAnimAdd->mapToGlobal({ 0, 0 }));

  const size_t prevAnimCount = m_script.m_animations.size();
  if (p_action == p_vertex)
  {
    const auto dir = m_script.m_mipModels.front().FileDir();
    const auto frames = _PickFrames(dir);
    if (frames.empty())
      return;

    m_script.m_animations.emplace_back();
    auto& anim = m_script.m_animations.back();
    anim.m_type = ModelScript::Animation::Type::Vertex;
    anim.m_name = "Animation " + std::to_string(m_script.m_animations.size());
    anim.m_optDuration = frames.size() * 0.1;
    anim.m_frames = frames;
  }
  else if (p_action == p_skeletal)
  {
    const auto dir = m_script.m_mipModels.front().FileDir();
    const auto filesAndAnims = _PickFilesWithAnimations(dir);
    if (filesAndAnims.empty())
      return;

    for (const auto& [animFile, anims] : filesAndAnims)
    {
      if (anims.size() == 1)
      {
        m_script.m_animations.emplace_back();
        auto& anim = m_script.m_animations.back();
        anim.m_type = ModelScript::Animation::Type::Skeletal;
        if (anims.front().empty())
          anim.m_name = "Animation " + std::to_string(m_script.m_animations.size());
        else
          anim.m_name = anims.front();
        anim.m_frames.push_back(animFile);
      }
      else
      {
        for (const auto& animName : anims)
        {
          m_script.m_animations.emplace_back();
          auto& anim = m_script.m_animations.back();
          anim.m_type = ModelScript::Animation::Type::Skeletal;
          anim.m_name = animName;
          anim.m_customSourceName = animName;
          anim.m_frames.push_back(animFile);
        }
      }
    }
  }
  if (m_script.m_animations.size() > prevAnimCount)
  {
    _FillAnims();
    mp_ui->listAnims->setCurrentRow(prevAnimCount, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
  }
}

void ModelConfigurationEditor::_OnFrameUp()
{
  auto& currAnim = m_script.m_animations[mp_ui->listAnims->currentRow()];
  const int currItem = mp_ui->listFrames->currentRow();
  if (currItem < 1 || currItem >= static_cast<int>(currAnim.m_frames.size()))
    return;
  std::swap(currAnim.m_frames[currItem - 1], currAnim.m_frames[currItem]);
  _FillFrames();
  mp_ui->listFrames->setCurrentRow(currItem - 1, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
}

void ModelConfigurationEditor::_OnFrameDown()
{
  auto& currAnim = m_script.m_animations[mp_ui->listAnims->currentRow()];
  const int currItem = mp_ui->listFrames->currentRow();
  if (currItem < 0 || currItem + 1 >= static_cast<int>(currAnim.m_frames.size()))
    return;
  std::swap(currAnim.m_frames[currItem + 1], currAnim.m_frames[currItem]);
  _FillFrames();
  mp_ui->listFrames->setCurrentRow(currItem + 1, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
}

void ModelConfigurationEditor::_OnFrameDuplicate()
{
  auto& currAnim = m_script.m_animations[mp_ui->listAnims->currentRow()];
  const int currItem = mp_ui->listFrames->currentRow();
  if (currItem < 0 || currItem >= static_cast<int>(currAnim.m_frames.size()))
    return;
  currAnim.m_frames.insert(currAnim.m_frames.begin() + currItem + 1, currAnim.m_frames[currItem]);
  _FillFrames();
  mp_ui->listFrames->setCurrentRow(currItem, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
}

void ModelConfigurationEditor::_OnFrameDelete()
{
  auto& currAnim = m_script.m_animations[mp_ui->listAnims->currentRow()];
  const int currItem = mp_ui->listFrames->currentRow();
  if (currItem < 0 || currItem >= static_cast<int>(currAnim.m_frames.size()))
    return;
  currAnim.m_frames.erase(currAnim.m_frames.begin() + currItem);
  _FillFrames();
  mp_ui->listFrames->setCurrentRow(currItem < static_cast<int>(currAnim.m_frames.size()) ? currItem : currItem - 1, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
}

void ModelConfigurationEditor::_OnAnimUp()
{
  const int currItem = mp_ui->listAnims->currentRow();
  if (currItem < 1 || currItem >= static_cast<int>(m_script.m_animations.size()))
    return;
  std::swap(m_script.m_animations[currItem - 1], m_script.m_animations[currItem]);
  _FillAnims();
  mp_ui->listAnims->setCurrentRow(currItem - 1, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
}

void ModelConfigurationEditor::_OnAnimDown()
{
  const int currItem = mp_ui->listAnims->currentRow();
  if (currItem < 0 || currItem + 1 >= static_cast<int>(m_script.m_animations.size()))
    return;
  std::swap(m_script.m_animations[currItem + 1], m_script.m_animations[currItem]);
  _FillAnims();
  mp_ui->listAnims->setCurrentRow(currItem + 1, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
}

void ModelConfigurationEditor::_OnAnimDelete()
{
  const int currItem = mp_ui->listAnims->currentRow();
  if (currItem < 0 || currItem >= static_cast<int>(m_script.m_animations.size()))
    return;
  m_script.m_animations.erase(m_script.m_animations.begin() + currItem);
  _FillAnims();
  mp_ui->listAnims->setCurrentRow(currItem < static_cast<int>(m_script.m_animations.size()) ? currItem : currItem - 1, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
}

void ModelConfigurationEditor::_OnMipUp()
{
  const int currItem = mp_ui->listMips->currentRow();
  if (currItem < 1 || currItem >= static_cast<int>(m_script.m_mipModels.size()))
    return;
  std::swap(m_script.m_mipModels[currItem - 1], m_script.m_mipModels[currItem]);
  _FillMips();
  mp_ui->listMips->setCurrentRow(currItem - 1, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
}

void ModelConfigurationEditor::_OnMipDown()
{
  const int currItem = mp_ui->listMips->currentRow();
  if (currItem < 0 || currItem + 1 >= static_cast<int>(m_script.m_mipModels.size()))
    return;
  std::swap(m_script.m_mipModels[currItem + 1], m_script.m_mipModels[currItem]);
  _FillMips();
  mp_ui->listMips->setCurrentRow(currItem + 1, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
}

void ModelConfigurationEditor::_OnMipDelete()
{
  const int currItem = mp_ui->listMips->currentRow();
  if (currItem < 0 || currItem >= static_cast<int>(m_script.m_mipModels.size()))
    return;
  m_script.m_mipModels.erase(m_script.m_mipModels.begin() + currItem);
  _FillMips();
  mp_ui->listMips->setCurrentRow(currItem < static_cast<int>(m_script.m_mipModels.size()) ? currItem : currItem - 1, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
}

void ModelConfigurationEditor::_OnPickMips()
{
  const auto dir = m_script.m_mipModels.front().FileDir();
  const auto newMips = _PickFrames(dir);
  if (newMips.empty())
    return;
  m_script.m_mipModels = newMips;
  _FillMips();
}

void ModelConfigurationEditor::_OnPickFrames()
{
  auto& currAnim = m_script.m_animations[mp_ui->listAnims->currentRow()];
  const auto dir = currAnim.m_frames.front().FileDir();
  const auto newFrames = _PickFrames(dir);
  if (newFrames.empty())
    return;
  currAnim.m_frames = newFrames;
  _FillFrames();
}

void ModelConfigurationEditor::_OnMipSelected(QListWidgetItem* current, QListWidgetItem* prev)
{
  Q_UNUSED(prev);
  const bool has_selection = current != nullptr;
  mp_ui->buttonMipUp->setEnabled(has_selection && mp_ui->listMips->row(current) > 0);
  mp_ui->buttonMipDown->setEnabled(has_selection && mp_ui->listMips->row(current) + 1 < static_cast<int>(m_script.m_mipModels.size()));
  mp_ui->buttonMipDelete->setEnabled(has_selection && m_script.m_mipModels.size() > 1);
}

void ModelConfigurationEditor::_OnAnimSelected(QListWidgetItem* current, QListWidgetItem* prev)
{
  Q_UNUSED(prev);
  for (auto& connection : m_animConnections)
    disconnect(connection);
  m_animConnections.clear();

  const bool has_selection = current != nullptr;
  mp_ui->buttonAnimUp->setEnabled(has_selection && mp_ui->listAnims->row(current) > 0);
  mp_ui->buttonAnimDown->setEnabled(has_selection && mp_ui->listAnims->row(current) + 1 < static_cast<int>(m_script.m_animations.size()));
  mp_ui->buttonAnimDelete->setEnabled(has_selection && m_script.m_animations.size() > 1);
  if (!has_selection)
  {
    mp_ui->stackedWidget->setCurrentWidget(mp_ui->pageEmpty);
    return;
  }
  auto& currAnim = m_script.m_animations[mp_ui->listAnims->row(current)];
  mp_ui->stackedWidget->setCurrentWidget(currAnim.m_type == ModelScript::Animation::Type::Vertex ? mp_ui->pageVertex : mp_ui->pageSkeletal);
  _FillAnimWidgets(currAnim);
}

void ModelConfigurationEditor::_OnFrameSelected(QListWidgetItem* current, QListWidgetItem* prev)
{
  Q_UNUSED(prev);

  auto& currAnim = m_script.m_animations[mp_ui->listAnims->currentRow()];
  const bool has_selection = current != nullptr;
  mp_ui->buttonFrameUp->setEnabled(has_selection && mp_ui->listFrames->row(current) > 0);
  mp_ui->buttonFrameDown->setEnabled(has_selection && mp_ui->listFrames->row(current) + 1 < static_cast<int>(currAnim.m_frames.size()));
  mp_ui->buttonFrameDuplicate->setEnabled(has_selection);
  mp_ui->buttonFrameDelete->setEnabled(has_selection && currAnim.m_frames.size() > 1);
}

void ModelConfigurationEditor::_FillAnimWidgets(ModelScript::Animation& anim)
{
  auto* p_anim = &anim;
  const bool vertexAnim = anim.m_type == ModelScript::Animation::Type::Vertex;

  auto* p_nameEdit = (vertexAnim ? mp_ui->lineeditAnimNameV : mp_ui->lineeditAnimNameS);
  p_nameEdit->setText(QString::fromStdString(anim.m_name));

  _FillFrames();
  _FillSkelAnimFile();
  _FillAnimSourceNames(ImportedSkeletalAnimation::GetAnimationsInFile(anim.m_frames.front()));
  _FillRefSkeleton();

  if (anim.m_customSourceName.has_value())
    mp_ui->comboAnimSourceName->setCurrentIndex(mp_ui->comboAnimSourceName->findData(QString::fromStdString(*anim.m_customSourceName).toUpper()));
  mp_ui->cbDuration->setChecked(anim.m_optDuration.has_value());
  mp_ui->spinboxDurationS->setEnabled(anim.m_optDuration.has_value());
  auto* p_durationEdit = (vertexAnim ? mp_ui->spinboxDurationV : mp_ui->spinboxDurationS);
  if (anim.m_optDuration.has_value())
    p_durationEdit->setValue(*anim.m_optDuration);

  mp_ui->cbCustomFrameCount->setChecked(anim.m_optNumFrames.has_value());
  mp_ui->spinboxCustomFrameCount->setEnabled(anim.m_optNumFrames.has_value());
  if (anim.m_optNumFrames.has_value())
    mp_ui->spinboxCustomFrameCount->setValue(*anim.m_optNumFrames);

  m_animConnections.emplace_back(connect(mp_ui->comboAnimSourceName, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, p_anim](int index)
    { p_anim->m_customSourceName = mp_ui->comboAnimSourceName->itemData(index).toString().toStdString(); }));
  m_animConnections.emplace_back(connect(mp_ui->comboReferenceSkeleton, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModelConfigurationEditor::_OnPickRefSkeleton));
  m_animConnections.emplace_back(connect(mp_ui->comboSourceFile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModelConfigurationEditor::_OnPickSkelAnimFile));
  m_animConnections.emplace_back(connect(p_nameEdit, &QLineEdit::textEdited, this, [this, p_anim](const QString& n)
    {
      auto newName = n.trimmed();
      if (!newName.isEmpty())
      {
        p_anim->m_name = newName.toStdString();
        mp_ui->listAnims->currentItem()->setText(newName);
      }
    }));
  m_animConnections.emplace_back(connect(p_nameEdit, &QLineEdit::editingFinished, this, [p_nameEdit, p_anim]
    { p_nameEdit->setText(QString::fromStdString(p_anim->m_name)); }));

  m_animConnections.emplace_back(connect(mp_ui->cbDuration, &QCheckBox::clicked, this, [this, p_anim](bool b)
    {
      mp_ui->spinboxDurationS->setEnabled(b);
      if (b)
        p_anim->m_optDuration = mp_ui->spinboxDurationS->value();
      else
        p_anim->m_optDuration = std::nullopt;
    }));
  m_animConnections.emplace_back(connect(p_durationEdit, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [p_anim](double val)
    { p_anim->m_optDuration = val; }));
  
  m_animConnections.emplace_back(connect(mp_ui->cbCustomFrameCount, &QCheckBox::clicked, this, [this, p_anim](bool b)
    {
      mp_ui->spinboxCustomFrameCount->setEnabled(b);
      if (b)
        p_anim->m_optNumFrames = mp_ui->spinboxCustomFrameCount->value();
      else
        p_anim->m_optNumFrames = std::nullopt;
    }));
  m_animConnections.emplace_back(connect(mp_ui->spinboxCustomFrameCount, QOverload<int>::of(&QSpinBox::valueChanged), this, [p_anim](int val)
    { p_anim->m_optNumFrames = val; }));
}

void ModelConfigurationEditor::_OnPickSkeleton(int index)
{
  const int itemData = mp_ui->comboSkeleton->itemData(index).toInt();
  if (itemData == g_label)
  {
    return;
  }
  else if (itemData == g_none)
  {
    m_script.m_skeleton = std::nullopt;
  }
  else if (itemData == g_browse)
  {
    const auto* p_skeleton = &m_script.m_mipModels.front();
    if (m_script.m_skeleton.has_value())
      p_skeleton = &(*m_script.m_skeleton);
    const auto currSkelDir = p_skeleton->FileDir();

    auto fileFilter = _EngineGUI.GetListOf3DFormats();
    const auto newSkeleton = _EngineGUI.FileRequester("Select Skeleton",
      fileFilter.data(),
      nullptr, currSkelDir);

    if (newSkeleton != "")
    {
      if (!ImportedSkeleton::ContainsSkeleton(newSkeleton))
        QMessageBox::warning(this, "Warning", "Selected file contains no skeleton!");
      else
        m_script.m_skeleton = newSkeleton;
    }
  }
  _FillSkeleton();
}

void ModelConfigurationEditor::_OnPickSkelAnimFile(int index)
{
  const int itemData = mp_ui->comboSourceFile->itemData(index).toInt();
  if (itemData == g_label)
    return;

  mp_ui->comboSourceFile->setCurrentIndex(mp_ui->comboSourceFile->findData(g_label));
  auto& currAnim = m_script.m_animations[mp_ui->listAnims->currentRow()];
  auto& src = currAnim.m_frames.front();
  const auto [newSrc, anims] = _PickFileWithAnimation(src.FileDir());

  if (newSrc == "")
    return;

  src = newSrc;
  _FillSkelAnimFile();
  _FillAnimSourceNames(anims);

  if (anims.size() <= 1)
    currAnim.m_customSourceName = std::nullopt;
  else
    currAnim.m_customSourceName = anims.front();
}

void ModelConfigurationEditor::_OnPickRefSkeleton(int index)
{
  const int itemData = mp_ui->comboReferenceSkeleton->itemData(index).toInt();
  auto& currAnim = m_script.m_animations[mp_ui->listAnims->currentRow()];
  if (itemData == g_label)
  {
    return;
  }
  else if (itemData == g_none)
  {
    currAnim.m_optRefSkeleton = std::nullopt;
  }
  else if (itemData == g_browse)
  {
    const auto* p_skelDir = &currAnim.m_frames.front();
    if (currAnim.m_optRefSkeleton.has_value())
      p_skelDir = &(*currAnim.m_optRefSkeleton);
    const auto currDir = p_skelDir->FileDir();

    auto fileFilter = _EngineGUI.GetListOf3DFormats();
    const auto newSkeleton = _EngineGUI.FileRequester("Select Skeleton",
      fileFilter.data(),
      nullptr, currDir);

    if (newSkeleton != "")
    {
      if (!ImportedSkeleton::ContainsSkeleton(newSkeleton))
        QMessageBox::warning(this, "Warning", "Selected file contains no skeleton!");
      else
        currAnim.m_optRefSkeleton = newSkeleton;
    }
  }
  _FillRefSkeleton();
}

void ModelConfigurationEditor::_OnPickTransform()
{
  AnglePicker anglePicker(this);
  if (anglePicker.exec() == QDialog::Rejected)
    return;
  MakeRotationMatrix(m_script.m_transformation, anglePicker.GetAngle());
  for (int row = 1; row <= 3; ++row)
    for (int col = 1; col <= 3; ++col)
    {
      QSignalBlocker block(m_uiTransform[row - 1][col - 1]);
      m_uiTransform[row - 1][col - 1]->setValue(m_script.m_transformation(row, col));
    }
  _AnalyzeTransform();
}

void ModelConfigurationEditor::_AnalyzeTransform()
{
  QString status("Not a rotation matrix");

  auto& mx = m_script.m_transformation;
  for (int row = 1; row <= 3; ++row)
    for (int col = 1; col <= 3; ++col)
      mx(row, col) = m_uiTransform[row - 1][col - 1]->value();

  // mx is a rotation matrix if det(mx) = 1, mxT = mx^-1 (ie mxT * mx = 1)
  if (std::abs(mx.Determinant() - 1.0f) < 0.001f)
  {
    FLOATmatrix3D mxt;
    for (int row = 1; row <= 3; ++row)
      for (int col = 1; col <= 3; ++col)
        mxt(row, col) = mx(col, row);
    mxt = mxt * mx;

    bool isRotationMatrix = true;
    for (int row = 1; row <= 3 && isRotationMatrix; ++row)
      for (int col = 1; col <= 3 && isRotationMatrix; ++col)
        isRotationMatrix = std::abs(mxt(row, col) - (row == col ? 1.0f : 0.0f)) < 0.001f;

    if (isRotationMatrix)
    {
      ANGLE3D euler;
      DecomposeRotationMatrixNoSnap(euler, mx);
      status = QString::fromLatin1("Rotation matrix: H=%1\u00B0 P=%2\u00B0 B=%3\u00B0").arg(euler(1)).arg(euler(2)).arg(euler(3));
    }
  }

  mp_ui->labelTransformDesc->setText(status);
}

ModelConfigurationEditor::TFileAndAnims ModelConfigurationEditor::_PickFileWithAnimation(const CTFileName& defaultDir)
{
  auto fileFilter = _EngineGUI.GetListOf3DFormats();
  const auto newSrc = _EngineGUI.FileRequester("Select animation file",
    fileFilter.data(),
    nullptr, defaultDir);

  if (newSrc == "")
    return { {}, {} };

  if (!ImportedSkeleton::ContainsSkeleton(newSrc))
  {
    QMessageBox::warning(this, "Warning", "Selected file contains no skeleton!");
    return { {}, {} };
  }
  const auto anims = ImportedSkeletalAnimation::GetAnimationsInFile(newSrc);
  if (anims.empty())
  {
    QMessageBox::warning(this, "Warning", "Selected file contains no animations!");
    return { {}, {} };
  }

  return { newSrc, anims };
}

std::vector<ModelConfigurationEditor::TFileAndAnims> ModelConfigurationEditor::_PickFilesWithAnimations(const CTFileName& defaultDir)
{
  CDynamicArray_CTFileName newAnimsDynArr;
  auto fileFilter = _EngineGUI.GetListOf3DFormats();
  _EngineGUI.FileRequester("Select animation file(s)",
    fileFilter.data(),
    nullptr, defaultDir, "", &newAnimsDynArr);

  if (newAnimsDynArr.Count() == 0)
    return {};

  std::vector<TFileAndAnims> filesAndAnims;
  FOREACHINDYNAMICARRAY(newAnimsDynArr, CTFileName, itFile)
  {
    const auto& animFile = itFile.Current();
    if (!ImportedSkeleton::ContainsSkeleton(*animFile))
    {
      QMessageBox::warning(this, "Warning", QString("'%1' contains no skeleton!").arg(static_cast<const char*>(*animFile)));
      continue;
    }
    const auto anims = ImportedSkeletalAnimation::GetAnimationsInFile(*animFile);
    if (anims.empty())
    {
      QMessageBox::warning(this, "Warning", QString("'%1' contains no animations!").arg(static_cast<const char*>(*animFile)));
      continue;
    }
    filesAndAnims.push_back({ *animFile, anims });
  }

  return filesAndAnims;
}

std::vector<CTFileName> ModelConfigurationEditor::_PickFrames(const CTFileName& defaultDir)
{
  CDynamicArray_CTFileName newFramesDynArr;
  auto fileFilter = _EngineGUI.GetListOf3DFormats();
  _EngineGUI.FileRequester("Select frames",
    fileFilter.data(),
    nullptr, defaultDir, "", &newFramesDynArr);

  if (newFramesDynArr.Count() == 0)
    return {};

  std::vector<CTFileName> newFrames;
  newFrames.reserve(newFramesDynArr.Count());
  FOREACHINDYNAMICARRAY(newFramesDynArr, CTFileName, itFrame)
  { newFrames.push_back(*itFrame.Current()); }

  if (newFrames.size() > 1 && std::any_of(newFrames.begin() + 1, newFrames.end(), [&](const CTFileName& f)
    { return f.FileDir() != newFrames.front().FileDir(); }))
  {
    QMessageBox::warning(this, "Warning", "All animation frames must be in common directory!");
    return {};
  }
  return newFrames;
}
