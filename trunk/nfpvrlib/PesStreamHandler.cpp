#include "PesStreamHandler.h"
#include "Utility.h"

namespace nfpvr
{

int PesStreamHandler::MAXIMUM_PES_SIZE = 60*1024;

PesStreamHandler::PesStreamHandler(uint8 streamId, int bufferSize):
	_streamId(streamId),
	_buffer(bufferSize)
{
	_state = StateFoundNothing;
	_lastFindOffset = 0;
}

PesStreamHandler::~PesStreamHandler()
{
}

void PesStreamHandler::reset()
{
	_state = StateFoundNothing;
	_lastFindOffset = 0;
	_buffer.reset();
}

int PesStreamHandler::write(const uint8* data, int length)
{
	return _buffer.write(data, length);
}

const Buffer& PesStreamHandler::getBuffer()
{
	return _buffer;
}

int PesStreamHandler::writeLeft()
{
	return _buffer.getLeft();
}

bool PesStreamHandler::isPesFound()
{
	return (_state == StateFoundNextHeader);
}

bool PesStreamHandler::isPesHeaderFound()
{
	return isPesFound() || (_state == StateFoundCurrentHeader);
}

void PesStreamHandler::getCurrentHeader(MpegHeader::PesInfo& pes)
{
	pes = _pesInfoCurrent;
}

void PesStreamHandler::getNextHeader(MpegHeader::PesInfo& pes)
{
	pes = _pesInfoNext;
}

int  PesStreamHandler::discardPes()
{
	int discarded = 0;
	if (_state == StateFoundNothing)
	{
		// this should not really happen I guess
		discarded=discard(_lastFindOffset);
	}
	else if (_state == StateFoundCurrentHeader)
	{
		// discard everything up to what we've searched for
		discarded=discard(_lastFindOffset);
		_state = StateFoundNothing;
	}
	else if (_state == StateFoundNextHeader)
	{
		// just discard the Current pes header + data
		discarded=discard(_pesInfoNext.headerOffset);
		_pesInfoCurrent = _pesInfoNext;
		_state = StateFoundCurrentHeader;
	}
	return discarded;
}

bool PesStreamHandler::findPesHeader()
{
	bool somethingNew = false;
	bool needMoreData = false;

	while (!isPesFound() && !needMoreData)
	{
		// startcode+streamid (4), pes packet length(2), extension(3), pts(5)
		const int   minimumLength = 4 + 2 + 3 + 5; 
		const uint8 pesPattern[4] = {0x00, 0x00, 0x01, _streamId};

		int offset = _buffer.find(pesPattern, sizeof(pesPattern), minimumLength, &_lastFindOffset);

		if (offset != -1)
		{
			// get a pointer to the found header
			int length=0;
			uint8* data = _buffer.get(offset, length);

			// try to read it
			MpegHeader::PesInfo newPesInfo;
			newPesInfo.headerOffset = offset;

			MpegHeader::MpegHeaderStatus status = 
				MpegHeader::readPesHeader(data, length, newPesInfo);

			if (newPesInfo.streamId != _streamId)
			{
				//Logger::log(Normal, Warning, "invalid pes stream id found in header");
			}

			if (status == MpegHeader::StatusIncomplete)
			{
				needMoreData = true;
			}
			else if (status == MpegHeader::StatusError)
			{
				_state = StateFoundNothing;
				_lastFindOffset ++;

				//Logger::log(Normal, Warning, "error reading mpeg header");

				discard(_lastFindOffset);
			}
			else if (status == MpegHeader::StatusOk)
			{
				if (_state == StateFoundNothing)
				{
					_state = StateFoundCurrentHeader;
					_pesInfoCurrent = newPesInfo;
				}

				else if (_state == StateFoundCurrentHeader)
				{
					_state = StateFoundNextHeader;
					_pesInfoNext = newPesInfo;
				}
				
				somethingNew = true;
				_lastFindOffset += newPesInfo.headerLength;
			}
		}
		else
		{
			needMoreData = true;
		}
	}

	return somethingNew;
}

void PesStreamHandler::writePes(Serializable& dst, uint64 pts)
{
	if (isPesFound())
	{
		uint64 currentPesPts         = pts;
		int    currentPesLength      = _pesInfoNext.headerOffset - (_pesInfoCurrent.headerOffset + _pesInfoCurrent.headerLength);
		const uint8* currentPesData  = (const uint8*)_buffer + _pesInfoCurrent.headerOffset + _pesInfoCurrent.headerLength;

		int pesCount = 0;
		while (currentPesLength>0)
		{
			bool hasPts = (pesCount == 0);
			int  toWrite = MIN(MAXIMUM_PES_SIZE, currentPesLength);

			MpegHeader::writePesHeader(dst, _streamId, toWrite, hasPts, currentPesPts);
			dst.write(currentPesData, toWrite);

			currentPesData   += toWrite;
			currentPesLength -= toWrite;

			pesCount++;
		}

		
		_pesInfoCurrent = _pesInfoNext;
		_state = StateFoundCurrentHeader;

		//int nextPesDataOffset = _pesInfoNext.headerOffset + _pesInfoNext.headerLength;
		//discard(nextPesDataOffset); let's not be too agressive...
		discard(_pesInfoNext.headerOffset);
	}
}

int PesStreamHandler::discard(int size)
{
	int discarded = _buffer.discard(size);
	
	_pesInfoCurrent.headerOffset -= discarded;
	_pesInfoNext.headerOffset -= discarded;
	_lastFindOffset -= discarded;

	return discarded;
}

}