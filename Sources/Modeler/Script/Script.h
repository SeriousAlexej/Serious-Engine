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

#ifndef SCRIPT_H
#define SCRIPT_H

#include <SeriousEngineCppAPI/Base/FileName.h>

#include <optional>
#include <string>
#include <vector>

struct ModelScript {
  ModelScript()
  {
    m_transformation.Diagonal(1.0f);
  }

  struct Animation {
    enum class Type {
      Skeletal,
      Vertex
    };

    Type m_type;
    std::string m_name;
    std::optional<std::string> m_customSourceName;
    std::optional<size_t> m_optNumFrames;
    std::optional<double> m_optDuration;
    std::optional<CTFileName> m_optRefSkeleton;
    std::optional<std::string> m_optOriginBone;
    std::vector<CTFileName> m_frames;
  };

  enum class Flat {
    No,
    Half,
    Full
  };

  using Animations = std::vector<Animation>;

  FLOATmatrix3D m_transformation;
  FLOAT2D m_textureScale = FLOAT2D(2.0f, 2.0f);
  FLOAT m_scale = 1.0f;
  INDEX m_maxShadow = 0;
  Flat m_flat = Flat::No;
  bool m_stretchDetail = false;
  bool m_highQuality = true;
  bool m_boneTriangles = true;
  size_t m_defaultUVChannel = 0;
  std::optional<CTFileName> m_skeleton;
  std::vector<CTFileName> m_mipModels;
  Animations m_animations;
};

#endif // SCRIPT_H
