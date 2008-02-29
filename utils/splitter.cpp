#include "Bitfield.h"
#include "Serializable.h"
#include "MpegHeader.h"

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

using namespace nfpvr;

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


	char audioFilename[255];
	strcpy(audioFilename, filename);
	strcat(audioFilename, ".audio");
	FILE* audioFile = fopen(audioFilename, "wb");

	char videoFilename[255];
	strcpy(videoFilename, filename);
	strcat(videoFilename, ".video");
	FILE* videoFile = fopen(videoFilename, "wb");

	int offset=0;
	while (offset<size)
	{
		uint16 header = Bitfield::decode<uint16>(data+0);
		uint16 length = Bitfield::decode<uint16>(data+2);
		
		if (header == 0x3800)
		{
			fwrite(data+4, length, 1, videoFile);
		}
		else if (header == 0x3801)
		{
			fwrite(data+4, length, 1, audioFile);
		}

		if (length != 1024)
		{
			printf("length= %d at %d\n", length, offset);
		}

		data += length+4;
		offset += length+4;

	}

	fclose(audioFile);
	fclose(videoFile);
}
