@echo off

set compiler_flags=-MTd -nologo -Gm- -GR- -EHa- -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -FC -Z7
set linker_flags=-incremental:no -opt:ref user32.lib Gdi32.lib opengl32.lib Ole32.lib

IF NOT EXIST .\data mkdir .\data
IF NOT EXIST .\build mkdir .\build

pushd .\build

del *.pdb > NUL 2> NUL

REM Game icon build
IF NOT EXIST ..\build\game_icon.res rc ..\data\game_icon.rc
IF NOT EXIST ..\build\game_icon.res move ..\data\game_icon.res ..\build\game_icon.res

REM Debug build
cl -DBUILD_DEBUG=1 %compiler_flags% ..\code\game.cpp -Fmgame.map -LD /link -incremental:no -opt:ref -PDB:game_%random%.pdb /EXPORT:initialize_game /EXPORT:update_game /EXPORT:render_game
cl -DBUILD_DEBUG=1 %compiler_flags% ..\code\windows_main.cpp -Fmwindows_main.map /link game_icon.res %linker_flags%

REM Release build for 32-bit
REM -DBUILD_DEBUG=1 cl %compiler_flags% ..\code\windows_main.cpp /link -subsystem:windows,5.02 %linker_flags%

REM Release build for 64-bit
cl %compiler_flags% ..\code\windows_main.cpp /link game_icon.res %linker_flags% -OUT:windows_main_release.exe

popd
