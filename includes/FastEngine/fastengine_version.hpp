#ifndef _FGE_FASTENGINE_VERSION_HPP_INCLUDED
#define _FGE_FASTENGINE_VERSION_HPP_INCLUDED

namespace fge{
	
	//Software Status
	static const char VERSION_STATUS[] =  "Alpha";
	static const char VERSION_STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long VERSION_MAJOR  = 0;
	static const long VERSION_MINOR  = 9;
	static const long VERSION_BUILD  = 329;
	static const long VERSION_REVISION  = 1829;
	
	//Miscellaneous Version Types
	static const long VERSION_BUILDS_COUNT  = 6748;
	#define VERSION_RC_FILEVERSION 0,9,329,1829
	#define VERSION_RC_FILEVERSION_STRING "0, 9, 329, 1829\0"
	static const char VERSION_FULLVERSION_STRING [] = "0.9.329.1829";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long VERSION_BUILD_HISTORY  = 0;
	

}
#endif //_FGE_FASTENGINE_VERSION_HPP_INCLUDED
