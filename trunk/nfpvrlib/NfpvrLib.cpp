#include "NfpvrLib.h"
#include "FilePacketReader.h"
#include "UdpPacketReader.h"

namespace nfpvr
{

int NfpvrLibReadFromFile(INfpvrInterface& iface, const char* filename)
{
	FilePacketReader reader(iface, filename);
	reader.read();
	return 0;
}

int NfpvrLibReadFromUdp(INfpvrInterfaceNetwork& iface, int udpPort)
{
	UdpPacketReader handler(iface, udpPort);
	handler.read();
	return 0;
}
}
