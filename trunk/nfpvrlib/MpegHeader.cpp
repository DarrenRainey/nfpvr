#include "MpegHeader.h"
#include "Bitfield.h"

void MpegHeader::writePackHeader(Serializable& dst, uint32 mux_rate, uint64 scr)
{
	uint16 scr_ext = 0;
	uint8 packHeader[14];

	// write start code
	packHeader[0] = 0x00;
	packHeader[1] = 0x00;
	packHeader[2] = 0x01;
	packHeader[3] = 0xba;

	// write scr
	BitfieldWriter bits(&packHeader[4], 6);
	bits.write(1, 1);
	bits.write(scr_ext , 9);
	bits.write(1, 1);
	bits.write(scr, 15);
	bits.write(1, 1);
	bits.write(scr, 15);
	bits.write(1, 1);
	bits.write(scr, 3);
	bits.write(0x01, 2);

	bits.attach(&packHeader[10], 4);
	bits.write(0, 3); // pack_stuffing_length
	bits.write(0, 5); // reserved
	bits.write(0x03, 2);
	bits.write(mux_rate, 22);

	dst.write(packHeader, 14);
}

void MpegHeader::writeProgramEndHeader(Serializable& dst)
{
	uint8 programEndHeader[4];

	programEndHeader[0] = 0x00;
	programEndHeader[1] = 0x00;
	programEndHeader[2] = 0x01;
	programEndHeader[3] = 0xb9;

	dst.write(programEndHeader, 4);
}

void MpegHeader::writePesHeader(Serializable& dst, uint8 streamId, uint16 dataLength, bool hasPts, uint64 pts)
{
	uint8 pesHeader[14];
	pesHeader[0] = 0x00;
	pesHeader[1] = 0x00;
	pesHeader[2] = 0x01;
	pesHeader[3] = streamId;

	uint16 pesLength = dataLength + 3 + ((hasPts)?(5):(0));
	Bitfield::encode(pesLength, &pesHeader[4]);

	pesHeader[6] = 0x80;
	pesHeader[7] = (hasPts)?(0x80):(0x00);
	pesHeader[8] = (hasPts)?(5):(0);

	if (hasPts)
	{
		BitfieldWriter bits(&pesHeader[9], 5);
		bits.write(1, 1);
		bits.write(pts, 15);
		bits.write(1, 1);
		bits.write(pts, 15);
		bits.write(1, 1);
		bits.write(pts, 3);
		bits.write(0x02, 4);
	}

	dst.write(pesHeader, (hasPts)?(14):(9));
}

int MpegHeader::readPackHeader(const uint8* data, int length, uint32& mux_rate, uint64& scr)
{
	if (!(data[0] == 0x00 &&
		  data[1] == 0x00 &&
		  data[2] == 0x01 &&
		  data[3] == 0xba))
	{
		return 0;
	}

	scr = 0;
	mux_rate = 0;

	BitfieldReader bits(&data[4], 10);
	bits.skip(2);
	bits.read(scr, 3);
	bits.skip(1);
	bits.read(scr, 15);
	bits.skip(1);
	bits.read(scr, 15);

	bits.skip(11);

	bits.read(mux_rate, 22);

	return 14;
}

enum MpegHeader::MpegHeaderStatus 
	MpegHeader::readPesHeader(const uint8* data, int length, PesInfo& pesInfo)
{
	int headerLength = 9;
	
	if (length<headerLength)
		return StatusIncomplete;

	pesInfo.streamId = data[3];

	// get packet length
	pesInfo.packetLength = Bitfield::decode<uint16>(data+4);

	// do we have a PTS and/or DTS?
	pesInfo.hasPts = (data[7]&(1<<7))?true:false;
	pesInfo.hasDts = (data[7]&(1<<6))?true:false;

	int pesHeaderDataLength = data[8];
	headerLength += pesHeaderDataLength;

	if (length<headerLength)
		return StatusIncomplete;

	pesInfo.pts = 0;
	pesInfo.dts = 0;

	if (pesInfo.hasPts)
	{
		BitfieldReader bits(&data[9+0], 5);
		bits.skip(4);
		bits.read(pesInfo.pts, 3);
		bits.skip(1);
		bits.read(pesInfo.pts, 15);
		bits.skip(1);
		bits.read(pesInfo.pts, 15);
	}

	if (pesInfo.hasDts)
	{
		BitfieldReader bits(&data[9+5], 5);
		bits.skip(4);
		bits.read(pesInfo.dts, 3);
		bits.skip(1);
		bits.read(pesInfo.dts, 15);
		bits.skip(1);
		bits.read(pesInfo.dts, 15);
	}

	pesInfo.headerLength = headerLength;

/*	printf("streamid %02x, bytes 6 7 8, %02x %02x %02x pl=%d hl=%d\n",
		data[3], data[6], data[7], data[8],
		pesInfo.packetLength, pesHeaderDataLength);
*/
	return StatusOk;
}
