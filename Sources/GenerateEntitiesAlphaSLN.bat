cd EntitiesAlphaBuild
cmake -G "Visual Studio 17 2022" -A Win32 -DSeriousEngine_ROOT="../../" ../EntitiesAlpha
cmake --build . --config Release
exit /b 0
