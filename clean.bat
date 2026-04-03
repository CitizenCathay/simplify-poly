@echo off
del /Q tests\my-output\*.txt 2>nul
if exist tests\my-output\experimental del /Q tests\my-output\experimental\*.txt 2>nul

del /Q tests\svg-overlay\*.svg 2>nul
if exist tests\svg-overlay\experimental del /Q tests\svg-overlay\experimental\*.svg 2>nul

if exist simplify del /Q simplify 2>nul
if exist overlay_svg del /Q overlay_svg 2>nul