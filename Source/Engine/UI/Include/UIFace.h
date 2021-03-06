#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Struct containing a pair of UV texture coordinates
// Author: Michael Lyashenko
//============================================================================================================

class UIFace
{
	friend class UISkin;

private:

	String		mName;
	Vector2i	mPos;
	Vector2i	mSize;
	short		mBorder;

	UIFace() : mBorder(0) {}

public:
	~UIFace() {}

	const String&	GetName()	const { return mName;	}
	const Vector2i& GetPos()	const { return mPos;	}
	const Vector2i& GetSize()	const { return mSize;	}
	short			GetBorder() const { return mBorder;	}

	// Calculates the 4 texture coordinates for the image of specified size
	Rectangle<float> GetRectangle (const Vector2i& size) const;

	// Serialization
	void SerializeFrom (const TreeNode& root);
	void SerializeTo (TreeNode& root) const;
};