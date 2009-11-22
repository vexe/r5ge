#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// XML-like hierarchy without the bulkiness of XML
//============================================================================================================

struct TreeNode
{
	String			mTag;
	Variable		mValue;
	Array<TreeNode>	mChildren;

	TreeNode() {}
	TreeNode(const char* tag) { mTag = tag; }

	void Lock()		{ mChildren.Lock(); }
	void Unlock()	{ mChildren.Unlock(); }

	// Release all memory used by the class
	void Release()
	{
		mTag.Release();
		mValue.Release();
		mChildren.Release();
	}

	// The node is valid as long as it has a tag
	bool IsValid() const { return mTag.IsValid(); }

	// Whether the node has children
	bool HasChildren() const { return mChildren.IsValid(); }

	// Not thread-safe. Lock the tree before using.
	TreeNode& AddChild(const char* tag)
	{
		TreeNode& node = mChildren.Expand();
		node.mTag = tag;
		return node;
	}

	// Implementations of various data types
	template <typename Type>
	TreeNode& AddChild(const char* tag, const Type& val)
	{
		TreeNode& node	= mChildren.Expand();
		node.mTag		= tag;
		node.mValue		= val;
		return node;
	}

	// Saves to the specified file, using the file's extension to determine whether it should be binary
	bool Save (const char* filename) const;

	// Saves the file, ensuring that it has a proper header
	bool Save (const char* filename, bool binary) const;

	// Loads a file, ensures that it's of proper format, then continues loading the structure from memory
	bool Load (const char* filename);

	// Loads the tree from memory loaded elsewhere
	bool Load (const byte* buffer, uint size);

	// Serialization to a memory buffer
	bool SerializeTo (Memory& mem) const;

	// Serialization to text format
	bool SerializeTo (String& s, uint level = 0) const;

	// Loads the tree structure from memory -- only binary format is accepted
	bool SerializeFrom (ConstBytePtr& buffer, uint& size);

	// Loads the tree structure from a previously loaded string.
	// Note that the specified string should not have any comments inside.
	// Also note that the binary serialization is *significantly* faster.
	bool SerializeFrom (const String& s);

private:

	// Recursive text format parsing
	bool ParseProperties (const String& data, uint& first, uint last);
};