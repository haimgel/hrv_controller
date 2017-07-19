@echo off
for /f "usebackq tokens=*" %%a in (`"git rev-parse --short=8 HEAD"`) do echo -DPIO_SRC_REV='"%%a"'
