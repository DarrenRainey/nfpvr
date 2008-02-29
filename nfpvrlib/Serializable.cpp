#include "Serializable.h"
#include "Utility.h"
#include <stdio.h>
#include <memory.h>
#include <string.h>

namespace nfpvr
{

File::File(FILE* file):
	_file(file)
{
}

int File::write(const uint8* data, int length)
{
	if (isOpened())
		return static_cast<int>(fwrite(data, 1, length, _file));
	else
		return 0;
}

bool File::open(const char* filename)
{
	_file = fopen(filename, "wb");
	return isOpened();
}

bool File::isOpened() const
{
	return _file != 0;
}

void File::close()
{
	if (isOpened())
	{
		fclose(_file);
		_file = 0;
	}
}

Buffer::Buffer(const int length):
	_data(0),
	_length(length),
	_index(0)
{
	_data = new uint8[_length];
}

Buffer::~Buffer()
{
	delete[] _data;
}

float Buffer::percentUsed() const
{
	return static_cast<float>(_index)/_length*100;
}

int Buffer::writeTo(Serializable& dst)
{
	if (_index)
		return dst.write(_data, _index);
	
	return 0;
}

int Buffer::write(const uint8* data, int length)
{
	int left = _length - _index;
	if (length>left)
	{
		printf("warning overflow");
	}
	length = MIN(left, length);
	
	memcpy(_data+_index, data, length);
	_index += length;

	return length;
}

int Buffer::find(const uint8* pattern, int patternLength, int minimumLength, int* lastOffset)
{
	int offset = 0;
	
	if (lastOffset)
		offset = *lastOffset;

	for (; offset<(_index-minimumLength); offset++)
	{
		if (!memcmp(&_data[offset], pattern, patternLength))
		{
			if (lastOffset) *lastOffset = offset;
			return offset;
		}
	}
	
	if (lastOffset) *lastOffset = offset;
	return -1;
}

Buffer::operator const uint8* ()
{
	return _data;
}

uint8* Buffer::get(int offset, int& length)
{
	if (offset<_index)
	{
		length = _index-offset;
		return _data+offset;
	}
	else
	{
		length=0;
		return 0;
	}
}

int Buffer::discard(int size)
{
	size = MIN(size, _index);
	int toMove = _index - size;

	if (toMove>0)
		memmove(_data, _data + size, toMove);

	_index -= size;

	return size;
}

void Buffer::reset()
{
	_index = 0;
}

FileBuffered::FileBuffered(int length):
	_buffer(length)
{
}

bool FileBuffered::open(const char* filename)
{
	_buffer.reset();
	return File::open(filename);
}

int FileBuffered::write(const uint8* data, int length)
{
	if (_buffer.getLeft()<length) flush();
	return _buffer.write(data, length);
}

void FileBuffered::close()
{
	flush();
	File::close();
}

void FileBuffered::flush()
{
	int length = 0;
	uint8* data = _buffer.get(0, length);

	File::write(data, length);
	_buffer.reset();
}
}