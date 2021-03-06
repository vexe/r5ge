#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Author: Eugene Gorodinsky
//============================================================================================================

class Sound: public ISound
{
friend class Audio;
friend class SoundInstance;

protected:
	uint	mRefCount;
	String	mName;

	LinkedList<Sound*>::Entry*	mSoundsEntry;

protected:
	Sound(): mRefCount(0) {}
	virtual ~Sound() {}

	virtual void SetAudioData(AudioData *audioData) = 0;
	virtual SoundInstance* Instantiate() = 0;
	
public:

	virtual const String& GetName() const
		{ return mName; }
};
