//============================================================================================================
//           R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev12: Multiple Scenes
//------------------------------------------------------------------------------------------------------------
// Required libraries: Basic, Math, Serialization, Core, OpenGL, SysWindow, Font, Image, UI, Render
//============================================================================================================

#include "../../Engine/OpenGL/Include/_All.h"
#include "../../Engine/Core/Include/_All.h"
#include "../../Engine/UI/Include/_All.h"

using namespace R5;

DebugCamera* g_cam0 = 0;
DebugCamera* g_cam1 = 0;

//============================================================================================================

class USMoveCamera : public UIScript
{
public:

	R5_DECLARE_INHERITED_CLASS("USMoveCamera", USMoveCamera, UIScript, UIScript);

	virtual void OnMouseMove (const Vector2i& pos, const Vector2i& delta)
	{
		if (g_cam1 != 0)
		{
			g_cam1->MouseMove(pos, delta);
		}
	}
};

//============================================================================================================

class TestApp : Thread::Lockable
{
	IWindow*		mWin;
	IGraphics*		mGraphics;
	UI*				mUI;
	Core*			mCore;

	Scene			mScene;
	Scene			mScene1;

public:

	TestApp();
	~TestApp();
	void Run();
	void OnDraw();
};

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI, 0, mScene);

	UIScript::Register<USMoveCamera>();
}

//============================================================================================================

TestApp::~TestApp()
{
	if (mCore)		delete mCore;
	if (mUI)		delete mUI;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}

//============================================================================================================

void TestApp::Run()
{
    if (*mCore << "Config/Dev12.txt")
	{
		mScene1.SetRoot( mScene.FindObject<Object>("Scene 1") );
		g_cam0 = mScene.FindObject<DebugCamera>("Camera 0");
		g_cam1 = mScene.FindObject<DebugCamera>("Camera 1");

		if (g_cam0 != 0 && g_cam1 != 0)
		{
			// Create the second render target
			// NOTE: If rendering to a deferred target, 'depth' is not required
			ITexture* color = mGraphics->GetTexture("Secondary Target");
			ITexture* depth = mGraphics->CreateRenderTexture();
			IRenderTarget* rt = mGraphics->CreateRenderTarget();

			// Secondary render target will always be 300x200
			rt->AttachColorTexture(0, color, ITexture::Format::RGBA);
			rt->AttachDepthTexture(depth);
			rt->SetBackgroundColor( Color4f(0, 0, 0, 0) );
			rt->SetSize( Vector2i(300, 200) );

			// Scene 1 will now be rendered into this render target
			mScene1.SetRoot( mScene.FindObject<Object>("Scene 1") );
			mScene1.SetRenderTarget(rt);

			mCore->AddOnDraw( bind(&TestApp::OnDraw, this) );

			while (mCore->Update());

			//*mCore >> "Config/Dev12.txt";
		}
	}
}

//============================================================================================================

void TestApp::OnDraw()
{
	// Draw the off-screen scene
	mScene1.Cull(g_cam1);
	mScene1.DrawAllForward();

	// Draw the main scene
	mScene.Cull(g_cam0);
	mScene.DrawAllForward();
}

//============================================================================================================

R5_MAIN_FUNCTION
{
#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif
	System::SetCurrentPath("../../../Resources/");
	TestApp app;
    app.Run();
	return 0;
}