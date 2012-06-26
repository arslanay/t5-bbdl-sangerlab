#include <string>
#include "Utilities.h"

template <class T>
bool StrToNum(const std::string& s,
			  T& t, 			 
			  std::ios_base& (*f)(std::ios_base&))
{
	std::istringstream iss(s);
	return !(iss >> f >> t).fail();
}

template <class T>
bool 
NumToStr(T t, 
		 const std::string& s)
{
	std::ostringstream oss(s);
	return !(oss << t).fail();
}

char* ToChar(std::string srcStr)
{
	char *retPtr(new char[srcStr.length() + 1]);

	copy(srcStr.begin(), srcStr.end(), retPtr);
	retPtr[srcStr.length()] = '\0';
	return retPtr;
}
