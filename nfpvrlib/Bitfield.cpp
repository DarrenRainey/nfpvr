#include "Bitfield.h"
#include "NfpvrTypes.h"

// Bitfield
namespace nfpvr
{

Bitfield::Bitfield(uint8* buffer, int length):
	_buffer(buffer),
	_length(length)
{
}

void Bitfield::attach(uint8* buffer, int length)
{
	_buffer = buffer;
	_length = length;
	reset();
}

template <typename T>
T Bitfield::decode(const uint8* buffer)
{
	T result=0;

	for (unsigned int x=0; x<sizeof(T); x++)
	{
		result<<=8;
		result|=buffer[x];
	}

	return result;
}

template <typename T>
void Bitfield::encode(const T data, uint8* buffer)
{
	T toWrite = data;
	for (int x=sizeof(T)-1; x>=0; x--)
	{
		buffer[x]=static_cast<uint8>(toWrite);
		toWrite>>=8;
	}
}

// BitfieldReader
BitfieldReader::BitfieldReader(const uint8* data, int length):
	Bitfield(const_cast<uint8*>(data), length)
{
	reset();
}

void BitfieldReader::reset()
{
	_bitIndex=7;
	_byteIndex=0;
}

void BitfieldReader::skip(int bits)
{
	while (bits--)
	{
		_bitIndex--;
		if (_bitIndex<0)
		{
			_bitIndex=7;
			_byteIndex++;
		}
	}
}

template <typename T>
void BitfieldReader::read(T& data, int bits)
{
	for (int x=0; x<bits && _byteIndex<_length; x++)
	{
		data<<=1;

		if (_buffer[_byteIndex]&(1<<_bitIndex))
			data|=1;

		skip(1);
	}
}

uint8 BitfieldReader::read(int bits)
{
	uint8 result=0;
	read<uint8>(result, bits);
	return result;
}

// BitfieldWriter
BitfieldWriter::BitfieldWriter(uint8* data, int length):
	Bitfield(data, length)
{
	reset();
}

void BitfieldWriter::reset()
{
	_bitIndex=0;
	_byteIndex=_length-1;
}

void BitfieldWriter::skip(int bits)
{
	while (bits--)
	{
		_bitIndex++;

		if (_bitIndex>7)
		{
			_bitIndex=0;
			_byteIndex--;
		}
	}
}

void BitfieldWriter::write(const uint8 data, int bits)
{
	uint8 toWrite = data;
	write<uint8>(toWrite, bits);
}

template <typename T>
void BitfieldWriter::write(T& data, int bits)
{
	for (int x=0; x<bits && _byteIndex<_length; x++)
	{
		if (data&1)
		{
			_buffer[_byteIndex]|=(1<<_bitIndex);
		}
		else
		{
			_buffer[_byteIndex]&=~(1<<_bitIndex);
		}

		data>>=1;
		skip(1);
	}
}

template uint16 Bitfield::decode<uint16>(const uint8* buffer);
template void   Bitfield::encode<uint16>(const uint16 data, uint8* buffer);

template void BitfieldReader::read<uint64>(uint64& data, int bits);
template void BitfieldReader::read<uint32>(uint32& data, int bits);

template void BitfieldWriter::write<uint64>(uint64& data, int bits);
template void BitfieldWriter::write<uint32>(uint32& data, int bits);
template void BitfieldWriter::write<uint16>(uint16& data, int bits);

}