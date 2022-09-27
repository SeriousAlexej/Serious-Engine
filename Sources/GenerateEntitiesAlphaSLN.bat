cd EntitiesAlphaBuild
cmake -G "Visual Studio 16 2019" -A Win32 -DSeriousEngine_ROOT="../../" ../EntitiesAlpha
cmake --build . --config Release
exit /b 0
