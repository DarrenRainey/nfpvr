#ifndef __programoptions_h__
#define __programoptions_h__

class ProgramOptions
{
public:
	ProgramOptions();

	static int DEFAULT_UDP_PORT;

	bool  _writeMpeg;
	bool  _writeRaw;
	bool  _readRaw;
	bool  _verbose;
	bool  _help;
	bool  _version;
	bool  _receiveThread;
	bool  _handleAudio;
	bool  _handleVideo;
	bool  _bufferOutput;

	int   _udpPort;

	char* _writeRawFilename;
	char* _readRawFilename;
	char* _outputDirectory;
};

#endif
