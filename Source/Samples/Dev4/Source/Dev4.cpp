#include "../Include/Dev4.h"
using namespace R5;

uint g_ssao = 1;

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mCam(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI, mScene);
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
    if (*mCore << "Config/Dev4.txt")
	{
		mCam = mScene.FindObject<Camera>("Default Camera");

		if (mCam != 0)
		{
			mCore->SetListener( bind(&TestApp::OnDraw, this) );
			mCore->SetListener( bind(&TestApp::OnKeyPress, this) );
			mCore->SetListener( bind(&Object::MouseMove, mCam) );
			mCore->SetListener( bind(&Object::Scroll, mCam) );

			while (mCore->Update());

			//*mCore >> "Config/Dev4.txt";
		}
	}
}

//============================================================================================================

void TestApp::OnDraw()
{
	static UILabel* tri = mUI->FindWidget<UILabel>("Triangles");
	static UILabel* fps = mUI->FindWidget<UILabel>("FPS");
	static UILabel* inf = mUI->FindWidget<UILabel>("Info");

	mScene.Cull(mCam);

	if (g_ssao > 0)
	{
		mScene.DrawAllDeferred(g_ssao);
	}
	else
	{
		mGraphics->SetActiveRenderTarget(0);
		mScene.DrawAllForward();
	}

	if (tri) tri->SetText( String("TRI: %u", mGraphics->GetFrameStats().mTriangles) );
	if (fps) fps->SetText( String("FPS: %u", Time::GetFPS()) );
	if (inf) inf->SetText( String(g_ssao == 0 ? "No SSAO (S)" : (g_ssao == 1 ?
		"SSAO: Low Quality (S)" : "SSAO: High Quality (S)")) );
}

//============================================================================================================

bool TestApp::OnKeyPress(const Vector2i& pos, byte key, bool isDown)
{
	if (!isDown)
	{
		if ( key == Key::Escape )
		{
			mCore->Shutdown();
		}
		else if ( key == Key::F5 )
		{
			mWin->SetStyle( mWin->GetStyle() == IWindow::Style::FullScreen ?
				IWindow::Style::Normal : IWindow::Style::FullScreen);
		}
		else if ( key == Key::F6 )
		{
			mWin->SetSize( Vector2i(900, 600) );
			mWin->SetStyle(IWindow::Style::Normal);
		}
		else if ( key == Key::S )
		{
			g_ssao += 1;
			if (g_ssao > 2) g_ssao = 0;
		}
	}
	return true;
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