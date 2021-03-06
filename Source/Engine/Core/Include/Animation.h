#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Defined keyframed animation within a skeleton
// Author: Michael Lyashenko
//============================================================================================================

class Animation
{
public:

	// Animated bones contain animation splines as well as the ID of their parent
	struct AnimatedBone
	{
		SplineV	mSplineV;		// Position spline with time keys in 0 to 1 range
		SplineQ	mSplineQ;		// Rotation spline
		byte	mSmoothV;		// Method of interpolation for positions
		byte	mSmoothQ;		// Method of interpolation for rotations

		AnimatedBone() : mSmoothV(2), mSmoothQ(2) {}
	};

	typedef Array<AnimatedBone>	AnimatedBones;
	typedef PointerArray<Bone>	Bones;

protected:

	uint			mId;		// This animation's ID (internal use)
	String			mName;		// Name of this animation
	String			mRootBone;	// Name of the root bone for this animation
	Vector2i		mFrames;	// Range of frames this animation is using
	Vector3f		mDuration;	// Fade in, play, and fade out duration
	uint			mLayer;		// Layer this animation is activated on
	bool			mLoop;		// Whether to loop this animation
	AnimatedBones	mBones;		// List of calculated bone splines for every bone
	uint			mRootIndex;	// Root bone index, set automatically
	bool			mIsDirty;	// Whether the animation needs to be rebuilt

public:

	R5_DECLARE_NAMED_CLASS(Animation);

	Animation (const String& name) : mName(name), mIsDirty(true) { _Reset(); }

private:

	void _Reset();

public:

	uint			GetID()			const	{ return mId;		}
	const String&	GetName()		const	{ return mName;		}
	const Vector2i&	GetFrames()		const	{ return mFrames;	}
	const Vector3f&	GetDuration()	const	{ return mDuration;	}
	uint			GetLayer()		const	{ return mLayer;	}
	bool			IsLooping()		const	{ return mLoop;		}
	bool			IsValid()		const	{ return mDuration.y > 0.0f; }
	bool			IsDirty()		const	{ return mIsDirty || mBones.IsEmpty(); }

	void Clear()							{ mBones.Lock(); mBones.Clear();   _Reset(); mBones.Unlock(); }
	void Release()							{ mBones.Lock(); mBones.Release(); _Reset(); mBones.Unlock(); }
	void ClearBones()						{ mBones.Lock(); mBones.Clear();			 mBones.Unlock();  }
	void SetID		(uint val)				{ mId		= val;	}
	void SetName	(const String& name)	{ mName		= name; }
	void SetFrames	(const Vector2i& val);
	void SetDuration(const Vector3f& val)	{ mDuration	= val;	}
	void SetLayer	(uint layer)			{ mLayer = layer;	}
	void SetLooping	(bool  val)				{ if (mLoop != val) { mLoop = val; mIsDirty = true; } }

	// Fills the local list of animated bones using the provided list of bones
	void Fill (const Bones& bones);

	// Samples the splines at the specified 0-1 range time
	// Returns 0x1 if position was set, 0x2 if rotation, and 0x3 if both
	uint Sample (uint boneIndex, float time, Vector3f& pos, Quaternion& rot) const;

	// Serialization
	bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
	bool SerializeTo (TreeNode& root) const;
};