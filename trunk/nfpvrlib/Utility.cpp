#include "Utility.h"
#include "NfpvrTypes.h"
#include "NfpvrSystem.h"
#include <stdexcept>

namespace nfpvr
{

void Utility::extractString(char* dst, int dstLength,
				   const char* src, const int srcOffset, const int srcLength)
{
	int srcIndex = srcOffset;
	int dstIndex = 0;

	while (	dstIndex<dstLength-1 && 
			srcIndex<srcLength && 
			src[srcIndex]!='_')
	{
		dst[dstIndex++] = src[srcIndex++];
	}

	dst[dstIndex]=0;
}

bool Utility::extractNumberChannel(const char* nfString, const int nfLength,
								   char* number, const int numberLength,
								   char* channel, const int channelLength)
{
	int index = nfLength-1;

	while (index>=0 && nfString[index] != '_') index--;
	if (index>0) 
	{ 
		extractString(channel, channelLength,
			nfString, index+1, nfLength);
	}
	else return false;

	index--;
	while (index>=0 && nfString[index] != '_') index--;
	if (index>0) 
	{ 
		extractString(number, numberLength,
			nfString, index+1, nfLength);
	}
	else return false;

	return true;
}

const char* Utility::composeDatedFilename(const char* extension, 
										  const bool hasDetails,
										  const char* number, const char* channel)
{
	const char* dayOfWeek[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	static char filename[42+1+3+1];


#if WIN32 || _XBOX
	SYSTEMTIME time;
	
	GetLocalTime(&time);
	
	int day = time.wDay;
	int month = time.wMonth;
	int year = time.wYear;
	int hour = time.wHour;
	int minute = time.wMinute;
	int weekDay = time.wDayOfWeek;
#else
	time_t theTime;
	time(&theTime);

	struct tm* timeInfo = localtime(&theTime);

	int day = timeInfo->tm_mday;
	int month = timeInfo->tm_mon+1;
	int year = timeInfo->tm_year;
	
	int hour = timeInfo->tm_hour;
	int minute = timeInfo->tm_min;
	int weekDay = timeInfo->tm_wday;
#endif

	const char* ampm = (hour<12)?("AM"):("PM");
	int hourAmpm = (hour<=12)?(hour):(hour-12);

	if (hasDetails)
	{
		snprintf(filename, sizeof(filename), 
			"%s #%s %s. %02d-%02d-%02d at %02d-%02d%s.%s",
			channel, 
			number,
			dayOfWeek[weekDay],
			day, month, (year%100),
			hourAmpm, minute, ampm,
			extension);
	}
	else
	{
		snprintf(filename, sizeof(filename), 
			"%s. %02d-%02d-%02d at %02d-%02d%s.%s",
			dayOfWeek[weekDay],
			day, month, (year%100),
			hourAmpm, minute, ampm,
			extension);
	}

	return filename;
}

const char* Utility::composePath(const char* path, const char* filename)
{
	static char result[256];
	char delimiter[2] = {PATH_DELIMITER, 0};

	int pathLength = (int)strlen(path);
	bool hasSlash  = path[pathLength-1] == PATH_DELIMITER;

	snprintf(result, sizeof(result), "%s%s%s",
		path, 
		(hasSlash)?(""):(delimiter),
		filename);

	return result;
}

}
