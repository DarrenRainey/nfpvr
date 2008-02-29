#ifndef __FilePacketReader_h__
#define __FilePacketReader_h__

#include "NfpvrLib.h"
#include "PacketHandler.h"

namespace nfpvr
{

class FilePacketReader
{
public:
	FilePacketReader(INfpvrInterface& nfpvrInterface, const char* filename);
	void read();

private:
	void readFromFile(const char* filename);

	const char* _filename;
	INfpvrInterface& _nfpvrInterface;
	FilePacketHandler _packetHandler;
};

}

#endif
