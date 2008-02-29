#define _WIN32_IE 0x0600
#include "NfpvrwinInterface.h"
#include "nfpvrwin.h"
#include <stdio.h>

using namespace nfpvr;

NfpvrwinInterface::NfpvrwinInterface():
	_hwnd(0),
	_isRecording(false),
	_totalReceived(0),
	_handlerCount(0)
{
	_outputDirectory[0]=0;
	_outputDirectoryWide[0]=0;

	_options._writeRaw = false;
	_options._writeMpeg = true;
	_options._handleAudio = true;
	_options._handleVideo = true;
	_options._bufferOutput = false;
	_options._writeRawFilename = 0;
	_options._outputDirectory = _outputDirectory;
}

void NfpvrwinInterface::notify(NotifyLevel level, const char* format, ...)
{
	if (level == NotifyMessage)
	{
		static char message[1024];
		va_list list;
		va_start(list, format);
		vsnprintf(message, sizeof(message), format, list);
		va_end(list);
		
		
		static wchar_t wideString[1024];
		size_t wideLength;
		mbstowcs_s(&wideLength, wideString, 1024, message, _TRUNCATE);

		showTooltip(wideString);
	}
}

const INfpvrInterface::Options& NfpvrwinInterface::getOptions()
{
	return _options;
}

void NfpvrwinInterface::networkNotifyHandlerCount(int handlerCount)
{
	_handlerCount = handlerCount;

	if (handlerCount && !_isRecording)
	{
		// start blinking
		SendMessage(_hwnd, WM_NFWIN_START_RECORD, 0, 0);
		_isRecording = true;
	}
	else if (!handlerCount && _isRecording)
	{
		// stop blinking
		SendMessage(_hwnd, WM_NFWIN_STOP_RECORD, 0, 0);
		_isRecording = false;
	}
}

void NfpvrwinInterface::networkListenComplete(nfpvr_socket_t theSocket)
{
}

int NfpvrwinInterface::networkReceive(nfpvr_socket_t theSocket, nfpvr_sockaddr_t* theSender, uint8* data, const int length)
{
	const int senderSize = sizeof(struct sockaddr_in);

	int received = 0;

	do
	{
		received = recvfrom(theSocket, (char*)(data), length, 0, 
			(nfpvr_sockaddr_t*)theSender, (nfpvr_socklen_t*)&senderSize);

	} while (received == NFPVR_SOCKET_ERROR);

	return received;
}

void NfpvrwinInterface::networkNotifyTotalReceived(uint64 bytes)
{
	_totalReceived = bytes;
}

void NfpvrwinInterface::showTooltip(const wchar_t* info)
{
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = _hwnd;
	nid.uID = 0;
	nid.uFlags = NIF_INFO;

	const wchar_t* title = L"nFusion PVR Recorder";
	wcscpy_s(nid.szInfoTitle, wcslen(title)+1, title);
	wcscpy_s(nid.szInfo, wcslen(info)+1, info);

	nid.dwInfoFlags = NIIF_NONE;
	nid.uVersion = NOTIFYICON_VERSION;

	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void NfpvrwinInterface::setOutputDirectory(const wchar_t* dir)
{
	wcscpy_s(_outputDirectoryWide, 1024, dir);
	wcstombs(_outputDirectory, _outputDirectoryWide, 1024);
}

const wchar_t* NfpvrwinInterface::getOutputDirectory()
{
	return _outputDirectoryWide;
}
