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

// set this to 1 to enable checks whether somethig is deleted while iterating some array/container
#define CHECKARRAYLOCKING 0

#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <math.h>
#include <search.h>   // for qsort
#include <float.h>    // for FPU control

/* rcg10042001 !!! FIXME: Move these somewhere. */
#if (defined WIN32)
#include <conio.h>
#include <crtdbg.h>
#include <winsock2.h>
#include <windows.h>
#include <mmsystem.h> // for timers
#endif

#include <Engine/Base/Base.h>
#include <Engine/Base/Types.h>

#include <Engine/Base/Input.h>
#include <Engine/Base/KeyNames.h>
#include <Engine/Base/Updateable.h>
#include <Engine/Base/ErrorReporting.h>
#include <Engine/Base/ErrorTable.h>
#include <Engine/Base/ReplaceFile.h>
#include <Engine/Base/Stream.h>
#include <Engine/Base/Lists.h>
#include <Engine/Base/Timer.h>
#include <Engine/Base/ListIterator.inl>
#include <Engine/Base/Console.h>
#include <Engine/Base/Console_internal.h>
#include <Engine/Base/Shell_internal.h>
#include <Engine/Base/Shell.h>
#include <Engine/Base/Statistics.h>
#include <Engine/Base/CRC.h>
#include <Engine/Base/Translation.h>
#include <Engine/Base/ProgressHook.h>

#include <Engine/Entities/EntityClass.h>
#include <Engine/Entities/EntityCollision.h>
#include <Engine/Entities/EntityProperties.h>
#include <Engine/Entities/Entity.h>
#include <Engine/Entities/InternalClasses.h>
#include <Engine/Entities/LastPositions.h>
#include <Engine/Entities/EntityCollision.h>
#include <Engine/Entities/ShadingInfo.h>
#include <Engine/Entities/FieldSettings.h>
#include <Engine/Entities/Precaching.h>

#include <Engine/Light/LightSource.h>
#include <Engine/Light/LensFlares.h>
#include <Engine/Light/Shadows_internal.h>
#include <Engine/Light/Gradient.h>

#include <Engine/Math/Geometry.inl>
#include <Engine/Math/Clipping.inl>
#include <Engine/Math/FixInt.h>
#include <Engine/Math/Float.h>
#include <Engine/Math/Object3D.h>
#include <Engine/Math/Functions.h>
#include <Engine/Math/Quaternion.h>
#include <Engine/Math/Projection.h>
#include <Engine/Math/Projection_DOUBLE.h>

#include <Engine/Network/Network.h>
#include <Engine/Network/Server.h>
#include <Engine/Network/NetworkMessage.h>
#include <Engine/Network/PlayerSource.h>
#include <Engine/Network/PlayerBuffer.h>
#include <Engine/Network/PlayerTarget.h>
#include <Engine/Network/SessionState.h>

#include <Engine/Brushes/Brush.h>
#include <Engine/Brushes/BrushTransformed.h>
#include <Engine/Brushes/BrushArchive.h>


//#include <Engine/Terrain/Terrain.h>

#include <Engine/World/World.h>
#include <Engine/World/WorldRayCasting.h>
#include <Engine/World/WorldSettings.h>
#include <Engine/World/WorldCollision.h>

#include <Engine/Rendering/Render.h>
#include <Engine/Rendering/Render_internal.h>

#include <Engine/Models/ModelObject.h>
#include <Engine/Models/ModelData.h>
#include <Engine/Models/Model_internal.h>
#include <Engine/Models/EditModel.h>
#include <Engine/Models/RenderModel.h>

#include <Engine/Ska/ModelInstance.h>
#include <Engine/Ska/Mesh.h>
#include <Engine/Ska/Skeleton.h>
#include <Engine/Ska/AnimSet.h>
#include <Engine/Ska/StringTable.h>
#include <Engine/Ska/Render.h>

#include <Engine/Sound/SoundObject.h>
#include <Engine/Sound/SoundLibrary.h>
#include <Engine/Sound/SoundListener.h>

#include <Engine/Graphics/Texture.h>
#include <Engine/Graphics/Color.h>
#include <Engine/Graphics/Font.h>
#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Graphics/ViewPort.h>
#include <Engine/Graphics/DrawPort.h>
#include <Engine/Graphics/ImageInfo.h>
#include <Engine/Graphics/RenderScene.h>
#include <Engine/Graphics/RenderPoly.h>
#include <Engine/Graphics/Fog.h>
#include <Engine/Graphics/Stereo.h>

#include <Engine/Templates/BSP.h>
#include <Engine/Templates/BSP_internal.h>
#include <Engine/Templates/DynamicStackArray.h>
#include <Engine/Templates/LinearAllocator.h>
#include <Engine/Templates/DynamicArray.h>
#include <Engine/Templates/DynamicContainer.h>
#include <Engine/Templates/StaticArray.h>
#include <Engine/Templates/StaticStackArray.h>
#include <Engine/Templates/Selection.h>


// some global stuff
ENGINE_API void SE_InitEngine( CTString strGameID);
ENGINE_API void SE_EndEngine(void);
ENGINE_API void SE_LoadDefaultFonts(void);
ENGINE_API void SE_UpdateWindowHandle( HWND hwndWindowed);

extern ENGINE_API CTString _strEngineBuild;  // not valid before InitEngine()!
extern ENGINE_API ULONG _ulEngineBuildMajor;
extern ENGINE_API ULONG _ulEngineBuildMinor;

extern ENGINE_API BOOL _bDedicatedServer;
extern ENGINE_API BOOL _bWorldEditorApp; // is this world edtior app
extern ENGINE_API CTString _strLogFile;

// temporary vars for adjustments
ENGINE_API extern FLOAT tmp_af[10];
ENGINE_API extern INDEX tmp_ai[10];
ENGINE_API extern INDEX tmp_i;
ENGINE_API extern INDEX tmp_fAdd;
