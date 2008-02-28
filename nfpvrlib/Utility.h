#ifndef __utility_h__
#define __utility_h__

#if (WIN32 || _XBOX)
#define PATH_DELIMITER '\\'
#else
#define PATH_DELIMITER '/'
#endif

class Utility
{
public:
	static void extractString(char* dst, int dstLength, const char* src, const int srcOffset, const int srcLength);
	static bool extractNumberChannel(const char* nfString, const int nfLength, char* number, const int numberLength, char* channel, const int channelLength);

	static const char* composePath(const char* path, const char* filename);
	static const char* composeDatedFilename(const char* extension, const bool hasDetails, const char* number=0, const char* channel=0);
};

#endif


