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

#ifndef IMPORTED_SKELETON_H
#define IMPORTED_SKELETON_H

#include <Engine/Base/Types.h>
#include <Engine/Math/Vector.h>

#include <string>
#include <vector>
#include <map>

struct aiScene;
struct aiNode;

struct ENGINE_API ImportedSkeleton
{
public:
  struct ENGINE_API Bone
  {
    std::string m_name;
    FLOATmatrix4D m_transformToParent;
    const Bone* mp_parent = nullptr;
    std::vector<const Bone*> m_children;

    FLOATmatrix4D GetAbsoluteTransform() const;
  };

  ImportedSkeleton() = default;
  ImportedSkeleton(const ImportedSkeleton& other);

  ImportedSkeleton& operator=(const ImportedSkeleton& other);

  void FillFromFile(const CTFileName& fileName);

  static bool ContainsSkeleton(const CTFileName& fileName);

public:
  std::map<std::string, Bone> m_bones;

private:
  void FillFromScene(const aiScene& scene);
  const Bone* AppendBone(const aiNode* node, const Bone* parent = nullptr);
};

#endif
