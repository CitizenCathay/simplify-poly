@echo off
del /Q tests\my-output\*.txt 2>nul
if exist simplify del /Q simplify