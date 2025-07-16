/* Copyright (c) 2002-2012 Croteam Ltd. 
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

#include "stdafx.h"

#include <SeriousEngineCppAPI/Math/Vector.h>
#include <SeriousEngineCppAPI/Base/FileName.h>
#include <SeriousEngineCppAPI/Base/ErrorReporting.h>

#include "MipMaker.h"

#include <unordered_map>
#include <unordered_set>

namespace
{
  template<typename TObject>
  INDEX _Index(const std::vector<std::unique_ptr<TObject>>& vector, const TObject* object)
  {
    for (size_t i = 0; i < vector.size(); ++i)
      if (vector.at(i).get() == object)
        return (INDEX)i;
    return 0;
  }
} // anonymous namespace

// if vertex removing should occure only inside surfaces
static BOOL _bPreserveSurfaces;

CMipModel::~CMipModel()
{
  mm_amsSurfaces.clear();
  mm_ampPolygons.clear();
  mm_amvVertices.clear();
}

CMipVertex::CMipVertex()
{
}
CMipVertex::~CMipVertex()
{
}

CMipPolygon::CMipPolygon()
{
  mp_pmpvFirstPolygonVertex = NULL;
}
CMipPolygon::~CMipPolygon()
{
  Clear();
}
void CMipPolygon::Clear()
{
  if (mp_pmpvFirstPolygonVertex!=NULL) {
    // delete all vertices in this polygon
    CMipPolygonVertex *pmpvCurrentInPolygon = mp_pmpvFirstPolygonVertex;
    do
    {
      CMipPolygonVertex *pvNextInPolygon = pmpvCurrentInPolygon->mpv_pmpvNextInPolygon;
      delete pmpvCurrentInPolygon;
      pmpvCurrentInPolygon = pvNextInPolygon;
    }
    while( pmpvCurrentInPolygon != mp_pmpvFirstPolygonVertex);
  }
  mp_pmpvFirstPolygonVertex = NULL;
}

ImportedMesh CMipModel::GetMesh()
{
  ImportedMesh mesh;
  // add vertices to sector
  mesh.m_uvs.resize(1, {});
  mesh.m_vertices.resize(mm_amvVertices.size(), FLOAT3D(0, 0, 0));
  INDEX iVertice = 0;
  for (auto& itVertice : mm_amvVertices)
  {
    FLOAT3D vRestFrame = itVertice->mv_vRestFrameCoordinate;
    mesh.m_vertices[ iVertice] = vRestFrame;
    iVertice++;
  }

  // add mip surfaces as materials to object 3d
  mesh.m_materials.resize(mm_amsSurfaces.size(), ImportedMesh::Material{});
  INDEX iMaterial = 0;
  for (auto& itSurface : mm_amsSurfaces)
  {
    mesh.m_materials[ iMaterial].cm_strName = itSurface->ms_strName;
    mesh.m_materials[ iMaterial].cm_colColor = itSurface->ms_colColor;
    iMaterial ++;
  }

  std::unordered_map<FLOAT2D, INDEX, FLOAT2DHasher> texCoordsRemap;

  // add polygons to object 3d
  for (auto& itPolygon : mm_ampPolygons)
  {
    // prepare array of polygon vertex indices
    INDEX aivVertices[32];
    INDEX texCoords[32];
    CMipPolygonVertex *pmpvPolygonVertex = itPolygon->mp_pmpvFirstPolygonVertex;
    INDEX ctPolygonVertices = 0;
    do
    {
      ASSERT( ctPolygonVertices<32);
      if( ctPolygonVertices >= 32) break;
      // add global index of vertex to list of vertex indices of polygon
      INDEX texCoordIndex;
      FLOAT2D vtxUV = pmpvPolygonVertex->m_uv;
      auto foundPos = texCoordsRemap.find(vtxUV);
      if (foundPos != texCoordsRemap.end())
      {
        texCoordIndex = foundPos->second;
      }
      else
      {
        texCoordIndex = mesh.m_uvs[0].size();
        mesh.m_uvs[0].push_back(vtxUV);
        texCoordsRemap[vtxUV] = texCoordIndex;
      }
      texCoords[ctPolygonVertices] = texCoordIndex;
      aivVertices[ ctPolygonVertices] = _Index(mm_amvVertices, pmpvPolygonVertex->mpv_pmvVertex);
      pmpvPolygonVertex = pmpvPolygonVertex->mpv_pmpvNextInPolygon;
      ctPolygonVertices ++;
    }
    while( pmpvPolygonVertex != itPolygon->mp_pmpvFirstPolygonVertex);
    // add current polygon splitted to triangles
    for (INDEX i = 2; i < ctPolygonVertices; ++i)
    {
      ImportedMesh::Triangle triangle;
      triangle.ct_iVtx[0] = aivVertices[0];
      triangle.ct_iVtx[1] = aivVertices[i - 1];
      triangle.ct_iVtx[2] = aivVertices[i];
      triangle.ct_iTVtx.resize(1, {texCoords[0], texCoords[i - 1], texCoords[i]});
      triangle.ct_iMaterial = itPolygon->mp_iSurface;
      mesh.m_triangles.push_back(triangle);
    }
  }

  return mesh;
}

CMipModel::CMipModel(const ImportedMesh& mesh)
{
  INDEX ctInvalidVertices = 0;
  CTString strInvalidVertices;
  char achrErrorVertice[ 256];

  // add mip surface
  mm_amsSurfaces.reserve(mesh.m_materials.size());
  // copy material data from object 3d to mip surfaces
  for (const auto& material : mesh.m_materials)
  {
    auto& itSurface = mm_amsSurfaces.emplace_back(std::make_unique<CMipSurface>());
    itSurface->ms_strName = material.cm_strName;
    itSurface->ms_colColor = material.cm_colColor;
  }

  // add mip vertices
  mm_amvVertices.reserve(mesh.m_vertices.size());
  // copy vertice coordinates from object3d to mip vertices
  for (const auto& vertex : mesh.m_vertices)
  {
    auto& itVertice = mm_amvVertices.emplace_back(std::make_unique<CMipVertex>());
    itVertice->m_vector = vertex;
    itVertice->mv_vRestFrameCoordinate = vertex;
    // calculate bounding box of all vertices
    mm_boxBoundingBox |= vertex;
  }

  // add mip polygons
  mm_ampPolygons.reserve(mesh.m_triangles.size());
  // copy polygons object 3d to mip polygons
  const size_t uv0 = mesh.m_defaultUVChannel;
  for (const auto& triangle : mesh.m_triangles)
  {
    auto& itPolygon = mm_ampPolygons.emplace_back(std::make_unique<CMipPolygon>());
    CMipPolygon &mpPolygon = *itPolygon;
    // allocate polygon vertices
    CMipPolygonVertex *ppvPolygonVertices[3];
    INDEX iPolygonVertice=0;
    for( ; iPolygonVertice<3; iPolygonVertice++)
    {
      // allocate one polygon vertex
      ppvPolygonVertices[ iPolygonVertice] = new( CMipPolygonVertex);
    }

    // for each polygon vertex in the polygon
    for( iPolygonVertice=0; iPolygonVertice<3; iPolygonVertice++)
    {
      CMipPolygonVertex *ppvPolygonVertex = ppvPolygonVertices[ iPolygonVertice];
      // get the object vertex as first vertex of the edge
      INDEX iVertexInSector = triangle.ct_iVtx[iPolygonVertice];
      // set references to mip polygon and mip vertex
      ppvPolygonVertex->mpv_pmpPolygon = &mpPolygon;
      ppvPolygonVertex->m_uv = mesh.m_uvs[uv0][triangle.ct_iTVtx[uv0][iPolygonVertice]];
      ppvPolygonVertex->mpv_pmvVertex = mm_amvVertices[iVertexInSector].get();
      // link to previous and next vertices in the mip polygon
      INDEX iNext=(iPolygonVertice+1)%3;
      ppvPolygonVertex->mpv_pmpvNextInPolygon = ppvPolygonVertices[ iNext];
    }

    // set first polygon vertex ptr and surface index to polygon
    itPolygon->mp_pmpvFirstPolygonVertex = ppvPolygonVertices[ 0];
    itPolygon->mp_iSurface = triangle.ct_iMaterial;
  }

  if( ctInvalidVertices != 0)
  {
    sprintf( achrErrorVertice,
      "%d invalid vertices found\n-------------------------\n\n", ctInvalidVertices);
    strInvalidVertices = CTString( achrErrorVertice) + strInvalidVertices;
    strInvalidVertices.Save_t( CTFileName(CTString("Temp\\ErrorVertices.txt")));
    ThrowF_t( "%d invalid vertices found.\nUnable to create mip models.\nList of vertices "
      "that must be fixed can be found in file: \"Temp\\ErrorVertices.txt\".",
      ctInvalidVertices);
  }
}

void CMipModel::CheckObjectValidity(void)
{
  // for all polygons
  for (auto& itPolygon : mm_ampPolygons)
  {
    CMipPolygon &mpMipPolygon = *itPolygon;
    CMipPolygonVertex *pvFirstInPolygon = mpMipPolygon.mp_pmpvFirstPolygonVertex;
    CMipPolygonVertex *pvCurrent = pvFirstInPolygon;
    do
    {
      ASSERT( pvCurrent->mpv_pmpPolygon == &mpMipPolygon);
      pvCurrent = pvCurrent->mpv_pmpvNextInPolygon;
    }
    while( pvCurrent != pvFirstInPolygon);
  }
}

FLOAT CMipModel::GetGoodness(CMipVertex *pmvSource, CMipVertex *pmvTarget)
{
  if( (_bPreserveSurfaces) && (pmvSource->mv_iSurface == -2) ) return -10000.0f;
  FLOAT fDistST = ( pmvSource->m_vector - pmvTarget->m_vector).Length();
  FLOAT fDistBBoxCenterT = ( mm_boxBoundingBox.Center() - pmvTarget->m_vector).Length();
  return fDistBBoxCenterT/100.0f + 1.0f/fDistST;
}

INDEX CMipModel::FindSurfacesForVertices(void)
{
  for (auto& itVertice : mm_amvVertices)
    itVertice->mv_iSurface = -1;

  // for all polygons
  for (auto& itPolygon : mm_ampPolygons)
  {
    // for all vertices in this polygon
    CMipPolygonVertex *pmpvCurrentInPolygon = itPolygon->mp_pmpvFirstPolygonVertex;
    do
    {
      CMipVertex *pmvVertex = pmpvCurrentInPolygon->mpv_pmvVertex;
      if( pmvVertex->mv_iSurface == -1) pmvVertex->mv_iSurface = itPolygon->mp_iSurface;
      else if( pmvVertex->mv_iSurface == -2); // do nothing
      else if( pmvVertex->mv_iSurface == itPolygon->mp_iSurface);  // do nothing
      else pmvVertex->mv_iSurface = -2;
      pmpvCurrentInPolygon = pmpvCurrentInPolygon->mpv_pmpvNextInPolygon;
    }
    while( pmpvCurrentInPolygon != itPolygon->mp_pmpvFirstPolygonVertex);
  }

  // count vertices that are sourounded with only one surface
  INDEX ctVerticesWithOneSurface = 0;
  // for all vertices
  for (auto& itVertice : mm_amvVertices)
    if( itVertice->mv_iSurface >= 0) ctVerticesWithOneSurface++;

  return ctVerticesWithOneSurface;
}

void CMipModel::JoinVertexPair( CMipVertex *pmvBestSource, CMipVertex *pmvBestTarget)
{
  // for all polygons
  for (auto& itPolygon : mm_ampPolygons)
  {
    // for all vertices in this polygon
    CMipPolygonVertex *pmpvCurrentInPolygon = itPolygon->mp_pmpvFirstPolygonVertex;
    do
    {
      if( pmpvCurrentInPolygon->mpv_pmvVertex == pmvBestSource)
      {
        pmpvCurrentInPolygon->mpv_pmvVertex = pmvBestTarget;
      }
      pmpvCurrentInPolygon = pmpvCurrentInPolygon->mpv_pmpvNextInPolygon;
    }
    while( pmpvCurrentInPolygon != itPolygon->mp_pmpvFirstPolygonVertex);
  }
  // delete best source vertex
  mm_amvVertices.erase(
    std::remove_if(mm_amvVertices.begin(), mm_amvVertices.end(),
      [=](const auto& v) { return v.get() == pmvBestSource; }),
    mm_amvVertices.end());

  // for all polygons
  for (auto& itPolygon : mm_ampPolygons)
  {
    // for all vertices in this polygon
    CMipPolygonVertex *pmpvCurrentInPolygon = itPolygon->mp_pmpvFirstPolygonVertex;
    do
    {
      CMipPolygonVertex *pmpvSuccesor = pmpvCurrentInPolygon->mpv_pmpvNextInPolygon;
      // if current vertex and its sucessor are same mip vertex
      if( pmpvCurrentInPolygon->mpv_pmvVertex == pmpvSuccesor->mpv_pmvVertex)
      {
        // enable looping even if vertex that is first in polygon is deleted
        if( pmpvSuccesor == itPolygon->mp_pmpvFirstPolygonVertex)
        {
          itPolygon->mp_pmpvFirstPolygonVertex = pmpvSuccesor->mpv_pmpvNextInPolygon;
        }
        // relink current vertex over sucessor
        pmpvCurrentInPolygon->mpv_pmpvNextInPolygon = pmpvSuccesor->mpv_pmpvNextInPolygon;
        // delete sucessor vertex
        delete pmpvSuccesor;
      }
      pmpvCurrentInPolygon = pmpvCurrentInPolygon->mpv_pmpvNextInPolygon;
    }
    while( pmpvCurrentInPolygon != itPolygon->mp_pmpvFirstPolygonVertex);
  }

  std::unordered_set<CMipPolygon*> cPolygonsToDelete;
  // for all polygons
  for (auto& itPolygon : mm_ampPolygons)
  {
    CMipPolygonVertex *pmpvFirst = itPolygon->mp_pmpvFirstPolygonVertex;
    // if this is polygon with one or two vertices
    if( (pmpvFirst->mpv_pmpvNextInPolygon == pmpvFirst) ||
        (pmpvFirst->mpv_pmpvNextInPolygon->mpv_pmpvNextInPolygon == pmpvFirst) )
    {
      // add it to container for deleting
      cPolygonsToDelete.insert( itPolygon.get());
    }
  }
  // delete polygons
  mm_ampPolygons.erase(
    std::remove_if(mm_ampPolygons.begin(), mm_ampPolygons.end(),
      [&](const auto& v) { return cPolygonsToDelete.contains(v.get()); }),
    mm_ampPolygons.end());
}

void CMipModel::FindBestVertexPair( CMipVertex *&pmvBestSource, CMipVertex *&pmvBestTarget)
{
  pmvBestSource = NULL;
  pmvBestTarget = NULL;
  FLOAT fBestGoodnes = -999999.9f;
  // for all polygons
  for (auto& itPolygon : mm_ampPolygons)
  {
    // for all vertices in this polygon
    CMipPolygonVertex *pmpvCurrentInPolygon = itPolygon->mp_pmpvFirstPolygonVertex;
    do
    {
      CMipVertex *pmvSource = pmpvCurrentInPolygon->mpv_pmvVertex;
      CMipVertex *pmvDestination = pmpvCurrentInPolygon->mpv_pmpvNextInPolygon->mpv_pmvVertex;
      FLOAT fCurrentGoodnes = GetGoodness( pmvSource, pmvDestination);
      if( fCurrentGoodnes > fBestGoodnes)
      {
        fBestGoodnes = fCurrentGoodnes;
        pmvBestSource = pmvSource;
        pmvBestTarget = pmvDestination;
      }
      // now for inverted order
      pmvSource = pmpvCurrentInPolygon->mpv_pmpvNextInPolygon->mpv_pmvVertex;
      pmvDestination = pmpvCurrentInPolygon->mpv_pmvVertex;
      fCurrentGoodnes = GetGoodness( pmvSource, pmvDestination);
      if( fCurrentGoodnes > fBestGoodnes)
      {
        fBestGoodnes = fCurrentGoodnes;
        pmvBestSource = pmvSource;
        pmvBestTarget = pmvDestination;
      }
      pmpvCurrentInPolygon = pmpvCurrentInPolygon->mpv_pmpvNextInPolygon;
    }
    while( pmpvCurrentInPolygon != itPolygon->mp_pmpvFirstPolygonVertex);
  }
  ASSERT( (pmvBestSource != NULL) && (pmvBestTarget != NULL) );
  ASSERT( pmvBestSource != pmvBestTarget);
}

void CMipModel::RemoveUnusedVertices(void)
{
  // if there are no vertices
  if (mm_amvVertices.empty()) {
    // do nothing
    return;
  }

  // clear all vertex tags
  for (auto& itmvtx : mm_amvVertices)
    itmvtx->mv_bUsed = FALSE;

  // mark all vertices that are used by some polygon
  for (auto& itpo : mm_ampPolygons)
  {
    CMipPolygonVertex *pmpvCurrentInPolygon = itpo->mp_pmpvFirstPolygonVertex;
    do
    {
      pmpvCurrentInPolygon->mpv_pmvVertex->mv_bUsed = TRUE;
      pmpvCurrentInPolygon = pmpvCurrentInPolygon->mpv_pmpvNextInPolygon;
    }
    while( pmpvCurrentInPolygon != itpo->mp_pmpvFirstPolygonVertex);
  }

  // find number of used vertices
  INDEX ctUsedVertices = 0;
  for (auto& itmvtx : mm_amvVertices)
    if (itmvtx->mv_bUsed) {
      ctUsedVertices++;
    }

  // create a new array with as much vertices as we have counted in last pass
  std::vector<std::unique_ptr<CMipVertex>> amvxNew;
  amvxNew.reserve(ctUsedVertices);
  for (INDEX i = 0; i < ctUsedVertices; ++i)
    amvxNew.emplace_back(std::make_unique<CMipVertex>());
  INDEX pmvxUsed = 0;

  // for each vertex
  for (auto& itmvtx : mm_amvVertices)
  {
    // if it is used
    if (itmvtx->mv_bUsed) {
      // copy it to new array
      *amvxNew[pmvxUsed] = *itmvtx;
      // set its remap pointer into new array
      itmvtx->mv_pmvxRemap = amvxNew[pmvxUsed].get();
      pmvxUsed++;
    // if it is not used
    } else {
      // clear its remap pointer (for debugging)
      #ifndef NDEBUG
      itmvtx->mv_pmvxRemap = NULL;
      #endif
    }
  }

  // for each polygon
  for (auto& itpo : mm_ampPolygons)
  {
    // for each polygon vertex in polygon
    CMipPolygonVertex *pmpvCurrentInPolygon = itpo->mp_pmpvFirstPolygonVertex;
    do
    {
      // remap pointer to vertex
      pmpvCurrentInPolygon->mpv_pmvVertex = pmpvCurrentInPolygon->mpv_pmvVertex->mv_pmvxRemap;
      pmpvCurrentInPolygon = pmpvCurrentInPolygon->mpv_pmpvNextInPolygon;
    }
    while( pmpvCurrentInPolygon != itpo->mp_pmpvFirstPolygonVertex);
  }

  // use new array of vertices instead of the old one
  mm_amvVertices = std::move(amvxNew);
}

BOOL CMipModel::CreateMipModel_t(INDEX ctVerticesToRemove, INDEX iSurfacePreservingFactor)
{
  if( ctVerticesToRemove > static_cast<INDEX>(mm_amvVertices.size()))
    return FALSE;

  for( INDEX ctRemoved = 0; ctRemoved<ctVerticesToRemove; ctRemoved++)
  {
    INDEX ctVerticesWithOneSurface = FindSurfacesForVertices();

    // setup flag for preserving surfaces
    _bPreserveSurfaces = TRUE;
    if( (ctVerticesWithOneSurface == 0) ||
        (( ((FLOAT)ctVerticesWithOneSurface) / mm_amvVertices.size())*100 <=
        (100-iSurfacePreservingFactor)) )
    {
      _bPreserveSurfaces = FALSE;
    }

    CMipVertex *pmvBestSource, *pmvBestTarget;
    FindBestVertexPair( pmvBestSource, pmvBestTarget);
    JoinVertexPair( pmvBestSource, pmvBestTarget);
    RemoveUnusedVertices();
    if( mm_amvVertices.empty()) return FALSE;
  }
  return TRUE;
}
