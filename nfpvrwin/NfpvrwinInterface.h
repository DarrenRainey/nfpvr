#ifndef __nfpvrwininterface_h__
#define __nfpvrwininterface_h__

#include <windows.h>
#include "NfpvrLib.h"

class NfpvrwinInterface: public INfpvrInterfaceNetwork
{
public:
	NfpvrwinInterface();

	virtual void           notify(NotifyLevel level, const char* format, ...);
	virtual const Options& getOptions();

	virtual void networkNotifyHandlerCount(int handlerCount);
	virtual void networkNotifyTotalReceived(uint64 bytes);
	virtual void networkListenComplete(nfpvr_socket_t theSocket);
	virtual int  networkReceive(nfpvr_socket_t theSocket, nfpvr_sockaddr_t* theSender, uint8* data, const int length);

	void           setOutputDirectory(const wchar_t* dir);
	const wchar_t* getOutputDirectory();

	void   setWindow(HWND hWnd) { _hwnd = hWnd; }
	uint64 getTotalReceived() { return _totalReceived; }
	int    getHandlerCount()  { return _handlerCount; }

protected:
	void showTooltip(const wchar_t* info);
	
	Options _options;
	HWND    _hwnd;
	bool    _isRecording;
	
	uint64  _totalReceived;
	int     _handlerCount;
	wchar_t _outputDirectoryWide[1024];
	char    _outputDirectory[1024];
};

#endif
