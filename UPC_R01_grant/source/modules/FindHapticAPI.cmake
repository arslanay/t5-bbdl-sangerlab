##########################################################################
#
# $Id$
#
# Author: Chris Scharver
# Email:  c-scharver@northwestern.edu
#
# Try to find the HapticAPI library and include files.
#
# HAPTICAPI_INCLUDE_DIR, where to find HapticAPI.h
# HAPTICAPI_LIBRARIES, the libraries to link against.
# HAPTICAPI_FOUND, if false, do not try to use the HapticAPI.
#
# also defined, but not for general use is
# HAPTICAPI_hapticapi_LIBRARY, full path to HapticAPI library.
#
##########################################################################

IF (WIN32)

  FIND_PATH( HAPTICAPI_INCLUDE_DIR HapticAPI.h
    "C:/HapticMASTER/HapticAPI 1.4.3 (4DOF)/Win32/HapticAPI/Include"
    DOC "Path to the HapticAPI.h header"
  )

  FIND_LIBRARY( HAPTICAPI_LIBRARIES
    NAMES HapticAPI
    PATHS "C:/HapticMASTER/HapticAPI 1.4.3 (4DOF)/Win32/HapticAPI"
          ${HAPTICAPI_INCLUDE_DIR}
    DOC "Path to the HapticAPI library"
  )

ELSE (WIN32)

  MESSAGE("HapticAPI not currently supported on this system.")

ENDIF (WIN32)

# Platform dependent libraries required by the HapticAPI.
IF (WIN32)
  FIND_PACKAGE(MFC REQUIRED)
  #SET( HAPTICAPI_PLATFORM_DEPENDENT_LIBS wsock32 )
  # add stuff to use MFC in this executable
  ADD_DEFINITIONS(-D_AFXDLL)
  SET(CMAKE_MFC_FLAG 2)
ENDIF (WIN32)

SET(HAPTICAPI_FOUND "NO")
IF (HAPTICAPI_INCLUDE_DIR)
	SET(HAPTICAPI_FOUND "YES")
ENDIF (HAPTICAPI_INCLUDE_DIR)
