#include "Serializable.h"
#include "MpegHeader.h"

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>

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

	int offset=0;
	if (argc>=3)
	{
		offset = atol(argv[2]);
	}

	while (offset<size)
	{
		uint64 scr;
		uint32 mux_rate;

		int length = size-offset;
		
		int n=0;
		if ((n=MpegHeader::readPackHeader(data+offset, length, mux_rate, scr)) != 0)
		{
			printf("%9d: read pack header, mux_rate %d, scr %lld\n", offset, mux_rate, scr);
			offset += n;
		}
		else
		{
			MpegHeader::PesInfo pesInfo;

			MpegHeader::MpegHeaderStatus status = 
				MpegHeader::readPesHeader(data+offset, length, pesInfo);

			if (status == MpegHeader::StatusOk)
			{
				printf("%9d: read pes header packetLength %d, streamId 0x%2x", offset, pesInfo.packetLength, pesInfo.streamId);
				if (pesInfo.hasPts) printf(", pts %lld", pesInfo.pts);
				printf("\n");

				offset += 4+2;
				offset += pesInfo.packetLength;
			}
			else
			{
				printf("%9d: error reading pes header\n", offset);
			}
		}
	}
}
