#
# try to find AntTweakBar library and include files
#

# Find Path and Library for AntTweakBar-Win32
FIND_PATH(IPP_INCLUDE_DIR ipp.h
	"C:/Program Files/Intel/Composer XE 2011 SP1/ipp/include"
	DOC "Path to Intel Parallel Primitives ipp headers"
	)
FIND_LIBRARY(IPP_LIBRARY
	NAMES ipps.lib
	PATHS "C:/Program Files/Intel/Composer XE 2011 SP1/ipp/lib/ia32"
	DOC "Path to the Intel Parallel Primitives ipp  win32 library"
	)

