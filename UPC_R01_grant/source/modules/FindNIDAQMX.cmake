#
# try to find pthread library and include files
#

# Find Path and Library for NIDAQmx
IF (WIN32)
	FIND_PATH(NIDAQMX_INCLUDE_DIR nidaqmx.h
		"C:/Program Files/National Instruments/NI-DAQ/DAQmx ANSI C Dev/include"
		DOC "Path to the nidaqmx.h header"
		)
	FIND_LIBRARY(NIDAQMX_LIBRARIES
		NAMES nidaqmx
		PATHS "C:/Program Files/National Instruments/NI-DAQ/DAQmx ANSI C Dev/lib/msvc"
		DOC "Path to the nidaqmx library"
		)
ENDIF (WIN32)