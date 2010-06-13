#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// All drawable objects are separated by techniques into different lists
//============================================================================================================

class DrawList
{
private:

	PointerHash<DrawGroup> mGroups;
	bool mNeedsSorting;

public:

	DrawList() : mNeedsSorting(false) {}

	// Add a new entry
	void Add (uint group, Object* object, float distance);

	// Sorts the array
	void Sort() { mNeedsSorting = true; }

	// Clear all entries in the draw list
	void Clear();

	// Draw all objects in the list
	uint Draw (const Deferred::Storage& storage, const ITechnique* tech, bool insideOut);
};