#ifndef __UdpPacketReader_h__
#define __UdpPacketReader_h__

#include "NfpvrLib.h"
#include "PacketHandler.h"

#define UDP_HANDLER_COUNT (8)

namespace nfpvr
{

class UdpReceiveThread;

class UdpPacketReader
{
public:
	static int UDP_RECEIVE_BUFFER_SIZE;
	
	UdpPacketReader(INfpvrInterfaceNetwork& nfpvrInterface, int udpPort);
	~UdpPacketReader();
	void read();

private:
	bool networkListen();
	bool networkInitialize();
	void networkGetLocalAddress(char* address, int length);
	void networkSetBufferSize(int sockbufsize);

	INfpvrInterfaceNetwork& _nfpvrInterface;

	int            _udpPort;
	nfpvr_socket_t _socket;
	FILE*          _rawFile;
	bool           _isBound;

	int _handlerCount;
	NetworkPacketHandler* _handlers[UDP_HANDLER_COUNT];
};

}

#endif
