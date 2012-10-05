##########################################################################
#
# $Id:$
#
# Author: Chris Scharver
# Email:  c-scharver@northwestern.edu
#
# Try to find the HapticAPI's ScopeAPI library and include files.
#
# SCOPEAPI_INCLUDE_DIR, where to find ScopeAPI.h
# SCOPEAPI_LIBRARIES, the libraries to link against.
# SCOPEAPI_FOUND, if false, do not try to use the ScopeAPI.
#
# also defined, but not for general use is
# SCOPEAPI_scopeapi_LIBRARY, full path to the ScopeAPI library.
#
##########################################################################

IF (WIN32)

  FIND_PATH( SCOPEAPI_INCLUDE_DIR ScopeAPI.h
    C:/HapticMASTER/ScopeAPI/Win32/include
    DOC "Path to the ScopeAPI.h header"
  )

  FIND_LIBRARY(SCOPEAPI_LIBRARIES
    NAMES ScopeAPI
    PATHS C:/HapticMASTER/ScopeAPI/Win32
    DOC "Path to the ScopeAPI library"
  )

ELSE (WIN32)

  MESSAGE("ScopeAPI not currently supported on this system.")

ENDIF (WIN32)

# Platform dependent libraries required by the HapticAPI.
IF (WIN32)
  FIND_PACKAGE(MFC REQUIRED)
  #SET( SCOPEAPI_PLATFORM_DEPENDENT_LIBS wsock32 )
  # add stuff to use MFC in this executable
  ADD_DEFINITIONS(-D_AFXDLL)
  SET(CMAKE_MFC_FLAG 2)
ENDIF (WIN32)

SET(SCOPEAPI_FOUND "NO")
IF (SCOPEAPI_INCLUDE_DIR)
	SET(SCOPEAPI_FOUND "YES")
ENDIF (SCOPEAPI_INCLUDE_DIR)

#MARK_AS_ADVANCED(
#  SCOPEAPI_scopeapi_LIBRARY
#)