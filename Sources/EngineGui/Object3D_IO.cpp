/* Copyright (c) 2025 SeriousAlexej (Oleksii Sierov).
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

#include <SeriousEngineCppAPI/Math/Object3D.h>

#include "Object3D_IO.h"
#include "ImportedMesh.h"
#include "TextureMapping_Utils.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

void FillObject3DFromMesh(CObject3D& o, const ImportedMesh& mesh)
{
  // create one sector
  CObjectSectorPtr posc = o.ob_aoscSectors.New(1);
  auto& osc = *posc;

  // ------------ Vertices
  INDEX ctVertices = mesh.m_vertices.size();
  osc.osc_aovxVertices.New(ctVertices);
  for (INDEX iVtx = 0; iVtx < ctVertices; iVtx++)
  {
    osc.osc_aovxVertices[iVtx]->value = FLOATtoDOUBLE(mesh.m_vertices[iVtx]);
  }

  // ------------ Materials
  INDEX ctMaterials = mesh.m_materials.size();
  osc.osc_aomtMaterials.New(ctMaterials);
  for (INDEX iMat = 0; iMat < ctMaterials; iMat++)
  {
    *osc.osc_aomtMaterials[iMat] = CObjectMaterial(mesh.m_materials[iMat].cm_strName);
    osc.osc_aomtMaterials[iMat]->omt_Color = mesh.m_materials[iMat].cm_colColor;
  }

  // ------------ Edges and polygons
  INDEX ctTriangles = mesh.m_triangles.size();
  osc.osc_aopoPolygons.New(ctTriangles);
  osc.osc_aoplPlanes.New(ctTriangles);
  // we need 3 edges for each polygon
  osc.osc_aoedEdges.New(ctTriangles * 3);
  for (INDEX iTri = 0; iTri < ctTriangles; iTri++)
  {
    // obtain triangle's vertices
    CObjectVertexPtr pVtx0 = osc.osc_aovxVertices[mesh.m_triangles[iTri].ct_iVtx[0]];
    CObjectVertexPtr pVtx1 = osc.osc_aovxVertices[mesh.m_triangles[iTri].ct_iVtx[1]];
    CObjectVertexPtr pVtx2 = osc.osc_aovxVertices[mesh.m_triangles[iTri].ct_iVtx[2]];

    // create edges
    *osc.osc_aoedEdges[iTri * 3 + 0] = CObjectEdge(*pVtx0, *pVtx1);
    *osc.osc_aoedEdges[iTri * 3 + 1] = CObjectEdge(*pVtx1, *pVtx2);
    *osc.osc_aoedEdges[iTri * 3 + 2] = CObjectEdge(*pVtx2, *pVtx0);

    // create polygon edges
    osc.osc_aopoPolygons[iTri]->opo_PolygonEdges.New(3);
    osc.osc_aopoPolygons[iTri]->opo_PolygonEdges[0]->ope_Edge = *osc.osc_aoedEdges[iTri * 3 + 0];
    osc.osc_aopoPolygons[iTri]->opo_PolygonEdges[1]->ope_Edge = *osc.osc_aoedEdges[iTri * 3 + 1];
    osc.osc_aopoPolygons[iTri]->opo_PolygonEdges[2]->ope_Edge = *osc.osc_aoedEdges[iTri * 3 + 2];

    // set material
    osc.osc_aopoPolygons[iTri]->opo_Material = *osc.osc_aomtMaterials[mesh.m_triangles[iTri].ct_iMaterial];
    osc.osc_aopoPolygons[iTri]->opo_colorColor = *CObjectMaterial_omt_Color(osc.osc_aopoPolygons[iTri]->opo_Material);

    // create and set plane
    osc.osc_aoplPlanes[iTri]->value = DOUBLEplane3D(pVtx0->value, pVtx1->value, pVtx2->value);
    osc.osc_aopoPolygons[iTri]->opo_Plane = *osc.osc_aoplPlanes[iTri];


    // copy UV coordinates to polygon texture mapping
    CMappingVectors mappingVectors;
    mappingVectors.FromPlane_DOUBLE(osc.osc_aoplPlanes[iTri]->value);
    CMappingDefinition defaultMapping;
    FLOAT2D p0_uv = defaultMapping.GetTextureCoordinates(mappingVectors, DOUBLEtoFLOAT(pVtx0->value));
    FLOAT2D p1_uv = defaultMapping.GetTextureCoordinates(mappingVectors, DOUBLEtoFLOAT(pVtx1->value));
    FLOAT2D p2_uv = defaultMapping.GetTextureCoordinates(mappingVectors, DOUBLEtoFLOAT(pVtx2->value));

    for (size_t uvIndex = 0; uvIndex < std::min(size_t(3), mesh.m_uvs.size()); ++uvIndex)
    {
      const auto& uvmap = mesh.m_uvs[uvIndex];
      if (uvmap.empty())
        continue;

      FLOAT2D p0_uvTarget(
        +uvmap[mesh.m_triangles[iTri].ct_iTVtx[uvIndex][0]](1),
        -uvmap[mesh.m_triangles[iTri].ct_iTVtx[uvIndex][0]](2));
      FLOAT2D p1_uvTarget(
        +uvmap[mesh.m_triangles[iTri].ct_iTVtx[uvIndex][1]](1),
        -uvmap[mesh.m_triangles[iTri].ct_iTVtx[uvIndex][1]](2));
      FLOAT2D p2_uvTarget(
        +uvmap[mesh.m_triangles[iTri].ct_iTVtx[uvIndex][2]](1),
        -uvmap[mesh.m_triangles[iTri].ct_iTVtx[uvIndex][2]](2));

      osc.osc_aopoPolygons[iTri]->opo_amdMappings[uvIndex] = GetMappingDefinitionFromReferenceToTarget({ p0_uv, p1_uv, p2_uv }, { p0_uvTarget, p1_uvTarget, p2_uvTarget });
    }
  }
}
