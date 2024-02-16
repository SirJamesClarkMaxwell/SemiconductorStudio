@echo off

REM Set the path to the directory containing Premake5 executable
set PREMAKE_DIR=E:\Programing\C++\PreMake5-poligon

REM Set the path to the directory containing your main project Premake5 script
set PROJECT_DIR=E:\Programing\C++\PreMake5-poligon

REM Debug: Print directory paths
echo "Premake Directory: %PREMAKE_DIR%"
echo "Project Directory: %PROJECT_DIR%"

REM Change to the project directory
cd %PROJECT_DIR%

REM Debug: Print the current working directory
echo "Current Working Directory: %cd%"

REM Run Premake5 to generate Visual Studio solution files
%PREMAKE_DIR%\premake5.exe vs2022

echo "Visual Studio solution files generated successfully!"
PAUSE
