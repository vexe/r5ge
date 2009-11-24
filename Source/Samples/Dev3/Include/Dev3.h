#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Development Testing Application
//============================================================================================================

#include "../../../Engine/Render/Include/_All.h"
#include "../../../Engine/OpenGL/Include/_All.h"
#include "../../../Engine/Core/Include/_All.h"
#include "../../../Engine/UI/Include/_All.h"

namespace R5
{
class TestApp
{
	IWindow*	mWin;
	IGraphics*	mGraphics;
	UI*			mUI;
	Core*		mCore;
	Scene		mScene;
	Camera*		mCam;
	bool		mFlag;
	bool		mDeferred;
	String		mDebug;
	uint		mTriangles;
	uint		mVisible;
	uint		mRendered;

public:

	TestApp();
	~TestApp();

	void  Init();
	void  Run();
	void  OnDraw();
	float UpdateStats();
	bool  OnKey (const Vector2i& pos, byte key, bool isDown);
};
}