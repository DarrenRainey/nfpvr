#ifndef __bitfield_h__
#define __bitfield_h__

#include "NfpvrTypes.h"

namespace nfpvr
{

class Bitfield
{
public:
	Bitfield(uint8* buffer, int length);

	void attach(uint8* buffer, int length);

	template <typename T>
	static T decode(const uint8* buffer);

	template <typename T>
	static void encode(const T data, uint8* buffer);

	virtual void skip(int bits)=0;

	virtual void reset()=0;

protected:
	uint8*    _buffer;
	int       _length;

	int   _bitIndex;
	int   _byteIndex;
};


class BitfieldWriter: public Bitfield
{
public:
	BitfieldWriter(uint8* data, int length);

	template <typename T>
	void write(T& data, int bits);

	void write(const uint8, int bits);

	void skip(int bits);

	void reset();
};

class BitfieldReader: public Bitfield
{
public:
	BitfieldReader(const uint8* data, int length);

	template <typename T>
	void read(T& data, int bits);

	uint8 read(int bits);

	void skip(int bits);

	void reset();
};

}

#endif
