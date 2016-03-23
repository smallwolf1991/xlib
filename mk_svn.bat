set DstPath=E:\work\sharkfin\xlib

rd /S /Q %DstPath%

mklink  /J   %DstPath%       ..\xlib

pause