#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// XML-like hierarchy without the bulkiness of XML
// Author: Michael Lyashenko
//============================================================================================================

struct TreeNode
{
	struct SaveFlag
	{
		enum
		{
			ASCII		= 0,
			Binary		= 1,
			Compressed	= 3,
		};
	};

	String			mTag;
	Variable		mValue;
	Array<TreeNode>	mChildren;
	Flags			mFlags;

	TreeNode(const char* tag = "Root") { mTag = tag; }
	TreeNode(const String& s) { mTag = s; }

	void Lock()		{ mChildren.Lock(); }
	void Unlock()	{ mChildren.Unlock(); }

	// Release all memory used by the class
	void Release()
	{
		mTag = "Root";
		mValue.Release();
		mChildren.Release();
		mFlags.Clear();
	}

	// The node is valid as long as it has a tag
	bool IsValid() const { return mTag.IsValid(); }

	// Whether the node has children
	bool HasChildren() const { return mChildren.IsValid(); }

	// Not thread-safe. Lock the tree before using.
	TreeNode& AddChild (const char* tag)
	{
		TreeNode& node = mChildren.Expand();
		node.mTag = tag;
		return node;
	}

	// Not thread-safe. Lock the tree before using.
	TreeNode& AddUnique (const char* tag)
	{
		FOREACH(i, mChildren)
		{
			if (mChildren[i].mTag == tag)
			{
				return mChildren[i];
			}
		}
		return AddChild(tag);
	}

	// Implementations of various data types
	template <typename Type>
	TreeNode& AddChild (const char* tag, const Type& val)
	{
		TreeNode& node	= mChildren.Expand();
		node.mTag		= tag;
		node.mValue		= val;
		return node;
	}

	// Finds a child with the specified tag
	TreeNode* FindChild (const String& tag, bool recursive = true);

public: // Simple serialization functions

	// Saves to the specified file, using the file's extension to determine whether it should be binary
	bool Save (const char* filename) const;

	// Saves the file, ensuring that it has a proper header
	bool Save (const char* filename, uint saveFlag) const;

	// Loads a file, ensures that it's of proper format, then continues loading the structure from memory
	bool Load (const char* filename);

	// Loads the tree from memory loaded elsewhere
	// NOTE: Must contain a header tag, such as '//R5A'
	bool Load (const byte* buffer, uint size);

	// Convenience function
	// NOTE: Must contain a header tag, such as '//R5A'
	bool Load (const Memory& mem) { return Load(mem.GetBuffer(), mem.GetSize()); }

public: // Advanced serialization functions

	// Serialization to a memory buffer
	bool SerializeTo (Memory& mem) const;

	// Serialization to text format
	bool SerializeTo (String& s, uint level = 0) const;

	// Loads the tree structure from memory -- only binary format is accepted
	bool SerializeFrom (ConstBytePtr& buffer, uint& size);

	// Convenience function
	bool SerializeFrom (const Memory& mem)
	{
		const byte* buffer = mem.GetBuffer();
		uint size = mem.GetSize();
		return SerializeFrom(buffer, size);
	}

	// Loads the tree structure from a previously loaded string.
	// Also that the binary serialization is *significantly* faster.
	bool SerializeFrom (const String& s);

private:

	// Recursive text format parsing
	bool ParseProperties (const String& data, uint& first, uint last);
};