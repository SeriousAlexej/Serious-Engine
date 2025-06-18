@echo off
rem TO BUILD WorldEditor, you should edit the 'WorldEditor\Qt.props' file to point to correct Qt5 installation dir!
for /f "tokens=*" %%a in ('findstr /R /C:"<Qt_dir>.*</Qt_dir>" WorldEditor\\Qt.props') do set QtConfigContent="%%a
set QtConfigContent=%QtConfigContent:<Qt_dir>=%
set QtConfigContent=%QtConfigContent:</Qt_dir>=%
cd qtwinmigrateBuild
cmake -G "Visual Studio 17 2022" -A Win32 -DQt5Core_DIR=%QtConfigContent%\lib\cmake\Qt5Core" -DQt5Gui_DIR=%QtConfigContent%\lib\cmake\Qt5Gui" -DQt5Widgets_DIR=%QtConfigContent%\lib\cmake\Qt5Widgets" ../qtsolutions/qtwinmigrate
exit /b 0
