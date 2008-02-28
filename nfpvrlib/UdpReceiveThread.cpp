#include "UdpReceiveThread.h"

UdpReceiveThread::UdpReceiveThread(nfpvr_socket_t socket, int bufferLength, int bufferCount):
	_socket(socket),
	_bufferLength(bufferLength),
	_bufferCount(bufferCount)
{
	_lengths = new int[_bufferCount];
	_data    = new uint8[_bufferCount*_bufferLength];
	_senders = new sockaddr_in[_bufferCount];
	_readIndex = 0;
	_writeIndex = 0;

	_available = CreateSemaphore(NULL, _bufferCount, bufferCount, NULL);
	_occupied  = CreateSemaphore(NULL, 0, _bufferCount, NULL);
	_access =    CreateMutex(NULL, false, NULL);

	HANDLE hThread = CreateThread(NULL, 0, entry, this, 0, NULL);
	SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
}

int UdpReceiveThread::get(uint8* data, const int length, sockaddr_in* sender)
{
	WaitForSingleObject(_occupied, INFINITE);

	int toRead = MIN(length, _lengths[_readIndex]);
	memcpy(data, &_data[_readIndex * _bufferLength], toRead);
	*sender = _senders[_readIndex];

	_readIndex++;
	_readIndex%=_bufferCount;

	ReleaseSemaphore(_available, 1, NULL);

	return toRead;
}

DWORD WINAPI UdpReceiveThread::entry(LPVOID param)
{
	const int senderSize = sizeof(sockaddr_in);
	UdpReceiveThread* reader = (UdpReceiveThread*)param;

	while (true)
	{
		uint8* receiveData =  &reader->_data[reader->_writeIndex*reader->_bufferLength];
		sockaddr_in* sender = &reader->_senders[reader->_writeIndex];

		WaitForSingleObject(reader->_available, INFINITE);
		int received = 0;

		do
		{
			received = recvfrom(reader->_socket, (char*)receiveData, reader->_bufferLength, 0, 
				(nfpvr_sockaddr_t*)sender, (nfpvr_socklen_t*)&senderSize);
		
		} while (received == NFPVR_SOCKET_ERROR);

		reader->_lengths[reader->_writeIndex] = received;
		reader->_writeIndex++;
		reader->_writeIndex%=reader->_bufferCount;

		ReleaseSemaphore(reader->_occupied, 1, NULL);

	}
}
