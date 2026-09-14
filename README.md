<p align="center">
  <img style="width: 10vw; min-width: 128px;" src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Sources/WorldEditor/res/WorldEditor_uncropped.svg">
  <img style="width: 10vw; min-width: 128px;" src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Sources/Modeler/res/Modeler_uncropped.svg">
</p>

# Serious Editor EX / Serious Modeler EX

[![Build status](https://ci.appveyor.com/api/projects/status/32r7s2skrgm9ubva?retina=true)](https://ci.appveyor.com/project/SeriousAlexej/Serious-Engine)

***
Enhanced and fixed version of the Serious Sam Classic modding tools - **Serious Editor** and **Serious Modeler** with complete forward and backward compatibility with 1.07 engine, mods and custom dlls!

Note that this is **NOT** an updated *Serious Engine 1.10*.

## What's new (since previous release):

### Video Demonstration (YouTube, clickable)
[![Feature Showcase](https://img.youtube.com/vi/SuW_IoAzXbI/maxresdefault.jpg)](https://youtu.be/SuW_IoAzXbI)

### Serious Editor EX / Serious Modeler EX
* Added **Gro Browser** to all tools - no need to extract gro files anymore, all archived resources are now directly accessible when picking a file!
* Integrated an optional crash handler library. It is possible to enable small/big crashdump generation, or disable it completely (via the `File` menu in the Editor/Modeler). Small crashdump generation is enabled by default, they will be generated in the `Bin\CrashRpt` subdirectory and can be used to report issues for future fixes.

### Serious Editor EX
* Full 1.07 and custom dlls/mods compatibility achieved via [Serious Engine C API](https://github.com/SeriousAlexej/SeriousEngineCAPI) and [Serious Engine Cpp API](https://github.com/SeriousAlexej/SeriousEngineCppAPI) interface libraries, that load vanilla engine for the level editor/modeler
* Added **Transform Gizmo** to the editor! With ability to perform translation and rotation in absolute, local and relative coordinate systems!
* Added **Camera Viewfinder** to the Serious Editor - now when camera is selected, a viewfinder can be optionally displayed to easily setup cutscenes!
* Added **Test Difficulty** and **Test Gamemode** options for quicktest - it is now easy to test the level for all difficulties and in multiplayer!
* Added **Filter** field to the *Select Entities* dialog - now it is easy to find a particular entity!
* Added **Edit String** dialog for simple editing of multiline messages/strings in Serious Editor!
* Improved usability of spawn flags selection in all entities and weapons/ammo selection in Player Marker entity.

### Serious Modeler EX
* Added **Bézier inter-frame interpolation** for skeletal animations. This is useful when some animation channels have fewer keyframes than others, or when custom number of frames is used to bake the animation (that is greater than the number of keyframes).
* Added **Relative origin bone** selection for skeletal animations. Very useful when creating animated models with animated attachments (like playermodels) - attached models can pick their parent bone as a relative origin, to properly generate the geometry and animations.

## Installation
Download the [latest release](https://github.com/SeriousAlexej/Serious-Engine/releases/latest) zip archive with binaries (i.e. `SeriousEditorEX-1.10.51.zip`) and extract it to your `Serious Sam - The Second Encounter` directory (i.e. for steam edition it will be something like `D:\Steam\steamapps\common\Serious Sam Classic The Second Encounter`)

If everything is correct, the `SeriousEditorEX.exe` and `SeriousModelerEX.exe` should appear in the `Bin` directory, next to the old editor and modeler executables.

## Previous release

### Video Demonstration (YouTube, clickable)
[![Feature Showcase](https://img.youtube.com/vi/0rrdHwJSGF8/maxresdefault.jpg)](https://youtu.be/0rrdHwJSGF8)

* Engine now supports **A LOT** more image formats for texture creation (such as PNG for example)
* Serious Modeler can now import skeletal animations! (baked into vertex frame animation during the import for compatibility)
* Added Property Tree for Serious Editor for more convenient workflow!
* Updated version of `assimp`, more import formats are available!
* Modeler upgrade to correctly read imported UV map without tears at seams (previously required creation of additional surfaces at seams to avoid that problem)
* Modeler has convenient UI for configuring model instead of manual script writing
* Replaced missing `exploration3D` library with `assimp` (for importing 3D models into Modeler / World Editor)
* Added ability to import UV maps (with up to 3 channels) when converting 3D model into brush (World Editor)
* World Editor bugfix to correctly display color selection window
* New advanced UV mapping tools for World Editor

<details>
  <summary>Skeletal animation demos</summary>
  
  <p align="center">
  Skeletal animation import support in Serious Modeler
  </p>
  <p align="center">
    <img src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/run_flesh.gif">
  </p>
  <p align="center">
  Same model with automatically generated triangles per each bone
    <img src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/run_bones.gif">
  </p>
</details>

<details>
  <summary>New Property Tree in Serious Editor</summary>

  <p align="center">
    <img src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/editor_property_tree.gif">
  </p>
  <p align="center">
    <img src="https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/editor_property_tree_2.gif">
  </p>
</details>

<details>
  <summary>New 3D formats importing demos</summary>
  
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
</details>

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

## Building
To build this project, you will need `Visual Studio 2022` with `MFC` and `ATL` packages (all available in Visual Studio installer) and a `CMake` (version >= 3.12).
Here is a step-by-step instruction on how to build the solution:
1. Checkout the repository recursively with all submodules
2. Go into the `Sources` directory and run `GenerateAssimpSLN.bat`, `GenerateQtWinMigrateSLN.bat`, `GenerateSeriousEngineCppAPI.bat` and `GenerateCrashRptSLN.bat` to generate extra project files for the submodules
3. Open the `All.sln` solution file in `Visual Studio 2022` and run the build, everything should be ready to go

## Issue reporting
If you experience an issue, please [report it here](https://github.com/SeriousAlexej/Serious-Engine/issues).

If the issue involves a crash, please attach a crash report as well. Crash dumps are put in the `Bin\CrashRpt` directory. Full crash dumps are always preferable, since they contain much more debugging information.

To enable a full crash dump generation, please, tick it in the `File` menu:
![File\Full crash dumps](https://raw.githubusercontent.com/SeriousAlexej/Serious-Engine/master/Help/fullcrashdumps.png)