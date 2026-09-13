@echo off
cd qtwinmigrateBuild
cmake -G "Visual Studio 17 2022" -A Win32 -DQt6Core_DIR="%cd%\..\Qt6\lib\cmake\Qt6Core" -DQt6Gui_DIR="%cd%\..\Qt6\lib\cmake\Qt6Gui" -DQt6Widgets_DIR="%cd%\..\Qt6\lib\cmake\Qt6Widgets" ../qtsolutions/qtwinmigrate
exit /b 0
