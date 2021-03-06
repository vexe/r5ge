#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Tileable noise generation class
// Author: Michael Lyashenko
//============================================================================================================

class Noise
{
public:

	#include "Parameters.h"

private:

	struct AppliedFilter
	{
		String		mName;		// Name of the filter being applied
		Parameters	mParams;	// Parameters that will be passed to the callback function
	};

	typedef Array<AppliedFilter>	AppliedFilters;

public:

	// Filter applying function passes the primary as well as secondary buffer data,
	// size of the noise, as well as the parameter set
	typedef float* FloatPtr;
	typedef FastDelegate<void (	Random&				rand,
								FloatPtr&			data,
								FloatPtr&			aux,
								const Vector2i&		size,
								const Parameters&	param,
								bool				seamless )>	OnApplyFilterDelegate;
	
private:

	AppliedFilters	mFilters;		// Filter applied to this particular noise
	uint			mSeed;			// Seed used to start the random number generation
	Random			mRand;			// Random number generator used for this noise
	Vector2i		mSize;			// Size of the noise (width and height)
	float*			mData;			// Primary buffer -- should always have the latest data
	float*			mAux;			// Secondary buffer -- should be used primarily for writing before swapping it for 'mData'
	float*			mTemp;			// Temporary buffer used to downsample/upsample the noise if GetBuffer() function requested a different size
	Vector2i		mTempSize;		// Size of the temporary buffer
	uint			mBufferSize;	// Current allocated buffer size (width*height)
	bool			mSeamless;		// Whether the noise should be seamless or cut at the edges
	bool			mIsDirty;		// Whether the noise needs to be regenerated

public:

	// STATIC: Registeres a new filter
	static void RegisterFilter (const String& name, const OnApplyFilterDelegate& fnct);

	// STATIC: Retrieves the names of all registered filters
	static void GetRegisteredFilters (Array<String>& list);

public:

	Noise();
	Noise(uint seed);
	~Noise();

public:

	// Release the allocated buffers
	void Release (bool clearFilters = true);

	Random& GetRandom()				{ return mRand; }
	uint	GetSeed()		const	{ return mSeed; }
	uint	GetWidth()		const	{ return (uint)mSize.x; }
	uint	GetHeight()		const	{ return (uint)mSize.y; }
	bool	IsSeamless()	const	{ return mSeamless; }

	const Vector2i&	GetSize() const { return mSize; }

	void SetSeed(uint seed)				{ if (mSeed != seed) { mSeed = seed; mIsDirty = true; } }
	void SetSeamless (bool val)			{ mSeamless = val; }
	void SetSize (int x, int y)			{ mSize.Set((short)x, (short)y); mIsDirty = true; }
	void SetSize (const Vector2i& size)	{ mSize = size; mIsDirty = true; }

	// Applies a new filter to the noise
	Parameters& ApplyFilter (const String& filterName);

	// Returns a pointer to the noise buffer (blocks until the noise is generated)
	float* GetBuffer (const Vector2i& size = 0);
	
public:

	// Serialization
	virtual bool SerializeFrom (const TreeNode& root);
	virtual bool SerializeTo (TreeNode& root) const;
};