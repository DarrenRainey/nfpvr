#include "Bitfield.h"
#include "Serializable.h"
#include "MpegHeader.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Buffer buffer(1024*512);

int main(int argc, char* argv[])
{
	if (argc<2)
		return -1;

	const char* filename = argv[1];
	int fid = open(filename, O_RDONLY);

	int size = (int)lseek(fid, 0, SEEK_END);
	lseek(fid, 0, SEEK_SET);

	printf("size is %d bytes\n", size);

	void* address = mmap(0, size, PROT_READ, MAP_PRIVATE, fid, 0);
	uint8* data = (uint8*) address;

	int _socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	printf("created socket\n");

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(50000);
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");

	int offset=0,
		total=0;

	while (offset<size)
	{
		uint16 header = Bitfield::decode<uint16>(data+0);
		uint16 length = Bitfield::decode<uint16>(data+2);

		int n = sendto(_socket, data, length+4, 0, (struct sockaddr*)&sin, sizeof(sin));
		total += n;
		printf("sent %d bytes, %d bytes total\n", n, total);

		//usleep(100);
		if (header == 0x3810)
		{
			char reply[255];
			sockaddr_in sender;
			int senderSize = sizeof(sender);
			int n = recvfrom(_socket, (char*)(reply), sizeof(reply), 0, (struct sockaddr*)&sender, &senderSize);
			printf("received %d bytes\n", n);
		}		

		data += length+4;
		offset += length+4;

	}
}
