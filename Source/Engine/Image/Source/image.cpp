#include "../Include/_All.h"
#include "../Include/_Codecs.h"

//============================================================================================================
// List of registered codecs is global for easy access across this file
//============================================================================================================

namespace R5
{
namespace Codec
{
struct RegisteredCodec
{
	String				mName;		// Name of the registered codec
	Image::ReadDelegate	mDelegate;	// Delegate function that decodes the buffer using this codec
};

Array<RegisteredCodec> g_allCodecs;

//============================================================================================================
// Registers a codec (used in the _RegisterAll() function below)
//============================================================================================================

inline void _Register (const String& name, const Image::ReadDelegate& fnct)
{
	RegisteredCodec& codec = g_allCodecs.Expand();
	codec.mName = name;
	codec.mDelegate = fnct;
}

//============================================================================================================
// Register all common codecs if they haven't been already
//============================================================================================================

void _RegisterAll()
{
	static bool doOnce = true;

	if (doOnce)
	{
		doOnce = false;
		_Register("JPG", &JPG);
		_Register("PNG", &PNG);
		_Register("HDR", &HDR);
	}
}
}; // namespace Codec
}; // namespace R5

using namespace R5;
using namespace Codec;

//============================================================================================================
// Retrieves a registered codec callback
//============================================================================================================

Image::ReadDelegate _GetCodec (const String& name)
{
	Image::ReadDelegate fnct;

	g_allCodecs.Lock();
	{
		_RegisterAll();

		for (uint b = 0; b < g_allCodecs.GetSize(); ++b)
		{
			RegisteredCodec& rf = g_allCodecs[b];

			// Match the names
			if (rf.mName == name)
			{
				fnct = rf.mDelegate;
				break;
			}
		}
	}
	g_allCodecs.Unlock();
	return fnct;
}

//============================================================================================================
// STATIC: Registers a new filter
//============================================================================================================

void Image::RegisterCodec (const String& name, const Image::ReadDelegate& fnct)
{
	g_allCodecs.Lock();
	{
		_RegisterAll();

		for (uint i = 0; i < g_allCodecs.GetSize(); ++i)
		{
			if (g_allCodecs[i].mName == name)
			{
				g_allCodecs[i].mDelegate = fnct;
				g_allCodecs.Unlock();
				return;
			}
		}
		RegisteredCodec& codec = g_allCodecs.Expand();
		codec.mName = name;
		codec.mDelegate = fnct;
	}
	g_allCodecs.Unlock();
}

//============================================================================================================
// STATIC: Retrieves the list of all registered codecs
//============================================================================================================

void Image::GetRegisteredCodecs (Array<String>& list)
{
	list.Clear();

	g_allCodecs.Lock();
	{
		_RegisterAll();

		for (uint i = 0; i < g_allCodecs.GetSize(); ++i)
		{
			RegisteredCodec& rf = g_allCodecs[i];

			if (rf.mName.IsValid() && rf.mDelegate != 0)
			{
				list.Expand() = rf.mName;
			}
		}
	}
	g_allCodecs.Unlock();
}

//============================================================================================================
// STATIC: Creates a normal map for the specified heightmap
//============================================================================================================

void Image::HeightMapToNormalMap (	const float*		buffer,
									uint				width,
									uint				height,
									Array<Color4ub>&	c,
									bool				seamless,
									const Vector3f&		scale )
{
	uint size = width * height;
	c.Clear();
	c.Reserve(size);

	// The "magic number" 0.5 * ((1.4142 * 4) + 8) = 6.8284.
	// There are 4 coordinates that are worth the most but get considered only once,
	// and there are 4 coordinates that get considered twice (vertical+horizontal), but are worth less.

	float fracX = scale.x / width;
	float fracY = scale.y / height;
	float fracZ = (fracX * fracY * 6.8284f) / scale.z;

	fracX /= fracZ;
	fracY /= fracZ;

	int y0w, y1w, y2w, x0, x1, x2;
	float myHeight, left, right, bottom, top;
	float temp[8];

	for (uint y = 0; y < height; ++y)
	{
		y1w = width * y;

		if (seamless)
		{
			y0w = width * Wrap(y - 1, height);
			y2w = width * Wrap(y + 1, height);
		}
		else
		{
			y0w = width * Clamp(y - 1, height);
			y2w = width * Clamp(y + 1, height);
		}

		for (uint x = 0; x < width; ++x)
		{
			x1 = x;

			if (seamless)
			{
				x0 = Wrap(x - 1, width);
				x2 = Wrap(x + 1, width);
			}
			else
			{
				x0 = Clamp(x - 1, width);
				x2 = Clamp(x + 1, width);
			}

			// 0 1 2
			// 3   4
			// 5 6 7

			myHeight = buffer[y1w + x1];

			temp[0] =  myHeight - buffer[y0w + x0];
			temp[1] = (myHeight - buffer[y0w + x1]) * 1.4142f;
			temp[2] =  myHeight - buffer[y0w + x2];
			temp[3] = (myHeight - buffer[y1w + x0]) * 1.4142f;
			temp[4] = (myHeight - buffer[y1w + x2]) * 1.4142f;
			temp[5] =  myHeight - buffer[y2w + x0];
			temp[6] = (myHeight - buffer[y2w + x1]) * 1.4142f;
			temp[7] =  myHeight - buffer[y2w + x2];

			left	= temp[0] + temp[3] + temp[5];
			right	= temp[2] + temp[4] + temp[7];
			bottom	= temp[0] + temp[1] + temp[2];
			top		= temp[5] + temp[6] + temp[7];

			Vector3f cross ( fracY * (right - left),
							 fracX * (top - bottom),
							 1.0f );

			// Normalize the result
			float mag = 1.0f / Float::Sqrt(1.0f + cross.x * cross.x + cross.y * cross.y);
			cross.x *= mag;
			cross.y *= mag;
			cross.z  = mag;

			Color4ub& clr (c.Expand());
			clr = cross;
			clr.a = Float::ToRangeByte(myHeight);
		}
	}
}

//============================================================================================================
// Allocates a buffer large enough to store the image of specified width, height, and format
//============================================================================================================

void* Image::Reserve (uint width, uint height, uint depth, uint format)
{
	Release();
	ulong size = (ulong)width * height * depth * ITexture::GetBitsPerPixel(format);
	size /= 8;

	if (size == 0) return 0;

	mBuffer.mWidth  = width;
	mBuffer.mHeight = height;
	mBuffer.mFormat = format;

	void* ptr = mBuffer.mBytes.Resize((uint)size);
	memset(ptr, 0, size);

#ifdef _DEBUG
	System::Log("[IMAGE]   Reserved an image memory buffer");
	System::Log("          - Address:        0x%x", GetBuffer());
	System::Log("          - Dimensions:     %u x %u x %u", width, height, depth);
	System::Log("          - Format:         %s", ITexture::FormatToString(format));
	System::Log("          - Size:           %s bytes", String::GetFormattedSize(size).GetBuffer());
#endif
	return ptr;
}

//============================================================================================================
// Release the image buffer
//============================================================================================================

void Image::Release()
{
#ifdef _DEBUG
	if (mBuffer.IsValid())
	{
		if (mSource.IsValid())
		{
			System::Log("[IMAGE]   Releasing  '%s'", mSource.GetBuffer());
		}
		else
		{
			System::Log("[IMAGE]   Releasing an image memory buffer");
		}

		System::Log("          - Address:  0x%x", GetBuffer());
		System::Log("          - Size:     %s bytes",
			String::GetFormattedSize(mBuffer.mBytes.GetSize()).GetBuffer());
	}
#endif
	mBuffer.Release();
}

//============================================================================================================
// Load texture information from the specified file
//============================================================================================================

bool Image::Load (const String& file)
{
	// Temporary memory buffer used to load the file
	Memory in;

	// Try to load the file, and ensure that it has enough data for a header, at least
	if (file.IsValid())
	{
		if (in.Load(file) || in.Load("Textures/" + file))
		{
			if (in.GetSize() > 4)
			{
				mLoadingFN = file;

				if ( Load(in.GetBuffer(), in.GetSize()) )
				{
					mSource = mLoadingFN;
					mLoadingFN.Release();
					return true;
				}
			}
		}
	}
	return false;
}

//============================================================================================================
// Decode a previously loaded buffer
//============================================================================================================

bool Image::Load (const void* buffer, uint size)
{
	Release();
	mSource.Clear();

	if (buffer == 0 || size == 0) return false;

	g_allCodecs.Lock();
	{
		_RegisterAll();

		String extension (System::GetExtensionFromFilename(mLoadingFN));
		const byte* ptr = (const byte*)buffer;

		// Run through all codecs
		for (uint i = 0; i < g_allCodecs.GetSize(); ++i)
		{
			RegisteredCodec& rf = g_allCodecs[i];

			// If the callback returns 'true', it means the image has loaded successfully
			if (rf.mDelegate != 0 && rf.mDelegate(ptr, size, extension, mBuffer))
			{
				g_allCodecs.Unlock();
#ifdef _DEBUG
				uint current = mBuffer.mBytes.GetSize();

				if (mLoadingFN.IsValid())
				{
					System::Log("[IMAGE]   Loaded '%s'", mLoadingFN.GetBuffer());
				}
				else
				{
					System::Log("[IMAGE]   Loaded image data from a memory buffer");
				}

				System::Log("          - Address:        0x%x", GetBuffer());
				System::Log("          - Dimensions:     %u x %u", mBuffer.mWidth, mBuffer.mHeight);
				System::Log("          - Format:         %s", ITexture::FormatToString(mBuffer.mFormat));
				System::Log("          - Original:       %s bytes", String::GetFormattedSize(size).GetBuffer());
				System::Log("          - Uncompressed:   %s bytes", String::GetFormattedSize(current).GetBuffer());
				System::Log("          - Compression:    %.2f%%", (100.0f * size) / current);
#endif
				return true;
			}
		}
	}
	g_allCodecs.Unlock();

	if (mLoadingFN.IsValid())
	{
		System::Log("[IMAGE]   ERROR! No registered codec was able to parse this file");
		System::Log("          - Filename: %s", mLoadingFN.GetBuffer());
	}
	else
	{
		System::Log("[IMAGE]   ERROR! No registered codec was able to parse the specified memory buffer");
	}

	// Header ID might be useful to know
	System::Log("          - Header: 0x%x", *(const uint*)buffer);
	return false;
}