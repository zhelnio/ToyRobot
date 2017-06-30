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

rem �������� ����� � ����������
rem %AVRDUDE% -p %CHIPMODEL% -c 2ftbb -B 4800 -P ft0 -U hfuse:r:high.txt:s -U lfuse:r:low.txt:s

rem �������� ����� �� ����������
rem 0xE3 - ��� ATMega88PA 
rem    - ��������� �������� �� 8
rem    - ���������� ��������� �� 128 ���
rem %AVRDUDE% -p %CHIPMODEL% -c 2ftbb -B 4800 -P ft0 -U lfuse:w:0xe3:m

rem �������� ��������� ����� �� ����������
rem %AVRDUDE% -p t13 -c 2ftbb -B 2400 -P ft0 -U lfuse:w:0x6A:m

pause