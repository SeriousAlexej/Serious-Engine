/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0 */
#include "StdH.h"
#include "BlenderFCurve.h"

namespace BlenderFCurve
{

void sub_v2_v2v2(float r[2], const float a[2], const float b[2])
{
  r[0] = a[0] - b[0];
  r[1] = a[1] - b[1];
}

void add_v2_v2v2(float r[2], const float a[2], const float b[2])
{
  r[0] = a[0] + b[0];
  r[1] = a[1] + b[1];
}

void madd_v2_v2v2fl(float r[2], const float a[2], const float b[2], float f)
{
  r[0] = a[0] + b[0] * f;
  r[1] = a[1] + b[1] * f;
}

float len_v2v2(const float v1[2], const float v2[2])
{
  float x, y;

  x = v1[0] - v2[0];
  y = v1[1] - v2[1];
  return sqrtf(x * x + y * y);
}

void calchandleNurb(Keyframe* bezt, const Keyframe* prev, const Keyframe* next)
{
  /* defines to avoid confusion */
#define p2_h1 ((p2) - 2)
#define p2_h2 ((p2) + 2)

  const float* p1;
  const float* p3;
  float* p2;
  float pt[2];
  float dvec_a[2], dvec_b[2];
  float len, len_a, len_b;
  const float eps = 1e-5f;

  if (prev == nullptr && next == nullptr) {
    /* Single point, no need to calculate handles. */
    return;
  }

  p2 = bezt->vec[1];

  if (prev == nullptr) {
    p3 = next->vec[1];
    pt[0] = 2.0f * p2[0] - p3[0];
    pt[1] = 2.0f * p2[1] - p3[1];
    p1 = pt;
  }
  else {
    p1 = prev->vec[1];
  }

  if (next == nullptr) {
    pt[0] = 2.0f * p2[0] - p1[0];
    pt[1] = 2.0f * p2[1] - p1[1];
    p3 = pt;
  }
  else {
    p3 = next->vec[1];
  }

  sub_v2_v2v2(dvec_a, p2, p1);
  sub_v2_v2v2(dvec_b, p3, p2);

  len_a = dvec_a[0];
  len_b = dvec_b[0];

  if (len_a == 0.0f) {
    len_a = 1.0f;
  }
  if (len_b == 0.0f) {
    len_b = 1.0f;
  }

  float tvec[2];
  tvec[0] = dvec_b[0] / len_b + dvec_a[0] / len_a;
  tvec[1] = dvec_b[1] / len_b + dvec_a[1] / len_a;

  len = 6.0f;

  bool leftviolate = false, rightviolate = false;

  len_a /= len;
  madd_v2_v2v2fl(p2_h1, p2, tvec, -len_a);

  if (next && prev) { /* keep horizontal if extrema */
    float ydiff1 = prev->vec[1][1] - bezt->vec[1][1];
    float ydiff2 = next->vec[1][1] - bezt->vec[1][1];
    if ((ydiff1 <= 0.0f && ydiff2 <= 0.0f) || (ydiff1 >= 0.0f && ydiff2 >= 0.0f)) {
      bezt->vec[0][1] = bezt->vec[1][1];
      bezt->final = true;
    }
    else { /* handles should not be beyond y coord of two others */
      if (ydiff1 <= 0.0f) {
        if (prev->vec[1][1] > bezt->vec[0][1]) {
          bezt->vec[0][1] = prev->vec[1][1];
          leftviolate = true;
        }
      }
      else {
        if (prev->vec[1][1] < bezt->vec[0][1]) {
          bezt->vec[0][1] = prev->vec[1][1];
          leftviolate = true;
        }
      }
    }
  }

  len_b /= len;
  madd_v2_v2v2fl(p2_h2, p2, tvec, len_b);

  if (next && prev) { /* keep horizontal if extrema */
    float ydiff1 = prev->vec[1][1] - bezt->vec[1][1];
    float ydiff2 = next->vec[1][1] - bezt->vec[1][1];
    if ((ydiff1 <= 0.0f && ydiff2 <= 0.0f) || (ydiff1 >= 0.0f && ydiff2 >= 0.0f)) {
      bezt->vec[2][1] = bezt->vec[1][1];
      bezt->final = true;
    }
    else { /* handles should not be beyond y coord of two others */
      if (ydiff1 <= 0.0f) {
        if (next->vec[1][1] < bezt->vec[2][1]) {
          bezt->vec[2][1] = next->vec[1][1];
          rightviolate = true;
        }
      }
      else {
        if (next->vec[1][1] > bezt->vec[2][1]) {
          bezt->vec[2][1] = next->vec[1][1];
          rightviolate = true;
        }
      }
    }
  }

  if (leftviolate || rightviolate) { /* align left handle */
    /* simple 2d calculation */
    float h1_x = p2_h1[0] - p2[0];
    float h2_x = p2[0] - p2_h2[0];

    if (leftviolate) {
      p2_h2[1] = p2[1] + ((p2[1] - p2_h1[1]) / h1_x) * h2_x;
    }
    else {
      p2_h1[1] = p2[1] + ((p2[1] - p2_h2[1]) / h2_x) * h1_x;
    }
  }

  len_a = len_v2v2(p2, p2_h1);
  len_b = len_v2v2(p2, p2_h2);

  if (len_a == 0.0f) {
    len_a = 1.0f;
  }
  if (len_b == 0.0f) {
    len_b = 1.0f;
  }

  const float len_ratio = len_a / len_b;

#undef p2_h1
#undef p2_h2
}

bool bezier_check_solve_end_handle(Keyframe* bezt, bool end)
{
  return end && !bezt->final;
}

void* allocate_arrays(int count, float*** floats, char*** chars)
{
  size_t num_floats = 0, num_chars = 0;

  while (floats && floats[num_floats]) {
    num_floats++;
  }

  while (chars && chars[num_chars]) {
    num_chars++;
  }

  void* buffer = malloc(count * (sizeof(float) * num_floats + num_chars));

  if (!buffer) {
    return nullptr;
  }

  float* fptr = static_cast<float*>(buffer);

  for (size_t i = 0; i < num_floats; i++, fptr += count) {
    *floats[i] = fptr;
  }

  char* cptr = reinterpret_cast<char*>(fptr);

  for (size_t i = 0; i < num_chars; i++, cptr += count) {
    *chars[i] = cptr;
  }

  return buffer;
}

const int NAN_INT = 0x7FC00000;
#define NAN_FLT (*((float *)(&NAN_INT)))

float min_ff(float a, float b)
{
  return (a < b) ? a : b;
}
float max_ff(float a, float b)
{
  return (a > b) ? a : b;
}

void bezier_clamp(
  float* hmax, float* hmin, int i, float dy, bool no_reverse, bool no_overshoot)
{
  if (dy > 0) {
    if (no_overshoot) {
      hmax[i] = min_ff(hmax[i], dy);
    }
    if (no_reverse) {
      hmin[i] = 0.0f;
    }
  }
  else if (dy < 0) {
    if (no_reverse) {
      hmax[i] = 0.0f;
    }
    if (no_overshoot) {
      hmin[i] = max_ff(hmin[i], dy);
    }
  }
  else if (no_reverse || no_overshoot) {
    hmax[i] = hmin[i] = 0.0f;
  }
}

void mul_v2_fl(float r[2], float f)
{
  r[0] *= f;
  r[1] *= f;
}

float bezier_calc_handle_adj(float hsize[2], float dx)
{
  /* if handles intersect in x direction, they are scaled to fit */
  float fac = dx / (hsize[0] + dx / 3.0f);
  if (fac < 1.0f) {
    mul_v2_fl(hsize, fac);
  }
  return 1.0f - 3.0f * hsize[0] / dx;
}

void bezier_lock_unknown(float* a, float* b, float* c, float* d, int i, float value)
{
  a[i] = c[i] = 0.0f;
  b[i] = 1.0f;
  d[i] = value;
}

void bezier_eq_noaccel_right(
  float* a, float* b, float* c, float* d, const float* dy, const float* l, int i)
{
  a[i] = 0.0f;
  b[i] = 2.0f;
  c[i] = 1.0f / l[i + 1];
  d[i] = dy[i + 1];
}

void bezier_eq_noaccel_left(
  float* a, float* b, float* c, float* d, const float* dy, const float* l, int i)
{
  a[i] = l[i] * l[i];
  b[i] = 2.0f * l[i];
  c[i] = 0.0f;
  d[i] = dy[i] * l[i] * l[i];
}

void bezier_eq_continuous(
  float* a, float* b, float* c, float* d, const float* dy, const float* l, int i)
{
  a[i] = l[i] * l[i];
  b[i] = 2.0f * (l[i] + 1);
  c[i] = 1.0f / l[i + 1];
  d[i] = dy[i] * l[i] * l[i] + dy[i + 1];
}

bool BLI_tridiagonal_solve(
  const float* a, const float* b, const float* c, const float* d, float* r_x, const int count)
{
  if (count < 1) {
    return false;
  }

  double* c1 = (double*)malloc(sizeof(double) * size_t(count) * 2);
  if (!c1) {
    return false;
  }
  double* d1 = c1 + count;

  int i;
  double c_prev, d_prev, x_prev;

  /* forward pass */

  c1[0] = c_prev = double(c[0]) / b[0];
  d1[0] = d_prev = double(d[0]) / b[0];

  for (i = 1; i < count; i++) {
    double denum = b[i] - a[i] * c_prev;

    c1[i] = c_prev = c[i] / denum;
    d1[i] = d_prev = (d[i] - a[i] * d_prev) / denum;
  }

  /* back pass */

  x_prev = d_prev;
  r_x[--i] = float(x_prev);

  while (--i >= 0) {
    x_prev = d1[i] - c1[i] * x_prev;
    r_x[i] = float(x_prev);
  }

  free(c1);

  return isfinite(x_prev);
}

bool BLI_tridiagonal_solve_cyclic(
  const float* a, const float* b, const float* c, const float* d, float* r_x, const int count)
{
  if (count < 1) {
    return false;
  }

  /* Degenerate case not handled correctly by the generic formula. */
  if (count == 1) {
    r_x[0] = d[0] / (a[0] + b[0] + c[0]);

    return isfinite(r_x[0]);
  }

  /* Degenerate case that works but can be simplified. */
  if (count == 2) {
    const float a2[2] = { 0, a[1] + c[1] };
    const float c2[2] = { a[0] + c[0], 0 };

    return BLI_tridiagonal_solve(a2, b, c2, d, r_x, count);
  }

  /* If not really cyclic, fall back to the simple solver. */
  float a0 = a[0], cN = c[count - 1];

  if (a0 == 0.0f && cN == 0.0f) {
    return BLI_tridiagonal_solve(a, b, c, d, r_x, count);
  }

  size_t bytes = sizeof(float) * unsigned int(count);
  float* tmp = (float*)malloc(sizeof(float) * size_t(count) * 2);
  if (!tmp) {
    return false;
  }
  float* b2 = tmp + count;

  /* Prepare the non-cyclic system; relies on tridiagonal_solve ignoring values. */
  memcpy(b2, b, bytes);
  b2[0] -= a0;
  b2[count - 1] -= cN;

  memset(tmp, 0, bytes);
  tmp[0] = a0;
  tmp[count - 1] = cN;

  /* solve for partial solution and adjustment vector */
  bool success = BLI_tridiagonal_solve(a, b2, c, tmp, tmp, count) &&
    BLI_tridiagonal_solve(a, b2, c, d, r_x, count);

  /* apply adjustment */
  if (success) {
    float coeff = (r_x[0] + r_x[count - 1]) / (1.0f + tmp[0] + tmp[count - 1]);

    for (int i = 0; i < count; i++) {
      r_x[i] -= coeff * tmp[i];
    }
  }

  free(tmp);

  return success;
}

float bezier_relax_direction(const float* a,
  const float* b,
  const float* c,
  const float* d,
  const float* h,
  int i,
  int count)
{
  /* current deviation between sides of the equation */
  float state = a[i] * h[(i + count - 1) % count] + b[i] * h[i] + c[i] * h[(i + 1) % count] - d[i];

  /* only the sign is meaningful */
  return -state * b[i];
}

void bezier_restore_equation(float* a,
  float* b,
  float* c,
  float* d,
  const float* a0,
  const float* b0,
  const float* c0,
  const float* d0,
  int i)
{
  a[i] = a0[i];
  b[i] = b0[i];
  c[i] = c0[i];
  d[i] = d0[i];
}

bool tridiagonal_solve_with_limits(float* a,
  float* b,
  float* c,
  float* d,
  float* h,
  const float* hmin,
  const float* hmax,
  int solve_count)
{
  float* a0, * b0, * c0, * d0;
  float** arrays[] = { &a0, &b0, &c0, &d0, nullptr };
  char* is_locked, * num_unlocks;
  char** flagarrays[] = { &is_locked, &num_unlocks, nullptr };

  void* tmps = allocate_arrays(solve_count, arrays, flagarrays);
  if (!tmps) {
    return false;
  }

  memcpy(a0, a, sizeof(float) * solve_count);
  memcpy(b0, b, sizeof(float) * solve_count);
  memcpy(c0, c, sizeof(float) * solve_count);
  memcpy(d0, d, sizeof(float) * solve_count);

  memset(is_locked, 0, solve_count);
  memset(num_unlocks, 0, solve_count);

  bool overshoot, unlocked;

  do {
    if (!BLI_tridiagonal_solve_cyclic(a, b, c, d, h, solve_count)) {
      free(tmps);
      return false;
    }

    /* first check if any handles overshoot the limits, and lock them */
    bool all = false, locked = false;

    overshoot = unlocked = false;

    do {
      for (int i = 0; i < solve_count; i++) {
        if (h[i] >= hmin[i] && h[i] <= hmax[i]) {
          continue;
        }

        overshoot = true;

        float target = h[i] > hmax[i] ? hmax[i] : hmin[i];

        /* heuristically only lock handles that go in the right direction if there are such ones */
        if (target != 0.0f || all) {
          /* mark item locked */
          is_locked[i] = 1;

          bezier_lock_unknown(a, b, c, d, i, target);
          locked = true;
        }
      }

      all = true;
    } while (overshoot && !locked);

    /* If no handles overshot and were locked,
     * see if it may be a good idea to unlock some handles. */
    if (!locked) {
      for (int i = 0; i < solve_count; i++) {
        /* to definitely avoid infinite loops limit this to 2 times */
        if (!is_locked[i] || num_unlocks[i] >= 2) {
          continue;
        }

        /* if the handle wants to move in allowable direction, release it */
        float relax = bezier_relax_direction(a0, b0, c0, d0, h, i, solve_count);

        if ((relax > 0 && h[i] < hmax[i]) || (relax < 0 && h[i] > hmin[i])) {
          bezier_restore_equation(a, b, c, d, a0, b0, c0, d0, i);

          is_locked[i] = 0;
          num_unlocks[i]++;
          unlocked = true;
        }
      }
    }
  } while (overshoot || unlocked);

  free(tmps);
  return true;
}

void copy_v2_v2(float r[2], const float a[2])
{
  r[0] = a[0];
  r[1] = a[1];
}

void bezier_output_handle_inner(Keyframe* bezt,
  bool right,
  const float newval[2],
  bool endpoint)
{
  float tmp[2];

  int idx = right ? 2 : 0;

  copy_v2_v2(bezt->vec[idx], newval);

  /* at end points of the curve, mirror handle to the other side */
  if (endpoint) {
    sub_v2_v2v2(tmp, bezt->vec[1], bezt->vec[idx]);
    add_v2_v2v2(bezt->vec[2 - idx], bezt->vec[1], tmp);
  }
}

void bezier_output_handle(Keyframe* bezt, bool right, float dy, bool endpoint)
{
  float tmp[2];

  copy_v2_v2(tmp, bezt->vec[right ? 2 : 0]);

  tmp[1] = bezt->vec[1][1] + dy;

  bezier_output_handle_inner(bezt, right, tmp, endpoint);
}

void bezier_handle_calc_smooth_curve(TAnimCurve& curve, int start, int count)
{
  float* dx, * dy, * l, * a, * b, * c, * d, * h, * hmax, * hmin;
  float** arrays[] = { &dx, &dy, &l, &a, &b, &c, &d, &h, &hmax, &hmin, nullptr };

  int solve_count = count;

  /* verify index ranges */

  if (count < 2) {
    return;
  }

  Keyframe* bezt_first = &curve[start];
  Keyframe* bezt_last = &curve[(start + count > static_cast<int>(curve.size())) ? start + count - static_cast<int>(curve.size()) : start + count - 1];

  bool solve_first = bezier_check_solve_end_handle(bezt_first, start == 0);
  bool solve_last = bezier_check_solve_end_handle(bezt_last, start + count == curve.size());

  if (count == 2 && solve_first == solve_last) {
    return;
  }

  /* allocate all */

  void* tmp_buffer = allocate_arrays(count, arrays, nullptr);
  if (!tmp_buffer) {
    return;
  }

  /* point locations */

  dx[0] = dy[0] = NAN_FLT;

  for (int i = 1, j = start + 1; i < count; i++, j++) {
    dx[i] = curve[j].vec[1][0] - curve[j - 1].vec[1][0];
    dy[i] = curve[j].vec[1][1] - curve[j - 1].vec[1][1];
  }

  /* ratio of x intervals */
  l[0] = l[count - 1] = 1.0f;

  for (int i = 1; i < count - 1; i++) {
    l[i] = dx[i + 1] / dx[i];
  }

  /* compute handle clamp ranges */

  bool clamped_prev = false, clamped_cur = true;

  for (int i = 0; i < count; i++) {
    hmax[i] = FLT_MAX;
    hmin[i] = -FLT_MAX;
  }

  for (int i = 1, j = start + 1; i < count; i++, j++) {
    clamped_prev = clamped_cur;
    clamped_cur = true;

    bezier_clamp(hmax, hmin, i - 1, dy[i], clamped_prev, clamped_prev);
    bezier_clamp(hmax, hmin, i, dy[i] * l[i], clamped_cur, clamped_cur);
  }

  /* full cycle merges first and last points into continuous loop */

  float first_handle_adj = 0.0f, last_handle_adj = 0.0f;

  float tmp[2];

  /* boundary condition: fixed handles or zero curvature */
  if (!solve_first) {
    sub_v2_v2v2(tmp, bezt_first->vec[2], bezt_first->vec[1]);
    first_handle_adj = bezier_calc_handle_adj(tmp, dx[1]);

    bezier_lock_unknown(a, b, c, d, 0, tmp[1]);
  }
  else {
    bezier_eq_noaccel_right(a, b, c, d, dy, l, 0);
  }

  if (!solve_last) {
    sub_v2_v2v2(tmp, bezt_last->vec[1], bezt_last->vec[0]);
    last_handle_adj = bezier_calc_handle_adj(tmp, dx[count - 1]);

    bezier_lock_unknown(a, b, c, d, count - 1, tmp[1]);
  }
  else {
    bezier_eq_noaccel_left(a, b, c, d, dy, l, count - 1);
  }

  /* main tridiagonal system of equations */

  for (int i = 1; i < count - 1; i++) {
    bezier_eq_continuous(a, b, c, d, dy, l, i);
  }

  /* apply correction for user-defined handles with nonstandard x positions */

  if (count > 2 || solve_last) {
    b[1] += l[1] * first_handle_adj;
  }

  if (count > 2 || solve_first) {
    b[count - 2] += last_handle_adj;
  }

  /* solve and output results */

  if (tridiagonal_solve_with_limits(a, b, c, d, h, hmin, hmax, solve_count)) {

    for (int i = 1, j = start + 1; i < count - 1; i++, j++) {
      bool end = (j == curve.size() - 1);

      bezier_output_handle(&curve[j], false, -h[i] / l[i], end);

      if (end) {
        j = 0;
      }

      bezier_output_handle(&curve[j], true, h[i], end);
    }

    if (solve_first) {
      bezier_output_handle(bezt_first, true, h[0], start == 0);
    }

    if (solve_last) {
      bezier_output_handle(bezt_last, false, -h[count - 1] / l[count - 1], start + count == curve.size());
    }
  }

  free(tmp_buffer);
}

void nurb_handle_smooth_curve(TAnimCurve& curve)
{
  int search_base = 0;
  int start = search_base, count = 1;

  for (int i = 1, j = start + 1; i < static_cast<int>(curve.size()); i++, j++) {
    if (!curve[j].final) {
      bezier_handle_calc_smooth_curve(curve, start, count + 1);
      start = j;
      count = 1;
    }
    else {
      count++;
    }
  }

  if (count > 1) {
    bezier_handle_calc_smooth_curve(curve, start, count);
  }
}

void CalculateCurveHandles(TAnimCurve& curve)
{
  if (curve.size() < 2)
    return;

  for (size_t i = 0; i < curve.size(); ++i)
  {
    Keyframe* bezt = &curve[i];
    Keyframe* prev = nullptr;
    Keyframe* next = nullptr;
    if (i > 0) {
      prev = (bezt - 1);
    }
    if (i < curve.size() - 1) {
      next = (bezt + 1);
    }

    calchandleNurb(bezt, prev, next);

    if (!prev || !next) {
      bezt->vec[0][1] = bezt->vec[2][1] = bezt->vec[1][1];
      bezt->final = true;
    }

    if (prev && prev->vec[1][0] >= bezt->vec[1][0]) {
      prev->final = bezt->final = true;
    }
  }

  nurb_handle_smooth_curve(curve);
}

#define IS_EQT(a, b, c) (((a) > (b)) ? ((((a) - (b)) <= (c))) : (((((b) - (a)) <= (c)))))

int curve_bezt_binarysearch_index_ex(const Keyframe array[],
  const float frame,
  const int arraylen,
  const float threshold,
  bool* r_replace)
{
  int start = 0, end = arraylen-1;

  /* Initialize replace-flag first. */
  *r_replace = false;

  /* Sneaky optimizations (don't go through searching process if...):
   * - Keyframe to be added is to be added out of current bounds.
   * - Keyframe to be added would replace one of the existing ones on bounds.
   */
  if (arraylen <= 0 || array == nullptr) {
    return 0;
  }

  /* Check whether to add before/after/on. */
  /* 'First' Keyframe (when only one keyframe, this case is used) */
  float framenum = array[0].vec[1][0];
  if (IS_EQT(frame, framenum, threshold) || frame < framenum) {
    *r_replace = true;
    return 0;
  }
  if (frame < framenum) {
    return 0;
  }

  /* 'Last' Keyframe */
  framenum = array[(arraylen - 1)].vec[1][0];
  if (IS_EQT(frame, framenum, threshold) || frame > framenum) {
    *r_replace = true;
    return (arraylen - 1);
  }

  while (start <= end) {
    /* Compute and get midpoint. */

    /* We calculate the midpoint this way to avoid int overflows... */
    const int mid = start + ((end - start) / 2);

    const float midfra = array[mid].vec[1][0];

    /* Check if exactly equal to midpoint. */
    if (IS_EQT(frame, midfra, threshold)) {
      *r_replace = true;
      return mid;
    }

    /* Repeat in upper/lower half. */
    if (frame > midfra) {
      start = mid + 1;
    }
    else {
      end = mid - 1;
    }
  }

  /* Not found, so return where to place it. */
  return start;
}

float easing_linear_ease(float time, float begin, float change, float duration)
{
  return change * time / duration + begin;
}

void curve_correct_bezpart(const float v1[2], float v2[2], float v3[2], const float v4[2])
{
  float h1[2], h2[2], len1, len2, len, fac;

  /* Calculate handle deltas. */
  h1[0] = v1[0] - v2[0];
  h1[1] = v1[1] - v2[1];

  h2[0] = v4[0] - v3[0];
  h2[1] = v4[1] - v3[1];

  /* Calculate distances:
   * - len  = Span of time between keyframes.
   * - len1 = Length of handle of start key.
   * - len2 = Length of handle of end key.
   */
  len = v4[0] - v1[0];
  len1 = fabsf(h1[0]);
  len2 = fabsf(h2[0]);

  /* If the handles have no length, no need to do any corrections. */
  if ((len1 + len2) == 0.0f) {
    return;
  }

  /* To prevent looping or rewinding, handles cannot
   * exceed the adjacent key-frames time position. */
  if (len1 > len) {
    fac = len / len1;
    v2[0] = (v1[0] - fac * h1[0]);
    v2[1] = (v1[1] - fac * h1[1]);
  }

  if (len2 > len) {
    fac = len / len2;
    v3[0] = (v4[0] - fac * h2[0]);
    v3[1] = (v4[1] - fac * h2[1]);
  }
}

double sqrt3d(double d)
{
  if (d == 0.0) [[unlikely]] {
    return 0.0;
  }
  if (d < 0.0) [[unlikely]] {
    return -exp(log(-d) / 3.0);
  }
  return exp(log(d) / 3.0);
}

/**
 * Find roots of cubic equation (c0 + c1 x + c2 x^2 + c3 x^3)
 * \return number of roots in `o`.
 *
 * \note it is up to the caller to allocate enough memory for `o`.
 */
#define SMALL -1.0e-10
int solve_cubic(double c0, double c1, double c2, double c3, float* o)
{
  double a, b, c, p, q, d, t, phi;
  int nr = 0;

  if (c3 != 0.0) {
    a = c2 / c3;
    b = c1 / c3;
    c = c0 / c3;
    a = a / 3;

    p = b / 3 - a * a;
    q = (2 * a * a * a - a * b + c) / 2;
    d = q * q + p * p * p;

    if (d > 0.0) {
      t = sqrt(d);
      o[0] = float(sqrt3d(-q + t) + sqrt3d(-q - t) - a);

      if ((o[0] >= float(SMALL)) && (o[0] <= 1.000001f)) {
        return 1;
      }
      return 0;
    }

    if (d == 0.0) {
      t = sqrt3d(-q);
      o[0] = float(2 * t - a);

      if ((o[0] >= float(SMALL)) && (o[0] <= 1.000001f)) {
        nr++;
      }
      o[nr] = float(-t - a);

      if ((o[nr] >= float(SMALL)) && (o[nr] <= 1.000001f)) {
        return nr + 1;
      }
      return nr;
    }

    phi = acos(-q / sqrt(-(p * p * p)));
    t = sqrt(-p);
    p = cos(phi / 3);
    q = sqrt(3 - 3 * p * p);
    o[0] = float(2 * t * p - a);

    if ((o[0] >= float(SMALL)) && (o[0] <= 1.000001f)) {
      nr++;
    }
    o[nr] = float(-t * (p + q) - a);

    if ((o[nr] >= float(SMALL)) && (o[nr] <= 1.000001f)) {
      nr++;
    }
    o[nr] = float(-t * (p - q) - a);

    if ((o[nr] >= float(SMALL)) && (o[nr] <= 1.000001f)) {
      return nr + 1;
    }
    return nr;
  }
  a = c2;
  b = c1;
  c = c0;

  if (a != 0.0) {
    /* Discriminant */
    p = b * b - 4 * a * c;

    if (p > 0) {
      p = sqrt(p);
      o[0] = float((-b - p) / (2 * a));

      if ((o[0] >= float(SMALL)) && (o[0] <= 1.000001f)) {
        nr++;
      }
      o[nr] = float((-b + p) / (2 * a));

      if ((o[nr] >= float(SMALL)) && (o[nr] <= 1.000001f)) {
        return nr + 1;
      }
      return nr;
    }

    if (p == 0) {
      o[0] = float(-b / (2 * a));
      if ((o[0] >= float(SMALL)) && (o[0] <= 1.000001f)) {
        return 1;
      }
    }

    return 0;
  }

  if (b != 0.0) {
    o[0] = float(-c / b);

    if ((o[0] >= float(SMALL)) && (o[0] <= 1.000001f)) {
      return 1;
    }
    return 0;
  }

  if (c == 0.0) {
    o[0] = 0.0;
    return 1;
  }

  return 0;
}

/* Find root(s) ('zero') of a Bezier curve. */
int findzero(float x, float q0, float q1, float q2, float q3, float* o)
{
  const double c0 = q0 - x;
  const double c1 = 3.0f * (q1 - q0);
  const double c2 = 3.0f * (q0 - 2.0f * q1 + q2);
  const double c3 = q3 - q0 + 3.0f * (q1 - q2);

  return solve_cubic(c0, c1, c2, c3, o);
}

void berekeny(float f1, float f2, float f3, float f4, float* o, int b)
{
  float t, c0, c1, c2, c3;
  int a;

  c0 = f1;
  c1 = 3.0f * (f2 - f1);
  c2 = 3.0f * (f1 - 2.0f * f2 + f3);
  c3 = f4 - f1 + 3.0f * (f2 - f3);

  for (a = 0; a < b; a++) {
    t = o[a];
    o[a] = c0 + t * c1 + t * t * c2 + t * t * t * c3;
  }
}

float EvaluateCurveAtTime(const TAnimCurve& curve, const float evaltime, const InterpolationMode interpolation)
{
  const float eps = 1.e-8f;

  /* Evaluation-time occurs somewhere in the middle of the curve. */
  bool exact = false;

  unsigned int a = curve_bezt_binarysearch_index_ex(curve.data(), evaltime, static_cast<int>(curve.size()), 0.0001f, &exact);
  const Keyframe* bezt = &curve[a];

  if (exact || fabsf(bezt->vec[1][0] - evaltime) < eps || bezt->vec[1][0] < evaltime) {
    return bezt->vec[1][1];
  }

  const Keyframe* prevbezt = (a > 0) ? (bezt - 1) : bezt;

  if (evaltime < prevbezt->vec[1][0]) {
    return prevbezt->vec[1][1];
  }

  /* Evaluation-time occurs within the interval defined by these two keyframes. */
  const float begin = prevbezt->vec[1][1];
  const float change = bezt->vec[1][1] - prevbezt->vec[1][1];
  const float duration = bezt->vec[1][0] - prevbezt->vec[1][0];
  const float time = evaltime - prevbezt->vec[1][0];

  if (duration == 0) {
    return prevbezt->vec[1][1];
  }

  switch (interpolation)
  {
    case InterpolationMode::Bezier:
    {
      float v1[2], v2[2], v3[2], v4[2], opl[32];

      /* Bezier interpolation. */
      /* (v1, v2) are the first keyframe and its 2nd handle. */
      v1[0] = prevbezt->vec[1][0];
      v1[1] = prevbezt->vec[1][1];
      v2[0] = prevbezt->vec[2][0];
      v2[1] = prevbezt->vec[2][1];
      /* (v3, v4) are the last keyframe's 1st handle + the last keyframe. */
      v3[0] = bezt->vec[0][0];
      v3[1] = bezt->vec[0][1];
      v4[0] = bezt->vec[1][0];
      v4[1] = bezt->vec[1][1];

      if (fabsf(v1[1] - v4[1]) < FLT_EPSILON &&
          fabsf(v2[1] - v3[1]) < FLT_EPSILON &&
          fabsf(v3[1] - v4[1]) < FLT_EPSILON)
      {
        /* Optimization: If all the handles are flat/at the same values,
         * the value is simply the shared value
         */
        return v1[1];
      }
      /* Adjust handles so that they don't overlap (forming a loop). */
      curve_correct_bezpart(v1, v2, v3, v4);

      /* Try to get a value for this position - if failure, try another set of points. */
      if (!findzero(evaltime, v1[0], v2[0], v3[0], v4[0], opl)) {
        return 0.0;
      }

      berekeny(v1[1], v2[1], v3[1], v4[1], opl, 1);
      return opl[0];
    }

    case InterpolationMode::Linear:
      /* Linear - simply linearly interpolate between values of the two keyframes. */
      return easing_linear_ease(time, begin, change, duration);

    default:
      return prevbezt->vec[1][1];
  }

  return 0.0f;
}

float dot_qtqt(const float a[4], const float b[4])
{
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

void mul_qt_fl(float q[4], const float f)
{
  q[0] *= f;
  q[1] *= f;
  q[2] *= f;
  q[3] *= f;
}

float normalize_qt(float q[4])
{
  const float len = sqrtf(dot_qtqt(q, q));

  if (len != 0.0f) {
    mul_qt_fl(q, 1.0f / len);
  }
  else {
    q[1] = 1.0f;
    q[0] = q[2] = q[3] = 0.0f;
  }

  return len;
}

void negate_v4_v4(float r[4], const float a[4])
{
  r[0] = -a[0];
  r[1] = -a[1];
  r[2] = -a[2];
  r[3] = -a[3];
}

void copy_qt_qt(float q[4], const float a[4])
{
  q[0] = a[0];
  q[1] = a[1];
  q[2] = a[2];
  q[3] = a[3];
}

void interp_dot_slerp(const float t, const float cosom, float r_w[2])
{
  const float eps = 1e-4f;

  /* within [-1..1] range, avoid aligned axis */
  if (fabsf(cosom) < (1.0f - eps)) [[likely]] {
    float omega, sinom;

    omega = acosf(cosom);
    sinom = sinf(omega);
    r_w[0] = sinf((1.0f - t) * omega) / sinom;
    r_w[1] = sinf(t * omega) / sinom;
  }
  else {
    /* fall back to lerp */
    r_w[0] = 1.0f - t;
    r_w[1] = t;
  }
}

void interp_qt_qtqt(float q[4], const float a[4], const float b[4], const float t)
{
  float quat[4], cosom, w[2];

  cosom = dot_qtqt(a, b);

  /* rotate around shortest angle */
  if (cosom < 0.0f) {
    cosom = -cosom;
    negate_v4_v4(quat, a);
  }
  else {
    copy_qt_qt(quat, a);
  }

  interp_dot_slerp(t, cosom, w);

  q[0] = w[0] * quat[0] + w[1] * b[0];
  q[1] = w[0] * quat[1] + w[1] * b[1];
  q[2] = w[0] * quat[2] + w[1] * b[2];
  q[3] = w[0] * quat[3] + w[1] * b[3];
}

void EvaluateQuaternionAtTime(
  const TAnimCurve& curveW,
  const TAnimCurve& curveX,
  const TAnimCurve& curveY,
  const TAnimCurve& curveZ,
  const float evaltime,
  const InterpolationMode interpolation,
  float quat[4])
{
  switch (interpolation)
  {
    case InterpolationMode::Bezier:
    {
      quat[0] = EvaluateCurveAtTime(curveW, evaltime, interpolation);
      quat[1] = EvaluateCurveAtTime(curveX, evaltime, interpolation);
      quat[2] = EvaluateCurveAtTime(curveY, evaltime, interpolation);
      quat[3] = EvaluateCurveAtTime(curveZ, evaltime, interpolation);
      normalize_qt(quat);
      return;
    }

    case InterpolationMode::Linear:
    {
      const float eps = 1.e-8f;

      /* Evaluation-time occurs somewhere in the middle of the curve. */
      bool exact = false;

      unsigned int a = curve_bezt_binarysearch_index_ex(curveW.data(), evaltime, static_cast<int>(curveW.size()), 0.0001f, &exact);
      const Keyframe* beztW = &curveW[a];
      const Keyframe* beztX = &curveX[a];
      const Keyframe* beztY = &curveY[a];
      const Keyframe* beztZ = &curveZ[a];

      if (exact || fabsf(beztW->vec[1][0] - evaltime) < eps || beztW->vec[1][0] < evaltime) {
        quat[0] = beztW->vec[1][1];
        quat[1] = beztX->vec[1][1];
        quat[2] = beztY->vec[1][1];
        quat[3] = beztZ->vec[1][1];
        return;
      }

      const Keyframe* prevbeztW = (a > 0) ? (beztW - 1) : beztW;
      const Keyframe* prevbeztX = (a > 0) ? (beztX - 1) : beztX;
      const Keyframe* prevbeztY = (a > 0) ? (beztY - 1) : beztY;
      const Keyframe* prevbeztZ = (a > 0) ? (beztZ - 1) : beztZ;

      if (evaltime < prevbeztW->vec[1][0]) {
        quat[0] = prevbeztW->vec[1][1];
        quat[1] = prevbeztX->vec[1][1];
        quat[2] = prevbeztY->vec[1][1];
        quat[3] = prevbeztZ->vec[1][1];
        return;
      }

      float prevq[4];
      float currq[4];

      prevq[0] = prevbeztW->vec[1][1];
      prevq[1] = prevbeztX->vec[1][1];
      prevq[2] = prevbeztY->vec[1][1];
      prevq[3] = prevbeztZ->vec[1][1];
      currq[0] = beztW->vec[1][1];
      currq[1] = beztX->vec[1][1];
      currq[2] = beztY->vec[1][1];
      currq[3] = beztZ->vec[1][1];

      const float duration = beztW->vec[1][0] - prevbeztW->vec[1][0];
      const float time = evaltime - prevbeztW->vec[1][0];
      interp_qt_qtqt(quat, prevq, currq, time / duration);
      return;
    }
  }
}

} // namespace BlenderFCurve
