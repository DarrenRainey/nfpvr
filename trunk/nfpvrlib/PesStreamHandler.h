#ifndef __pesstreamhandler_h__
#define __pesstreamhandler_h__

#include "NfpvrTypes.h"
#include "Serializable.h"
#include "MpegHeader.h"

class PesStreamHandler
{
public:
	PesStreamHandler(uint8 streamId, int bufferSize);
	~PesStreamHandler();

	enum PesStreamState
	{
		StateFoundNothing,
		StateFoundCurrentHeader,
		StateFoundNextHeader
	};

	int    write(const uint8* data, int length);
	int    writeLeft();

	bool   findPesHeader();
	int    discardPes(); 
	void   writePes(Serializable& dst, uint64 pts);

	bool   isPesHeaderFound();
	bool   isPesFound();
	
	void   getCurrentHeader(MpegHeader::PesInfo& pes);
	void   getNextHeader(MpegHeader::PesInfo& pes);
	
	const  Buffer& getBuffer();
	void   reset();

private:
	int    discard(int size);

	static int    MAXIMUM_PES_SIZE;

	enum PesStreamState        _state;
	struct MpegHeader::PesInfo _pesInfoCurrent;
	struct MpegHeader::PesInfo _pesInfoNext;
	int                        _lastFindOffset;
	uint8                      _streamId;
	Buffer                     _buffer;
};

#endif
