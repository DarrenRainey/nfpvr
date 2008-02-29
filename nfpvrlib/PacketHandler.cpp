#include "PacketHandler.h"
#include "Utility.h"
#include "Bitfield.h"
#include <memory.h>
#include <string.h>

#if (WIN32 || _XBOX)
#else
#include <unistd.h>
#endif

namespace nfpvr
{

int    PacketHandler::OUTPUT_BUFFER_SIZE = 1024*1024;

int64  PacketHandler::REFERENCE_SCR_OFFSET = 169970;
uint32 PacketHandler::MUX_RATE = 5697;

int    PacketHandler::PACK_BUFFER_SIZE = 512*1024;

uint8  PacketHandler::VIDEO_STREAM_ID = 0xE0;
int    PacketHandler::VIDEO_BUFFER_SIZE = 2024*1024;

uint8  PacketHandler::AUDIO_STREAM_ID = 0xC0;
int    PacketHandler::AUDIO_BUFFER_SIZE = 512*1024;

PacketHandler::PacketHandler(INfpvrInterface& nfpvrInterface):
	_nfpvrInterface(nfpvrInterface),
	_state(StateStarted),
	_pMpegFile(0),
	_packBuffer(PACK_BUFFER_SIZE),
	_packStartScr(0),
	_referenceTimestamp(0),
	_videoStream(VIDEO_STREAM_ID, VIDEO_BUFFER_SIZE),
	_audioStream(AUDIO_STREAM_ID, AUDIO_BUFFER_SIZE)
{
	if (nfpvrInterface.getOptions()._bufferOutput)
		_pMpegFile = new FileBuffered(OUTPUT_BUFFER_SIZE);
	else
		_pMpegFile = new File();
}

PacketHandler::~PacketHandler()
{
	if (_pMpegFile)
		delete _pMpegFile;
}

enum PacketHandler::PacketHandlerState PacketHandler::getState() const
{
	return _state;
}

int64 PacketHandler::computeTimestamp(int64 timestamp)
{
	switch (_state)
	{
	case StatePackStarted:
		return (int64)timestamp - 
			(int64)_referenceTimestamp + REFERENCE_SCR_OFFSET;

	default:
		return 0;
	}
}

void PacketHandler::writeStreams()
{
	const bool handleAudio = _nfpvrInterface.getOptions()._handleAudio;
	const bool handleVideo = _nfpvrInterface.getOptions()._handleVideo;

	while (_videoStream.isPesFound())
	{
		MpegHeader::PesInfo videoPes;
		_videoStream.getCurrentHeader(videoPes);

		int64 videoDts = computeTimestamp(videoPes.dts);
		int64 videoPts = computeTimestamp(videoPes.pts);

		if (videoPes.hasDts)
		{
			switch (_state)
			{
			case StateNoReference:
				_referenceTimestamp = videoDts;
				_packStartScr = videoDts;
				_state = StatePackStarted;

				MpegHeader::writePackHeader(_packBuffer, MUX_RATE, videoDts);
				
				if (handleVideo)
					_videoStream.writePes(_packBuffer, videoPts);
				else
					_videoStream.discardPes();

				_videoStream.findPesHeader();
				break;

			case StatePackStarted:
				{
					if (videoDts<_packStartScr)
						_nfpvrInterface.notify(INfpvrInterface::NotifyWarning, "Video dts inconsistency (dts: %lld)", videoPes.dts);

					bool handleAudioFinished = false;

					if (handleAudio)
					{
						if (!_audioStream.isPesFound())
							return;

						while (_audioStream.isPesFound())
						{
							MpegHeader::PesInfo audioPes;
							_audioStream.getCurrentHeader(audioPes);
							int64 audioPts = computeTimestamp(audioPes.pts);

							if (audioPts < _packStartScr)
							{
								_nfpvrInterface.notify(INfpvrInterface::NotifyWarning, "Discarding premature audio (pts: %lld)", audioPes.pts);
								_audioStream.discardPes();
								_audioStream.findPesHeader();
							}
							else if (audioPts >= _packStartScr &&
									 audioPts < videoDts)
							{
								_audioStream.writePes(*_pMpegFile, audioPts);
								_audioStream.findPesHeader();
							}
							else // if (audioPts >= videoDts)
							{
								handleAudioFinished = true;
								break;
							}
						}
					}
					else // if (!handleAudio)
					{
						handleAudioFinished = true;
					}

					if (handleAudioFinished)
					{
						_packBuffer.writeTo(*_pMpegFile);
						_packBuffer.reset();

						MpegHeader::writePackHeader(_packBuffer, MUX_RATE, videoDts);
						_packStartScr = videoDts;

						if (handleVideo)
							_videoStream.writePes(_packBuffer, videoPts);
						else
							_videoStream.discardPes();

						_videoStream.findPesHeader();

					}
					break;
				}

			default:
				break;
			}
		}
		else // if (!videoPes.hasDts)
		{
			switch (_state)
			{
			case StateNoReference:
				_videoStream.discardPes();
				_videoStream.findPesHeader();
				break;

			case StatePackStarted:
				if (handleVideo)
					_videoStream.writePes(_packBuffer, videoPts);
				else
					_videoStream.discardPes();

				_videoStream.findPesHeader();
				break;

			default:
				break;
			}
		}

	}	
}

void PacketHandler::handleStream(const uint8* data, int length,
								 PesStreamHandler& stream, const char* streamName)
{
	int discarded = 0;

	while (stream.writeLeft() < length)
	{
		stream.discardPes();
		stream.findPesHeader();
		discarded++;
	}

	if (discarded)
	{
		_nfpvrInterface.notify(INfpvrInterface::NotifyWarning, "Stream '%s' discarded %d headers", streamName, discarded);
	}
	
	stream.write(data, length);

	if(stream.findPesHeader())
		writeStreams();
}

void PacketHandler::handleVideo(const uint8* data, int length)
{
	handleStream(data, length, _videoStream, "video");
}

void PacketHandler::handleAudio(const uint8* data, int length)
{
	const bool handleAudio = _nfpvrInterface.getOptions()._handleAudio;
	
	if (handleAudio)
		handleStream(data, length, _audioStream, "audio");
}

void PacketHandler::handleStop()
{
	if (_nfpvrInterface.getOptions()._writeMpeg && _pMpegFile->isOpened())
	{
		MpegHeader::writeProgramEndHeader(*_pMpegFile);
		_pMpegFile->close();
		_nfpvrInterface.notify(INfpvrInterface::NotifyMessage, "Stopped recording");
	}
	else
	{
		_nfpvrInterface.notify(INfpvrInterface::NotifyMessage, "Stopped recording simulation");
	}

	_nfpvrInterface.notifyStopRecording(_mpegFilename);
	_state = StateStopped;
}

void PacketHandler::sendFilenameReply()
{
	uint8 reply[4];
	reply[0] = 0x38;
	reply[1] = 0x80;
	reply[2] = 0x00;
	reply[3] = 0x00;

	send(reply, 4);	
	_state = StateNoReference;
}

void PacketHandler::handleFilename(const uint8* data, int length)
{
	const char* extension = "mpg";
	const char* path = 0;

	char number[5], channel[6];

	bool hasDetails = Utility::extractNumberChannel((const char*)data, length,
		number, sizeof(number),
		channel, sizeof(channel));
	
	const char* filename = Utility::composeDatedFilename(extension, hasDetails, number, channel);
	const char* outputDirectory = _nfpvrInterface.getOptions()._outputDirectory;

	// copy filename locally
	strncpy(_mpegFilename, filename, sizeof(_mpegFilename)-1);

	if (outputDirectory)
	{
		path = Utility::composePath(outputDirectory, filename);
	}
	else
	{
		path = filename;
	}

	if (_nfpvrInterface.getOptions()._writeMpeg)
	{
		_pMpegFile->open(path);

		if (_pMpegFile->isOpened())
		{
			_nfpvrInterface.notify(INfpvrInterface::NotifyMessage, "Started recording on \"%s\" from %s", path, getSource());
		}
		else
		{
			_nfpvrInterface.notify(INfpvrInterface::NotifyError, "Can't open mpeg file \"%s\"", path);
		}
	}
	else
	{
		_nfpvrInterface.notify(INfpvrInterface::NotifyMessage, "Simulating recording on \"%s\"", path);
	}

	if (hasDetails)
		_nfpvrInterface.notifyStartRecording(_mpegFilename, number, channel);
	else
		_nfpvrInterface.notifyStartRecording(_mpegFilename);

	sendFilenameReply();
}

void PacketHandler::receive(const uint8* data, int length)
{
	// not even a header & length
	if (length<4)
	{
		_nfpvrInterface.notify(INfpvrInterface::NotifyWarning, "Received packet with length<4");
		return;
	}
	
	uint16 header     = Bitfield::decode<uint16>(data+0);
	uint16 dataLength = Bitfield::decode<uint16>(data+2);

	bool streaming = (_state != StateStarted) && (_state != StateStopped);
		
	if (header == 0x3810)
	{
		handleFilename(data+4, dataLength);	
	}
	else if (header == 0x381A)
	{
		if (streaming)
			handleStop();
	}
	else if (header == 0x3800)
	{
		if (_nfpvrInterface.getOptions()._writeMpeg && streaming)
			handleVideo(data+4, dataLength);
	}
	else if (header == 0x3801)
	{
		if (_nfpvrInterface.getOptions()._writeMpeg && streaming)
			handleAudio(data+4, dataLength);
	}
	else if (header == 0x3880)
	{
		// ignore, reply from PC to NF
	}
	else
	{
		_nfpvrInterface.notify(INfpvrInterface::NotifyWarning, "Received packet with unknown header 0x%04x", header);
	}
}

NetworkPacketHandler::NetworkPacketHandler(INfpvrInterface& nfpvrInterface, 
										   struct sockaddr_in& sender):
	PacketHandler(nfpvrInterface),
	_sender(sender),
	_senderSize(sizeof(sender)),
	_socket(0)
{
	_socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (_socket == NFPVR_INVALID_SOCKET)
	{
		_socket=0;
		_nfpvrInterface.notify(INfpvrInterface::NotifyError, "NetworkPacketHandler::NetworkPacketHandler(), socket() failed");
	}
}

NetworkPacketHandler::~NetworkPacketHandler()
{
#if (WIN32 || _XBOX)
	closesocket(_socket);
#else
	close(_socket);
#endif
}

bool NetworkPacketHandler::isSender(struct sockaddr_in& sender)
{
	if (sender.sin_addr.s_addr == _sender.sin_addr.s_addr &&
		sender.sin_port == _sender.sin_port)
	{
		return true;
	}
	else
		return false;
}

void NetworkPacketHandler::send(const uint8* data, int length)
{
	int n = ::sendto(_socket, (const char*)(data), length, 0, (nfpvr_sockaddr_t*)&_sender, _senderSize);

#ifndef _XBOX
	//const char* address = inet_ntoa(_sender.sin_addr);
#endif

	if (n != length)
	{
		_nfpvrInterface.notify(INfpvrInterface::NotifyWarning, "UdpPacketReader::send(), sendto returned %d\n", n);
	}
}

const char* NetworkPacketHandler::getSource()
{
#if _XBOX
	// do I need to implement inet_ntoa?
	return "(network)";
#else
	return inet_ntoa(_sender.sin_addr);
#endif
}

FilePacketHandler::FilePacketHandler(INfpvrInterface& nfpvrInterface,
									 const char* filename):
	PacketHandler(nfpvrInterface)
{
	memset(_filename, 0, sizeof(_filename));
	strncpy(_filename, filename, sizeof(_filename)-1);
}

void FilePacketHandler::send(const uint8* data, int length)
{
	// do nothing
}

const char* FilePacketHandler::getSource()
{
	return _filename;
}

}
