#
# try to find pthread library and include files
#

# Find Path and Library for pthread-Win32

FIND_LIBRARY(OK_WIN32_LIBRARY
	NAMES okFrontPanel
	PATHS ${PROJECT_SOURCE_DIR}/OpalKelly
	DOC "Path to the OpalKelly-win32 DLL"
	)

