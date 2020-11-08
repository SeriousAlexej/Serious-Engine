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



#include <Engine/Math/Object3D.h>

#include <Engine/Base/Registry.h>
#include <Engine/Base/Stream.h>
#include <Engine/Base/Memory.h>
#include <Engine/Base/ErrorReporting.h>
#include <Engine/Graphics/Color.h>


#include <Engine/Templates/DynamicContainer.cpp>
#include <Engine/Templates/DynamicArray.cpp>
#include <Engine/Templates/StaticStackArray.cpp>

#include <Engine/Math/TextureMapping_Utils.h>

#include <assimp/Importer.hpp>
#include <assimp/importerdesc.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <vector>

#ifdef max
#undef max
#endif

namespace
{
  struct aiHasher
  {
    std::size_t operator()(const aiVector3D& vec3d) const
    {
      std::size_t result = 0;
      HashCombine(result, vec3d.x);
      HashCombine(result, vec3d.y);
      HashCombine(result, vec3d.z);
      return result;
    }
  };

  INDEX AI_GetNumFaces(const aiScene* aiSceneMain)
  {
    INDEX faces = 0;
    for (size_t i = 0; i < aiSceneMain->mNumMeshes; ++i)
      faces += aiSceneMain->mMeshes[i]->mNumFaces;
    return faces;
  }

  BOOL AI_SceneIsValid(const aiScene* aiSceneMain)
  {
    INDEX nonEmptyMeshes = 0;
    INDEX nonEmptyWithUV = 0;
    for (size_t i = 0; i < aiSceneMain->mNumMeshes; ++i)
      if (aiSceneMain->mMeshes[i]->HasFaces())
      {
        ++nonEmptyMeshes;
        if (aiSceneMain->mMeshes[i]->HasTextureCoords(0))
          ++nonEmptyWithUV;
      }

    return nonEmptyWithUV == nonEmptyMeshes && nonEmptyMeshes > 0;
  }
} // anonymous namespace

#undef W
#undef NONE

void FillConversionArrays_t(const FLOATmatrix3D &mTransform, const aiScene* aiSceneMain);
void ClearConversionArrays( void);


/*
 *  Intermediate structures used for converting from Exploration 3D data format into O3D
 */
struct ConversionTriangle {
  INDEX ct_iVtx[3];     // indices of vertices
  INDEX ct_iTVtx[3*3];    // indices of texture vertices
  INDEX ct_iMaterial;   // index of material
};

struct ConversionMaterial {
  ULONG cm_ulTag;                           // for recognition of material
  CTString cm_strName;                      // material's name
  COLOR cm_colColor;                        // material's color
  CDynamicContainer<INDEX> ms_Polygons;     // indices of polygons in this material
};
// conversion arrays
CDynamicContainer<ConversionMaterial> acmMaterials;
CStaticArray<ConversionTriangle> actTriangles;
CStaticArray<FLOAT3D> avVertices;
CStaticStackArray<FLOAT3D> avDst;
CStaticArray<FLOAT2D> avTextureVertices[3];
CStaticArray<INDEX> aiRemap;

/////////////////////////////////////////////////////////////////////////////
// Helper functions

//--------------------------------------------------------------------------------------------
class CObjectSectorLock {
private:
	CObjectSector *oscl_posc;						// ptr to object sector that will do lock/unlock
public:
	CObjectSectorLock( CObjectSector *posc);		// lock all object sector arrays
	~CObjectSectorLock();										// unlock all object sector arrays
};

//--------------------------------------------------------------------------------------------
/*
 * To lock all object 3D dyna arrays one must create an instance of CObject3DLock.
 * Locking job is done inside class constructor
 */
CObjectSectorLock::CObjectSectorLock( CObjectSector *posc) {
	ASSERT( posc != NULL);
  oscl_posc = posc;
  posc->LockAll();
}

//--------------------------------------------------------------------------------------------
/*
 * Unlocking of all object 3D dynamic arrays will occur automatically when exiting
 * current scope (routine). This is done in class destructor
 */
CObjectSectorLock::~CObjectSectorLock() {
  oscl_posc->UnlockAll();
}

//--------------------------------------------------------------------------------------------
// function recognizes and loads many 3D file formats, throws char* errors
BOOL _bBatchLoading = FALSE;

// start/end batch loading of 3d objects
void CObject3D::BatchLoading_t(BOOL bOn)
{
  // check for dummy calls
  if (!_bBatchLoading==!bOn) {
    return;
  }

  _bBatchLoading = bOn;
}

const std::vector<CObject3D::TFormatDescr>& CObject3D::GetSupportedFormats()
{
  static std::vector<TFormatDescr> formats;

  if (formats.empty())
  {
    Assimp::Importer importer;
    for (size_t i = 0; i < importer.GetImporterCount(); ++i)
    {
      const aiImporterDesc* pDescription = importer.GetImporterInfo(i);
      TFormatDescr descr;
      descr.first = pDescription->mName;
      descr.second = "*.";
      for (const char* c = pDescription->mFileExtensions; *c; ++c)
      {
        if (std::isspace(*c))
          descr.second += ";*.";
        else
          descr.second += *c;
      }
      descr.first += " (" + descr.second + ")";
      formats.emplace_back(std::move(descr));
    }
    std::sort(formats.begin(), formats.end(), [](const TFormatDescr& lhs, const TFormatDescr& rhs) { return lhs.first < rhs.first; });
  }

  return formats;
}

void CObject3D::LoadAny3DFormat_t(const CTFileName &fnmFileName, const FLOATmatrix3D &mTransform)
{
  BOOL bWasOn = _bBatchLoading;
  try {
    if (!_bBatchLoading) {
      BatchLoading_t(TRUE);
    }
    // call file load with file's full path name
    CTString strFile = _fnmApplicationPath+fnmFileName;
    char acFile[MAX_PATH];
    wsprintfA(acFile,"%s",strFile);

    Assimp::Importer importerWithMaterials;
    // do not read normals from input file
    importerWithMaterials.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
    const aiScene* aiSceneMain = importerWithMaterials.ReadFile(acFile,
      aiProcess_JoinIdenticalVertices |
      aiProcess_Triangulate |
      aiProcess_PreTransformVertices |
      aiProcess_GenUVCoords |
      aiProcess_RemoveComponent |
      aiProcess_FlipUVs);

    // if scene is successefuly loaded
    if(aiSceneMain && aiSceneMain->mNumMeshes > 0 && aiSceneMain->mNumMaterials > 0)
    {
      FillConversionArrays_t(mTransform, aiSceneMain);
      ConvertArraysToO3D();
      ClearConversionArrays();
    }
    else 
    {
      ThrowF_t("Unable to load 3D object: %s", (const char *)fnmFileName);
    }
  
    if (!bWasOn) {
      BatchLoading_t(FALSE);
    }
  } catch (char *) {
    if (!bWasOn) {
      BatchLoading_t(FALSE);
    }
    throw;
  }
}

/*
 * Converts data from Assimp scene format into arrays used for conversion to O3D
 */
void FillConversionArrays_t(const FLOATmatrix3D &mTransform, const aiScene* aiSceneMain)
{
  // all polygons must be triangles
  if(!AI_SceneIsValid(aiSceneMain))
  {
    throw("Error: UV map must be in channel 0!");
  }

  // check if we need flipping (if matrix is flipping, polygons need to be flipped)
  const FLOATmatrix3D &m = mTransform;
  FLOAT fDet = 
    m(1,1)*(m(2,2)*m(3,3)-m(2,3)*m(3,2))+
    m(1,2)*(m(2,3)*m(3,1)-m(2,1)*m(3,3))+
    m(1,3)*(m(2,1)*m(3,2)-m(2,2)*m(3,1));
  FLOAT bFlipped = fDet<0;

  // ------------  Convert object vertices (coordinates)
  std::unordered_map<aiVector3D, INDEX, aiHasher> uniqueVertices;
  std::vector<aiVector3D*> orderedUniqueVertices;
  for (size_t i = 0; i < aiSceneMain->mNumMeshes; ++i)
  {
    auto* mesh = aiSceneMain->mMeshes[i];
    for (size_t v = 0; v < mesh->mNumVertices; ++v)
    {
      if (uniqueVertices.find(mesh->mVertices[v]) == uniqueVertices.end())
      {
        uniqueVertices[mesh->mVertices[v]] = orderedUniqueVertices.size();
        orderedUniqueVertices.push_back(&mesh->mVertices[v]);
      }
    }
  }
  avVertices.New(orderedUniqueVertices.size());
  // copy vertices
  for (size_t iVtx = 0; iVtx < orderedUniqueVertices.size(); ++iVtx)
  {
    avVertices[iVtx] = ((FLOAT3D&)*orderedUniqueVertices[iVtx]) * mTransform;
    avVertices[iVtx](1) = -avVertices[iVtx](1);
    avVertices[iVtx](3) = -avVertices[iVtx](3);
  }
  orderedUniqueVertices.clear();

  // ------------ Convert object's mapping vertices (texture vertices)
  std::unordered_map<aiVector3D, INDEX, aiHasher> uniqueTexCoords[3];
  for (size_t iUVMapIndex = 0; iUVMapIndex < 3; ++iUVMapIndex)
  {
    std::vector<aiVector3D*> orderedUniqueTexCoords;
    for (size_t i = 0; i < aiSceneMain->mNumMeshes; ++i)
    {
      auto* mesh = aiSceneMain->mMeshes[i];
      if (!mesh->HasTextureCoords(iUVMapIndex))
        continue;

      for (size_t v = 0; v < mesh->mNumVertices; ++v)
      {
        if (uniqueTexCoords[iUVMapIndex].find(mesh->mTextureCoords[iUVMapIndex][v]) == uniqueTexCoords[iUVMapIndex].end())
        {
          uniqueTexCoords[iUVMapIndex][mesh->mTextureCoords[iUVMapIndex][v]] = orderedUniqueTexCoords.size();
          orderedUniqueTexCoords.push_back(&mesh->mTextureCoords[iUVMapIndex][v]);
        }
      }
    }
    if (orderedUniqueTexCoords.empty())
      continue;

    avTextureVertices[iUVMapIndex].New(orderedUniqueTexCoords.size());
    // copy texture vertices
    for (size_t iTVtx = 0; iTVtx < orderedUniqueTexCoords.size(); ++iTVtx)
      avTextureVertices[iUVMapIndex][iTVtx] = (FLOAT2D&)*orderedUniqueTexCoords[iTVtx];
  }

  // ------------ Organize triangles as list of surfaces
  // allocate triangles
  actTriangles.New(AI_GetNumFaces(aiSceneMain));

  acmMaterials.Lock();
  
  // sort triangles per surfaces
  INDEX trianglesOffset = 0;
  for (size_t i = 0; i < aiSceneMain->mNumMeshes; ++i)
  {
    auto* mesh = aiSceneMain->mMeshes[i];
    for (INDEX iTriangle = 0; iTriangle < mesh->mNumFaces; iTriangle++)
    {
      ConversionTriangle& ctTriangle = actTriangles[trianglesOffset + iTriangle];

      const aiFace* ai_face = &mesh->mFaces[iTriangle];
      // copy vertex indices
      if (bFlipped) {
        ctTriangle.ct_iVtx[0] = uniqueVertices[mesh->mVertices[ai_face->mIndices[2]]];
        ctTriangle.ct_iVtx[1] = uniqueVertices[mesh->mVertices[ai_face->mIndices[1]]];
        ctTriangle.ct_iVtx[2] = uniqueVertices[mesh->mVertices[ai_face->mIndices[0]]];
      }
      else {
        ctTriangle.ct_iVtx[0] = uniqueVertices[mesh->mVertices[ai_face->mIndices[0]]];
        ctTriangle.ct_iVtx[1] = uniqueVertices[mesh->mVertices[ai_face->mIndices[1]]];
        ctTriangle.ct_iVtx[2] = uniqueVertices[mesh->mVertices[ai_face->mIndices[2]]];
      }


      for (size_t iUVMapIndex = 0; iUVMapIndex < 3; ++iUVMapIndex)
      {
        if (!mesh->HasTextureCoords(iUVMapIndex))
          continue;

        // copy texture vertex indices
        if (bFlipped) {
          ctTriangle.ct_iTVtx[iUVMapIndex*3 + 0] = uniqueTexCoords[iUVMapIndex][mesh->mTextureCoords[iUVMapIndex][ai_face->mIndices[2]]];
          ctTriangle.ct_iTVtx[iUVMapIndex*3 + 1] = uniqueTexCoords[iUVMapIndex][mesh->mTextureCoords[iUVMapIndex][ai_face->mIndices[1]]];
          ctTriangle.ct_iTVtx[iUVMapIndex*3 + 2] = uniqueTexCoords[iUVMapIndex][mesh->mTextureCoords[iUVMapIndex][ai_face->mIndices[0]]];
        } else {
          ctTriangle.ct_iTVtx[iUVMapIndex*3 + 0] = uniqueTexCoords[iUVMapIndex][mesh->mTextureCoords[iUVMapIndex][ai_face->mIndices[0]]];
          ctTriangle.ct_iTVtx[iUVMapIndex*3 + 1] = uniqueTexCoords[iUVMapIndex][mesh->mTextureCoords[iUVMapIndex][ai_face->mIndices[1]]];
          ctTriangle.ct_iTVtx[iUVMapIndex*3 + 2] = uniqueTexCoords[iUVMapIndex][mesh->mTextureCoords[iUVMapIndex][ai_face->mIndices[2]]];
        }
      }

      // obtain material
      ULONG materialIndex = mesh->mMaterialIndex;
      BOOL bNewMaterial = TRUE;
      // attach triangle into one material
      for (INDEX iMat = 0; iMat < acmMaterials.Count(); iMat++)
      {
        // if this material already exist in array of materu
        if (acmMaterials[iMat].cm_ulTag == materialIndex)
        {
          // set index of surface
          ctTriangle.ct_iMaterial = iMat;
          // add triangle into surface list of triangles
          INDEX* piNewTriangle = new INDEX(1);
          *piNewTriangle = trianglesOffset+iTriangle;
          acmMaterials[iMat].ms_Polygons.Add(piNewTriangle);
          bNewMaterial = FALSE;
          continue;
        }
      }
      // if material hasn't been added yet
      if (bNewMaterial)
      {
        // add new material
        ConversionMaterial* pcmNew = new ConversionMaterial;
        acmMaterials.Unlock();
        acmMaterials.Add(pcmNew);
        acmMaterials.Lock();
        // set polygon's material index 
        INDEX iNewMaterial = acmMaterials.Count() - 1;
        ctTriangle.ct_iMaterial = iNewMaterial;
        // add triangle into new surface's list of triangles
        INDEX* piNewTriangle = new INDEX(1);
        *piNewTriangle = trianglesOffset+iTriangle;
        acmMaterials[iNewMaterial].ms_Polygons.Add(piNewTriangle);

        // remember recognition tag (ptr)
        pcmNew->cm_ulTag = materialIndex;

        // ---------- Set material's name
        // if not default material

        aiString materialName;
        aiSceneMain->mMaterials[materialIndex]->Get(AI_MATKEY_NAME, materialName);

        if (materialName.length > 1)
        {
          acmMaterials[iNewMaterial].cm_strName = CTString(materialName.C_Str());
          // get color
          const double materialCoefficient = static_cast<double>(i+1) / aiSceneMain->mNumMeshes;
          COLOR colColor = static_cast<COLOR>(std::numeric_limits<COLOR>::max() * materialCoefficient);
          acmMaterials[iNewMaterial].cm_colColor = colColor;
        }
        else
        {
          acmMaterials[iNewMaterial].cm_strName = "Default";
          acmMaterials[iNewMaterial].cm_colColor = C_GRAY;
        }
      }
    }
    trianglesOffset += mesh->mNumFaces;
  }
  acmMaterials.Unlock();
}

void ClearConversionArrays( void)
{
  acmMaterials.Clear();
  actTriangles.Clear();
  avVertices.Clear();
  for (size_t i = 0; i < 3; ++i)
    avTextureVertices[i].Clear();
  aiRemap.Clear();
}

/*
 * Convert streihgtfowrard from intermediate structures into O3D
 */
void CObject3D::ConvertArraysToO3D( void)
{
  acmMaterials.Lock();
  // create one sector
  CObjectSector &osc = *ob_aoscSectors.New(1);
  // this will lock at the instancing and unlock while destructing all sector arrays
	CObjectSectorLock OSectorLock(&osc);

  // ------------ Vertices
  INDEX ctVertices = avVertices.Count();
  CObjectVertex *pVtx = osc.osc_aovxVertices.New(ctVertices);
  for(INDEX iVtx=0; iVtx<ctVertices; iVtx++)
  {
    pVtx[ iVtx] = FLOATtoDOUBLE( avVertices[iVtx]);
  }
	
  // ------------ Materials
  INDEX ctMaterials = acmMaterials.Count();
  osc.osc_aomtMaterials.New( ctMaterials);
  for( INDEX iMat=0; iMat<ctMaterials; iMat++)
  {
    osc.osc_aomtMaterials[iMat] = CObjectMaterial( acmMaterials[iMat].cm_strName);
    osc.osc_aomtMaterials[iMat].omt_Color = acmMaterials[iMat].cm_colColor;
  }

  // ------------ Edges and polygons
  INDEX ctTriangles = actTriangles.Count();
	CObjectPolygon *popo = osc.osc_aopoPolygons.New(ctTriangles);
	CObjectPlane *popl = osc.osc_aoplPlanes.New(ctTriangles);
  // we need 3 edges for each polygon
  CObjectEdge *poedg = osc.osc_aoedEdges.New(ctTriangles*3);
  for(INDEX iTri=0; iTri<ctTriangles; iTri++)
  {
    // obtain triangle's vertices
    CObjectVertex *pVtx0 = &osc.osc_aovxVertices[ actTriangles[iTri].ct_iVtx[0]];
    CObjectVertex *pVtx1 = &osc.osc_aovxVertices[ actTriangles[iTri].ct_iVtx[1]];
    CObjectVertex *pVtx2 = &osc.osc_aovxVertices[ actTriangles[iTri].ct_iVtx[2]];

    // create edges
    poedg[iTri*3+0] = CObjectEdge( *pVtx0, *pVtx1);
    poedg[iTri*3+1] = CObjectEdge( *pVtx1, *pVtx2);
    poedg[iTri*3+2] = CObjectEdge( *pVtx2, *pVtx0);

    // create polygon edges
    popo[iTri].opo_PolygonEdges.New(3);
    popo[iTri].opo_PolygonEdges.Lock();
    popo[iTri].opo_PolygonEdges[0].ope_Edge = &poedg[iTri*3+0];
    popo[iTri].opo_PolygonEdges[1].ope_Edge = &poedg[iTri*3+1];
    popo[iTri].opo_PolygonEdges[2].ope_Edge = &poedg[iTri*3+2];
    popo[iTri].opo_PolygonEdges.Unlock();

    // set material
    popo[iTri].opo_Material = &osc.osc_aomtMaterials[ actTriangles[iTri].ct_iMaterial];
    popo[iTri].opo_colorColor = popo[iTri].opo_Material->omt_Color;

    // create and set plane
    popl[iTri] = DOUBLEplane3D( *pVtx0, *pVtx1, *pVtx2);
    popo[iTri].opo_Plane = &popl[iTri];


    // copy UV coordinates to polygon texture mapping
    CMappingVectors mappingVectors;
    mappingVectors.FromPlane_DOUBLE(popl[iTri]);
    CMappingDefinition defaultMapping;
    FLOAT2D p0_uv = defaultMapping.GetTextureCoordinates(mappingVectors, DOUBLEtoFLOAT(*pVtx0));
    FLOAT2D p1_uv = defaultMapping.GetTextureCoordinates(mappingVectors, DOUBLEtoFLOAT(*pVtx1));
    FLOAT2D p2_uv = defaultMapping.GetTextureCoordinates(mappingVectors, DOUBLEtoFLOAT(*pVtx2));

    for (size_t i = 0; i < 3; ++i)
    {
      if (avTextureVertices[i].Count() == 0)
        continue;

      FLOAT2D p0_uvTarget(
        +avTextureVertices[i][actTriangles[iTri].ct_iTVtx[i*3 + 0]](1),
        -avTextureVertices[i][actTriangles[iTri].ct_iTVtx[i*3 + 0]](2));
      FLOAT2D p1_uvTarget(
        +avTextureVertices[i][actTriangles[iTri].ct_iTVtx[i*3 + 1]](1),
        -avTextureVertices[i][actTriangles[iTri].ct_iTVtx[i*3 + 1]](2));
      FLOAT2D p2_uvTarget(
        +avTextureVertices[i][actTriangles[iTri].ct_iTVtx[i*3 + 2]](1),
        -avTextureVertices[i][actTriangles[iTri].ct_iTVtx[i*3 + 2]](2));

      popo[iTri].opo_amdMappings[i] = GetMappingDefinitionFromReferenceToTarget({ p0_uv, p1_uv, p2_uv }, { p0_uvTarget, p1_uvTarget, p2_uvTarget });
    }
  }
  acmMaterials.Unlock();
}
