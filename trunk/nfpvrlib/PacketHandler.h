#ifndef __packethandler_h__
#define __packethandler_h__

#include "NfpvrLib.h"
#include "Serializable.h"
#include "PesStreamHandler.h"

#if _XBOX
#define MPEG_FILE_BUFFERED
#endif

class PacketHandler
{
public:
	enum PacketHandlerState
	{
		StateStarted,
		StateNoReference,
		StatePackStarted,
		StateStopped
	};

	PacketHandler(INfpvrInterface& nfpvrInterface);
	virtual ~PacketHandler();
	
	int64 computeTimestamp(int64 timestamp);
	void receive(const uint8* data, int length);
	
	virtual void send(const uint8* data, int length)=0;
	virtual const char* getSource()=0;

	void sendFilenameReply();

	void handleStop();
	void handleFilename(const uint8* data, int length);
	void handleVideo(const uint8* data, int length);
	void handleAudio(const uint8* data, int length);

	enum PacketHandlerState getState() const;

protected:
	INfpvrInterface& _nfpvrInterface;

private:
	static int    OUTPUT_BUFFER_SIZE;
	static int    PACK_BUFFER_SIZE;
	static uint8  VIDEO_STREAM_ID;
	static int    VIDEO_BUFFER_SIZE;
	static uint8  AUDIO_STREAM_ID;
	static int    AUDIO_BUFFER_SIZE;
	static int64  REFERENCE_SCR_OFFSET;
	static uint32 MUX_RATE;

	void writeStreams();

	void handleStream(const uint8* data, int length,
		PesStreamHandler& stream, const char* streamName);

	enum PacketHandlerState _state;
	File*  _pMpegFile;
	Buffer _packBuffer;
	int64  _packStartScr;
	int64  _referenceTimestamp;
	char   _mpegFilename[256];

	PesStreamHandler _videoStream;
	PesStreamHandler _audioStream;
};

class NetworkPacketHandler: public PacketHandler
{
public:
	NetworkPacketHandler(INfpvrInterface& nfpvrInterface,
		struct sockaddr_in& sender);

	~NetworkPacketHandler();

	bool isSender(struct sockaddr_in& sender);

	void send(const uint8* data, int length);
	const char* getSource();

private:

	struct sockaddr_in _sender;
	int                _senderSize;
	nfpvr_socket_t     _socket;
};

class FilePacketHandler: public PacketHandler
{
public:
	FilePacketHandler(INfpvrInterface& nfpvrInterface,
		const char* filename);

	void send(const uint8* data, int length);
	const char* getSource();

private:
	char _filename[256];
};


#endif
