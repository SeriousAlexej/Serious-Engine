# Serious Engine

[![Build status](https://ci.appveyor.com/api/projects/status/32r7s2skrgm9ubva?retina=true)](https://ci.appveyor.com/project/SeriousAlexej/Serious-Engine)

Enhanced and somewhat fixed version of engine tools (Modeler, World Editor)
* Compatible with 1.07!
* Updated version of `assimp`, more import formats are available!
* Modeler upgrade to correctly read imported UV map without tears at seams (previously required creation of additional surfaces at seams to avoid that problem)
* Modeler bugfix to correctly handle UTF8
* Replaced missing `exploration3D` library with `assimp` (for importing 3D models into Modeler / World Editor)
* Added ability to import UV maps (with up to 3 channels) when converting 3D model into brush (World Editor)
* World Editor bugfix to correctly display color selection window
* New advanced UV mapping tools for World Editor

<p align="center">
Importing 3D meshes with multiple UV maps as brushes in World Editor
</p>
<p align="center">
  <img src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/editor_model.gif">
</p>

<p align="center">
List of new supported 3D file formats
</p>
<p align="center">
  <img src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/modeler_formats.gif">
</p>

<p align="center">
Modeler correctly imports UV map without distorsions by default
</p>
<p align="center">
  <img src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/modeler.gif">
</p>

<details>
  <summary>World Editor mapping demos</summary>
  
  <p align="center">
  3D Importing with UV maps - General demo:
  </p>
  <p align="center">
    <img src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/Import3D_Demo.gif">
  </p>
  
  <p align="center">
  Advanced mapping - General demo:
  </p>
  <p align="center">
    <img src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/AdvancedMapping_Demo.gif">
  </p>
  
  <p align="center">
  Advanced mapping - Rotation alignment:
  </p>
  <p align="center">
    <img src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/AdvancedMapping_Rotate.gif">
  </p>
  
  <p align="center">
  Advanced mapping - Alignment by adjacent edge:
  </p>
  <p align="center">
    <img src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/AdvancedMapping_Adjacent.gif">
  </p>
  
  <p align="center">
  Advanced mapping - Alignment by tangent edge:
  </p>
  <p align="center">
    <img src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/AdvancedMapping_Tangent.gif">
  </p>
  
  <p align="center">
  Advanced mapping - Alignment by adjacent and tangent edges:
  </p>
  <p align="center">
    <img src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/AdvancedMapping_Adjacent_Tangent.gif">
  </p>
</details>

Original ReadMe:
=======================

This is the source code for Serious Engine v.1.10, including the following projects:

* `DedicatedServer`
* `Ecc` The *Entity Class Compiler*, a custom build tool used to compile *.es files
* `Engine` Serious Engine 1.10
* `EngineGUI` Common GUI things for game tools
* `EntitiesMP` All the entity logic
* `GameGUIMP` Common GUI things for game tools
* `GameMP` All the game logic
* `Modeler` Serious Modeler
* `RCon` Used to connect to servers using an admin password
* `SeriousSam` The main game executable
* `SeriousSkaStudio` Serious Ska Studio
* `WorldEditor` Serious Editor
* `DecodeReport` Used to decode crash *.rpt files
* `Depend` Used to build a list of dependency files based on a list of root files
* `LWSkaExporter` Exporter for use in LightWave
* `MakeFONT` Used for generating *.fnt files
* `Shaders` Compiled shaders
* `GameAgent` The serverlist masterserver written in Python
* `libogg`, `libvorbis` Third party libraries used for playing OGG-encoded ingame music (see http://www.vorbis.com/ for more information)

These have been modified to run correctly under the recent version of Windows. (Tested: Win7 x64, Win8 x64, Win8.1 x64)

Building
--------

To build Serious Engine 1, you'll need Visual Studio 2013 or 2015, Professional or Community edition ( https://www.visualstudio.com/post-download-vs?sku=community ).

Do not use spaces in the path to the solution.

Once you've installed Visual Studio and (optionally) DirectX8 SDK, you can build the engine solution (`/Sources/All.sln`). Press F7 or Build -> Build solution. The libraries and executables will be put into `\Bin\` directory (or `\Bin\Debug\` if you are using the Debug configuration).

Optional features
-----------------

DirectX support is disabled by default. If you need DirectX support you'll have to download DirectX8 SDK (headers & libraries) ( http://files.seriouszone.com/download.php?fileid=759 or https://www.microsoft.com/en-us/download/details.aspx?id=6812 ) and then enable the SE1_D3D switch for all projects in the solution (Project properties -> Configuration properties -> C/C++ -> Preprocessor -> Preprocessor definitions -> Add "SE1_D3D" for both Debug and Release builds). You will also need to make sure the DirectX8 headers and libraries are located in the following folders (make the folder structure if it's not existing yet):
* `/Tools.Win32/Libraries/DX8SDK/Include/..`
* `/Tools.Win32/Libraries/DX8SDK/Lib/..`

MP3 playback is disabled by default. If you need this feature, you will have to copy amp11lib.dll to the '\Bin\' directory (and '\Bin\Debug\' for MP3 support in debug mode). The amp11lib.dll is distributed with older versions of Serious Sam: The First Encounter.

3D Exploration support is disabled in the open source version of Serious Engine 1 due to copyright issues. In case if you need to create new models you will have to either use editing tools from any of the original games, or write your own code for 3D object import/export.

IFeel support is disabled in the open source version of Serious Engine 1 due to copyright issues. In case if you need IFeel support you will have to copy IFC22.dll and ImmWrapper.dll from the original game into the `\Bin\` folder.

Running
-------

This version of the engine comes with a set of resources (`\SE1_10.GRO`) that allow you to freely use the engine without any additional resources required. However if you want to open or modify levels from Serious Sam Classic: The First Encounter or The Second Encounter (including most user-made levels), you will have to copy the game's resources (.GRO files) into the engine folder. You can buy the original games on Steam, as a part of a bundle with Serious Sam Revolution ( http://store.steampowered.com/app/227780 )

When running a selected project, make sure its project settings on Debugging is set to the right command:
* For debug:
    $(SolutionDir)..\Bin\Debug\$(TargetName).exe`
* For release:
    $(SolutionDir)..\Bin\$(TargetName).exe`
And its working directory:
    $(SolutionDir)..\

Common problems
---------------

Before starting the build process, make sure you have a "Temp" folder in your development directory. If it doesn't exist, create it.
SeriousSkaStudio has some issues with MFC windows that can prevent the main window from being displayed properly.

License
-------

Serious Engine is licensed under the GNU GPL v2 (see LICENSE file).

Some of the code included with the engine sources is not licensed under the GNU GPL v2:

* zlib (located in `Sources/Engine/zlib`) by Jean-loup Gailly and Mark Adler
* LightWave SDK (located in `Sources/LWSkaExporter/SDK`) by NewTek Inc.
* libogg/libvorbis (located in `Sources/libogg` and `Sources/libvorbis`) by Xiph.Org Foundation
