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

#include "ImportedSkeleton.h"

#include <Engine/Base/Stream.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

FLOATmatrix4D ImportedSkeleton::Bone::GetAbsoluteTransform() const
{
  if (mp_parent)
    return mp_parent->GetAbsoluteTransform() * m_transformToParent;

  return m_transformToParent;
}

ImportedSkeleton::ImportedSkeleton(const ImportedSkeleton& other)
{
  *this = other;
}

ImportedSkeleton& ImportedSkeleton::operator=(const ImportedSkeleton& other)
{
  m_bones = other.m_bones;
  for (auto it = other.m_bones.begin(); it != other.m_bones.end(); ++it)
  {
    const auto& bone = it->second;
    auto& myBone = m_bones[bone.m_name];
    myBone.m_children.clear();
    myBone.m_children.reserve(bone.m_children.size());
    for (const auto* childbone : bone.m_children)
    {
      auto& myChildBone = m_bones[childbone->m_name];
      myBone.m_children.push_back(&myChildBone);
      myChildBone.mp_parent = &myBone;
    }
  }
  return *this;
}

void ImportedSkeleton::FillFromFile(const CTFileName& fileName)
{
  const CTString strFile = _fnmApplicationPath + fileName;

  Assimp::Importer importer;
  const aiScene* aiSceneMain = importer.ReadFile(strFile.str_String, 0);

  if (aiSceneMain)
    FillFromScene(*aiSceneMain);
  else
    ThrowF_t("Unable to load file %s: %s", (const char*)fileName, importer.GetErrorString());
}

bool ImportedSkeleton::ContainsSkeleton(const CTFileName& fileName)
{
  const CTString strFile = _fnmApplicationPath + fileName;

  Assimp::Importer importer;
  const aiScene* aiSceneMain = importer.ReadFile(strFile.str_String, 0);

  return aiSceneMain && aiSceneMain->mRootNode->mNumChildren > 0;
}

void ImportedSkeleton::FillFromScene(const aiScene& scene)
{
  m_bones.clear();
  AppendBone(scene.mRootNode);
}

const ImportedSkeleton::Bone* ImportedSkeleton::AppendBone(const aiNode* node, const Bone* parent)
{
  std::string boneName = node->mName.C_Str();
  if (boneName.empty())
    boneName = "UnnamedNode_RandomName_" + std::to_string(m_bones.size());

  FLOATmatrix4D transformToParent;
  for (int row = 0; row < 4; ++row)
    for (int col = 0; col < 4; ++col)
      transformToParent(row + 1, col + 1) = node->mTransformation[row][col];

  // according to assimp documentation, bone nodes have unique names,
  // but other nodes might share name, so it is enough to simply make it unique here
  // just for the map structure to be valid, actual names for such nodes are not important
  while (m_bones.find(boneName) != m_bones.end())
    boneName += '_';

  auto& bone = m_bones[boneName];
  bone.m_name = boneName;
  bone.mp_parent = parent;
  bone.m_transformToParent = transformToParent;

  for (size_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
    bone.m_children.push_back(AppendBone(node->mChildren[childIndex], &bone));

  return &bone;
}
