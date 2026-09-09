/* Copyright (C) 2020 SeriousAlexej
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

#include "TextureMapping_Utils.h"

CMappingDefinition GetMappingDefinitionFromReferenceToTarget(const TUVMappingBasis& uvReference, const TUVMappingBasis& uvTarget)
{
  // transform UV coordinates from default mapping to 'identity'
  FLOAT2D id_X = uvReference.at(2) - uvReference.at(0);
  FLOAT2D id_Y = uvReference.at(1) - uvReference.at(0);
  FLOAT2D id_T = uvReference.at(0);
  FLOATmatrix3D uvToIdentity;
  uvToIdentity(1, 1) = id_X(1);
  uvToIdentity(1, 2) = id_Y(1);
  uvToIdentity(1, 3) = id_T(1);
  uvToIdentity(2, 1) = id_X(2);
  uvToIdentity(2, 2) = id_Y(2);
  uvToIdentity(2, 3) = id_T(2);
  uvToIdentity(3, 1) = 0.0f;
  uvToIdentity(3, 2) = 0.0f;
  uvToIdentity(3, 3) = 1.0f;
  uvToIdentity = uvToIdentity.InverseMatrix();

  // transform UV coordinates from 'identity' mapping to target mapping
  FLOAT2D target_X = uvTarget.at(2) - uvTarget.at(0);
  FLOAT2D target_Y = uvTarget.at(1) - uvTarget.at(0);
  FLOAT2D target_T = uvTarget.at(0);
  FLOATmatrix3D uvToTarget;
  uvToTarget(1, 1) = target_X(1);
  uvToTarget(1, 2) = target_Y(1);
  uvToTarget(1, 3) = target_T(1);
  uvToTarget(2, 1) = target_X(2);
  uvToTarget(2, 2) = target_Y(2);
  uvToTarget(2, 3) = target_T(2);
  uvToTarget(3, 1) = 0.0f;
  uvToTarget(3, 2) = 0.0f;
  uvToTarget(3, 3) = 1.0f;
  uvToTarget = uvToTarget * uvToIdentity;

  // now create the mapping definition
  CMappingDefinition result;
  result.md_fUoS = uvToTarget(1, 1);
  result.md_fUoT = uvToTarget(1, 2);
  result.md_fVoS = -uvToTarget(2, 1);
  result.md_fVoT = -uvToTarget(2, 2);
  result.md_fUOffset = -uvToTarget(1, 3);
  result.md_fVOffset = +uvToTarget(2, 3);
  return result;
}
