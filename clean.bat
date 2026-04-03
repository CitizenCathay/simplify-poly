@echo off
del /Q tests\my-output\*.txt 2>nul
if exist tests\my-output\experimental del /Q tests\my-output\experimental\*.txt
if exist simplify del /Q simplify