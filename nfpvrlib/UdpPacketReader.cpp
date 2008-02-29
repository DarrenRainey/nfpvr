#include "UdpPacketReader.h"
#include "Utility.h"
#include <memory.h>

namespace nfpvr
{

int UdpPacketReader::UDP_RECEIVE_BUFFER_SIZE = 256*1024;

UdpPacketReader::UdpPacketReader(INfpvrInterfaceNetwork& nfpvrInterface,
								 int udpPort):

	_nfpvrInterface(nfpvrInterface),
	_udpPort(udpPort),
	_socket(0),
	_rawFile(0),
	_isBound(false),
	_handlerCount(0)
{
	memset(_handlers, 0, sizeof(_handlers));
	networkInitialize();

	if (_nfpvrInterface.getOptions()._writeRaw)
	{
		const char* filename = _nfpvrInterface.getOptions()._writeRawFilename;
		if (filename)
		{
			_rawFile = fopen(filename, "wb");

			if (_rawFile)
			{
				_nfpvrInterface.notify(INfpvrInterface::NotifyMessage, "Opened raw file %s", filename);
			}
			else
			{
				_nfpvrInterface.notify(INfpvrInterface::NotifyError, "Can't open raw file %s", filename);
			}
		}
	}

}

UdpPacketReader::~UdpPacketReader()
{
	if (_rawFile)
		fclose(_rawFile);
}

void UdpPacketReader::read()
{
	if (!networkListen())
		return;

	_nfpvrInterface.networkListenComplete(_socket);

	char localAddress[16];
	networkGetLocalAddress(localAddress, sizeof(localAddress));
	_nfpvrInterface.notify(INfpvrInterface::NotifyVerbose, "Your local IP is %s", localAddress);

	bool running = true;
	uint64 totalReceived = 0;

	while (running)
	{
		uint8 data[1028];
		sockaddr_in sender;

		int length = _nfpvrInterface.networkReceive(_socket, (nfpvr_sockaddr_t*)&sender, data, sizeof(data));

		if (length)
		{
			if (_nfpvrInterface.getOptions()._writeRaw && _rawFile)
				fwrite(data, length, 1, _rawFile);

			NetworkPacketHandler* current=0;
			int                   index=0;

			for (index=0; index<UDP_HANDLER_COUNT; index++)
			{
				if (_handlers[index] && _handlers[index]->isSender(sender))
				{
					current = _handlers[index];
					break;
				}
			}

			if (!current)
			{
				for (index=0; index<UDP_HANDLER_COUNT; index++)
				{
					if (!_handlers[index])
					{
						current = new NetworkPacketHandler(_nfpvrInterface, sender);
						_handlers[index] = current;

						_handlerCount++;
						_nfpvrInterface.networkNotifyHandlerCount(_handlerCount);
						break;
					}
				}
			}

			if (current)
			{
				current->receive(data, length);
				if (current->getState() == PacketHandler::StateStopped)
				{
					delete current;
					_handlers[index] = 0;

					_handlerCount--;
					_nfpvrInterface.networkNotifyHandlerCount(_handlerCount);
				}
			}

			totalReceived += length;
			_nfpvrInterface.networkNotifyTotalReceived(totalReceived);
		}
		else
		{
			_nfpvrInterface.notify(INfpvrInterface::NotifyWarning, "UdpPacketReader::read, recvfrom() returned -1");
		}
	}
}

void UdpPacketReader::networkGetLocalAddress(char* address, int length)
{
#ifndef _XBOX
	char hostname[255];
	if (!gethostname(hostname, sizeof(hostname)))
	{
		struct hostent* he = gethostbyname(hostname);
		if (he != 0)
		{
			const char* tempAddress = inet_ntoa(*(struct in_addr*)he->h_addr);
			strncpy(address, tempAddress, length);
		}
	}
#endif
}

bool UdpPacketReader::networkInitialize()
{
#if WIN32
	WORD dwVersion = MAKEWORD(2,2);
	WSADATA wsaData;

	if (WSAStartup(dwVersion, &wsaData) != 0)
	{
		_nfpvrInterface.notify(INfpvrInterface::NotifyError, "UdpPacketReader::networkInitialize(), WSAStartup() failed, WSAGetLastError() = %d", WSAGetLastError());
	}
#endif
	return true;
}

bool UdpPacketReader::networkListen()
{
	if (!_socket)
	{
		_socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (_socket == NFPVR_INVALID_SOCKET)
		{
			_socket=0;
			_nfpvrInterface.notify(INfpvrInterface::NotifyError, "UdpPacketReader::networkListen(), socket() failed");
			return false;
		}
	}	
	
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(_udpPort);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if (::bind(_socket, (struct sockaddr*)&sin, sizeof(sockaddr_in)) != 0)
	{
		_nfpvrInterface.notify(INfpvrInterface::NotifyError, "UdpPacketReader::networkListen(), bind() failed");
		return false;
	}

	/*	This is necessary, at least on my machine, for the cygwin build to work,
		otherwise UDP packets were being dropped (the default buffer size is 8192 bytes).

		Otherwise, packets were being dropped and hence video skipped like crazy.
	*/
	networkSetBufferSize(UDP_RECEIVE_BUFFER_SIZE);
	_nfpvrInterface.notify(INfpvrInterface::NotifyVerbose, "Bound to UDP port %d, waiting for data", _udpPort);
	_isBound = true;

	return true;
}


void UdpPacketReader::networkSetBufferSize(int sockbufsize)
{
	int newsize = 0;
	int size = sizeof(int);
	setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char *)&sockbufsize, size);
	getsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char *)&newsize, (nfpvr_socklen_t*)&size);
	
	if (newsize != sockbufsize)
	{
		_nfpvrInterface.notify(INfpvrInterface::NotifyWarning, "UdpPacketReader::networkSetBufferSize(), setsockopt() failed for size %d, getsockopt() returned %d", 
			sockbufsize, newsize);
	}
}
}
