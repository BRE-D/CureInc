@echo off
REM ============================================================
REM  CureInc - build.bat
REM  Run this file to compile the project.
REM  Usage: just double-click it, or type  build  in the terminal
REM ============================================================

set RAYLIB=C:\raylib\raylib\src

gcc -std=c2x -Wall ^
    -I%RAYLIB% ^
    src/main.c ^
    src/virus.c ^
    src/cure.c ^
    src/region.c ^
    src/events.c ^
    src/skills.c ^
    src/ui.c ^
    -o CureInc.exe ^
    -L%RAYLIB% -lraylib -lopengl32 -lgdi32 -lwinmm -lm

if %ERRORLEVEL% == 0 (
    echo.
    echo Build successful! Run CureInc.exe to start the game.
) else (
    echo.
    echo Build FAILED. Check the errors above.
)
