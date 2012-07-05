##########################################################################
#
# Author: C. Minos Niu 
# Email:  minos.niu AT sangerlab.net
#
# Try to find the HapticAPI library and include files. Note that
# Windows is the only supported platform at this time, and the library
# requires MFC as well.
#
#
# also defined, but not for general use is
#
##########################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(reflex_single_muscle)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

FIND_PATH(CMN_CMAKE_MODULES FindHapticAPI.cmake
	${PROJECT_SOURCE_DIR}/modules
	DOC "Folder containing custom CMake modules")
SET(CMAKE_MODULE_PATH ${CMN_CMAKE_MODULES} ${CMAKE_MODULE_PATH})

FIND_PACKAGE(GLUT REQUIRED)
#FIND_PACKAGE(OpenGL REQUIRED)
#FIND_PACKAGE(HapticAPI REQUIRED)
#FIND_PACKAGE(ScopeAPI REQUIRED)
FIND_PACKAGE(Pthreads REQUIRED)
#FIND_PACKAGE(Inventor REQUIRED)
#FIND_PACKAGE Boost
#SET(Boost_USE_STATIC_LIBS   ON)
#SET(Boost_USE_MULTITHREADED ON)
#FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)
FIND_PACKAGE(NIDAQMX REQUIRED)
FIND_PACKAGE(OpalKelly REQUIRED)
FIND_PACKAGE(AntTweakBar REQUIRED)


# Include the following directories when compiling 
INCLUDE_DIRECTORIES(
 	${PROJECT_SOURCE_DIR}/include
#	${HAPTICAPI_INCLUDE_DIR}
# 	${OPENGL_INCLUDE_DIR}
#	${GLUT_INCLUDE_DIR}
# 	${SCOPEAPI_INCLUDE_DIR}
 	${PTHREADS_WIN32_INCLUDE_DIR}
#  	${INVENTOR_INCLUDE_DIR}	
# 	${Boost_INCLUDE_DIR} # Library names ARE CaSE-sENsITve!!!!!!!
 	${NIDAQMX_INCLUDE_DIR}
	${ATB_WIN32_INCLUDE_DIR}
	)


# Compile the ExampleLib library.
# ADD_LIBRARY(ExampleLib ExampleLib.cpp)

# THe examples link against the same libraries, so group them here to
# make the TARGET_LINK_LIBRARIES easier to read.
SET(SYS_LIBRARIES
	${GLUT_LIBRARIES}
# 	${HAPTICAPI_LIBRARIES}
# 	${SCOPEAPI_LIBRARIES}
 	${PTHREADS_WIN32_LIBRARY}
# 	${OPENGL_LIBRARIES}
# 	${INVENTOR_LIBRARIES}
# 	${Boost_PROGRAM_OPTIONS_LIBRARY}
# 	${Boost_PROGRAM_OPTIONS_LIBRARY_DEBUG}
 	${NIDAQMX_LIBRARIES}
	${ATB_WIN32_LIBRARY}
# 	ExampleLib
	)
# Make sure custom-written libraries get the correct dependencies
SET(CMN_LIBRARIES
	Utilities
	PxiDAQ
	OGLGraph
	#ReadParadigm
	#TriggerOutput
	)

FOREACH(LIB ${CMN_LIBRARIES})
	ADD_LIBRARY(${LIB} ${LIB}.cpp)
	TARGET_LINK_LIBRARIES(${LIB} ${SYS_LIBRARIES})
ENDFOREACH(LIB)

#ADD_DEPENDENCIES(ReadParadigm Utilities)
ADD_DEPENDENCIES(PxiDAQ Utilities okFrontPanelDLL OGLGraph)

# Rather than individually listing each example, let's take advantage
# of CMake's loop capabilities. Just list each example's base name.
SET(CMN_EXECUTABLE_NAMES
	reflex_single_muscle
	OGLGraph_test
	)

# Now loop through each name, defining the executable by grabbing the
# identically-named source file and linking the required libraries.
FOREACH(EXEC ${CMN_EXECUTABLE_NAMES})
	ADD_EXECUTABLE(${EXEC} ${EXEC}.cpp)
	TARGET_LINK_LIBRARIES(${EXEC} ${SYS_LIBRARIES} ${CMN_LIBRARIES})
ENDFOREACH(EXEC)