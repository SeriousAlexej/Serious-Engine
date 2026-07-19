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

// WorldEditor.h : main header file for the WORLDEDITOR application
//
#ifndef WORLDEDITOR_H
#define WORLDEDITOR_H 1

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "Viewers.h"

#include <functional>

#define CHILD_CONFIGURATION_VER "V013"
#define VIEW_PREFERENCES_VER "V013"

#define VIEW_PREFERENCES_CT 10
#define CHILD_CONFIGURATIONS_CT 10

extern FLOAT _fLastMipBrushingOptionUsed;
extern INDEX wed_iMaxFPSActive;
extern GameGUI_interface *_pGameGUI;

extern UINT _uiMessengerMsg;
extern UINT _uiMessengerForcePopup;
extern FLOAT _fFlyModeSpeedMultiplier;

class CWorldEditorDoc;
class CWorldEditorView;
class CVirtualTreeNode;

extern CEntityPtr GetTerrainEntity(void);
extern CTerrainPtr GetTerrain(void);
extern CTerrainLayerPtr GetLayer(void);
extern void SelectLayer(INDEX iLayer);
extern INDEX GetLayerIndex(void);
extern CTerrainLayerPtr GetLayer(INDEX iLayer);

#define ALLOW_TERRAINS 1

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp:
// See WorldEditor.cpp for the implementation of this class
//

enum PrimitiveType {
  PT_NONE =0,
  PT_CONUS,
  PT_TORUS,
  PT_STAIRCASES,
  PT_SPHERE,
  PT_TERRAIN,
};

enum TriangularisationType {
  TT_NONE=0,
  TT_CENTER_VERTEX,
  TT_FROM_VTX00,
  TT_FROM_VTX01,
  TT_FROM_VTX02,
  TT_FROM_VTX03,
  TT_FROM_VTX04,
  TT_FROM_VTX05,
  TT_FROM_VTX06,
  TT_FROM_VTX07,
  TT_FROM_VTX08,
  TT_FROM_VTX09,
  TT_FROM_VTX10,
  TT_FROM_VTX11,
  TT_FROM_VTX12,
  TT_FROM_VTX13,
  TT_FROM_VTX14,
  TT_FROM_VTX15,
};

#define VALUES_FOR_PRIMITIVE_VERSION2 "VFP2"
#define VALUES_FOR_PRIMITIVE_VERSION3 "VFP3"
// values for primitive
class CValuesForPrimitive
{
public:  
  enum CSGType vfp_csgtCSGOperation;
  // global parameters
  enum PrimitiveType vfp_ptPrimitiveType;
  std::vector<DOUBLE3D> vfp_avVerticesOnBaseOfPrimitive;
  CObject3D vfp_o3dPrimitive;
  CPlacement3D vfp_plPrimitive;
  enum TriangularisationType vfp_ttTriangularisationType;
  BOOL vfp_bClosed;
  BOOL vfp_bDummy;
  BOOL vfp_bAutoCreateMipBrushes;
  COLOR vfp_colSectorsColor;
  COLOR vfp_colPolygonsColor;

  // parameters for *Conus* primitives
  FLOAT vfp_fXMin;
  FLOAT vfp_fXMax;
  FLOAT vfp_fYMin;
  FLOAT vfp_fYMax;
  FLOAT vfp_fZMin;
  FLOAT vfp_fZMax;
  FLOAT vfp_fShearX;
  FLOAT vfp_fShearZ;
  FLOAT vfp_fStretchX;
  FLOAT vfp_fStretchY;

  BOOL vfp_bLinearStaircases;
  BOOL vfp_bOuter;
  INDEX vfp_iSlicesIn360;
  INDEX vfp_iNoOfSlices;
  FLOAT vfp_fRadius;

  INDEX vfp_iMeridians;
  INDEX vfp_iParalels;

  INDEX vfp_iSlicesPerWidth;
  INDEX vfp_iSlicesPerHeight;
  
  INDEX vfp_iTopShape;
  INDEX vfp_iBottomShape;
  
  FLOAT vfp_fAmplitude;
  FLOAT vfp_fMipStart;
  FLOAT vfp_fMipStep;
  CTFileName vfp_fnDisplacement;

  CValuesForPrimitive();
  inline CValuesForPrimitive(CValuesForPrimitive& vfpToCopy)
  {
    *this = vfpToCopy;
  }

  inline CValuesForPrimitive& operator=(const CValuesForPrimitive& vfpToCopy)
  {
    vfp_avVerticesOnBaseOfPrimitive = vfpToCopy.vfp_avVerticesOnBaseOfPrimitive;
    vfp_ptPrimitiveType = vfpToCopy.vfp_ptPrimitiveType;
    vfp_o3dPrimitive = vfpToCopy.vfp_o3dPrimitive;
    vfp_plPrimitive = vfpToCopy.vfp_plPrimitive;
    vfp_ttTriangularisationType = vfpToCopy.vfp_ttTriangularisationType;
    vfp_bDummy = vfpToCopy.vfp_bDummy;
    vfp_ptPrimitiveType = vfpToCopy.vfp_ptPrimitiveType;
    vfp_bAutoCreateMipBrushes = vfpToCopy.vfp_bAutoCreateMipBrushes;
    vfp_colSectorsColor = vfpToCopy.vfp_colSectorsColor;
    vfp_colPolygonsColor = vfpToCopy.vfp_colPolygonsColor;
    vfp_fXMin = vfpToCopy.vfp_fXMin;
    vfp_fXMax = vfpToCopy.vfp_fXMax;
    vfp_fYMin = vfpToCopy.vfp_fYMin;
    vfp_fYMax = vfpToCopy.vfp_fYMax;
    vfp_fZMin = vfpToCopy.vfp_fZMin;
    vfp_fZMax = vfpToCopy.vfp_fZMax;
    vfp_fShearX = vfpToCopy.vfp_fShearX;
    vfp_fShearZ = vfpToCopy.vfp_fShearZ;
    vfp_fStretchX = vfpToCopy.vfp_fStretchX;
    vfp_fStretchY = vfpToCopy.vfp_fStretchY;
    vfp_bClosed = vfpToCopy.vfp_bClosed;

    vfp_bLinearStaircases = vfpToCopy.vfp_bLinearStaircases;
    vfp_bOuter = vfpToCopy.vfp_bOuter;
    vfp_iSlicesIn360 = vfpToCopy.vfp_iSlicesIn360;
    vfp_iNoOfSlices = vfpToCopy.vfp_iNoOfSlices;
    vfp_fRadius = vfpToCopy.vfp_fRadius;

    vfp_iMeridians = vfpToCopy.vfp_iMeridians;
    vfp_iParalels = vfpToCopy.vfp_iParalels;

    vfp_iSlicesPerWidth = vfpToCopy.vfp_iSlicesPerWidth;
    vfp_iSlicesPerHeight = vfpToCopy.vfp_iSlicesPerHeight;
  
    vfp_iTopShape = vfpToCopy.vfp_iTopShape;
    vfp_iBottomShape = vfpToCopy.vfp_iBottomShape;

    vfp_fAmplitude = vfpToCopy.vfp_fAmplitude;
    vfp_fnDisplacement = vfpToCopy.vfp_fnDisplacement;
    vfp_fMipStart = vfpToCopy.vfp_fMipStart;
    vfp_fMipStep = vfpToCopy.vfp_fMipStep;

    return *this;
  }

  BOOL operator==(const CValuesForPrimitive &vfpToCompare) const;
  inline CValuesForPrimitive operator+(const CValuesForPrimitive &vfpToAdd);
  inline CValuesForPrimitive &operator+=(const CValuesForPrimitive &vfpToAdd);
  inline CValuesForPrimitive operator-(const CValuesForPrimitive &vfpToSub);
  inline CValuesForPrimitive &operator-=(const CValuesForPrimitive &vfpToSub);
  void CalculatePrimitiveBase(void);
  inline void CorrectInvalidValues(void);
  void ReadFromIniFile(CTString strPrimitiveType);
  void WriteToIniFile(CTString strPrimitiveType);
  void Read_t(CTStream &strmFile);
  void Write_t(CTStream &strmFile);
};

// Class used for holding global modeler's preferences
class CAppPrefs
{
public:
	~CAppPrefs();
  BOOL ap_CopyExistingWindowPrefs;
  BOOL ap_AutoMaximizeWindow;
  BOOL ap_SetDefaultColors;
  BOOL ap_AutomaticInfo;
  BOOL ap_BinaryGrid;
  BOOL ap_UpdateAllways;
  BOOL ap_bSaveUndoForDelete;
  BOOL ap_bAutoColorize;
  BOOL ap_bShowAllOnOpen;
  BOOL ap_bHideShadowsOnOpen;
  BOOL ap_bAutoUpdateDisplaceMap;
  
  INDEX ap_iUndoLevels;
  INDEX ap_iStartupWindowSetup;

  COLOR ap_DefaultInkColor;
  COLOR ap_DefaultPaperColor;
  COLOR ap_DefaultSelectionColor;
  COLOR ap_DefaultGridColor;

  CTString ap_strSourceSafeProject;
  FLOAT ap_fDefaultFlyModeSpeed;

  INDEX ap_iTerrainSelectionVisible;
  INDEX ap_iTerrainSelectionHidden;

  INDEX ap_iMemoryForTerrainUndo;
  BOOL ap_bAutoUpdateTerrainDistribution;

  void ReadFromIniFile();
  void WriteToIniFile();
};

// Class for holding view preferences
class CViewPrefs
{
public:
  // default constructor
  CViewPrefs(void);
  // set default values
  void SetDefaultValues(void);
  void ClearInvalidConfigPointers(void);
  void Read_t(CTStream& istrFile);   // throw char * // read and
  void Write_t(CTStream& ostrFile);  // throw char * // write functions
  // world's rendering preferences
  CWorldRenderPrefs m_wrpWorldRenderPrefs;
  // model's rendering preferences
  CModelRenderPrefs m_mrpModelRenderPrefs;
  // automatic rendering range
  BOOL m_bAutoRenderingRange;
  // rendering range if not automatic
  FLOAT m_fRenderingRange;
  // color for background paper (if not picture)
  COLOR m_PaperColor;
  // selection color
  COLOR m_SelectionColor;
  // if this view contains measurement tape
  BOOL m_bMeasurementTape;
  // grid color
  COLOR m_GridColor;
  // reserved for background texture
  char m_achrBcgPicture[256];
};

// Class for holding view configurations
class CChildConfiguration
{
public:
  // set default values
  void SetDefaultValues(void);
  void ClearInvalidConfigPointers(void);
  void Read_t(CTStream& istrFile);   // throw char * // read and
  void Write_t(CTStream& ostrFile);  // throw char * // write functions
  // numbers of horizontal splitters
  INDEX m_iHorizontalSplitters;
  // numbers of vertical splitters
  INDEX m_iVerticalSplitters;
  // percentage of left splitter window
  FLOAT m_fPercentageLeft;
  // percentage of top splitter window
  FLOAT m_fPercentageTop;
  // grid on/off flag
  BOOL m_bGridOn;
  // view prefs
  CViewPrefs m_vpViewPrefs[4];
  // type of projection
  enum CSlaveViewer::ProjectionType m_ptProjectionType[ 4];
};

enum CopyType {
  CT_NONE = 0,
  CT_POLYGON_PROPERTIES,
  CT_POLYGON_PROPERTIES_ALTERNATIVE,
  CT_ENTITY,
  CT_SECTOR,
  CT_WORLD,    
};

typedef void TTCFunction_type(void *pView, char *pToolTipText);

class CCustomToolTip {
public:
  HWND cct_hwndCaller;
  TTCFunction_type *cct_pCallback;
  void *cct_pThis;
  void ManualUpdate(void);
  void ManualOn( PIX pixManualX, PIX pixManualY, 
    TTCFunction_type *pCallBack, void *pThis);
  void ManualOff( void);
  void MouseMoveNotify( HWND hwndCaller, ULONG ulTime, TTCFunction_type *pCallBack, void *pThis);
};

class CWorldEditorApp : public CWinAppQt
{
private:
  CWorldEditorDoc *m_pLastActivatedDocument;
  std::function<void(CEntity_*)> m_selection_stealer;
public:
// Atributes
  QObject* mp_qtContext = nullptr;
  FLOAT3D m_vLastTerrainHit;
  CEntityPtr m_penLastTerrainHit;
  FLOAT m_fCurrentTerrainBrush;
  FLOAT m_fTerrainBrushPressure;
  FLOAT m_iTerrainEditMode;
  FLOAT m_iTerrainBrushMode;
  FLOAT m_fTerrainBrushPressureEnum;
  CTFileName m_fnDistributionNoiseTexture;
  CTFileName m_fnContinousNoiseTexture;

  INDEX m_iFBMOctaves;
  FLOAT m_fFBMHighFrequencyStep;
  FLOAT m_fFBMStepFactor;
  FLOAT m_fFBMMaxAmplitude;
  FLOAT m_fFBMfAmplitudeDecreaser;
  BOOL m_bFBMAddNegativeValues;
  BOOL m_bFBMRandomOffset;

  UWORD m_uwEditAltitude;
  FLOAT m_fPaintPower;
  FLOAT m_fSmoothPower;
  INDEX m_iFilter;
  FLOAT m_fFilterPower;
  FLOAT m_fPosterizeStep;
  FLOAT m_fNoiseAltitude;
  INDEX m_iRNDSubdivideAndDisplaceItterations;
  INDEX m_iTerrainGenerationMethod;

  INDEX m_iApi;
  CCustomToolTip m_cttToolTips;
  int m_iLastClassSortAplied;
  BOOL m_bInvertClassSort;
  FLOAT m_tmStartStatusLineInfo;

  CPlacement3D m_plClipboard1;
  CPlacement3D m_plClipboard2;

  std::unique_ptr<CBrushPolygon> m_pbpoClipboardPolygon;
  std::unique_ptr<CBrushPolygon> m_pbpoPolygonWithDeafultValues;
  CTFileName m_fnClassForDropMarker;
  // flag is set while changing display mode
  BOOL m_bChangeDisplayModeInProgress;
  // flag is set while adding/removing property pages
  BOOL m_bDisableDataExchange;
  // curently opened virtual directory path
  CTString m_strOpenedVTDirectory;
  // sector ambient color is stored here on copy paste ambient
  COLOR m_colSectorAmbientClipboard;
  CMappingDefinition m_mdMapping;
  CMappingDefinition m_mdMapping1;
  CMappingDefinition m_mdMapping2;
  CMappingDefinition m_mdMapping3;
  FLOATplane3D m_plMapping;
  FLOATplane3D m_plMapping1;
  FLOATplane3D m_plMapping2;
  FLOATplane3D m_plMapping3;
  FLOAT m_fTerrainSwitchStart;
  FLOAT m_fTerrainSwitchStep;
  // last copy operation type
  enum CopyType m_ctLastCopyType;
  CTString m_strCSGAndShadowStatistics;
  // if should display status info
  BOOL m_bShowStatusInfo;
  // visivility for texture layers
  BOOL m_bTexture1;
  BOOL m_bTexture2;
  BOOL m_bTexture3;
  // if CSG report is enabled
  BOOL m_bCSGReportEnabled;
  // false if undo remembering is disabled
  BOOL m_bRememberUndo;
  // if we are currently in measure mode
  BOOL m_bMeasureModeOn;
  // if we are currently in cut mode
  BOOL m_bCutModeOn;
  // if WED started for the fitst time
  BOOL m_bFirstTimeStarted;
  // show tip of the day on startup
  BOOL m_bShowTipOfTheDay;
  INDEX m_iCurrentTipOfTheDay;
  // rendering preferences for single view
  CViewPrefs m_vpViewPrefs[ VIEW_PREFERENCES_CT+1];
  // child-view configurations remembered
  // pre-last buffer is for full screen
  // last buffer is for maximizing view
  CChildConfiguration m_ccChildConfigurations[ CHILD_CONFIGURATIONS_CT+2];
  // object to hold our document template
  CMultiDocTemplate* m_pDocTemplate;
  // Only instance of CAppPrefs holding preferences
  class CAppPrefs m_Preferences;
  // error texture
  CTextureDataPtr m_ptdError;
  std::unique_ptr<CTextureObject> m_ptoError;
  // icons tray texture
  CTextureDataPtr m_ptdIconsTray;
  // default texture for primitives
  CTextureDataPtr m_ptdActiveTexture;
  // view icons texture
  CTextureDataPtr m_pViewIconsTD;
  // window background texture
	CTFileName m_fnWinBcgTexture;
  // application font
  CFontDataPtr m_pfntSystem;
  // application's windows font
  CFont m_Font;
  CFont m_FixedFont;
  // for holding entity selection marker model
  CTextureDataPtr m_ptdEntityMarkerTexture;
	CModelDataPtr m_pEntityMarkerModelData;
  std::unique_ptr<CModelObject> m_pEntityMarkerModelObject;
  CTextureDataPtr m_gizmo_texture;
  CModelDataPtr m_axis_data;
  std::unique_ptr<CModelObject> m_axis_model;
  CModelDataPtr m_axis_selected_data;
  std::unique_ptr<CModelObject> m_axis_model_selected;
  CModelDataPtr m_ring_data;
  std::unique_ptr<CModelObject> m_ring_model;
  CModelDataPtr m_ring_selected_data;
  std::unique_ptr<CModelObject> m_ring_model_selected;
  // for holding portal selection marker model
  CTextureDataPtr m_ptdPortalMarkerTexture;
	CModelDataPtr m_pPortalMarkerModelData;
  std::unique_ptr<CModelObject> m_pPortalMarkerModelObject;
  // for holding empty brush model
  CTextureDataPtr m_ptdEmptyBrushTexture;
	CModelDataPtr m_pEmptyBrushModelData;
  std::unique_ptr<CModelObject> m_pEmptyBrushModelObject;
  // for holding range sphere model
  CTextureDataPtr m_ptdRangeSphereTexture;
	CModelDataPtr m_pRangeSphereModelData;
  std::unique_ptr<CModelObject> m_pRangeSphereModelObject;
  // for holding angle3D model
  CTextureDataPtr m_ptdAngle3DTexture;
	CModelDataPtr m_pAngle3DModelData;
  std::unique_ptr<CModelObject> m_pAngle3DModelObject;
  // for holding bounding box model
  CTextureDataPtr m_ptdBoundingBoxTexture;
	CModelDataPtr m_pBoundingBoxModelData;
  std::unique_ptr<CModelObject> m_pBoundingBoxModelObject;

  // variables for full screen display mode
	CDisplayMode m_dmFullScreen;
  GfxAPIType   m_gatFullScreen;

  // index of color last used for auto primitive colorization
  INDEX m_iLastAutoColorizeColor; 

  // flag telling function GetActiveDocument() to return NULL (during document chg situation)
  BOOL m_bDocumentChangeOn;
  // flag telling which type of grid is curently on
  BOOL m_bDecadicGrid;
  // current values for primitive
  CValuesForPrimitive m_vfpCurrent;
  // and values for last primitive (for wizard)
  CValuesForPrimitive m_vfpLast;
  // and values for pre last primitive (for wizard)
  CValuesForPrimitive m_vfpPreLast;
  // and delta values for last primitive (for wizard)
  CValuesForPrimitive m_vfpDelta;
  // default values for conus primitives
  CValuesForPrimitive m_vfpConus;
  // default values for torus primitives
  CValuesForPrimitive m_vfpTorus;
  // default values for staircase primitives
  CValuesForPrimitive m_vfpStaircases;
  // default values for sphere primitives
  CValuesForPrimitive m_vfpSphere;
  // default values for terrain primitives
  CValuesForPrimitive m_vfpTerrain;
  // for linking primitives
  std::list<std::unique_ptr<CValuesForPrimitive>> m_lhPrimitiveHistory;
  // obtain currently active view
  CWorldEditorDoc *GetActiveDocument(void);
  // obtain currently active view
  CWorldEditorView *GetActiveView(void);

  CChangeableRT m_ctTerrainPage;
  CChangeableRT m_ctTerrainPageCanvas;

// Operations
  CWorldEditorApp();
	~CWorldEditorApp();

  void AddToRecentFileList(LPCTSTR lpszPathName) override;
  void InstallOneTimeSelectionStealer(std::function<void(CEntity_*)>&& selection_stealer, void* source);
  const std::function<void(CEntity_*)>& GetSelectionStealer() const;
	void MyParseCommandLine(void);
	BOOL SubInitInstance(void);
  void OnFileNew();
  CEntityPtr CreateWorldBaseEntity(CWorld &woWorld, BOOL bZoning, CPlacement3D pl=CPlacement3D(FLOAT3D(0,0,0),ANGLE3D(0,0,0)));
  BOOL Add3DObject(CWorldEditorDoc *pDoc, CEntityPtr penwb, CTFileName fnFile, BOOL bAdd);
  INDEX Insert3DObjects(CWorldEditorDoc *pDoc);
  CWorldEditorDoc *GetLastActivatedDocument(void);
  void ActivateDocument(CWorldEditorDoc *pDocToActivate);
  void RefreshAllDocuments( void);
  // load world and model rendering preferences
  BOOL LoadRenderingPreferences(void);
  // save world and model rendering preferences
  void SaveRenderingPreferences(void);
  // load child configurations
  BOOL LoadChildConfigurations(void);
  // save child configurations
  void SaveChildConfigurations(void);
  // clear possibly invalid pointers in view configurations
  void ClearInvalidConfigPointers(void);
  // reads ini data that can be loaded many times
  void ReadFromIniFile(void);
  // writes ini data that can be saved many times
  void WriteToIniFile(void);
  // store/restore application ini variables
  void WriteToIniFileOnEnd(void);
  void ReadFromIniFileOnInit(void);
  // store/restore default polygon properties
  void WriteDefaultPolygonValues(void); 
  void ReadDefaultPolygonValues(void);
  // sets new active texture for primitive's default material
  void SetNewActiveTexture( CTFileName fnFullTexName);
  // tries to find given item in browser
  void FindItemInBrowser( CTFileName fnItemFileName);
  // texturize selection
  void TexturizeSelection(void);
  // gets curently active document ptr
  CWorldEditorDoc *GetDocument();
  // gets name for given virtual tree node
  CTString GetNameForVirtualTreeNode( CVirtualTreeNode *pvtnNode);
  void DisplayHelp(const CTFileName &fnHlk, UINT uCommand, DWORD dwData);
  void ApplyTerrainEditing(CCastRay &crRayHit);
  
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorldEditorApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL SaveAllModified();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	virtual int Run();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWorldEditorApp)
	afx_msg void OnAppAbout();
  afx_msg void OnQtAbout();
	afx_msg void OnFilePreferences();
	afx_msg void OnFileOpen();
	afx_msg void OnImport3DObject();
	afx_msg void OnDecadicGrid();
	afx_msg void OnUpdateDecadicGrid(CCmdUI* pCmdUI);
	afx_msg void OnConvertWorlds();
	afx_msg void OnSetAsDefault();
	afx_msg void OnHelpShowTipOfTheDay();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// Inline functions

inline BOOL CValuesForPrimitive::operator==(const CValuesForPrimitive &vfpToCompare) const
{
  return (
    (vfp_avVerticesOnBaseOfPrimitive.size() == vfpToCompare.vfp_avVerticesOnBaseOfPrimitive.size() ) &&
    (vfp_ptPrimitiveType == vfpToCompare.vfp_ptPrimitiveType) &&
    (vfp_plPrimitive == vfpToCompare.vfp_plPrimitive) &&
    (vfp_ttTriangularisationType == vfpToCompare.vfp_ttTriangularisationType) &&
    (vfp_bDummy == vfpToCompare.vfp_bDummy) &&
    (vfp_bAutoCreateMipBrushes == vfpToCompare.vfp_bAutoCreateMipBrushes) &&
    (vfp_colSectorsColor == vfpToCompare.vfp_colSectorsColor) &&
    (vfp_colPolygonsColor == vfpToCompare.vfp_colPolygonsColor) &&
    (vfp_fXMin == vfpToCompare.vfp_fXMin) &&
    (vfp_fXMax == vfpToCompare.vfp_fXMax) &&
    (vfp_fYMin == vfpToCompare.vfp_fYMin) &&
    (vfp_fYMax == vfpToCompare.vfp_fYMax) &&
    (vfp_fZMin == vfpToCompare.vfp_fZMin) &&
    (vfp_fZMax == vfpToCompare.vfp_fZMax) &&
    (vfp_fShearX == vfpToCompare.vfp_fShearX) &&
    (vfp_fShearZ == vfpToCompare.vfp_fShearZ) &&
    (vfp_fStretchX == vfpToCompare.vfp_fStretchX) &&
    (vfp_fStretchY == vfpToCompare.vfp_fStretchY) &&
    (vfp_bClosed == vfpToCompare.vfp_bClosed) &&
    (vfp_bLinearStaircases == vfpToCompare.vfp_bLinearStaircases) &&
    (vfp_bOuter == vfpToCompare.vfp_bOuter) &&
    (vfp_iSlicesIn360 == vfpToCompare.vfp_iSlicesIn360) &&
    (vfp_iNoOfSlices == vfpToCompare.vfp_iNoOfSlices) &&
    (vfp_fRadius == vfpToCompare.vfp_fRadius) &&
    (vfp_iMeridians == vfpToCompare.vfp_iMeridians) &&
    (vfp_iParalels == vfpToCompare.vfp_iParalels) &&
    (vfp_iSlicesPerWidth == vfpToCompare.vfp_iSlicesPerWidth) &&
    (vfp_iSlicesPerHeight == vfpToCompare.vfp_iSlicesPerHeight) &&
    (vfp_iTopShape == vfpToCompare.vfp_iTopShape) &&
    (vfp_iBottomShape == vfpToCompare.vfp_iBottomShape) &&
    (vfp_fAmplitude == vfpToCompare.vfp_fAmplitude) &&
    (vfp_fMipStart == vfpToCompare.vfp_fMipStart) &&
    (vfp_fMipStep == vfpToCompare.vfp_fMipStep) &&
    (vfp_fnDisplacement == vfpToCompare.vfp_fnDisplacement)
  );
}
inline CValuesForPrimitive &CValuesForPrimitive::operator+=(const
                                                            CValuesForPrimitive &vfpToAdd)
{
  // add member by member
  vfp_fXMin += vfpToAdd.vfp_fXMin;
  vfp_fXMax += vfpToAdd.vfp_fXMax;
  vfp_fYMin += vfpToAdd.vfp_fYMin;
  vfp_fYMax += vfpToAdd.vfp_fYMax;
  vfp_fZMin += vfpToAdd.vfp_fZMin;
  vfp_fZMax += vfpToAdd.vfp_fZMax;
  vfp_fShearX += vfpToAdd.vfp_fShearX;
  vfp_fShearZ += vfpToAdd.vfp_fShearZ;
  vfp_fStretchX += vfpToAdd.vfp_fStretchX;
  vfp_fStretchY += vfpToAdd.vfp_fStretchY;
  return *this;
}
inline CValuesForPrimitive CValuesForPrimitive::operator+(const
                                                          CValuesForPrimitive &vfpToAdd)
{
  return CValuesForPrimitive(*this) += vfpToAdd;
}

inline CValuesForPrimitive &CValuesForPrimitive::operator-=(const
                                                            CValuesForPrimitive &vfpToSub)
{
  // sub member by member
  vfp_fXMin -= vfpToSub.vfp_fXMin;
  vfp_fXMax -= vfpToSub.vfp_fXMax;
  vfp_fYMin -= vfpToSub.vfp_fYMin;
  vfp_fYMax -= vfpToSub.vfp_fYMax;
  vfp_fZMin -= vfpToSub.vfp_fZMin;
  vfp_fZMax -= vfpToSub.vfp_fZMax;
  vfp_fShearX -= vfpToSub.vfp_fShearX;
  vfp_fShearZ -= vfpToSub.vfp_fShearZ;
  vfp_fStretchX -= vfpToSub.vfp_fStretchX;
  vfp_fStretchY -= vfpToSub.vfp_fStretchY;
  return *this;
}
inline CValuesForPrimitive CValuesForPrimitive::operator-(const
                                                          CValuesForPrimitive &vfpToSub)
{
  return CValuesForPrimitive(*this) -= vfpToSub;
}

#define SNAP_FLOAT_GRID 0.25f
inline void CValuesForPrimitive::CorrectInvalidValues(void)
{
  // some values must be valid, so if they are not, coorect them
  if( vfp_fXMax < vfp_fXMin) vfp_fXMax = vfp_fXMin+SNAP_FLOAT_GRID;
  if( vfp_fYMax < vfp_fYMin) vfp_fYMax = vfp_fYMin+SNAP_FLOAT_GRID;
  if( vfp_fZMax < vfp_fZMin) vfp_fZMax = vfp_fZMin+SNAP_FLOAT_GRID;
}

/////////////////////////////////////////////////////////////////////////////
extern CWorldEditorApp theApp;

#include "CtrlEnumCombo.h"
#include "CtrlEditString.h"
#include "CtrlEditFloat.h"
#include "CtrlEditBoolean.h"
#include "CtrlBrowseFile.h"
#include "CtrlAxisRadio.h"
#include "WndAnimationFrames.h"
#include "WndTestAnimation.h"
#include "WndDisplayTexture.h"
#include "PrimitiveHistoryCombo.h"
#include "EntitySelectionListBox.h"
#include "CSGDesitnationCombo.h"
#include "TriangularisationCombo.h"
#include "EditMipSwitchDistance.h"
#include "ActiveTextureWnd.h"
#include "BrushPaletteWnd.h"
#include "CustomComboWnd.h"
#include "TerrainInterface.h"
#include "TerrainEditing.h"
#include "WndTerrainTilePalette.h"
#include "ViewTexture.h"
#include "ColorPaletteWnd.h"
#include "ToolTipWnd.h"
#include "ColoredButton.h"
#include "CtrlEditFlags.h"
#include "DlgEditFloat.h"
#include "DlgStretchChildOffset.h"
#include "DlgFilterPolygonSelection.h"
#include "DlgFilterVertexSelection.h"
#include "DlgAllignVertices.h"
#include "DlgSnapVertex.h"
#include "DlgWorldSettings.h"
#include "DlgDisplaceMapSize.h"
#include "DlgSelectByName.h"
#include "DlgRenderingPreferences.h"
#include "DlgPgGlobal.h"
#include "DlgPgTerrain.h"
#include "DlgPgPosition.h"
#include "DlgPgPrimitive.h"
#include "DlgPgPolygon.h"
#include "DlgPgShadow.h"
#include "DlgPgSector.h"
#include "DlgPgTexture.h"
#include "DlgPgRenderingStatistics.h"
#include "DlgAutoDeltaCSG.h"
#include "DlgProgress.h"
#include "DlgLightAnimationEditor.h"
#include "DlgTreeShortcuts.h"
#include "DlgBrowseByClass.h"
#include "DlgNumericAlpha.h"
#include "DlgMirrorAndStretch.h"
#include "DlgComment.h"
#include "DlgAutTexturize.h"
#include "DlgEditTerrainBrush.h"
#include "DlgEditTerrainLayer.h"
#include "DlgTEOperationSettings.h"
#include "DlgTerrainProperties.h"
#include "DlgGenerateFBM.h"
#include "InfoSheet.h"
#include "VirtualTreeCtrl.h"
#include "DlgCreateVirtualDirectory.h"
#include "VirtualTreeNode.h"
#include "BrowseWindow.h"
#include "Browser.h"
#include "InfoFrame.h"
#include "PropertyComboBox.h"
#include "PropertyComboBar.h"
#include "DlgPreferences.h"
#include "DlgLinkTree.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "WorldEditorDoc.h"
#include "WorldEditorView.h"

#include "SkyFloat.h"
#endif // WORLDEDITOR_H

