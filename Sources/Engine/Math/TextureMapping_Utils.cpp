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



#include <Engine/Math/TextureMapping_Utils.h>

CMappingDefinition GetMappingDefinitionFromReferenceToTarget(const TUVMappingBasis& uvReference, const TUVMappingBasis& uvTarget)
{
  // transform UV coordinates from default mapping to 'identity'
  FLOAT2D id_X = uvReference.at(2) - uvReference.at(0);
  FLOAT2D id_Y = uvReference.at(1) - uvReference.at(0);
  FLOAT2D id_T = uvReference.at(0);
  FLOATmatrix3D uvToIdentity;
  uvToIdentity.matrix[0][0] = id_X(1);
  uvToIdentity.matrix[0][1] = id_Y(1);
  uvToIdentity.matrix[0][2] = id_T(1);
  uvToIdentity.matrix[1][0] = id_X(2);
  uvToIdentity.matrix[1][1] = id_Y(2);
  uvToIdentity.matrix[1][2] = id_T(2);
  uvToIdentity.matrix[2][0] = 0.0f;
  uvToIdentity.matrix[2][1] = 0.0f;
  uvToIdentity.matrix[2][2] = 1.0f;
  uvToIdentity = InverseMatrix(uvToIdentity);

  // transform UV coordinates from 'identity' mapping to target mapping
  FLOAT2D target_X = uvTarget.at(2) - uvTarget.at(0);
  FLOAT2D target_Y = uvTarget.at(1) - uvTarget.at(0);
  FLOAT2D target_T = uvTarget.at(0);
  FLOATmatrix3D uvToTarget;
  uvToTarget.matrix[0][0] = target_X(1);
  uvToTarget.matrix[0][1] = target_Y(1);
  uvToTarget.matrix[0][2] = target_T(1);
  uvToTarget.matrix[1][0] = target_X(2);
  uvToTarget.matrix[1][1] = target_Y(2);
  uvToTarget.matrix[1][2] = target_T(2);
  uvToTarget.matrix[2][0] = 0.0f;
  uvToTarget.matrix[2][1] = 0.0f;
  uvToTarget.matrix[2][2] = 1.0f;
  uvToTarget = uvToTarget * uvToIdentity;

  // now create the mapping definition
  CMappingDefinition result;
  result.md_fUoS = uvToTarget.matrix[0][0];
  result.md_fUoT = uvToTarget.matrix[0][1];
  result.md_fVoS = -uvToTarget.matrix[1][0];
  result.md_fVoT = -uvToTarget.matrix[1][1];
  result.md_fUOffset = -uvToTarget.matrix[0][2];
  result.md_fVOffset = uvToTarget.matrix[1][2];
  return result;
}
