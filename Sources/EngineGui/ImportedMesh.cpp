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

#include "StdH.h"

#include "ImportedMesh.h"
#include "ImportedSkeleton.h"

#include <Engine/Base/Stream.h>
#include <Engine/Graphics/Color.h>
#include <Engine/Math/Functions.h>

#include <assimp/Importer.hpp>
#include <assimp/importerdesc.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>

#include <algorithm>
#include <array>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>

#ifdef max
#undef max
#undef min
#endif

#undef W
#undef NONE

template<>
struct std::hash<aiVector3D>
{
  std::size_t operator()(const aiVector3D& vec3d) const noexcept
  {
    std::size_t result = 0;
    HashCombine(result, vec3d.x);
    HashCombine(result, vec3d.y);
    HashCombine(result, vec3d.z);
    return result;
  }
};

template<>
struct std::hash<std::pair<size_t, const aiMesh*>>
{
  std::size_t operator()(const std::pair<size_t, const aiMesh*>& weight) const noexcept
  {
    std::size_t result = 0;
    HashCombine(result, weight.first);
    HashCombine(result, weight.second);
    return result;
  }
};

struct _VertexWeights
{
public:
  _VertexWeights() = default;

  const ImportedMesh::TWeights& Get(size_t vertexIndex, const aiMesh* mesh) const
  {
    auto foundPos = m_cache.find({ vertexIndex, mesh });
    if (foundPos != m_cache.end())
      return foundPos->second;

    float totalWeight = 0.0f;
    auto& result = m_cache[{ vertexIndex, mesh }];
    for (size_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
      const aiBone* bone = mesh->mBones[boneIndex];
      for (size_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex)
      {
        const aiVertexWeight& weight = bone->mWeights[weightIndex];
        if (weight.mVertexId == vertexIndex)
        {
          size_t boneNameIndex;
          const std::string boneName(bone->mName.C_Str());
          auto foundPos = std::find(m_bones.begin(), m_bones.end(), boneName);
          if (foundPos != m_bones.end())
          {
            boneNameIndex = std::distance(m_bones.begin(), foundPos);
          } else {
            m_bones.push_back(boneName);
            boneNameIndex = m_bones.size() - 1;
          }

          result[boneNameIndex] = weight.mWeight;
          totalWeight += weight.mWeight;
          break;
        }
      }
    }
    if (totalWeight > 0.0f)
      for (auto& weight : result)
        weight.second /= totalWeight;
    else
      result.clear();
    return result;
  }

  const std::vector<std::string>& GetBones() const
  {
    return m_bones;
  }

private:
  using TWeightCache = std::unordered_map<std::pair<size_t, const aiMesh*>, ImportedMesh::TWeights>;
  mutable TWeightCache m_cache;
  mutable std::vector<std::string> m_bones;
};

template<>
struct std::hash<std::pair<aiVector3D, const ImportedMesh::TWeights>>
{
  std::size_t operator()(const std::pair<aiVector3D, const ImportedMesh::TWeights>& vertexAndWeights) const noexcept
  {
    std::size_t result = 0;
    const auto& vec3d = vertexAndWeights.first;
    HashCombine(result, vec3d.x);
    HashCombine(result, vec3d.y);
    HashCombine(result, vec3d.z);
    const ImportedMesh::TWeights& weights = vertexAndWeights.second;
    for (auto it = weights.begin(); it != weights.end(); ++it)
    {
      HashCombine(result, it->first);
      HashCombine(result, it->second);
    }
    return result;
  }
};

namespace
{
  INDEX AI_GetNumFaces(const std::vector<aiMesh*>& meshes)
  {
    INDEX faces = 0;
    for (auto* mesh : meshes)
      faces += mesh->mNumFaces;
    return faces;
  }

  std::vector<aiMesh*> AI_GetValidMeshes(const aiScene* aiSceneMain)
  {
    std::vector<aiMesh*> validMeshes;

    for (size_t i = 0; i < aiSceneMain->mNumMeshes; ++i)
      if (aiSceneMain->mMeshes[i]->HasFaces())
      {
        for (size_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j)
          if (aiSceneMain->mMeshes[i]->HasTextureCoords(j))
          {
            validMeshes.push_back(aiSceneMain->mMeshes[i]);
            break;
          }
      }

    return validMeshes;
  }

  const aiNode* AI_FindMeshNode(const aiScene* aiSceneMain, const aiNode* node, const aiMesh* mesh)
  {
    for (size_t i = 0; i < node->mNumMeshes; ++i)
      if (aiSceneMain->mMeshes[node->mMeshes[i]] == mesh)
        return node;

    for (size_t i = 0; i < node->mNumChildren; ++i)
      if (const auto* foundInChild = AI_FindMeshNode(aiSceneMain, node->mChildren[i], mesh))
        return foundInChild;

    return nullptr;
  }

  aiMatrix4x4 AI_GetAbsoluteTransform(const aiNode* node)
  {
    if (node->mParent)
      return AI_GetAbsoluteTransform(node->mParent) * node->mTransformation;

    return node->mTransformation;
  }

  std::map<const aiMesh*, aiMatrix4x4> AI_GetWorldTransforms(const aiScene* aiSceneMain, const std::vector<aiMesh*>& meshes)
  {
    std::map<const aiMesh*, aiMatrix4x4> meshTransform;
    for (const auto* mesh : meshes)
      if (const auto* node = AI_FindMeshNode(aiSceneMain, aiSceneMain->mRootNode, mesh))
        meshTransform[mesh] = AI_GetAbsoluteTransform(node);

    return meshTransform;
  }
} // anonymous namespace

const std::vector<ImportedMesh::TFormatDescr>& ImportedMesh::GetSupportedFormats()
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

ImportedMesh::ImportedMesh(const CTFileName& fnmFileName, const FLOATmatrix3D& mTransform)
{
  FillFromFile(fnmFileName, mTransform);
}

void ImportedMesh::ApplySkinning(const ImportedSkeleton& animSkeleton, const FLOATmatrix3D& mTransform)
{
  struct _TransformsCache
  {
  public:
    const FLOATmatrix4D& GetAbsoluteTransform(const ImportedSkeleton::Bone& bone) const
    {
      auto foundPos = m_transforms.find(&bone);
      if (foundPos != m_transforms.end())
        return foundPos->second;
      return m_transforms.insert({ &bone, bone.GetAbsoluteTransform() }).first->second;
    }
  private:
    mutable std::unordered_map<const ImportedSkeleton::Bone*, FLOATmatrix4D> m_transforms;
  };
  _TransformsCache transformCache;

  const auto inverseMeshTransform = InverseMatrix(mTransform);
  for (size_t v = 0; v < m_vertices.size(); ++v)
  {
    const auto& weights = m_verticeWeights[v];
    if (weights.empty())
      continue;
    auto& vtx3D = m_vertices[v];
    auto vtxUnTransformed = FLOAT3D(-vtx3D(1), vtx3D(2), -vtx3D(3)) * inverseMeshTransform;
    const FLOAT4D vertex(vtxUnTransformed(1), vtxUnTransformed(2), vtxUnTransformed(3), 1.0f);
    FLOAT4D result(0, 0, 0, 0);

    for (const auto& weight : weights)
    {
      const auto& weightBone = m_weightBones.at(weight.first);
      FLOATmatrix4D animBoneTransform;
      const auto& animBoneIt = animSkeleton.m_bones.find(weightBone.m_name);
      if (animBoneIt != animSkeleton.m_bones.end())
        animBoneTransform = transformCache.GetAbsoluteTransform(animBoneIt->second);
      else
        animBoneTransform.Diagonal(1.0f);
      const FLOATmatrix4D transform = animBoneTransform * weightBone.m_offset;
      result += (vertex * transform) * weight.second;
    }

    vtx3D = FLOAT3D(result(1), result(2), result(3)) * mTransform;
    vtx3D(1) = -vtx3D(1);
    vtx3D(2) = +vtx3D(2);
    vtx3D(3) = -vtx3D(3);
  }
}

void ImportedMesh::FillFromFile(const CTFileName& fnmFileName, const FLOATmatrix3D& mTransform)
{
  Clear();
  // call file load with file's full path name
  const CTString strFile = _fnmApplicationPath + fnmFileName;

  Assimp::Importer importerWithoutNormals;
  // do not read normals from input file
  importerWithoutNormals.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
  const aiScene* aiSceneMain = importerWithoutNormals.ReadFile(strFile.str_String,
    aiProcess_JoinIdenticalVertices |
    aiProcess_Triangulate |
    aiProcess_GenUVCoords |
    aiProcess_RemoveComponent |
    aiProcess_FlipUVs);

  // if scene is successefuly loaded
  if (aiSceneMain && aiSceneMain->mNumMeshes > 0 && aiSceneMain->mNumMaterials > 0)
  {
    FillConversionArrays_t(mTransform, aiSceneMain);
  }
  else
  {
    ThrowF_t("Unable to load file %s: %s", (const char*)fnmFileName, importerWithoutNormals.GetErrorString());
  }
}

size_t ImportedMesh::GetUVChannelCount(const CTFileName& fileName)
{
  const CTString strFile = _fnmApplicationPath + fileName;

  Assimp::Importer importerWithoutNormals;
  // do not read normals from input file
  importerWithoutNormals.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
  const aiScene* aiSceneMain = importerWithoutNormals.ReadFile(strFile.str_String,
    aiProcess_JoinIdenticalVertices |
    aiProcess_Triangulate |
    aiProcess_GenUVCoords |
    aiProcess_RemoveComponent |
    aiProcess_FlipUVs);

  if (!aiSceneMain)
    return 0;

  std::unordered_set<size_t> usedChannels;
  for (auto* mesh : AI_GetValidMeshes(aiSceneMain))
    for (size_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j)
      if (mesh->HasTextureCoords(j))
        usedChannels.insert(j);
  return std::max(size_t(1), usedChannels.size());
}

void ImportedMesh::Clear()
{
  m_triangles.clear();
  m_materials.clear();
  m_vertices.clear();
  m_verticeWeights.clear();
  m_uvs.clear();
  m_weightBones.clear();
}

void ImportedMesh::FillConversionArrays_t(const FLOATmatrix3D& mTransform, const aiScene* aiSceneMain)
{
  // all polygons must be triangles
  const auto validMeshes = AI_GetValidMeshes(aiSceneMain);
  if (validMeshes.empty())
    throw("Error: model has no UV map!");

  const auto meshTransform = AI_GetWorldTransforms(aiSceneMain, validMeshes);
  if (meshTransform.size() != validMeshes.size())
    throw("Error: model has broken scene graph!");

  // check if we need flipping (if matrix is flipping, polygons need to be flipped)
  const FLOATmatrix3D& m = mTransform;
  FLOAT fDet =
    m(1, 1) * (m(2, 2) * m(3, 3) - m(2, 3) * m(3, 2)) +
    m(1, 2) * (m(2, 3) * m(3, 1) - m(2, 1) * m(3, 3)) +
    m(1, 3) * (m(2, 1) * m(3, 2) - m(2, 2) * m(3, 1));
  FLOAT bFlipped = fDet < 0;

  // ------------  Find UV map indices
  std::map<aiMesh*, std::vector<unsigned int>> uvChannels;
  std::map<size_t, size_t> channelRemap;
  std::map<std::string, FLOATmatrix4D> boneOffsets;
  for (auto* mesh : validMeshes)
  {
    auto& channelsOfMesh = uvChannels[mesh];
    for (size_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j)
    {
      if (mesh->HasTextureCoords(j))
      {
        if (channelRemap.find(j) == channelRemap.end())
        {
          size_t val = channelRemap.size();
          channelRemap[j] = val;
        }
        channelsOfMesh.push_back(j);
      }
    }
    for (int i = 0; i < mesh->mNumBones; ++i)
    {
      const auto* bone = mesh->mBones[i];
      auto& offset = boneOffsets[bone->mName.C_Str()];
      const auto aiOffset = bone->mOffsetMatrix * aiMatrix4x4(meshTransform.at(mesh)).Inverse();
      for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
          offset(row + 1, col + 1) = aiOffset[row][col];
    }
  }
  const size_t numUVChannels = channelRemap.size();

  // ------------  Convert object vertices (coordinates)
  _VertexWeights vertexWeights;
  std::unordered_map<std::pair<aiVector3D, const ImportedMesh::TWeights>, INDEX> uniqueVertices;
  for (auto* mesh : validMeshes)
  {
    for (size_t v = 0; v < mesh->mNumVertices; ++v)
    {
      const auto& weights = vertexWeights.Get(v, mesh);
      if (uniqueVertices.find({ mesh->mVertices[v], weights }) == uniqueVertices.end())
      {
        uniqueVertices[{ mesh->mVertices[v], weights }] = m_vertices.size();
        const auto aiVtx = meshTransform.at(mesh) * mesh->mVertices[v];
        FLOAT3D vtx = FLOAT3D(aiVtx[0], aiVtx[1], aiVtx[2]) * mTransform;
        vtx(1) = -vtx(1);
        vtx(3) = -vtx(3);
        m_vertices.push_back(vtx);
        m_verticeWeights.push_back(weights);
      }
    }
  }
  const auto boneNames = vertexWeights.GetBones();
  m_weightBones.reserve(boneNames.size());
  for (const auto& boneName : boneNames)
  {
    m_weightBones.emplace_back();
    auto& weightBone = m_weightBones.back();
    weightBone.m_name = boneName;
    const auto offsetIt = boneOffsets.find(boneName);
    if (offsetIt != boneOffsets.end())
      weightBone.m_offset = offsetIt->second;
    else
      weightBone.m_offset.Diagonal(1.0f);
  }

  // ------------ Convert object's mapping vertices (texture vertices)
  m_uvs.resize(numUVChannels, {});
  std::vector<std::unordered_map<aiVector3D, INDEX>> uniqueTexCoords(numUVChannels, std::unordered_map<aiVector3D, INDEX>());
  for (auto* mesh : validMeshes)
  {
    for (auto uv : uvChannels[mesh])
    {
      const size_t uvIndex = channelRemap[uv];
      for (size_t v = 0; v < mesh->mNumVertices; ++v)
      {
        if (uniqueTexCoords[uvIndex].find(mesh->mTextureCoords[uv][v]) == uniqueTexCoords[uvIndex].end())
        {
          uniqueTexCoords[uvIndex][mesh->mTextureCoords[uv][v]] = m_uvs[uvIndex].size();
          const auto& aiUV = mesh->mTextureCoords[uv][v];
          m_uvs[uvIndex].push_back(FLOAT2D(aiUV[0], aiUV[1]));
        }
      }
    }
  }

  // ------------ Organize triangles as list of surfaces
  // allocate triangles
  m_triangles.resize(AI_GetNumFaces(validMeshes));

  std::map<ULONG, ULONG> materialRemap;
  // sort triangles per surfaces
  INDEX trianglesOffset = 0;
  size_t meshIndex = 0;
  for (auto* mesh : validMeshes)
  {
    for (INDEX iTriangle = 0; iTriangle < mesh->mNumFaces; iTriangle++)
    {
      auto& ctTriangle = m_triangles[trianglesOffset + iTriangle];

      const aiFace* ai_face = &mesh->mFaces[iTriangle];
      if (true)
      {
        auto v0Index = ai_face->mIndices[0];
        auto v1Index = ai_face->mIndices[1];
        auto v2Index = ai_face->mIndices[2];
        const auto& v0Weights = vertexWeights.Get(v0Index, mesh);
        const auto& v1Weights = vertexWeights.Get(v1Index, mesh);
        const auto& v2Weights = vertexWeights.Get(v2Index, mesh);
        auto v0 = mesh->mVertices[v0Index];
        auto v1 = mesh->mVertices[v1Index];
        auto v2 = mesh->mVertices[v2Index];
        // copy vertex indices
        if (bFlipped) {
          ctTriangle.ct_iVtx[0] = uniqueVertices[{ v2, v2Weights}];
          ctTriangle.ct_iVtx[1] = uniqueVertices[{ v1, v1Weights}];
          ctTriangle.ct_iVtx[2] = uniqueVertices[{ v0, v0Weights}];
        }
        else {
          ctTriangle.ct_iVtx[0] = uniqueVertices[{ v0, v0Weights}];
          ctTriangle.ct_iVtx[1] = uniqueVertices[{ v1, v1Weights}];
          ctTriangle.ct_iVtx[2] = uniqueVertices[{ v2, v2Weights}];
        }
      }

      ctTriangle.ct_iTVtx.resize(numUVChannels, { 0, 0, 0 });
      for (auto uv : uvChannels[mesh])
      {
        const size_t uvIndex = channelRemap[uv];

        // copy texture vertex indices
        if (bFlipped) {
          ctTriangle.ct_iTVtx[uvIndex][0] = uniqueTexCoords[uvIndex][mesh->mTextureCoords[uv][ai_face->mIndices[2]]];
          ctTriangle.ct_iTVtx[uvIndex][1] = uniqueTexCoords[uvIndex][mesh->mTextureCoords[uv][ai_face->mIndices[1]]];
          ctTriangle.ct_iTVtx[uvIndex][2] = uniqueTexCoords[uvIndex][mesh->mTextureCoords[uv][ai_face->mIndices[0]]];
        }
        else {
          ctTriangle.ct_iTVtx[uvIndex][0] = uniqueTexCoords[uvIndex][mesh->mTextureCoords[uv][ai_face->mIndices[0]]];
          ctTriangle.ct_iTVtx[uvIndex][1] = uniqueTexCoords[uvIndex][mesh->mTextureCoords[uv][ai_face->mIndices[1]]];
          ctTriangle.ct_iTVtx[uvIndex][2] = uniqueTexCoords[uvIndex][mesh->mTextureCoords[uv][ai_face->mIndices[2]]];
        }
      }

      // obtain material
      ULONG materialIndex = mesh->mMaterialIndex;
      // attach triangle into one material
      auto foundPos = materialRemap.find(materialIndex);
      if (foundPos != materialRemap.end())
      {
        ctTriangle.ct_iMaterial = foundPos->second;
        continue;
      }

      // add new material
      m_materials.emplace_back();
      // set polygon's material index 
      INDEX iNewMaterial = m_materials.size() - 1;
      ctTriangle.ct_iMaterial = iNewMaterial;

      // remember recognition tag
      materialRemap[materialIndex] = iNewMaterial;

      // ---------- Set material's name
      // if not default material

      aiString materialName;
      aiSceneMain->mMaterials[materialIndex]->Get(AI_MATKEY_NAME, materialName);

      if (materialName.length > 0)
      {
        m_materials[iNewMaterial].cm_strName = CTString(materialName.C_Str());
        // get color
        const double materialCoefficient = static_cast<double>(++meshIndex) / validMeshes.size();
        COLOR colColor = static_cast<COLOR>(std::numeric_limits<COLOR>::max() * materialCoefficient);
        m_materials[iNewMaterial].cm_colColor = colColor;
      }
      else
      {
        m_materials[iNewMaterial].cm_strName = "Default";
        m_materials[iNewMaterial].cm_colColor = C_GRAY;
      }
    }
    trianglesOffset += mesh->mNumFaces;
  }
}
