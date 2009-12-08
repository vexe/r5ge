#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Since R5::Core has worker threads, they need to have a pointer to the core
//============================================================================================================

Core*			g_core			= 0;
Thread::ValType	g_threadCount	= 0;

//============================================================================================================
// Resource executioner thread callback
//============================================================================================================

R5_THREAD_FUNCTION(WorkerThread, ptr)
{
	if (g_core)
	{
		Thread::Increment( g_threadCount );
		Resource* resource = (Resource*)ptr;

#ifdef _DEBUG
		long threadId = g_threadCount;
		ulong timestamp = Time::GetMilliseconds();
		System::Log("[THREAD]  Executing '%s' [ID: %u]", resource->GetName().GetBuffer(), threadId);
#endif

		resource->Lock();
		g_core->SerializeFrom( resource->GetRoot() );
		resource->Unlock();

		Thread::Decrement( g_threadCount );

#ifdef _DEBUG
		System::Log("[THREAD]  Finished executing '%s' in %u ms [ID: %u]",
			resource->GetName().GetBuffer(), Time::GetMilliseconds() - timestamp, threadId);
#endif
	}
	return 0;
}

//============================================================================================================
// Default initialization function
//============================================================================================================

void Core::Init()
{
	ASSERT(g_core == 0, "Only one instance of R5::Core is possible per application at this time");
	g_core = this;

#ifdef _DEBUG
	mSleepDelay = 1;
#else
	mSleepDelay = 0;
#endif

	// Root of the scene needs to know who owns it
	mRoot.mCore = this;

	// All keys start as inactive
	memset(mIsKeyDown, 0, sizeof(bool) * 256);

	// If a window was specified, ensure that R5::Core is the target of all of its callbacks
	if (mWin) mWin->SetEventHandler(this);

	// Ensure that the timer frequency is 1 millisecond
	Thread::ImproveTimerFrequency(true);

	// Automatically assign the graphics controller to the window
	if (mWin != 0) mWin->SetGraphics(mGraphics);
}

//============================================================================================================
// Core constructor and destructor
//============================================================================================================

Core::Core (IWindow* window, IGraphics* graphics, IUI* gui) :
	mWin(window), mGraphics(graphics), mUI(gui)
{
	Init();
}

//============================================================================================================

Core::Core (IWindow* window, IGraphics* graphics, IUI* gui, Scene& scene) :
	mWin(window), mGraphics(graphics), mUI(gui)
{
	Init();
	scene.SetRoot(&mRoot);
}

//============================================================================================================

Core::~Core()
{
	// Ensure that the core won't be deleted until all worker threads finish
	while (g_threadCount > 0) Thread::Sleep(1);
	if (mWin != 0) mWin->SetGraphics(0);
	g_core = 0;

	// We no longer need the improved timer frequency
	Thread::ImproveTimerFrequency(false);

	// Root has to be released explicitly as it has to be cleared before meshes
	mRoot.Release();
}

//============================================================================================================
// Releases the meshes and the scene graph
//============================================================================================================

void Core::Release()
{
	mRoot.Lock();
	mRoot.Release();
	mRoot.Unlock();

	mMeshes.Lock();
	mMeshes.Release();
	mMeshes.Unlock();

	mSkeletons.Lock();
	mSkeletons.Release();
	mSkeletons.Unlock();
}

//============================================================================================================
// Starting a new frame
//============================================================================================================

bool Core::Update()
{
	Time::Update();

	if (mWin != 0)
	{
		// Window update function can return 'false' if the window has been closed
		if (!mWin->Update()) return false;
		bool minimized = mWin->IsMinimized();

		// Size update is queued rather than executed in Core::OnResize() because that call needs to update
		// the graphics controller, and the controller may be created in a different thread than the window.
		if (mUpdatedSize)
		{
			if (mGraphics)	mGraphics->SetViewport(mUpdatedSize);
			if (mUI)		mUI->OnResize(mUpdatedSize);

			// The size no longer needs to be updated
			mUpdatedSize = 0;
		}

		// Do not update anything unless some time has passed
		if (Time::GetDelta() != 0)
		{
			// Update all props and models
			mModels.Lock();
			{
				for (uint i = mModels.GetSize(); i > 0; )
				{
					Model* model = mModels[--i];
					if (model != 0) model->Update();
				}
			}
			mModels.Unlock();

			// Pre-update callbacks
			mPre.Execute();

			// Update the entire scene
			if (mRoot.GetFlag(Object::Flag::Enabled))
			{
				mRoot._Update(Vector3f(), Quaternion(), 1.0f, false);
			}

			// Post-update callbacks
			mPost.Execute();

			// The UI should only be updated if the window is not minimized
			if (mUI != 0 && !minimized) mUI->Update();

			// Post-GUI updates
			mLate.Execute();
		}

		// If we have an OnDraw listener, call it
		if (mOnDraw && !minimized)
		{
			// Start the drawing process
			mWin->BeginFrame();
			if (mGraphics != 0)	mGraphics->BeginFrame();

			// Draw the scene
			mOnDraw();

			// Draw the UI
			if (mUI != 0) mUI->Draw();

			// Finish the drawing process
			if (mGraphics != 0)	mGraphics->EndFrame();
			mWin->EndFrame();

			// Increment the framerate
			Time::IncrementFPS();
		}

		// Sleep the thread, letting others run in the background
		Thread::Sleep(mSleepDelay);
		return true;
	}
	return false;
}

//============================================================================================================
// Properly shuts down everything
//============================================================================================================

void Core::Shutdown()
{
	if (mWin != 0) mWin->Close();
}

//============================================================================================================
// Retrieves a mesh with the specified name
//============================================================================================================

Mesh* Core::GetMesh (const String& name, bool createIfMissing)
{
	if (name.IsEmpty()) return 0;
	return (createIfMissing ? mMeshes.AddUnique(name) : mMeshes.Find(name));
}

//============================================================================================================
// Creates a new model or retrieves an existing one
//============================================================================================================

Model* Core::GetModel (const String& name, bool createIfMissing)
{
	if (name.IsEmpty()) return 0;

	if (createIfMissing)
	{
		Model* model = mModels.AddUnique(name);
		model->mCore = this;
		return model;
	}
	return mModels.Find(name);
}

//============================================================================================================
// Retrieves a resource with the specified name
//============================================================================================================

Resource* Core::GetResource (const String& name, bool createIfMissing)
{
	if (name.IsEmpty()) return 0;
	return (createIfMissing ? mResources.AddUnique(name) : mResources.Find(name));
}

//============================================================================================================
// Retrieves a skeleton with the specified name
//============================================================================================================

Skeleton* Core::GetSkeleton (const String& name, bool createIfMissing)
{
	if (name.IsEmpty()) return 0;
	return (createIfMissing ? mSkeletons.AddUnique(name) : mSkeletons.Find(name));
}

//============================================================================================================
// Creates a new model template or retrieves an existing one
//============================================================================================================

ModelTemplate* Core::GetModelTemplate (const String& name, bool createIfMissing)
{
	if (name.IsEmpty()) return 0;

	if (createIfMissing)
	{
		ModelTemplate* temp = mModelTemplates.AddUnique(name);

		// If it's a brand-new model, try to load it
		if (temp->GetCore() == 0)
		{
			temp->mCore = this;

			if ( !temp->IsValid() )
			{
				if ( temp->Load(name, false) )
				{
					temp->SetSerializable(false);
				}
			}
		}
		return temp;
	}
	return mModelTemplates.Find(name);
}

//============================================================================================================
// Load the specified file
//============================================================================================================

bool Core::operator << (const char* file)
{
	TreeNode node;
	return node.Load(file) ? SerializeFrom(node) : false;
}

//============================================================================================================
// Save everything into the specified file
//============================================================================================================

bool Core::operator >> (const char* file) const
{
	TreeNode node;
	node.mTag = "Root";
	if (!SerializeTo(node)) return false;
	return node.Save(file);
}

//============================================================================================================
// Triggered by IWindow -- responds to char events
//============================================================================================================

bool Core::OnChar(byte key)
{
	if (mUI && mUI->OnChar(key)) return true;
	return false;
}

//============================================================================================================
// Triggered by IWindow -- responds to key events
//============================================================================================================

bool Core::OnKey(const Vector2i& pos, byte key, bool isDown)
{
	mIsKeyDown[key] = isDown;
	if (mUI && mUI->OnKey(pos, key, isDown)) return true;

	// If we have a key event listener, let it respond
	if (mOnKey) return mOnKey(pos, key, isDown);

	// Default behavior with no set listener
	if (mWin != 0 && !isDown)
	{
		if (key == Key::Escape)
		{
			Shutdown();
		}
		else if (key == Key::F5)
		{
			mWin->SetStyle( (mWin->GetStyle() & IWindow::Style::FullScreen) == 0 ?
				IWindow::Style::FullScreen : IWindow::Style::Normal );
		}
		else if (key == Key::F6)
		{
			mWin->SetPosition( Vector2i(100, 100) );
			mWin->SetSize( Vector2i(900, 600) );
			mWin->SetStyle(IWindow::Style::Normal);
		}
	}
	return false;
}

//============================================================================================================
// Triggered by IWindow -- responds to mouse movement
//============================================================================================================

bool Core::OnMouseMove(const Vector2i& pos, const Vector2i& delta)
{
	mMousePos = pos;
	if (mUI && mUI->OnMouseMove(pos, delta)) return true;
	return (mOnMouseMove) ? mOnMouseMove(pos, delta) : false;
}

//============================================================================================================
// Triggered by IWindow -- responds to scrolling
//============================================================================================================

bool Core::OnScroll(const Vector2i& pos, float delta)
{
	if (mUI && mUI->OnScroll(pos, delta)) return true;
	return (mOnScroll) ? mOnScroll(pos, delta) : false;
}

//============================================================================================================
// Triggered by IWindow -- responds to window resizing
//============================================================================================================

void Core::OnResize(const Vector2i& size)
{
	// Size update is queued rather than executed in Core::OnResize() because this call needs to update
	// the graphics controller, and the controller may be created in a different thread than the window.
	mUpdatedSize = size;
}

//============================================================================================================
// Loads the resources node
//============================================================================================================

void Core::ParseResources(const TreeNode& root)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;

		Resource* resource = GetResource(tag);
		resource->SerializeFrom(node);
	}
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Core::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	bool serializable = true;

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node	= root.mChildren[i];
		const String&	tag		= node.mTag;
		const Variable&	value	= node.mValue;

		if ( tag == Core::ClassID() )
		{
			// SerializeFrom only returns 'false' if something important failed
			if ( !SerializeFrom(node, forceUpdate) )
				return false;
		}
		else if ( tag == IWindow::ClassID() )
		{
			// If window creation fails, let the calling function know
			if (mWin != 0 && !mWin->SerializeFrom(node))
				return false;
		}
		else if ( tag == IGraphics::ClassID() )
		{
			// If graphics init fails, let the calling function know
			if (mGraphics != 0 && !mGraphics->SerializeFrom(node, forceUpdate))
				return false;
		}
		else if ( tag == IUI::ClassID() )
		{
			if (mUI != 0 && mGraphics != 0)
				mUI->SerializeFrom(node);
		}
		else if ( tag == Scene::ClassID() )
		{
			mRoot.SerializeFrom(node, forceUpdate);
		}
		else if ( tag == Mesh::ClassID() )
		{
			Mesh* mesh = GetMesh(value.IsString() ? value.AsString() : value.GetString(), true);
			if (mesh != 0) mesh->SerializeFrom(node, forceUpdate);
		}
		else if ( tag == Skeleton::ClassID() )
		{
			Skeleton* skel = GetSkeleton(value.IsString() ? value.AsString() : value.GetString(), true);
			if (skel != 0) skel->SerializeFrom(node, forceUpdate);
		}
		else if ( tag == Resource::ClassID() )
		{
			Resource* res = GetResource(value.IsString() ? value.AsString() : value.GetString(), true);

			if (res != 0)
			{
				res->SerializeFrom(node, forceUpdate);
				if (!serializable) res->SetSerializable(false);
			}
		}
		else if ( tag == ModelTemplate::ClassID() )
		{
			ModelTemplate* temp = GetModelTemplate(value.IsString() ? value.AsString() : value.GetString(), true);

			if (temp != 0)
			{
				temp->SerializeFrom(node, forceUpdate);
				if (!serializable) temp->SetSerializable(false);
			}
		}
		else if ( tag == Model::ClassID() )
		{
			Model* model = GetModel(value.IsString() ? value.AsString() : value.GetString(), true);

			if (model != 0)
			{
				model->SerializeFrom(node, forceUpdate);
				if (!serializable) model->SetSerializable(false);
			}
		}
		else if ( tag == "Serializable" )
		{
			value >> serializable;
		}
		else if ( tag == "Sleep" )
		{
			uint ms;
			if (value >> ms) Thread::Sleep( ms );
		}
		else if ( tag == "Execute" )
		{
			// Find the resource
			String name (value.IsString() ? value.AsString() : value.GetString());
			Resource* res = GetResource(name);

			// If the resource is valid, create a worker thread for it
			if (res->IsValid())
			{
#ifndef R5_MEMORY_TEST
				Thread::Create( WorkerThread, res );
#else
				SerializeFrom( res->GetRoot() );
#endif
				if (serializable)
				{
					mExecuted.Lock();
					mExecuted.Expand() = name;
					mExecuted.Unlock();
				}
			}
		}
		// Registered serialization callback
		else if (mOnFrom) mOnFrom(node);
	}
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool Core::SerializeTo (TreeNode& root) const
{
	// Window information should always be saved first as it *has* to come first
	if (mWin) mWin->SerializeTo(root);

	// Makes sense to have graphics second
	if (mGraphics) mGraphics->SerializeTo(root);

	// User interface comes next
	if (mUI) mUI->SerializeTo(root);

	// Save all resources and models
	if (mResources.IsValid() || mExecuted.IsValid() || mModels.IsValid())
	{
		TreeNode& node = root.AddChild( Core::ClassID() );

		mResources.Lock();
		{
			for (uint i = 0; i < mResources.GetSize(); ++i)
				mResources[i]->SerializeTo(node);
		}
		mResources.Unlock();

		mExecuted.Lock();
		{
			for (uint i = 0; i < mExecuted.GetSize(); ++i)
				node.AddChild("Execute").mValue = mExecuted[i];
		}
		mExecuted.Unlock();

		mModelTemplates.Lock();
		{
			for (uint i = 0; i < mModelTemplates.GetSize(); ++i)
			{
				const ModelTemplate* temp = mModelTemplates[i];

				if (temp != 0)
				{
					temp->SerializeTo(node);
				}
			}
		}
		mModelTemplates.Unlock();

		mModels.Lock();
		{
			for (uint i = 0; i < mModels.GetSize(); ++i)
			{
				const Model* model = mModels[i];

				if (model != 0)
				{
					model->SerializeTo(node);
				}
			}
		}
		mModels.Unlock();
	}

	// Save out the scenegraph only if there is something to save
	mRoot.Lock();
	{
		const Object::Children& children = mRoot.GetChildren();
		const Object::Scripts&  scripts  = mRoot.GetScripts();

		if (children.IsValid() || scripts.IsValid())
		{
			TreeNode& node = root.AddChild(Scene::ClassID());

			for (uint i = 0; i < scripts.GetSize(); ++i)
				scripts[i]->SerializeTo(node);

			for (uint i = 0; i < children.GetSize(); ++i)
				children[i]->SerializeTo(node);
		}
	}
	mRoot.Unlock();

	// Registered serialization callback
	if (mOnTo) mOnTo(root);
	return true;
}

//============================================================================================================
// Executes an existing (loaded) resource in a different thread
//============================================================================================================

void Core::SerializeFrom (Resource* resource)
{
	if (resource != 0 && resource->IsValid())
	{
#ifndef R5_MEMORY_TEST
		Thread::Create( WorkerThread, resource );
#else
		SerializeFrom( resource->GetRoot() );
#endif
	}
}