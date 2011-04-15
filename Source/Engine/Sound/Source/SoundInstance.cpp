#include "../Include/_All.h"
#include <IrrKlang/Include/irrKlang.h>

//============================================================================================================
// Sound Instance library
//============================================================================================================

using namespace R5;

//============================================================================================================
// In order to abstract cAudio and make it invisible to the outside projects we keep it as a void*
//============================================================================================================

#define SOUND(source) ((irrklang::ISound*)source)

SoundInstance::~SoundInstance()
{
	SOUND(mAudioSource)->drop();
	mAudioSource = 0;
}

//============================================================================================================
// Update the sound
//============================================================================================================

void SoundInstance::Update(ulong time)
{
	if (mIsPlaying)
	{
		mDuration += Time::GetDeltaMS();
	}

	float atten = 1.0f;
		
	if (mIs3D)
	{
		atten = 1.0f - Min((mPosition - mSound->GetAudio()->GetListener()).Magnitude() / 
			(mRange.y - mRange.x), 1.0f);
		atten *= atten;
	}

	irrklang::ISound* source = SOUND(mAudioSource);

	// Only continue if the sound is actually playing
	if (source != 0 && mIsPlaying)
	{
		// If the sound volume is changing, we need to adjust it inside cAudio
		if (mVolume.y != mVolume.z)
		{
			// Calculate the current fade-out factor
			float factor = (mFadeDuration > 0) ? (float)(0.001 * (time - mFadeStart)) /	mFadeDuration : 1.0f;

			if (factor < 1.0f)
			{
				// Update the volume
				mVolume.y = Interpolation::Linear(mVolume.x, mVolume.z, factor);
			}
			else
			{
				// We've reached the end of the fading process
				mVolume.y = mVolume.z;

				// If the sound was fading out, perform the target action
				if (mVolume.y < 0.0001f)
				{
					if (mAction == TargetAction::Stop)
					{
						source->setIsPaused(true);
						mIsPlaying = false;
						mIsPaused = false;
						mDuration = 0;
					}
					else if (mAction == TargetAction::Pause)
					{
						source->setIsPaused(true);
						mIsPlaying = false;
						mIsPaused = true;
					}
				}
			}
		}

		if (mIs3D)
		{
			Vector3f velocity = (mPosition - mLastPosition) * Time::GetDelta();
			irrklang::vec3df pos (mPosition.x, mPosition.z, mPosition.y);
			irrklang::vec3df vel (velocity.x, velocity.z, velocity.y);
			source->setPosition(pos);
			source->setVelocity(vel);
		}

		source->setVolume(mVolume.y * atten);
	}
}

//============================================================================================================
// Destoryes the sound instance releasing the memory
//============================================================================================================

void SoundInstance::DestroySelf()
{
	((Audio*)mSound->GetAudio())->ReleaseInstance(this);
}

//============================================================================================================
// Play the sound
//============================================================================================================

void SoundInstance::Play()
{
	if (mAudioSource !=0)
	{
		irrklang::ISound* source = SOUND(mAudioSource);
		source->setPlayPosition(mDuration);
		source->setVolume(0.0f);

		if (!mIsPlaying)
		{
			source->setIsPaused(false);
			SetVolume(mVolume.w, 0.0f);
		}
	}
	
	mIsPlaying = true;
	mIsPaused = false;
}

//============================================================================================================
// Pause the sound
//============================================================================================================

void SoundInstance::Pause (float duration)
{
	if (IsPlaying())
	{
		if (mVolume.z > 0.0f)
		{
			mAction = TargetAction::Pause;
			_SetVolume(mVolume.w, 0.0f, duration);
		}
	}
}

//============================================================================================================
// Stop the sound playback
//============================================================================================================

void SoundInstance::Stop (float duration)
{
	if (IsPlaying())
	{
		if (mVolume.z > 0.0f)
		{
			mAction = TargetAction::Stop;
			SetVolume (0.0f, duration);
		}
	}
}

//============================================================================================================
// Sets the 3D position of the specified sound (Should only be set once per frame)
//============================================================================================================

void SoundInstance::SetPosition (const Vector3f& position)
{
	mLastPosition = mPosition;
	mPosition = position;
}

//============================================================================================================
// Changes the volume of the specified sound
//============================================================================================================

void SoundInstance::SetVolume (float volume, float duration)
{
	float layerVolume = mSound->GetAudio()->GetLayerVolume(mLayer);
	
	_SetVolume (volume, layerVolume * volume, duration);
}

//============================================================================================================
// Sets whether the sound will repeat after it ends
//============================================================================================================

void SoundInstance::SetRepeat (bool repeat)
{
	mRepeat = repeat;

	if (mAudioSource != 0)
	{
		SOUND(mAudioSource)->setIsLooped(repeat);
	}
}

//============================================================================================================
// Sets the range of the sound x = min distance (max sound), y = max distance(no sound)
//============================================================================================================

void SoundInstance::SetRange (const Vector2f& range)
{
	mRange = range;
}

//============================================================================================================
// The effect that is going to be played on this sound. Null will disable the effect
//============================================================================================================

void SoundInstance::SetEffect (byte effect)
{
#ifndef _MACOS
	if (mEffect != effect)
	{
		mEffect = effect;

		if (effect != 0)
		{
			irrklang::ISoundEffectControl* fx = SOUND(mAudioSource)->getSoundEffectControl();

			if (effect == Effect::Auditorium)
			{
				fx->enableI3DL2ReverbSoundEffect(-1000, -476, 0, 4.32f, 0.59f, -789,
					0.020f, -789, 0.020f, 100.0f, 100.0f, 5000.0f);
				return;
			}
		}

		irrklang::ISoundEffectControl* fx = SOUND(mAudioSource)->getSoundEffectControl();
		fx->disableI3DL2ReverbSoundEffect();
	}
#endif
}

//============================================================================================================
// INTERNAL: Set the volume of the sound
//============================================================================================================

void SoundInstance::_SetVolume (float volume, float calculatedVolume, float duration)
{
	mVolume.x = mVolume.y;
	mVolume.z = calculatedVolume;
	mVolume.w = volume;
	mFadeStart = Time::GetMilliseconds();
	mFadeDuration = duration;
}