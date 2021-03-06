#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Most basic game object -- can be positioned somewhere in the scene and can contain children.
// Author: Michael Lyashenko
//============================================================================================================

class Core;
class Object
{
	friend class Script;	// Script needs access to 'mScripts' so it can remove itself
	friend class Scene;		// Scene needs to be able to use 'mCore'
	friend class Core;		// Core needs to be able to set 'mCore'

public:

	// Object creation delegate
	typedef FastDelegate<Object* (void)> CreateDelegate;

	// Flags used by the Object
	struct Flag
	{
		enum
		{
			Enabled			= 1 << 1,
			Visible			= 1 << 2,
			BoxCollider		= 1 << 3,
		};
	};

	// Types used by this class
	typedef Object*					ObjectPtr;
	typedef PointerArray<Object>	Children;
	typedef PointerArray<Script>	Scripts;
	typedef Thread::Lockable		Lockable;

protected:

	String		mName;				// Name of this object
	Flags		mFlags;				// Internal flags associated with this object
	Object*		mParent;			// Object's parent
	void*		mSubParent;			// Optional abstract sub-parent the object belongs to (such as a QuadTree node)
	Core*		mCore;				// Engine's core that the object was created with
	IGraphics*	mGraphics;			// Graphics manager, cached for convenience

	Vector3f	mLastPos;			// Saved last relative position (used for velocity)
	Vector3f	mRelativePos;		// Local space position
	Quaternion	mRelativeRot;		// Local space rotation
	Vector3f	mRelativeScale;		// Local space scale
	Vector3f	mRelativeVel;		// Local space velocity, calculated every Update()
	Vector3f	mAbsolutePos;		// World space position, calculated every Update()
	Quaternion	mAbsoluteRot;		// World space rotation
	Vector3f	mAbsoluteScale;		// World space scale
	Vector3f	mAbsoluteVel;		// World space velocity
	
	Bounds		mRelativeBounds;	// Local bounds
	Bounds		mAbsoluteBounds;	// Calculated bounds that include only this object
	Bounds		mCompleteBounds;	// Calculated bounds that include the bounds of all children
	bool		mCalcRelBounds;		// Whether relative bounds will be auto-calculated ('true' in most cases)
	bool		mCalcAbsBounds;		// Whether absolute bounds will be auto-calculated ('true' in most cases)
	bool		mIncChildBounds;	// Whether to include children when re-calculating the bounds ('true' in most cases)

	byte		mLayer;				// Draw layer on which this object resides
	bool		mIsDirty;			// Whether the object's absolute coordinates should be recalculated
	bool		mHasMoved;			// Whether the object has moved since last update
	bool		mSerializable;		// Whether the object will be serialized out
	bool		mShowOutline;		// Whether to show the bounding outline -- useful for debugging
	ulong		mLastFilltime;		// Timestamp of the last time Fill() was called
	uint		mVisibility;		// Visibility counter -- how many times the object is visible per frame

	Lockable	mLock;
	Children	mChildren;
	Children	mDeletedObjects;
	Scripts		mScripts;
	Scripts		mDeletedScripts;

private:

	// In order to eliminate virtual function calls, all virtual functions in this class
	// automatically set these flags, thus marking themselves as not being overwritten,
	// which in turn will make sure that they are never called again.

	struct Ignore
	{
		enum
		{
			Fill			= 1 << 0,
			PreUpdate		= 1 << 1,
			Update			= 1 << 2,
			PostUpdate		= 1 << 3,
			Raycast			= 1 << 4,
			Draw			= 1 << 5,
			SerializeFrom	= 1 << 6,
			SerializeTo		= 1 << 7,
			Subscriptions	= 1 << 8,
		};
	};

	mutable Flags mIgnore;

protected:

	// Objects should never be created manually. Use the AddObject<> template instead.
	Object();

public:

	virtual ~Object() { Release(); }

	// This is a top-level base class
	R5_DECLARE_BASE_CLASS(Object);

private:

	// INTERNAL Registers a new object of specified type
	static void _Register (const String& type, const CreateDelegate& callback);

	// INTERNAL: Creates a new object of specified type
	static Object* _Create (const String& type);

	// INTERNAL: Use templates instead of these functions
	Object*			_AddObject	(const String& type, const String& name);
	const Object*	_FindObject	(const String& name, bool recursive = true) const;
	Script*			_AddScript	(const char* type);
	Script*			_AddScript	(const String& type);
	const Script*	_GetScript	(const char* type) const;
	const Script*	_GetScript	(const String& type) const;

	// INTERNAL: Use the AddObject<> template instead
	void _Add	(Object* ptr);
	void _Remove(Object* ptr);

	// Draws the outline of the bounding box
	uint _DrawOutline (IGraphics* graphics, const ITechnique* tech);

public:

	// Clears the object, removing all children and scripts
	void Clear()
	{
		DestroyAllChildren();
		DestroyAllScripts();
	}

	// Destroys the object -- this action is queued until next update
	void DestroySelf();

	// Destroys all of the object's children
	void DestroyAllChildren();

	// Destroys all of the object's scripts
	void DestroyAllScripts();

	// Release all resources associated with this object
	void Release();

	// Retrieves the Core that was ultimately owns this object
	Core* GetCore() { return mCore; }

	// Convenience functionality
	IGraphics* GetGraphics() { return mGraphics; }

	// Returns whether the object is currently visible
	bool IsVisible() const { return mFlags.Get(Flag::Visible) && (mParent == 0 && mParent->IsVisible()); }

	// Name and flag field retrieval
	const String&		GetName()				const	{ return mName;				}
	Object*				GetParent()						{ return mParent;			}
	const Object*		GetParent()				const	{ return mParent;			}
	void*				GetSubParent()					{ return mSubParent;		}
	const void*			GetSubParent()			const	{ return mSubParent;		}
	byte				GetLayer()				const	{ return mLayer;			}
	const Flags&		GetFlags()				const	{ return mFlags;			}
	bool				GetFlag (uint flag)		const	{ return mFlags.Get(flag);	}
	bool				IsDirty()				const	{ return mIsDirty;			}
	bool				IsShowingOutline()		const	{ return mShowOutline;		}
	bool				HasMoved()				const	{ return mHasMoved;			}
	bool				IsSerializable()		const	{ return mSerializable;		}
	Children&			GetChildren()					{ return mChildren;			}
	const Children&		GetChildren()			const	{ return mChildren;			}
	const Scripts&		GetScripts()			const	{ return mScripts;			}

	// Retrieves relative (local space) position / rotation
	const Vector3f&		GetRelativePosition()	const	{ return mRelativePos;		}
	const Quaternion&	GetRelativeRotation()	const	{ return mRelativeRot;		}
	const Vector3f&		GetRelativeScale()		const	{ return mRelativeScale;	}
	const Vector3f&		GetRelativeVelocity()	const	{ return mRelativeVel;		}

	// Retrieves absolute (world space) position / rotation
	const Vector3f&		GetAbsolutePosition()	const	{ return mAbsolutePos;		}
	const Quaternion&	GetAbsoluteRotation()	const	{ return mAbsoluteRot;		}
	const Vector3f&		GetAbsoluteScale()		const	{ return mAbsoluteScale;	}
	const Vector3f&		GetAbsoluteVelocity()	const	{ return mAbsoluteVel;		}

	// Retrieves object bounds
	const Bounds&		GetRelativeBounds()		const	{ return mRelativeBounds;	}
	const Bounds&		GetAbsoluteBounds()		const	{ return mAbsoluteBounds;	}
	const Bounds&		GetCompleteBounds()		const	{ return mCompleteBounds;	}

public:

	// Every object has a name that can be changed
	void SetName (const String& name) { mName = name; }

	// Every object can have object-specific flags that can be changed
	void SetFlag (uint flag, bool val) { mFlags.Set(flag, val); }

	// Marks the object as dirty, making it recalculate its absolute position next update
	void SetDirty() { mIsDirty = true; }

	// It's possible to switch object's parents
	void SetParent (Object* ptr);

	// Sets the optional sub-parent
	void SetSubParent (void* ptr) { mSubParent = ptr; }

	// Changes this object's draw layer
	void SetLayer (byte layer) { mLayer = (byte)(layer & 31); }

	// Whether the object will be saved out
	void SetSerializable (bool val) { mSerializable = val; }

	// Whether to show the object's bounding box
	void SetShowOutline	(bool val) { mShowOutline = val; }

	// Sets all relative values
	void SetRelativePosition ( const Vector3f& pos )	{ mRelativePos	  = pos;	mIsDirty = true; }
	void SetRelativeRotation ( const Quaternion& rot )	{ mRelativeRot	  = rot;	mIsDirty = true; }
	void SetRelativeScale	 ( const Vector3f& scale )	{ mRelativeScale  = scale;	mIsDirty = true; }
	void SetRelativeBounds	 ( const Bounds& b )		{ mRelativeBounds = b;		mIsDirty = true; mCalcRelBounds = false; }

	// Sets both relative and absolute values using provided absolute values
	void SetAbsolutePosition ( const Vector3f& pos );
	void SetAbsoluteRotation ( const Quaternion& rot );
	void SetAbsoluteScale	 ( const Vector3f& scale );

	// It should be possible to temporarily override the absolute values
	void OverrideAbsolutePosition (const Vector3f&   pos)	{ mAbsolutePos	 = pos;		}
	void OverrideAbsoluteRotation (const Quaternion& rot)	{ mAbsoluteRot	 = rot;		}
	void OverrideAbsoluteScale	  (const Vector3f& scale)	{ mAbsoluteScale = scale;	}

	// Convenience function -- force-updates the object based on the parent's absolute values
	void Update();

	// Updates the object, calling appropriate virtual functions
	bool Update (const Vector3f& pos, const Quaternion& rot, const Vector3f& scale, bool parentMoved);

	// Fills the render queues and updates the visibility mask
	void Fill (FillParams& params);

	// Draws the object with the specified technique
	uint Draw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut);

	// Cast a ray into space and fill the list with objects that it intersected with
	void Raycast (const Vector3f& pos, const Vector3f& dir, Array<RaycastHit>& hits);

	// Subscribe to events with specified priority
	void SubscribeToKeyPress	(uint priority);
	void SubscribeToMouseMove	(uint priority);
	void SubscribeToScroll		(uint priority);

	// Unsubscribe from events with specified priority
	void UnsubscribeFromKeyPress	(uint priority);
	void UnsubscribeFromMouseMove	(uint priority);
	void UnsubscribeFromScroll		(uint priority);

	// Manually forward the OnKeyPress notification to the scripts
	uint KeyPress (const Vector2i& pos, byte key, bool isDown);

	// Manually forward the OnMouseMove notification to the scripts
	uint MouseMove (const Vector2i& pos, const Vector2i& delta);

	// Manually forward the OnScroll notification to the scripts
	uint Scroll (const Vector2i& pos, float delta);

	// Serialization
	bool SerializeTo (TreeNode& root) const;
	bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);

protected:

	// NOTE: Don't forget to set 'mIsDirty' to 'true' if you modify relative coordinates
	// in your virtual functions, or absolute coordinates won't be recalculated!

	// The very first function called on the object -- called after the object's parent and root have been set
	virtual void OnInit() {}

	// Called before the object gets destroyed
	virtual void OnDestroy() {}

	// Callback triggered after SerializeFrom() completes
	virtual void OnPostSerialize() {}

	// Function called when a new child object has been added
	virtual void OnAddChild (Object* obj) {}

	// Function called just before the child gets removed
	virtual void OnRemoveChild (Object* obj) {}

	// Called prior to object's Update function, before absolute coordinates are calculated
	virtual void OnPreUpdate();

	// Called after the object's absolute coordinates have been calculated
	// but before all children and absolute bounds have been updated.
	virtual void OnUpdate();

	// Called after the object has updated all of its children and absolute bounds
	virtual void OnPostUpdate();

	// Called when the scene is being culled. Should update the 'mask' parameter.
	// Return 'true' if the Object should recurse through children, 'false' if the function already did.
	virtual bool OnFill (FillParams& params);

	// Draw the object using the specified technique. This function will only be
	// called if this object has been added to the list of drawable objects in
	// OnFill. It should return the number of triangles rendered.
	virtual uint OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut) { mIgnore.Set(Ignore::Draw, true); return 0; }

	// Called when the object is being raycast into -- should return 'false' if children were already considered
	virtual bool OnRaycast (const Vector3f& pos, const Vector3f& dir, Array<RaycastHit>& hits);

	// Called when the object is being saved
	virtual void OnSerializeTo (TreeNode& node) const;

	// Called when the object is being loaded
	virtual bool OnSerializeFrom (const TreeNode& node);

public:

	// Registers a new object of the specified type
	template <typename Type> static void Register() { _Register( Type::ClassName(), &Type::_CreateNew ); }

	// Finds a child object of the specified name and type
	template <typename Type> Type* FindObject (const String& name, bool recursive = true)
	{
		const Object* obj = _FindObject(name, recursive);
		return R5_CAST(Type, obj);
	}

	// Finds a child object of the specified name and type
	template <typename Type> const Type* FindObject (const String& name, bool recursive = true) const
	{
		const Object* obj = _FindObject(name, recursive);
		return R5_CAST(Type, obj);
	}

	// Creates a new child of specified type and name
	template <typename Type> Type* AddObject (const String& name)
	{
		Object* obj = _AddObject(Type::ClassName(), name);
		return R5_CAST(Type, obj);
	}

	// Retrieves an existing script on the object
	template <typename Type> Type* GetScript()
	{
		const Script* script = _GetScript(Type::ClassName());
		return R5_CAST(Type, script);
	}

	// Retrieves an existing script on the object
	template <typename Type> const Type* GetScript() const
	{
		const Script* script = _GetScript(Type::ClassName());
		return R5_CAST(Type, script);
	}

	// Adds a new script to the object (only one script of each type can be active on the object)
	template <typename Type> Type* AddScript()
	{
		Script* script = _AddScript(Type::ClassName());
		return R5_CAST(Type, script);
	}

	// Removes the specified script from the game object
	template <typename Type> void RemoveScript()
	{
		Script* script = (Script*)_GetScript(Type::ClassName());
		if (script != 0) script->DestroySelf();
	}
};

//============================================================================================================
// Useful assert macro
//============================================================================================================

#ifdef _DEBUG
#define ASSERT_IF_CORE_IS_UNLOCKED ASSERT(mCore->IsLocked() || \
	(mCore->GetNumberOfThreads() == 0 && mCore->GetThreadID() == Thread::GetID()), \
	"You must lock the core before you work with objects!");
#else
#define ASSERT_IF_CORE_IS_UNLOCKED
#endif