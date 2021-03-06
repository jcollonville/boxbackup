// --------------------------------------------------------------------------
//
// File
//		Name:    Archive.h
//		Purpose: Backup daemon state archive
//		Created: 2005/04/11
//
// --------------------------------------------------------------------------

#ifndef ARCHIVE__H
#define ARCHIVE__H

#include <vector>
#include <string>
#include <memory>

#include "IOStream.h"
#include "Guards.h"

#define ARCHIVE_GET_SIZE(hdr)		(( ((uint8_t)((hdr)[0])) | ( ((uint8_t)((hdr)[1])) << 8)) >> 2)

#define ARCHIVE_MAGIC_VALUE_RECURSE 0x4449525F
#define ARCHIVE_MAGIC_VALUE_NOOP 0x5449525F

class Archive
{
public:
	Archive(IOStream &Stream, int Timeout)
	: mrStream(Stream)
	{
		mTimeout = Timeout;
	}
private:
	// no copying
	Archive(const Archive &);
	Archive & operator=(const Archive &);
public:
	~Archive()
	{
	}

	//
	//
	//
	void Write(bool Item)
	{
		Write((int) Item);
	}
	void WriteExact(uint32_t Item) { Write((int)Item); }
	// TODO FIXME: use of "int" here is dangerous and deprecated. It can lead to
	// incompatible serialisation on non-32-bit machines. Passing anything other
	// than one of the specifically supported fixed size types should be forbidden.
	void Write(int Item)
	{
		int32_t privItem = htonl(Item);
		mrStream.Write(&privItem, sizeof(privItem), mTimeout);
	}
	void Write(int64_t Item)
	{
		int64_t privItem = box_hton64(Item);
		mrStream.Write(&privItem, sizeof(privItem), mTimeout);
	}
	void WriteInt16(uint16_t Item)
	{
		uint16_t privItem = htons(Item);
		mrStream.Write(&privItem, sizeof(privItem), mTimeout);
	}
	void WriteExact(uint64_t Item) { Write(Item); }
	void Write(uint64_t Item)
	{
		uint64_t privItem = box_hton64(Item);
		mrStream.Write(&privItem, sizeof(privItem), mTimeout);
	}
	void Write(uint8_t Item)
	{
		int privItem = Item;
		Write(privItem);
	}
	void Write(const std::string &Item)
	{
		int size = Item.size();
		Write(size);
		mrStream.Write(Item.c_str(), size, mTimeout);
	}
	//
	//
	//
	void Read(bool &rItemOut)
	{
		int privItem;
		Read(privItem);

		if(privItem)
		{
			rItemOut = true;
		}
		else
		{
			rItemOut = false;
		}
	}
	void ReadIfPresent(bool &rItemOut, bool ValueIfNotPresent)
	{
		int privItem;
		ReadIfPresent(privItem, ValueIfNotPresent ? 1 : 0);
		rItemOut = privItem ? true : false;
	}
	void ReadExact(uint32_t &rItemOut) { Read((int&)rItemOut); }
	void Read(int &rItemOut)
	{
		int32_t privItem;
		if(!mrStream.ReadFullBuffer(&privItem, sizeof(privItem),
			0 /* not interested in bytes read if this fails */,
			mTimeout))
		{
			THROW_EXCEPTION(CommonException, ArchiveBlockIncompleteRead);
		}
		rItemOut = ntohl(privItem);
	}
	void ReadFullBuffer(void* Buffer, size_t Size)
	{
		if(!mrStream.ReadFullBuffer(Buffer, Size,
			0 /* not interested in bytes read if this fails */,
			mTimeout))
		{
			THROW_EXCEPTION(CommonException, ArchiveBlockIncompleteRead);
		}
	}
	void ReadIfPresent(int &rItemOut, int ValueIfNotPresent)
	{
		int32_t privItem;
		int bytesRead;
		if(mrStream.ReadFullBuffer(&privItem, sizeof(privItem),
			&bytesRead, mTimeout))
		{
			rItemOut = ntohl(privItem);
		}
		else if(bytesRead == 0)
		{
			// item is simply not present
			rItemOut = ValueIfNotPresent;
		}
		else
		{
			// bad number of remaining bytes
			THROW_EXCEPTION(CommonException, ArchiveBlockIncompleteRead);
		}
	}
	void Read(int64_t &rItemOut)
	{
		int64_t privItem;
		ReadFullBuffer(&privItem, sizeof(privItem));
		rItemOut = box_ntoh64(privItem);
	}
	void ReadExact(uint64_t &rItemOut) { Read(rItemOut); }
	void Read(uint64_t &rItemOut)
	{
		uint64_t privItem;
		ReadFullBuffer(&privItem, sizeof(privItem));
		rItemOut = box_ntoh64(privItem);
	}
	void ReadInt16(uint16_t &rItemOut)
	{
		uint16_t privItem;
		ReadFullBuffer(&privItem, sizeof(privItem));
		rItemOut = ntohs(privItem);
	}
	void Read(uint8_t &rItemOut)
	{
		int privItem;
		Read(privItem);
		rItemOut = privItem;
	}
	void ReadIfPresent(std::string &rItemOut, const std::string& ValueIfNotPresent)
	{
		ReadString(rItemOut, &ValueIfNotPresent);
	}
	void Read(std::string &rItemOut)
	{
		ReadString(rItemOut, NULL);
	}
private:
	void ReadString(std::string &rItemOut, const std::string* pValueIfNotPresent)
	{
		int size;
		int bytesRead;
		if(!mrStream.ReadFullBuffer(&size, sizeof(size), &bytesRead, mTimeout))
		{
			if(bytesRead == 0 && pValueIfNotPresent != NULL)
			{
				// item is simply not present
				rItemOut = *pValueIfNotPresent;
				return;
			}
			else
			{
				// bad number of remaining bytes
				THROW_EXCEPTION(CommonException,
					ArchiveBlockIncompleteRead)
			}
		}
		size = ntohl(size);

		// Assume most strings are relatively small
		char buf[256];
		if(size < (int) sizeof(buf))
		{
			// Fetch rest of pPayload, relying on the Protocol to error on stupidly large sizes for us
			ReadFullBuffer(buf, size);
			// assign to this string, storing the header and the extra payload
			rItemOut.assign(buf, size);
		}
		else
		{
			// Block of memory to hold it
			MemoryBlockGuard<char*> dataB(size);
			char *ppayload = dataB;

			// Fetch rest of pPayload, relying on the Protocol to error on stupidly large sizes for us
			ReadFullBuffer(ppayload, size);
			// assign to this string, storing the header and the extra pPayload
			rItemOut.assign(ppayload, size);
		}
	}
private:
	IOStream &mrStream;
	int mTimeout;
};

#endif // ARCHIVE__H
