#ifndef __mpegheader_h__
#define __mpegheader_h__

#include "NfpvrTypes.h"
#include "Serializable.h"

class MpegHeader
{
public:
	enum MpegHeaderStatus
	{
		StatusIncomplete,
		StatusError,
		StatusOk
	};

	typedef struct PesInfo
	{
		bool   hasPts;
		bool   hasDts;
		uint8  streamId;
		uint64 pts;
		uint64 dts;
		uint16 packetLength;
		int    headerOffset;
		int    headerLength;
	};

	static enum MpegHeaderStatus readPesHeader(const uint8* data, int length, struct PesInfo& pesInfo);
	static int                   readPackHeader(const uint8* data, int length, uint32& mux_rate, uint64& scr);

	static void writePackHeader(Serializable& dst, uint32 mux_rate, uint64 scr);
	static void writePesHeader(Serializable& dst, uint8 streamId, uint16 dataLength, bool hasPts = false, uint64 pts = 0);
	static void writeProgramEndHeader(Serializable& dst);

};

#endif
