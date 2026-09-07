/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0 */
#ifndef BLENDER_FCURVE_H
#define BLENDER_FCURVE_H
#include <vector>

namespace BlenderFCurve
{
  /*
   * - vec[0][0] = x location of handle 1
   * - vec[0][1] = y location of handle 1
   * - vec[1][0] = x location of control point
   * - vec[1][1] = y location of control point
   * - vec[2][0] = x location of handle 2
   * - vec[2][1] = y location of handle 2
   */
  struct Keyframe
  {
    Keyframe() = default;
    Keyframe(const Keyframe& other)
    {
      *this = other;
    }
    Keyframe& operator=(const Keyframe& other)
    {
      final = other.final;
      for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 2; ++j)
          vec[i][j] = other.vec[i][j];
      return *this;
    }
    float vec[3][2] = { {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f} };
    bool final = false;
  };
  using TAnimCurve = std::vector<Keyframe>;

  enum class InterpolationMode : int
  {
    Linear,
    Bezier
  };

  void CalculateCurveHandles(TAnimCurve& curve);
  float EvaluateCurveAtTime(const TAnimCurve& curve, const float evaltime, const InterpolationMode interpolation);
  void EvaluateQuaternionAtTime(
    const TAnimCurve& curveW,
    const TAnimCurve& curveX,
    const TAnimCurve& curveY,
    const TAnimCurve& curveZ,
    const float evaltime,
    const InterpolationMode interpolation,
    float quat[4]);
} // namespace BlenderFCurve

#endif
