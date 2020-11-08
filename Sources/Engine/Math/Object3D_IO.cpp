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
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>

#include <unordered_map>
#include <vector>

namespace
{
  static const size_t AI_DEFAULT_UV_CHANNEL = 0;

  void HashCombine(std::size_t& seed, float v)
  {
    std::hash<float> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }

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
        if (aiSceneMain->mMeshes[i]->HasTextureCoords(AI_DEFAULT_UV_CHANNEL))
          ++nonEmptyWithUV;
      }

    return nonEmptyWithUV == nonEmptyMeshes && nonEmptyMeshes > 0;
  }
} // anonymous namespace

#undef W
#undef NONE

void FillConversionArrays_t(const FLOATmatrix3D &mTransform, const aiScene* aiSceneMain);
void ClearConversionArrays( void);
void RemapVertices(BOOL bAsOpened);


/*
 *  Intermediate structures used for converting from Exploration 3D data format into O3D
 */
struct ConversionTriangle {
  INDEX ct_iVtx[3];     // indices of vertices
  INDEX ct_iTVtx[3];    // indices of texture vertices
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
CStaticArray<FLOAT2D> avTextureVertices;
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
// function makes Little-Big indian conversion of 4 bytes and returns valid SLONG
inline SLONG ConvertLong( SBYTE *pfm)
{
  UBYTE i;
  UBYTE ret_long[ 4];

  for( i=0; i<4; i++)
    ret_long[ i] = *((UBYTE *) pfm + 3 - i);
  return( *((SLONG *) ret_long) );
};

//--------------------------------------------------------------------------------------------
// function makes Little-Big indian conversion of 2 bytes and returns valid WORD
inline INDEX ConvertWord( SBYTE *pfm)
{
  char aret_word[ 2];

  aret_word[ 0] = *(pfm+1);
  aret_word[ 1] = *(pfm+0);
  INDEX ret_word = (INDEX) *((SWORD *) aret_word);
	return( ret_word);
};

//--------------------------------------------------------------------------------------------
// function makes Little-Big indian conversion of 4 bytes representing float and returns valid float
inline float ConvertFloat( SBYTE *pfm)
{
  UBYTE i;
  char float_no[ 4];

  for( i=0; i<4; i++)
    float_no[ i] = *( pfm + 3 - i);
  return( *((float *) float_no) );
};

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

void CObject3D::LoadAny3DFormat_t(
  const CTFileName &fnmFileName,
  const FLOATmatrix3D &mTransform,
  enum LoadType ltLoadType/*= LT_NORMAL*/)
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
      // use different methods to convert into Object3D
      switch( ltLoadType)
      {
      case LT_NORMAL:
        FillConversionArrays_t(mTransform, aiSceneMain);
        ConvertArraysToO3D();
        break;
      case LT_OPENED:
        FillConversionArrays_t(mTransform, aiSceneMain);
        RemapVertices(TRUE);
        ConvertArraysToO3D();
        break;
      case LT_UNWRAPPED:
        FLOATmatrix3D mOne;
        mOne.Diagonal(1.0f);
        FillConversionArrays_t(mOne, aiSceneMain);
        if( avTextureVertices.Count() == 0)
        {
    		  ThrowF_t("Unable to import mapping from 3D object because it doesn't contain mapping coordinates.");
        }

        RemapVertices(FALSE);
        ConvertArraysToO3D();
        break;
      }
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
  std::unordered_map<aiVector3D, INDEX, aiHasher> uniqueTexCoords;
  std::vector<aiVector3D*> orderedUniqueTexCoords;
  for (size_t i = 0; i < aiSceneMain->mNumMeshes; ++i)
  {
    auto* mesh = aiSceneMain->mMeshes[i];
    for (size_t v = 0; v < mesh->mNumVertices; ++v)
    {
      if (uniqueTexCoords.find(mesh->mTextureCoords[AI_DEFAULT_UV_CHANNEL][v]) == uniqueTexCoords.end())
      {
        uniqueTexCoords[mesh->mTextureCoords[AI_DEFAULT_UV_CHANNEL][v]] = orderedUniqueTexCoords.size();
        orderedUniqueTexCoords.push_back(&mesh->mTextureCoords[AI_DEFAULT_UV_CHANNEL][v]);
      }
    }
  }
  avTextureVertices.New(orderedUniqueTexCoords.size());
  // copy texture vertices
  for (size_t iTVtx = 0; iTVtx < orderedUniqueTexCoords.size(); ++iTVtx)
  {
    avTextureVertices[iTVtx] = (FLOAT2D&)*orderedUniqueTexCoords[iTVtx];
    avTextureVertices[iTVtx](2) -= 1.0f;
  }
  orderedUniqueTexCoords.clear();
  
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

      // copy texture vertex indices
      if (bFlipped) {
        ctTriangle.ct_iTVtx[0] = uniqueTexCoords[mesh->mTextureCoords[AI_DEFAULT_UV_CHANNEL][ai_face->mIndices[2]]];
        ctTriangle.ct_iTVtx[1] = uniqueTexCoords[mesh->mTextureCoords[AI_DEFAULT_UV_CHANNEL][ai_face->mIndices[1]]];
        ctTriangle.ct_iTVtx[2] = uniqueTexCoords[mesh->mTextureCoords[AI_DEFAULT_UV_CHANNEL][ai_face->mIndices[0]]];
      }
      else {
        ctTriangle.ct_iTVtx[0] = uniqueTexCoords[mesh->mTextureCoords[AI_DEFAULT_UV_CHANNEL][ai_face->mIndices[0]]];
        ctTriangle.ct_iTVtx[1] = uniqueTexCoords[mesh->mTextureCoords[AI_DEFAULT_UV_CHANNEL][ai_face->mIndices[1]]];
        ctTriangle.ct_iTVtx[2] = uniqueTexCoords[mesh->mTextureCoords[AI_DEFAULT_UV_CHANNEL][ai_face->mIndices[2]]];
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
          aiColor3D color(0.f, 0.f, 0.f);
          aiSceneMain->mMaterials[materialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color);

          COLOR colColor = CLR_CLRF(RGB(UBYTE(color.r * 255), UBYTE(color.g * 255), UBYTE(color.b * 255)));
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
  avTextureVertices.Clear();
  aiRemap.Clear();
}

void RemapVertices(BOOL bAsOpened)
{
  {INDEX ctSurf = 0;
  // fill remap array with indices of vertices in order how they appear per polygons
  {FOREACHINDYNAMICCONTAINER(acmMaterials, ConversionMaterial, itcm)
  {
    _RPT1(_CRT_WARN, "Indices of polygons in surface %d:", ctSurf);
    // for each polygon in surface
    {FOREACHINDYNAMICCONTAINER(itcm->ms_Polygons, INDEX, itipol)
    {
      _RPT1(_CRT_WARN, " %d,", *itipol);
    }}
    _RPT0(_CRT_WARN, "\n");
    ctSurf++;
  }}
  
  _RPT0(_CRT_WARN, "Polygons and their vertex indices:\n");
  for( INDEX ipol=0; ipol<actTriangles.Count(); ipol++)
  {
    INDEX idxVtx0 = actTriangles[ipol].ct_iVtx[0];
    INDEX idxVtx1 = actTriangles[ipol].ct_iVtx[1];
    INDEX idxVtx2 = actTriangles[ipol].ct_iVtx[2];
    _RPT4(_CRT_WARN, "Indices of vertices in polygon %d : (%d, %d, %d)\n", ipol, idxVtx0, idxVtx1, idxVtx2);
  }}

  INDEX ctVertices = avVertices.Count();
  aiRemap.New(ctVertices);

  // fill remap array with indices of vertices in order how they appear per polygons
  FOREACHINDYNAMICCONTAINER(acmMaterials, ConversionMaterial, itcm)
  {
    // fill remap array with -1
    for( INDEX iRemap=0; iRemap<ctVertices; iRemap++)
    {
      aiRemap[iRemap] = -1;
    }
    // reset 'vertex in surface' counter
    INDEX ctvx = 0;

    // for each polygon in surface
    {FOREACHINDYNAMICCONTAINER(itcm->ms_Polygons, INDEX, itipol)
    {
      INDEX idxPol = *itipol;
      // for each vertex in polygon
      for(INDEX iVtx=0; iVtx<3; iVtx++)
      {
        // get vertex's index
        INDEX idxVtx = actTriangles[idxPol].ct_iVtx[iVtx];
        if( aiRemap[idxVtx] == -1)
        {
          aiRemap[idxVtx] = ctvx;
          ctvx++;
        }
      }
    }}

    INDEX ctOld = avDst.Count();
    // allocate new block of vertices used in this surface
    FLOAT3D *pavDst = avDst.Push( ctvx);

    // for each polygon in surface
    {FOREACHINDYNAMICCONTAINER(itcm->ms_Polygons, INDEX, itipol)
    {
      INDEX iPol=*itipol;
      // for each vertex in polygon
      for(INDEX iVtx=0; iVtx<3; iVtx++)
      {
        // get vertex's index
        INDEX idxVtx = actTriangles[iPol].ct_iVtx[iVtx];
        // get remapped index
        INDEX iRemap = aiRemap[idxVtx];
        // if cutting object
        if( bAsOpened)
        {
          // copy vertex coordinate
          pavDst[ iRemap] = avVertices[idxVtx];
        }
        // if creating unwrapped mapping
        else
        {
          // copy texture coordinate
          FLOAT3D vMap;
          vMap(1) = avTextureVertices[actTriangles[iPol].ct_iTVtx[iVtx]](1);
          vMap(2) = -avTextureVertices[actTriangles[iPol].ct_iTVtx[iVtx]](2);
          vMap(3) = 0;
          pavDst[ iRemap] = vMap;
        }
        // remap index of polygon vertex
        actTriangles[iPol].ct_iVtx[iVtx] = iRemap+ctOld;
      }
    }}
  }
  aiRemap.Clear();
  
  // replace remapped array of vertices over original one
  avVertices.Clear();
  avVertices.New(avDst.Count());
  for( INDEX iVtxNew=0; iVtxNew<avDst.Count(); iVtxNew++)
  {
    avVertices[iVtxNew] = avDst[iVtxNew];
  }
  avDst.PopAll();

  {INDEX ctSurf = 0;
  // fill remap array with indices of vertices in order how they appear per polygons
  {FOREACHINDYNAMICCONTAINER(acmMaterials, ConversionMaterial, itcm)
  {
    _RPT1(_CRT_WARN, "Indices of polygons in surface %d:", ctSurf);
    // for each polygon in surface
    {FOREACHINDYNAMICCONTAINER(itcm->ms_Polygons, INDEX, itipol)
    {
      _RPT1(_CRT_WARN, " %d,", *itipol);
    }}
    _RPT0(_CRT_WARN, "\n");
    ctSurf++;
  }}
  
  _RPT0(_CRT_WARN, "Polygons and their vertex indices:\n");
  for( INDEX ipol=0; ipol<actTriangles.Count(); ipol++)
  {
    INDEX idxVtx0 = actTriangles[ipol].ct_iVtx[0];
    INDEX idxVtx1 = actTriangles[ipol].ct_iVtx[1];
    INDEX idxVtx2 = actTriangles[ipol].ct_iVtx[2];
    _RPT4(_CRT_WARN, "Indices of vertices in polygon %d : (%d, %d, %d)\n", ipol, idxVtx0, idxVtx1, idxVtx2);
  }}
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
  bool hasTextureCoordinates = avTextureVertices.Count() > 0;
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

    if (!hasTextureCoordinates)
      continue;

    // copy UV coordinates to polygon texture mapping
    CMappingVectors mappingVectors;
    mappingVectors.FromPlane_DOUBLE(popl[iTri]);
    CMappingDefinition defaultMapping;
    FLOAT2D p0_uv = defaultMapping.GetTextureCoordinates(mappingVectors, DOUBLEtoFLOAT(*pVtx0));
    FLOAT2D p1_uv = defaultMapping.GetTextureCoordinates(mappingVectors, DOUBLEtoFLOAT(*pVtx1));
    FLOAT2D p2_uv = defaultMapping.GetTextureCoordinates(mappingVectors, DOUBLEtoFLOAT(*pVtx2));

    FLOAT2D p0_uvTarget(
      +avTextureVertices[actTriangles[iTri].ct_iTVtx[0]](1),
      -avTextureVertices[actTriangles[iTri].ct_iTVtx[0]](2));
    FLOAT2D p1_uvTarget(
      +avTextureVertices[actTriangles[iTri].ct_iTVtx[1]](1),
      -avTextureVertices[actTriangles[iTri].ct_iTVtx[1]](2));
    FLOAT2D p2_uvTarget(
      +avTextureVertices[actTriangles[iTri].ct_iTVtx[2]](1),
      -avTextureVertices[actTriangles[iTri].ct_iTVtx[2]](2));

    popo[iTri].opo_amdMappings[0] = GetMappingDefinitionFromReferenceToTarget({ p0_uv, p1_uv, p2_uv }, { p0_uvTarget, p1_uvTarget, p2_uvTarget });
    popo[iTri].opo_amdMappings[1] = popo[iTri].opo_amdMappings[0];
    popo[iTri].opo_amdMappings[2] = popo[iTri].opo_amdMappings[0];
  }
  acmMaterials.Unlock();
}
