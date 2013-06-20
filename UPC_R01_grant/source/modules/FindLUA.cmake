#
# try to find AntTweakBar library and include files
#


# Find Path and Library for AntTweakBar-Win32
FIND_PATH(LUA_INCLUDE_DIR lua.h
	"C:/Program Files/Lua/5.1/include"
	DOC "Path to Lua headers"
	)
FIND_PATH(LUA_INCLUDE_DIR lualib.h
	"C:/Program Files/Lua/5.1/include"
	DOC "Path to Lua headers"
	)
FIND_PATH(LUA_INCLUDE_DIR lauxlib.h
	"C:/Program Files/Lua/5.1/include"
	DOC "Path to Lua headers"
	)

FIND_LIBRARY(LUA_LIBRARY
	NAMES lua51.lib
	PATHS "C:/Program Files/Lua/5.1/lib"
	DOC "Path to the Lua library files"
	)

