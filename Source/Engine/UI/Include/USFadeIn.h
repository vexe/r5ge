#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Fade the widget in over the duration
//============================================================================================================

class USFadeIn : public UIScript
{
protected:

	float mDuration;
	byte mEvents;

	USFadeIn() : mDuration (0.25f), mEvents(0) {}

public:

	R5_DECLARE_INHERITED_CLASS("USFadeIn", USFadeIn, UIScript, UIScript);

	// Set the duration of the fade
	void SetDuration(float duration) { mDuration = duration; }

	// Get the duration of the fade
	const float GetDuration() const { return mDuration; }

	virtual void OnInit();
	virtual void OnDestroy() { mWidget->SetEventHandling(mEvents); }
	virtual void OnUpdate(bool areaChanged);
};