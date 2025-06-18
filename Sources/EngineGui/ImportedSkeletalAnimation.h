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

#ifndef IMPORTED_SKELETAL_ANIMATION_H
#define IMPORTED_SKELETAL_ANIMATION_H

#include "ImportedSkeleton.h"

#include <Engine/Base/Types.h>
#include <Engine/Math/Vector.h>

#include <string>

struct aiAnimation;

struct ENGINE_API ImportedSkeletalAnimation
{
public:
  ImportedSkeletalAnimation(
    const CTFileName& fileName,
    const std::string& animName,
    const ImportedSkeleton& skeleton,
    size_t optNumFrames,
    double optDuration);

  void ReapplyByReference(const ImportedSkeleton& refSkeleton);

  static std::vector<std::string> GetAnimationsInFile(const CTFileName& fileName);

public:
  double m_duration;
  std::vector<ImportedSkeleton> m_frames;
  ImportedSkeleton m_defaultPose;

private:
  void ImportedSkeletalAnimation::BakeFrames(const aiAnimation& anim);
};

#endif
