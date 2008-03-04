#include "FilePacketReader.h"
#include "Bitfield.h"
#include <stdio.h>

namespace nfpvr
{

FilePacketReader::FilePacketReader(INfpvrInterface& nfpvrInterface, const char* filename):
	_filename(filename),
	_nfpvrInterface(nfpvrInterface),
	_packetHandler(_nfpvrInterface, _filename)
{
}

void FilePacketReader::read()
{
	readFromFile(_filename);
}

void FilePacketReader::readFromFile(const char* filename)
{
	FILE* file;
	file = fopen(filename, "rb");

	if (file)
	{
		while (!feof(file))
		{
			uint16 dataLength = 0;
			uint8 buffer[1024*4];

			// read header
			if (fread(buffer, 4, 1, file))
			{
				/*header = Bitfield::decode<uint16>(buffer+0);
				
				if (header == 0x381a)
				{
					Bitfield::encode<uint16>(0x3801, buffer+0);
					Bitfield::encode<uint16>(0x0400, buffer+2);
				}*/

				// decode length
				dataLength = Bitfield::decode<uint16>(buffer+2);

				// read data
				if (dataLength)
					fread(buffer+4, dataLength, 1, file);
			}
			
			// handle the packet
			_packetHandler.receive(buffer, dataLength+4);
		}

		fclose(file);
	}
}

}
