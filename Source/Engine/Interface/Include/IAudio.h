#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Basic interface for the Audio controller class
// Author: Michael Lyashenko
//============================================================================================================

struct IAudio
{
	R5_DECLARE_INTERFACE_CLASS(Audio);

	virtual ~IAudio() {}

	// Release all audio resources
	virtual void Release()=0;

	// Update notification
	virtual void Update()=0;
	
	// Release all resources associated with the specified sound
	virtual bool Release (ISound* sound)=0;

	// Sets the sound listener pos/dir/up (usually should be the camera)
	virtual void SetListener(const Vector3f& position, const Vector3f& dir, const Vector3f& up, const Vector3f& velocity)=0;

	// Gets the volume of the specified layer
	virtual IAudioLayer* GetLayer(uint layer)=0;

	// Get the sound by name
	virtual ISound* GetSound (const String& name, bool createIfMissing = true)=0;

	// Create a 2D sound instance
	virtual ISoundInstance* Instantiate (ISound* sound, uint layer, float fadeInTime, bool repeat)=0;

	// Create a 3D sound instance
	virtual ISoundInstance* Instantiate (ISound* sound, const Vector3f& position, uint layer, float fadeInTime, bool repeat)=0;
};
