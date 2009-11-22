#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Basic memory buffer
//============================================================================================================

class Memory
{
protected:

	byte*				mBuffer;	// Allocated memory buffer
	uint				mAllocated;	// Actual amount of memory allocated
	uint				mSize;		// Memory that's actually "used" by the buffer
	Thread::Lockable	mLock;

public:

	Memory() : mBuffer(0), mAllocated(0), mSize(0) {}
	Memory(uint size) : mBuffer(0), mAllocated(0), mSize(0) { Reserve(size); mSize = 0; }
	~Memory() { if (mBuffer != 0) delete [] mBuffer; }

public:

	// Copying memory buffers doesn't actually copy -- it moves the memory
	Memory (const Memory& in);
	void operator = (const Memory& in);

public:

	void		Lock()		const		{ mLock.Lock();		}
	void		Unlock()	const		{ mLock.Unlock();	}
	byte*		GetBuffer()				{ return mBuffer;	}
	const byte*	GetBuffer() const		{ return mBuffer;	}
	uint		GetSize()	const		{ return mSize;		}
	void		Clear()					{ mSize = 0;		}

	// Always useful to have
	void MemSet (int val) const { if (mBuffer != 0) memset(mBuffer, val, mAllocated); }

	// Release the memory buffer
	void Release();

	// Reserve the specified amount of memory, keeping the data in place
	byte* Reserve (uint size);

	// Expand the buffer by the specified maount of bytes and return a pointer to that location
	byte* Expand (uint bytes);

	// Remove the specified number of bytes from the front of the buffer
	void Remove (uint size);

	// Append types that can't be templated
	void Append (int val)						{ *(int32*)Expand(4)  = (int32)val; }
	void Append (uint val)						{ *(uint32*)Expand(4) = (uint32)val; }
	void Append (const char* s);
	void Append (const String& s);
	void Append (const void* buffer, uint size) { if (size > 0) { memcpy(Expand(size), buffer, size); } }

	// Appends an integer as either 1 or 5 bytes, depending on its own size
	void AppendSize (uint val);

	// Extract types that can't be templated
	static bool Extract (ConstBytePtr& buffer, uint& size, int& val);
	static bool Extract (ConstBytePtr& buffer, uint& size, uint& val);
	static bool Extract (ConstBytePtr& buffer, uint& size, String& val);
	static bool Extract (ConstBytePtr& buffer, uint& size, void* data, uint bytes);

	// Extracts a 1 to 5 byte integer encoded with AppendSize()
	static bool ExtractSize (ConstBytePtr& buffer, uint& size, uint& val);

	// Templated Append function for all other types not covered by the functions above
	template <typename Type> void Append (const Type& v) { *(Type*)Expand(sizeof(Type)) = v; }

	// Templated function to do the reverse of the Append operation above
	template <typename Type> static bool Extract (ConstBytePtr& buffer, uint& size, Type& val)
	{
		uint needed = sizeof(Type);
		if (size < needed) return false;
		val = *(Type*)buffer;
		buffer += needed;
		size -= needed;
		return true;
	}

public:

	// Load the specified file fully into memory
	bool Load (const char* filename);

	// Dump the current buffer into the file
	bool Save (const char* filename);
};