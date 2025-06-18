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

#include <Engine/Base/Stream.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>

#ifdef max
#undef max
#endif

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
    std::map<double, aiVector3D> m_posKeys;
    std::map<double, aiQuaternion> m_rotKeys;
    std::map<double, aiVector3D> m_scaleKeys;

    FLOATmatrix4D GetTransformAtTime(double time) const
    {
      const auto position = GetValueAtTime(m_posKeys, time);
      const auto rotation = GetValueAtTime(m_rotKeys, time);
      const auto scaling = GetValueAtTime(m_scaleKeys, time);

      const aiMatrix4x4 transform(scaling, rotation, position);

      FLOATmatrix4D result;
      for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
          result(row + 1, col + 1) = transform[row][col];
      return result;
    }

    template<typename TValueType>
    TValueType GetValueAtTime(const std::map<double, TValueType>& keys, double time) const
    {
      auto itLarger = keys.lower_bound(time);
      if (itLarger == keys.begin())
        return keys.begin()->second;
      if (itLarger == keys.end())
        return keys.rbegin()->second;
      auto itSmaller = itLarger;
      --itSmaller;

      TValueType res;
      const double d = (time - itSmaller->first) / (itLarger->first - itSmaller->first);
      Assimp::Interpolator<TValueType>()(res, itSmaller->second, itLarger->second, d);
      return res;
    }
  };

  std::map<std::string, AnimKeys> GetBoneAnimKeys(const aiAnimation& anim)
  {
    std::map<std::string, AnimKeys> result;

    for (size_t nodeIndex = 0; nodeIndex < anim.mNumChannels; ++nodeIndex)
    {
      const auto& animNode = *anim.mChannels[nodeIndex];
      std::string boneName = animNode.mNodeName.C_Str();
      auto& animKeys = result[boneName];
      for (size_t i = 0; i < animNode.mNumPositionKeys; ++i)
      {
        const auto& key = animNode.mPositionKeys[i];
        animKeys.m_posKeys[key.mTime] = key.mValue;
      }
      for (size_t i = 0; i < animNode.mNumRotationKeys; ++i)
      {
        const auto& key = animNode.mRotationKeys[i];
        animKeys.m_rotKeys[key.mTime] = key.mValue;
      }
      for (size_t i = 0; i < animNode.mNumScalingKeys; ++i)
      {
        const auto& key = animNode.mScalingKeys[i];
        animKeys.m_scaleKeys[key.mTime] = key.mValue;
      }
    }

    return result;
  }
}

ImportedSkeletalAnimation::ImportedSkeletalAnimation(
  const CTFileName& fileName,
  const std::string& animName,
  const ImportedSkeleton& skeleton,
  size_t optNumFrames,
  double optDuration)
{
  const CTString strFile = _fnmApplicationPath + fileName;

  Assimp::Importer importer;
  const aiScene* aiSceneMain = importer.ReadFile(strFile.str_String, 0);

  if (!aiSceneMain)
    ThrowF_t("Unable to load file %s: %s", (const char*)fileName, importer.GetErrorString());

  if (aiSceneMain->mNumAnimations == 0)
    ThrowF_t("'%s' contains no animations!", (const char*)fileName);

  if (aiSceneMain->mNumAnimations > 1 && animName.empty())
    ThrowF_t("'%s' contains multiple animations but it is not known which one to use!\n'ANIM_NAME_IN_FILE <animation name>' expected!", (const char*)fileName);

  aiAnimation* animation = FindAnimation(*aiSceneMain, animName);
  if (!animation)
    ThrowF_t("Animation '%s' not found in file '%s'!", animName.c_str(), (const char*)fileName);

  if (animation->mNumChannels <= 0)
    ThrowF_t("Animation '%s' in file '%s' has no bone transformations!\nOnly skeletal animation is supported!", animName.c_str(), (const char*)fileName);

  size_t numFrames = optNumFrames;
  if (numFrames == 0)
    numFrames = GetNumFrames(*animation);

  m_defaultPose = skeleton;
  m_frames.resize(numFrames, m_defaultPose);

  if (animation->mTicksPerSecond > 0.0001 && optDuration <= 0.0)
    m_duration = static_cast<float>(animation->mDuration / animation->mTicksPerSecond);
  else
    m_duration = optDuration;

  BakeFrames(*animation);
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
      const FLOATmatrix4D transform = InverseMatrix(refBone.m_transformToParent) * bone.m_transformToParent;
      const auto& origBone = m_defaultPose.m_bones.at(bone.m_name);
      bone.m_transformToParent = origBone.m_transformToParent * transform;
    }
  }
}

std::vector<std::string> ImportedSkeletalAnimation::GetAnimationsInFile(const CTFileName& fileName)
{
  const CTString strFile = _fnmApplicationPath + fileName;

  Assimp::Importer importer;
  const aiScene* aiSceneMain = importer.ReadFile(strFile.str_String, 0);

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

void ImportedSkeletalAnimation::BakeFrames(const aiAnimation& anim)
{
  const double ticksPerFrame = anim.mDuration / m_frames.size();

  const auto& animKeys = GetBoneAnimKeys(anim);

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
      bone.m_transformToParent = boneKeys.GetTransformAtTime(frameTime);
    }
  }
}
