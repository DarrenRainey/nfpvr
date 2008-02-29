#ifndef __buffer_h__
#define __buffer_h__

#include <stdio.h>
#include "NfpvrTypes.h"

namespace nfpvr
{

class Serializable
{
public:
	virtual int write(const uint8* data, int length)=0;
	virtual void flush() {};
	virtual ~Serializable() {};
};

class File: public Serializable
{
public:
	File(FILE* file=0);
	virtual int write(const uint8* data, int length);
	virtual void close();
	virtual bool open(const char* filename);

	bool isOpened() const;

private:
	FILE* _file;
};

class Buffer: public Serializable
{
public:
	Buffer(const int length);

	~Buffer();

	int writeTo(Serializable& dst);
	int write(const uint8* data, int length);
	int find(const uint8* pattern, int patternLength, int minimumLength, int* lastOffset=0);

	operator const uint8* ();

	uint8* get(int offset, int& length);

	void   reset();
	int   discard(int size);

	inline int   getLength() const { return _length; };
	inline int   getUsed() const   { return _index; };
	inline int   getLeft() const   { return _length-_index; };
	float percentUsed() const;

private:
	uint8 * _data;
	int    _length;
	int    _index;
};

class FileBuffered: public File
{
public:
	FileBuffered(int length);
	
	int write(const uint8* data, int length);
	void close();
	bool open(const char* filename);
	
private:
	void flush();

	Buffer _buffer;
};

}
#endif
