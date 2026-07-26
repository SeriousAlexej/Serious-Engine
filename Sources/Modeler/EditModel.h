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

#ifndef SE_INCL_EDITMODEL_H
#define SE_INCL_EDITMODEL_H
#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <SeriousEngineCppAPI/Base/Lists.h>
#include <SeriousEngineCppAPI/Base/CTString.h>
#include <SeriousEngineCppAPI/Base/FileName.h>
#include <SeriousEngineCppAPI/Math/Vector.h>
#include <SeriousEngineCppAPI/Math/Matrix.h>
#include <SeriousEngineCppAPI/Models/RenderModel.h>

#include "Script/Script.h"

#include <vector>
#include <functional>
#include <map>
#include <array>

#define MAPPING_VERSION_WITHOUT_POLYGONS_PER_SURFACE "0001"
#define MAPPING_VERSION_WITHOUT_SOUNDS_AND_ATTACHMENTS "0002"
#define MAPPING_VERSION_WITHOUT_COLLISION "0003"
#define MAPPING_VERSION_WITHOUT_PATCHES "0004"
#define MAPPING_WITHOUT_SURFACE_COLORS "0005"
#define MAPPING_VERSION "0006"

class CProgressRoutines
{
public:
  CProgressRoutines();
  void (*SetProgressMessage)(const char *strMessage);      // sets message for modeler's "new progress dialog"
  void (*SetProgressRange)( INDEX iProgresSteps);     // sets range of modeler's "new progress dialog"
  void (*SetProgressState)( INDEX iCurrentStep);      // sets current modeler's "new progress dialog" state
};

extern CProgressRoutines ProgresRoutines;

class CTextureDataInfo
{
public:
  CTextureDataPtr tdi_TextureData;
  CTFileName tdi_FileName;
};

class CAttachedModel {
public:
  BOOL am_bVisible;
  CModelObject am_moAttachedModel; // used as smart pointer (holds file name of attachment), never rendered
  CTString am_strName;
  INDEX am_iAnimation;

  CAttachedModel(void);
  ~CAttachedModel(void);
  void SetModel_t(CTFileName fnModel);
  void Read_t( CTStream *strFile); // throw char *
  void Write_t( CTStream *strFile); // throw char *
  void Clear(void); // clear the object.
};

class CAttachedSound {
public:
  BOOL as_bLooping;
  BOOL as_bPlaying;
  FLOAT as_fDelay;
  CTFileName as_fnAttachedSound;

  CAttachedSound(void);
  void Read_t( CTStream *strFile); // throw char *
  void Write_t( CTStream *strFile); // throw char *
  void Clear(void) { as_fnAttachedSound = CTString("");};
};

class CThumbnailSettings {
public:
  BOOL ts_bSet;
  CPlacement3D ts_plLightPlacement;
  CPlacement3D ts_plModelPlacement;
  FLOAT ts_fTargetDistance;
  FLOAT3D ts_vTarget;
  ANGLE3D ts_angViewerOrientation;
  FLOAT ts_LightDistance;
  COLOR ts_LightColor;
  COLOR ts_colAmbientColor;
  COLORREF ts_PaperColor;
  COLORREF ts_InkColor;
  BOOL ts_IsWinBcgTexture;
  CTFileName ts_WinBcgTextureName;
  CModelRenderPrefs ts_RenderPrefs;

  CThumbnailSettings( void);
  void Read_t( CTStream *strFile); // throw char *
  void Write_t( CTStream *strFile); // throw char *
};

struct ImportedMesh;
struct ImportedSkeleton;

class CEditModel
{
private:
  struct FrameGenerator
  {
    CTFileName m_filename;
    std::function<void(ImportedMesh&)> m_generator;
  };

  void NewModel(const ImportedMesh& mesh);                  // creates new model, surface, vertice and polygon arrays
  void AddMipModel(const ImportedMesh& mesh);              // adds one mip model
  void CreateBoneTriangles(ImportedMesh& mesh, const ImportedSkeleton& skel, const FLOATmatrix3D& transform, FLOAT stretch);
  std::vector<FrameGenerator> LoadFrameGenerators(const ModelScript::Animations& animations, const ImportedMesh& baseMesh, const ImportedSkeleton& skeleton, const FLOATmatrix3D& mStretch);
  // loads and converts model's animation data from script file
  void LoadModelAnimationData_t(const ModelScript::Animations& animations, const ImportedMesh& baseMesh, const ImportedSkeleton& skeleton, const FLOATmatrix3D &mStretch);  // throw char *
  INDEX edm_iActiveCollisionBox;                  // collision box that is currently edited

public:
  using TBoneToTriangle = std::map<std::string, std::array<INDEX, 3>>;

  INDEX ser_ctUsed = 0;         // use count
  CTFileName ser_FileName;  // last file name loaded
  TIME ch_LastChangeTime = TIME(-1);

  void MarkUsed();
  void MarkUnused();
  BOOL IsUsed();
  INDEX GetUsedCount();
  void MarkChanged();

public:
  CEditModel();                                    // default contructor
  ~CEditModel();                                  // default destructor
  CModelData edm_md;                              // edited model data
  TBoneToTriangle m_boneTriangleMapping; // If bone triangles were generated, this map shall contain triangle indices for each bone
  INDEX m_boneTriangleMappingGeneration = 0;
  std::vector<std::unique_ptr<CAttachedModel>> edm_aamAttachedModels;// array of attached models
  std::vector<std::unique_ptr<CAttachedSound>> edm_aasAttachedSounds;// array of attached sounds
  CThumbnailSettings edm_tsThumbnailSettings;     // remembered parameters for taking thumbnail
  std::vector<std::unique_ptr<CTextureDataInfo>> edm_WorkingSkins; // list of file names and texture data objects
  INDEX edm_Action;                               // type of last mapping change action (used by undo/redo)
  CTFileName edm_fnSpecularTexture;               // names of textures saved in ini file
  CTFileName edm_fnReflectionTexture;
  CTFileName edm_fnBumpTexture;
  // create empty attaching sounds
  void CreateEmptyAttachingSounds(void);
  // creates default script file
  void CreateScriptFile_t(CTFileName &fnFile);    // throw char *
  // creates mip-model and mapping default constructios after it loads data from script
  void LoadFromScript_t(CTFileName &fnFileName); // throw char *
  // updates animations
  void UpdateAnimations_t(CTFileName &fnScriptName);  // throw char *
  // updates mip models configuration, looses their mapping !
  void UpdateMipModels_t(CTFileName &fnScriptName); // throw char *
  void CreateMipModels_t(const ImportedMesh& baseMesh, INDEX iVertexRemoveRate, INDEX iSurfacePreservingFactor);
  void DrawWireSurface( CDrawPortPtr pDP, INDEX iCurrentMip, INDEX iCurrentSurface,
       FLOAT fMagnifyFactor, PIX offx, PIX offy, COLOR clrVisible, COLOR clrInvisible); // draws given surface in wire frame
  void DrawFilledSurface( CDrawPortPtr pDP, INDEX iCurrentMip, INDEX iCurrentSurface,
       FLOAT fMagnifyFactor, PIX offx, PIX offy, COLOR clrVisible, COLOR clrInvisible); // fills given surface with color
  void PrintSurfaceNumbers( CDrawPortPtr pDP, CFontDataPtr pFont, INDEX iCurrentMip,
       FLOAT fMagnifyFactor, PIX offx, PIX offy, COLOR clrInk); // prints surface numbers
  void ExportSurfaceNumbersAndNames( CTFileName fnFile);
  // add one texture to list of working textures
  CTextureDataInfo *AddTexture_t(const CTFileName &fnFileName, const MEX mexWidth,
        const MEX mexHeight); // throw char *
  const char *GetSurfaceName(INDEX iCurrentMip, INDEX iCurrentSurface); // Retrieves given surface's name
  void MovePatchRelative( INDEX iMaskBit, MEX2D mexOffset);
  void SetPatchStretch( INDEX iMaskBit, FLOAT fNewStretch);
  BOOL EditAddPatch( CTFileName fnPatchName, MEX2D mexPos, INDEX &iMaskBit); // Adds one patch
  void EditRemovePatch( INDEX iMaskBit);            // Removes given patch
  void EditRemoveAllPatches( void);
  INDEX CountPatches(void);
  ULONG GetExistingPatchesMask(void);
  BOOL GetFirstEmptyPatchIndex( INDEX &iMaskBit);   // Finds first empty space ready to receive new patch
  BOOL GetFirstValidPatchIndex( INDEX &iMaskBit);   // Finds first valid patch index
  void GetPreviousValidPatchIndex( INDEX &iMaskBit);// Sets previous valid patch index
  void GetNextValidPatchIndex( INDEX &iMaskBit);    // Sets next valid patch index
  void CalculatePatchesPerPolygon(void);
  INDEX GetMipCt(){ return edm_md.md_MipCt;};       // Returns number of mip models
  MEX GetWidth(){ return edm_md.md_Width;};         // Returns allowed width for model's texture
  MEX GetHeight(){ return edm_md.md_Height;};       // Returns allowed height for model's texture
  // collision box handling functions
  FLOAT3D GetCollisionBoxMin(void);
  FLOAT3D GetCollisionBoxMax(void);
  void AddCollisionBox(void);
  void DeleteCurrentCollisionBox(void);
  INDEX GetActiveCollisionBoxIndex(void) { return edm_iActiveCollisionBox;};
  void ActivatePreviousCollisionBox(void);
  void ActivateNextCollisionBox(void);
  void SetCollisionBox(FLOAT3D vMin, FLOAT3D vMax);
  CTString GetCollisionBoxName(INDEX iCollisionBox);
  CTString GetCollisionBoxName(void);
  void SetCollisionBoxName(CTString strNewName);
  void CorrectCollisionBoxSize(void);
  // returns HEIGHT_EQ_WIDTH, LENGTH_EQ_WIDTH or LENGTH_EQ_HEIGHT
  INDEX GetCollisionBoxDimensionEquality();
  // set new collision box equality value
  void SetCollisionBoxDimensionEquality( INDEX iNewDimEqType);
  // overloaded load function
  void Load_t( CTFileName fnFileName); // throw char *
  void Load_t_base(const CTFileName fnFileName);
  // overloaded save function
  void Save_t( CTFileName fnFileName); // throw char *
  // exports .h file (#define ......)
  void SaveIncludeFile_t( CTFileName fnFileName, CTString strDefinePrefix);  // throw char *

  // know how to load
  void Read_t( CTStream *istrFile); // throw char *
  // and save modeler data (i.e. vindow positions, view prefs, texture file names...)
  void Write_t( CTStream *ostrFile); // throw char *

  // load and save mapping data for whole model (iMip = -1) or just for one mip model
  void LoadMapping_t( CTFileName fnFileName, INDEX iMip = -1);
  void SaveMapping_t( CTFileName fnFileName, INDEX iMip = -1);
  // read and write settings for given mip
  void ReadMipSettings_t( CTStream *istrFile, INDEX iMip);  // throw char *
  void WriteMipSettings_t( CTStream *ostrFile, INDEX iMip);
};


#endif  /* include-once check. */

