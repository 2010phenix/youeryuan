@ECHO OFF
SETLOCAL

set /p var=0<build.txt
set /a var+=1
echo %var% > build.txt

ECHO #define VER_MAJOR 1 > version_parsed.h
ECHO #define VER_MINOR 1 >> version_parsed.h
ECHO #define VER_BUGFIX 0 >> version_parsed.h
ECHO #define VER_BUILDNUM %var% >> version_parsed.h
ECHO #define VER_COMPANY "�����������о���"  >> version_parsed.h
ECHO #define VER_PRODUCT	"����Զ�̼�ع�����ƶ�"  >> version_parsed.h
ECHO #define DO_STRINGIFY(x) #x >> version_parsed.h
ECHO #define STRINGIFY(x) DO_STRINGIFY(x) >> version_parsed.h
ENDLOCAL
EXIT /B
