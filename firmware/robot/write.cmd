echo off

set SOLUTIONNAME=robot
set PROJECTNAME=robot

rem set STEPNAME=Release
set STEPNAME=Debug

set PROJECTSPATH=D:\ZSL\electro\projects
set AVRDUDE="D:\ZSL\electro\bin\avrdude\avrdude.exe"
set HEX=%PROJECTSPATH%\%SOLUTIONNAME%\%PROJECTNAME%\%STEPNAME%\%PROJECTNAME%.hex"

set CHIPMODEL=m88 -F

%AVRDUDE% -p %CHIPMODEL% -c 2ftbb -B 4800 -P ft0 -U flash:w:%HEX%:a

rem получить фьюзы с контролера
rem %AVRDUDE% -p %CHIPMODEL% -c 2ftbb -B 4800 -P ft0 -U hfuse:r:high.txt:s -U lfuse:r:low.txt:s

rem записать фьюзы на контроллер
rem 0xE3 - дл€ ATMega88PA 
rem    - отключаем делитель на 8
rem    - выставл€ем генератор на 128  √ц
rem %AVRDUDE% -p %CHIPMODEL% -c 2ftbb -B 4800 -P ft0 -U lfuse:w:0xe3:m

rem записать дефолтные фьюзы на контроллер
rem %AVRDUDE% -p t13 -c 2ftbb -B 2400 -P ft0 -U lfuse:w:0x6A:m

pause