#
# try to find pthread library and include files
#

# Find Path and Library for pthread-Win32
FIND_PATH(PTHREADS_WIN32_INCLUDE_DIR pthread.h
	"C:/SDK/pthreads/include"
	DOC "Path to the pthread.h header"
	)
FIND_LIBRARY(PTHREADS_WIN32_LIBRARY
	NAMES pthreadVC2
	PATHS "C:/SDK/pthreads/lib/x86"
	DOC "Path to the pthread-win32 library"
	)

