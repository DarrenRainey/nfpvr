#include "ProgramOptions.h"

int ProgramOptions::DEFAULT_UDP_PORT = 50000;

ProgramOptions::ProgramOptions()
{
	_bufferOutput  = false;
	_help          = false;
	_handleAudio   = true;
	_readRaw       = false;
	_receiveThread = false;
	_verbose       = false;
	_version       = false;
	_writeMpeg     = true;
	_writeRaw      = false;

	_udpPort = DEFAULT_UDP_PORT;

	_writeRawFilename = 0;
	_readRawFilename  = 0;
	_outputDirectory  = 0;
}

