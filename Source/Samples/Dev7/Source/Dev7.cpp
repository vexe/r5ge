#include "../Include/_All.h"
using namespace R5;

R5::Random randomGen;

//============================================================================================================
// Constructor and destructor pair
//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0), mCam(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI, 0, mScene);

	mCore->SetSleepDelay(0);

	// Register the new fire and smoke emitters
	Object::Register<FireEmitter>();
	Object::Register<SmokeEmitter>();
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
    if (*mCore << "Config/Dev7.txt")
	{
		mCam = mScene.FindObject<Camera>("Default Camera");

		if (mCam != 0)
		{
			mCore->AddOnDraw( bind(&TestApp::OnDraw, this) );

			while (mCore->Update());

			//*mCore >> "Config/Dev7.txt";
		}
	}
}

//============================================================================================================

void TestApp::OnDraw()
{
	mScene.Cull(mCam);

	mGraphics->Clear();
	mGraphics->SetScreenProjection( false );
	mGraphics->Draw( IGraphics::Drawable::Grid );

	mScene.DrawAllForward(false);

	static Object* place = mScene.FindObject<Object>("Stage");
	if (place != 0) place->SetRelativeRotation( Quaternion(0.0f, 0.0f, Time::GetTime()) );
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