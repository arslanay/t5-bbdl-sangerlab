#
# try to find AntTweakBar library and include files
#

# Find Path and Library for AntTweakBar-Win32
FIND_PATH(ATB_WIN32_INCLUDE_DIR AntTweakBar.h
	"C:/SDK/AntTweakBar/include"
	DOC "Path to the AntTweakBar.h header"
	)
FIND_LIBRARY(ATB_WIN32_LIBRARY
	NAMES AntTweakBar
	PATHS "C:/SDK/AntTweakBar/lib"
	DOC "Path to the AntTweakBar-win32 library"
	)

