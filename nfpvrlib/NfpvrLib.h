#ifndef __NfpvrInterface_h__
#define __NfpvrInterface_h__

#include "NfpvrSystem.h"
#include "NfpvrTypes.h"

namespace nfpvr
{

class INfpvrInterface
{
public:
	typedef enum
	{
		NotifyMessage,
		NotifyWarning,
		NotifyVerbose,
		NotifyError
	} NotifyLevel;

	typedef struct
	{
		bool _writeRaw;
		bool _writeMpeg;
		bool _handleAudio;
		bool _handleVideo;
		bool _bufferOutput;
		const char* _writeRawFilename;
		const char* _outputDirectory;
	} Options;

	INfpvrInterface() {}
	
	virtual void           notifyStartRecording(const char* filename, const char* number=0, const char* channel=0) {}
	virtual void           notifyStopRecording(const char* filename) {}
	virtual void           notify(NotifyLevel level, const char* format, ...)=0;
	virtual const Options& getOptions()=0;
};

class INfpvrInterfaceNetwork: public INfpvrInterface
{
public:
	virtual void networkNotifyHandlerCount(int handlerCount) {}
	virtual void networkNotifyTotalReceived(uint64 bytes) {}
	virtual void networkListenComplete(nfpvr_socket_t theSocket) {}
	virtual int  networkReceive(nfpvr_socket_t theSocket, nfpvr_sockaddr_t* theSender, uint8* data, const int length)=0;
};

int NfpvrLibReadFromFile(INfpvrInterface& iface, const char* filename);
int NfpvrLibReadFromUdp(INfpvrInterfaceNetwork& iface, int udpPort);

}

#endif

/*

#if _XBOX
		g_application.m_guiDialogKaiToast.QueueNotification("Stopped recording", "nFusion PVR");
#endif

#if _XBOX
			CStdString caption;
			if (hasDetails)
				caption.Format("Started recording on %s (#%s)", channel, number);
			else
				caption.Format("Started recording from %s", getSource());

			g_application.m_guiDialogKaiToast.QueueNotification(caption, "nFusion PVR");
#endif


int UdpPacketReader::receive(uint8* data, const int length, sockaddr_in* sender)
{
	int received = 0;

	if (_options._receiveThread && _receiveThread)
	{
#if _XBOX || WIN32
		return _receiveThread->get(data, length, sender);
#endif
	}
	else
	{
		const int senderSize = sizeof(struct sockaddr_in);

		int received = 0;

		do
		{
			received = recvfrom(_socket, (char*)(data), length, 0, 
				(nfpvr_sockaddr_t*)sender, (nfpvr_socklen_t*)&senderSize);
		} while (received == NFPVR_SOCKET_ERROR);

		return received;
	}
}

#if _XBOX || WIN32
	if (_options._receiveThread)
	{
		_receiveThread = new UdpReceiveThread(_socket, 1028, 128);
	}
#endif

#ifndef _XBOX
	char hostname[255];
	if (!gethostname(hostname, sizeof(hostname)))
	{
		struct hostent* he = gethostbyname(hostname);
		if (he != 0)
		{
			const char* address = inet_ntoa(*(struct in_addr*)he->h_addr);
			Logger::log(Normal, Info, "your IP address is %s", address);
		}
	}
#endif


#if _XBOX
#include "../xbmc/trunk/XBMC/xbmc/utils/log.h"
#endif
#if _XBOX
		CLog::Log(LOGNOTICE, message);
#else

*/
