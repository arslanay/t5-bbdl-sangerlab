##########################################################################
#
# $Id: FindInventor.cmake 319 2005-08-19 01:22:38Z scharver $
#
# Author: Chris Scharver
# Email:  scharver@evl.uic.edu
#
# Try to find the Inventor library and include files. It will locate
# files for SGI Open Inventor, TGS Inventor, or Coin. TGS Inventor
# is used if the OIVHOME environment variable is present, but paths
# may be manually overridden with CMake.
#
# INVENTOR_INCLUDE_DIR, where to find main Inventor/.
# INVENTOR_LIBRARIES, the libraries to link against to use Inventor
# INVENTOR_FOUND, If false, do not try to use Inventor.
#
# Also defined, but not for general use are:
# INVENTOR_oiv_LIBRARY = the full path to the Inventor library.
# ??? = perhaps add SoXt, SoQt, etc.?
#
##########################################################################

SET( TGS_OIVHOME "$ENV{OIVHOME}" )

IF (WIN32)

  IF(CYGWIN)

    FIND_PATH( INVENTOR_INCLUDE_DIR Inventor/SoDB.h
      /usr/include
      /usr/local/include
    )

    FIND_LIBRARY( INVENTOR_oiv_LIBRARY
      NAMES Inv311d Inventor
      PATHS /usr/lib
            /usr/lib/w32api
            /usr/local/lib
    )

  ELSE(CYGWIN)

    # One of the basic header files is the SoDB header. Locate
    # this header in the search paths. Look in the default TGS
    # and Coin3D directories.
    FIND_PATH( INVENTOR_INCLUDE_DIR Inventor/SoDB.h
      C:/Coin3D/include
      ${TGS_OIVHOME}/include
    )

    # There are several different available library names
    # depending upon the thread model, debug settings, or
    # version used. Multiple names are specified, and the first
    # matching library will be used.
    FIND_LIBRARY( INVENTOR_oiv_LIBRARY
      NAMES coin2d coin2 Inv311d Inv311 Inventor
      PATHS C:/Coin3D/lib
            ${TGS_OIVHOME}/lib
    )

  ENDIF(CYGWIN)

ELSE (WIN32)

  IF (APPLE)

    FIND_PATH( INVENTOR_FW_INCLUDE_DIR Inventor.framework
      /usr/local/include
      /Library/Frameworks
    )
    SET(INVENTOR_FRAMEWORK "-framework Inventor" CACHE
      STRING "Inventor framework")

  ENDIF (APPLE)

  FIND_PATH( INVENTOR_INCLUDE_DIR Inventor/SoDB.h
    /usr/include
    /usr/local/include
    /usr/openwin/share/include
    /opt/graphics/include
    /opt/local/include
  )

  FIND_LIBRARY( INVENTOR_oiv_LIBRARY
    NAMES Inv311d Inventor Coin
    PATHS /usr/lib
          /usr/local/lib
          /opt/local/lib
  )

ENDIF (WIN32)

SET( INVENTOR_FOUND "NO" )
IF(INVENTOR_INCLUDE_DIR)
  IF(INVENTOR_oiv_LIBRARY)
    # Is -lXi and -lXmu required on all platforms that have it?
    # If not, we need some way to figure out what platform we are on.
    SET( INVENTOR_LIBRARIES
      ${INVENTOR_oiv_LIBRARY}
    )
    SET( INVENTOR_FOUND "YES" )

    IF (WIN32)
      OPTION (USE_COIN_DLL "Use Coin as a DLL?" YES)
      IF (USE_COIN_DLL)
        SET(COIN_DEF "-DCOIN_DLL")
      ELSE (USE_COIN_DLL)
        SET(COIN_DEF "-DCOIN_NOT_DLL")
      ENDIF (USE_COIN_DLL)
      ADD_DEFINITIONS(${COIN_DEF})
    ENDIF (WIN32)

  ENDIF(INVENTOR_oiv_LIBRARY)
ELSE(INVENTOR_INCLUDE_DIR)
  IF (INVENTOR_FW_INCLUDE_DIR)
    SET(INVENTOR_LIBRARIES ${INVENTOR_FRAMEWORK})
    SET(INVENTOR_FOUND "YES")
  ENDIF (INVENTOR_FW_INCLUDE_DIR)
ENDIF(INVENTOR_INCLUDE_DIR)

