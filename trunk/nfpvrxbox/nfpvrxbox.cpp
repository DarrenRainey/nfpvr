#include "../nfpvrlib/NfpvrLib.h"
#include "../nfpvrlib/UdpReceiveThread.h"
#include "../../xbmc/trunk/XBMC/xbmc/utils/log.h"
#include "../../xbmc/trunk/XBMC/xbmc/Application.h"

using namespace nfpvr;

static const char* NFPVR_OUTPUT_DIRECTORY = "F:\\";
static const int   NFPVR_UDP_PORT = 50000;

class NfpvrInterfaceNetwork: public INfpvrInterfaceNetwork
{
public:
	Options _options;
	UdpReceiveThread* _pReceiveThread;

	NfpvrInterfaceNetwork():
		_pReceiveThread(0)
	{
		_options._writeRaw = false;
		_options._writeMpeg = true;
		_options._handleAudio = true;
		_options._handleVideo = true;
		_options._bufferOutput = true;
		_options._writeRawFilename = 0;
		_options._outputDirectory = NFPVR_OUTPUT_DIRECTORY;
	}

	const Options& getOptions()
	{
		return _options;		
	}

	void notify(NotifyLevel level, const char* format, ...)
	{
		static char message[1024];
		va_list list;
		va_start(list, format);
		vsnprintf(message, sizeof(message), format, list);
		va_end(list);

		char* messageType="";
		switch (level)
		{
		case NotifyWarning: messageType = "warning: "; break;
		case NotifyError:   messageType = "error: ";   break;
		}
		
		char logMessage[1024];
		sprintf("nfpvr: %s%s", messageType, message);
		CLog::Log(LOGNOTICE, logMessage);
	}

	void notifyStartRecording(const char* filename, const char* number, const char* channel)
	{
		CStdString caption;

		if (number && channel)
			caption.Format("Started recording on %s (#%s)", channel, number);
		else
			caption.Format("Started recording from %s", filename);

		g_application.m_guiDialogKaiToast.QueueNotification(caption, "nFusion PVR");
	}

	void notifyStopRecording(const char* filename)
	{
		CStdString caption = "Stopped recording";
		g_application.m_guiDialogKaiToast.QueueNotification(caption, "nFusion PVR");
	}

	void networkListenComplete(nfpvr_socket_t theSocket)
	{
		if (!_pReceiveThread)
			_pReceiveThread = new UdpReceiveThread(theSocket, 1028, 128);
	}

	int networkReceive(nfpvr_socket_t theSocket, nfpvr_sockaddr_t* theSender, uint8* data, const int length)
	{
		if (_pReceiveThread)
			return _pReceiveThread->get(data, length, theSender);
		else
			return 0; // something wrong here...
	}

};

DWORD WINAPI nfpvrxboxEntry(LPVOID param)
{
	NfpvrInterfaceNetwork iface;
	NfpvrLibReadFromUdp(iface, NFPVR_UDP_PORT);
	return 0;
}

void nfpvrxboxCreateThread()
{
	CreateThread(NULL, 0, nfpvrxboxEntry, 0, 0, 0);
}