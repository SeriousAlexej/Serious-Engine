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

#include "StdH.h"

#include "ImportedSkeletalAnimation.h"

#include <SeriousEngineCppAPI/Base/Stream.h>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>

namespace
{
  aiAnimation* FindAnimation(const aiScene& scene, const std::string& animName)
  {
    if (scene.mNumAnimations == 1 && (strlen(scene.mAnimations[0]->mName.C_Str()) == 0 || animName.empty()))
      return scene.mAnimations[0];

    for (size_t animIndex = 0; animIndex < scene.mNumAnimations; ++animIndex)
    {
      auto& anim = *scene.mAnimations[animIndex];
      std::string currAnimName(anim.mName.C_Str());
      std::transform(currAnimName.begin(), currAnimName.end(), currAnimName.begin(),
        [](unsigned char c) { return std::toupper(c); });
      if (currAnimName == animName)
        return &anim;
    }

    return nullptr;
  }

  size_t GetNumFrames(const aiAnimation& anim)
  {
    size_t frames = 0;
    for (size_t nodeIndex = 0; nodeIndex < anim.mNumChannels; ++nodeIndex)
    {
      const auto& animNode = *anim.mChannels[nodeIndex];
      frames = std::max(frames, animNode.mNumPositionKeys);
      frames = std::max(frames, animNode.mNumRotationKeys);
      frames = std::max(frames, animNode.mNumScalingKeys);
    }
    return frames;
  }

  struct AnimKeys
  {
  public:
    BlenderFCurve::TAnimCurve m_curvePosX;
    BlenderFCurve::TAnimCurve m_curvePosY;
    BlenderFCurve::TAnimCurve m_curvePosZ;
    BlenderFCurve::TAnimCurve m_curveQuatW;
    BlenderFCurve::TAnimCurve m_curveQuatX;
    BlenderFCurve::TAnimCurve m_curveQuatY;
    BlenderFCurve::TAnimCurve m_curveQuatZ;
    BlenderFCurve::TAnimCurve m_curveScaleX;
    BlenderFCurve::TAnimCurve m_curveScaleY;
    BlenderFCurve::TAnimCurve m_curveScaleZ;

    FLOATmatrix4D GetTransformAtTime(double time, const BlenderFCurve::InterpolationMode interpolation) const
    {
      const aiVector3D position(
        BlenderFCurve::EvaluateCurveAtTime(m_curvePosX, time, interpolation),
        BlenderFCurve::EvaluateCurveAtTime(m_curvePosY, time, interpolation),
        BlenderFCurve::EvaluateCurveAtTime(m_curvePosZ, time, interpolation));
      float rotation[4];
      BlenderFCurve::EvaluateQuaternionAtTime(
        m_curveQuatW,
        m_curveQuatX,
        m_curveQuatY,
        m_curveQuatZ,
        time, interpolation, rotation);
      const aiVector3D scaling(
        BlenderFCurve::EvaluateCurveAtTime(m_curveScaleX, time, interpolation),
        BlenderFCurve::EvaluateCurveAtTime(m_curveScaleY, time, interpolation),
        BlenderFCurve::EvaluateCurveAtTime(m_curveScaleZ, time, interpolation));

      const aiMatrix4x4 transform(scaling, { rotation[0], rotation[1], rotation[2], rotation[3] }, position);

      FLOATmatrix4D result;
      for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
          result(row + 1, col + 1) = transform[row][col];
      return result;
    }
  };

  std::map<std::string, AnimKeys> GetBoneAnimKeys(const aiAnimation& anim, const BlenderFCurve::InterpolationMode interpolation)
  {
    std::map<std::string, AnimKeys> result;

    for (size_t nodeIndex = 0; nodeIndex < anim.mNumChannels; ++nodeIndex)
    {
      const auto& animNode = *anim.mChannels[nodeIndex];
      std::string boneName = animNode.mNodeName.C_Str();
      auto& animKeys = result[boneName];

      auto finalizeCurve = [interpolation](auto& curve)
        {
          std::ranges::sort(curve, std::less<>{}, [](const auto& keyframe) { return keyframe.vec[1][0]; });
          if (interpolation == BlenderFCurve::InterpolationMode::Bezier)
            BlenderFCurve::CalculateCurveHandles(curve);
        };

      animKeys.m_curvePosX.resize(animNode.mNumPositionKeys);
      animKeys.m_curvePosY.resize(animNode.mNumPositionKeys);
      animKeys.m_curvePosZ.resize(animNode.mNumPositionKeys);
      for (size_t i = 0; i < animNode.mNumPositionKeys; ++i)
      {
        const auto& key = animNode.mPositionKeys[i];
        auto& keyX = animKeys.m_curvePosX[i];
        auto& keyY = animKeys.m_curvePosY[i];
        auto& keyZ = animKeys.m_curvePosZ[i];
        keyX.vec[1][0] = key.mTime;
        keyX.vec[1][1] = key.mValue.x;
        keyY.vec[1][0] = key.mTime;
        keyY.vec[1][1] = key.mValue.y;
        keyZ.vec[1][0] = key.mTime;
        keyZ.vec[1][1] = key.mValue.z;
      }
      finalizeCurve(animKeys.m_curvePosX);
      finalizeCurve(animKeys.m_curvePosY);
      finalizeCurve(animKeys.m_curvePosZ);

      animKeys.m_curveQuatW.resize(animNode.mNumRotationKeys);
      animKeys.m_curveQuatX.resize(animNode.mNumRotationKeys);
      animKeys.m_curveQuatY.resize(animNode.mNumRotationKeys);
      animKeys.m_curveQuatZ.resize(animNode.mNumRotationKeys);
      for (size_t i = 0; i < animNode.mNumRotationKeys; ++i)
      {
        const auto& key = animNode.mRotationKeys[i];
        auto& keyW = animKeys.m_curveQuatW[i];
        auto& keyX = animKeys.m_curveQuatX[i];
        auto& keyY = animKeys.m_curveQuatY[i];
        auto& keyZ = animKeys.m_curveQuatZ[i];
        keyW.vec[1][0] = key.mTime;
        keyW.vec[1][1] = key.mValue.w;
        keyX.vec[1][0] = key.mTime;
        keyX.vec[1][1] = key.mValue.x;
        keyY.vec[1][0] = key.mTime;
        keyY.vec[1][1] = key.mValue.y;
        keyZ.vec[1][0] = key.mTime;
        keyZ.vec[1][1] = key.mValue.z;
      }
      finalizeCurve(animKeys.m_curveQuatW);
      finalizeCurve(animKeys.m_curveQuatX);
      finalizeCurve(animKeys.m_curveQuatY);
      finalizeCurve(animKeys.m_curveQuatZ);

      animKeys.m_curveScaleX.resize(animNode.mNumScalingKeys);
      animKeys.m_curveScaleY.resize(animNode.mNumScalingKeys);
      animKeys.m_curveScaleZ.resize(animNode.mNumScalingKeys);
      for (size_t i = 0; i < animNode.mNumScalingKeys; ++i)
      {
        const auto& key = animNode.mScalingKeys[i];
        auto& keyX = animKeys.m_curveScaleX[i];
        auto& keyY = animKeys.m_curveScaleY[i];
        auto& keyZ = animKeys.m_curveScaleZ[i];
        keyX.vec[1][0] = key.mTime;
        keyX.vec[1][1] = key.mValue.x;
        keyY.vec[1][0] = key.mTime;
        keyY.vec[1][1] = key.mValue.y;
        keyZ.vec[1][0] = key.mTime;
        keyZ.vec[1][1] = key.mValue.z;
      }
      finalizeCurve(animKeys.m_curveScaleX);
      finalizeCurve(animKeys.m_curveScaleY);
      finalizeCurve(animKeys.m_curveScaleZ);
    }

    return result;
  }
}

ImportedSkeletalAnimation::ImportedSkeletalAnimation(
  const CTFileName& fileName,
  const std::string& animName,
  const ImportedSkeleton& skeleton,
  size_t optNumFrames,
  double optDuration,
  const BlenderFCurve::InterpolationMode interpolation)
{
  const CTString strFile = _fnmApplicationPath + fileName;

  Assimp::Importer importer;
  const aiScene* aiSceneMain = importer.ReadFile(strFile, 0);

  if (!aiSceneMain)
    ThrowF_t("Unable to load file %s: %s", static_cast<const char*>(fileName), importer.GetErrorString());

  if (aiSceneMain->mNumAnimations == 0)
    ThrowF_t("'%s' contains no animations!", static_cast<const char*>(fileName));

  if (aiSceneMain->mNumAnimations > 1 && animName.empty())
    ThrowF_t("'%s' contains multiple animations but it is not known which one to use!\n'ANIM_NAME_IN_FILE <animation name>' expected!", static_cast<const char*>(fileName));

  aiAnimation* animation = FindAnimation(*aiSceneMain, animName);
  if (!animation)
    ThrowF_t("Animation '%s' not found in file '%s'!", animName.c_str(), static_cast<const char*>(fileName));

  if (animation->mNumChannels <= 0)
    ThrowF_t("Animation '%s' in file '%s' has no bone transformations!\nOnly skeletal animation is supported!", animName.c_str(), static_cast<const char*>(fileName));

  size_t numFrames = optNumFrames;
  if (numFrames == 0)
    numFrames = GetNumFrames(*animation);

  m_defaultPose = skeleton;
  m_frames.resize(numFrames, m_defaultPose);

  if (animation->mTicksPerSecond > 0.0001 && optDuration <= 0.0)
    m_duration = static_cast<float>(animation->mDuration / animation->mTicksPerSecond);
  else
    m_duration = optDuration;

  BakeFrames(*animation, interpolation);
}

void ImportedSkeletalAnimation::ReapplyByReference(const ImportedSkeleton& refSkeleton)
{
  for (auto& frame : m_frames)
  {
    for (auto& nameAndBone : frame.m_bones)
    {
      auto& bone = nameAndBone.second;
      auto refBonePos = refSkeleton.m_bones.find(bone.m_name);
      if (refBonePos == refSkeleton.m_bones.end())
        continue;
      auto& refBone = refBonePos->second;
      const FLOATmatrix4D transform = refBone.m_transformToParent.InverseMatrix() * bone.m_transformToParent;
      const auto& origBone = m_defaultPose.m_bones.at(bone.m_name);
      bone.m_transformToParent = origBone.m_transformToParent * transform;
    }
  }
}

std::vector<std::string> ImportedSkeletalAnimation::GetAnimationsInFile(const CTFileName& fileName)
{
  const CTString strFile = _fnmApplicationPath + fileName;

  Assimp::Importer importer;
  const aiScene* aiSceneMain = importer.ReadFile(strFile, 0);

  if (!aiSceneMain)
    return {};

  std::vector<std::string> anims;
  anims.reserve(aiSceneMain->mNumAnimations);
  for (size_t animIndex = 0; animIndex < aiSceneMain->mNumAnimations; ++animIndex)
  {
    auto& anim = *aiSceneMain->mAnimations[animIndex];
    anims.emplace_back(anim.mName.C_Str());
  }
  return anims;
}

void ImportedSkeletalAnimation::BakeFrames(const aiAnimation& anim, const BlenderFCurve::InterpolationMode interpolation)
{
  const double ticksPerFrame = anim.mDuration / m_frames.size();

  const auto& animKeys = GetBoneAnimKeys(anim, interpolation);

  for (auto it = animKeys.begin(); it != animKeys.end(); ++it)
  {
    const auto& boneName = it->first;
    const auto& boneKeys = it->second;

    for (size_t frameIndex = 0; frameIndex < m_frames.size(); ++frameIndex)
    {
      const double frameTime = frameIndex * ticksPerFrame;
      auto& frameBones = m_frames[frameIndex].m_bones;
      auto foundPos = frameBones.find(boneName);
      if (foundPos == frameBones.end())
        break;
      auto& bone = foundPos->second;
      bone.m_transformToParent = boneKeys.GetTransformAtTime(frameTime, interpolation);
    }
  }
}
