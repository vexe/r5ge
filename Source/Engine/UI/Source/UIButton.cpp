#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

UIButton::UIButton() : mState(State::Enabled), mSticky(false), mIgnoreMouseKey(false)
{
	mLabel.SetAlignment(UILabel::Alignment::Center);
	mImage.SetLayer(0, false);
	mLabel.SetLayer(1, false);
}

//============================================================================================================
// Changes the button's state by changing the background face
//============================================================================================================

bool UIButton::SetState (uint state, bool val)
{
	uint newState ( val ? (mState | state) : (mState & ~state) );

	if ( mState != newState )
	{
		if ( state == State::Pressed && val == false )
		{
			mIgnoreMouseKey = false;
		}

		mState = newState;

		const ITexture* tex = mImage.GetTexture();
		if (tex != 0) OnDirty(tex);

		OnStateChange(state, val);
		return true;
	}
	return false;
}

//============================================================================================================
// Internal functions. These values are normally set by Root::CreateArea
//============================================================================================================

void UIButton::_SetParentPtr (UIWidget* ptr)
{
	UIWidget::_SetParentPtr(ptr);
	mImage._SetParentPtr(this);
	mLabel._SetParentPtr(this);
}

//============================================================================================================

void UIButton::_SetRootPtr (UIManager* ptr)
{
	UIWidget::_SetRootPtr(ptr);
	mImage._SetRootPtr(ptr);
	mLabel._SetRootPtr(ptr);
}

//============================================================================================================
// Marking the button as dirty should mark the associated texture as dirty regardless of 'face' being set.
//============================================================================================================

void UIButton::SetDirty()
{
	const ITexture* tex = mImage.GetTexture();
	if (tex != 0) OnDirty(tex);
	mLabel.SetDirty();
}

//============================================================================================================
// Any per-frame animation should go here
//============================================================================================================

bool UIButton::OnUpdate (bool dimensionsChanged)
{
	// Use the default face for the sake of calculating proper dimensions
	if (mImage.GetFace() == 0) mImage.SetFace("Button: Enabled", false);

	dimensionsChanged |= mImage.Update(mRegion, dimensionsChanged);
	mLabel.Update(mImage.GetSubRegion(), dimensionsChanged);
	return false;
}

//============================================================================================================
// Fills the rendering queue
//============================================================================================================

void UIButton::OnFill (UIQueue* queue)
{
	if (queue->mLayer == mLayer && queue->mWidget == 0 && queue->mTex == mImage.GetTexture())
	{
		static String faceName[] =
		{
			"Button: Disabled",
			"Button: Enabled",
			"Button: Highlighted",
			"Button: Pressed"
		};

		if (mState & State::Enabled)
		{
			mImage.SetFace(faceName[1], false);
			mImage.OnFill(queue);

			if (mState & State::Highlighted)
			{
				mImage.SetFace(faceName[2], false);
				mImage.OnFill(queue);
			}

			if (mState & State::Pressed)
			{
				mImage.SetFace(faceName[3], false);
				mImage.OnFill(queue);
			}
		}
		else
		{
			mImage.SetFace(faceName[0], false);
			mImage.OnFill(queue);
		}
	}
	else mLabel.OnFill(queue);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UIButton::OnSerializeFrom (const TreeNode& node)
{
	if ( mImage.OnSerializeFrom(node) )
	{
		return true;
	}
	else if (node.mTag == "State")
	{
		if (node.mValue.IsString())
		{
			const String& state = node.mValue.AsString();

			if		(state == "Disabled")	SetState(State::Enabled, false);
			else if (state == "Pressed")	SetState(State::Enabled | State::Pressed, true);
			else if (state == "Enabled")	SetState(State::Enabled, true);
		}
		return true;
	}
	else if ( node.mTag == "Sticky" )
	{
		bool val;
		if (node.mValue >> val) SetSticky(val);
		return true;
	}
	return mLabel.OnSerializeFrom (node);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void UIButton::OnSerializeTo (TreeNode& node) const
{
	// Only the skin is saved from the SubPicture. Face is ignored.
	const UISkin* skin = mImage.GetSkin();

	// Only save the skin if it's something other than the default one
	if (skin != 0 && skin != mUI->GetDefaultSkin())
		node.AddChild("Skin", skin->GetName());

	// Label settings are saved fully
	mLabel.OnSerializeTo (node);

	// Save the state
	if		(mState & State::Pressed && mSticky)	node.AddChild("State", "Pressed");
	else if (mState & State::Enabled)				node.AddChild("State", "Enabled");
	else											node.AddChild("State", "Disabled");

	// Whether the button is a sticky (push) button
	node.AddChild("Sticky", mSticky);
}

//============================================================================================================
// Event handling -- mouse hovering over the button should highlight it
//============================================================================================================

bool UIButton::OnMouseOver (bool inside)
{
	if (mState & State::Enabled)
	{
		SetState (State::Highlighted, inside);
		UIWidget::OnMouseOver(inside);
	}
	return true;
}

//============================================================================================================
// Event handling -- mouse movement should be intercepted
//============================================================================================================

bool UIButton::OnMouseMove(const Vector2i& pos, const Vector2i& delta)
{
	if (mState & State::Enabled) UIWidget::OnMouseMove(pos, delta);
	return true;
}

//============================================================================================================
// Event handling -- left mouse button presses the button
//============================================================================================================

bool UIButton::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
{
	bool retVal = false;

	if (mState & State::Enabled)
	{
		if (key == Key::MouseLeft)
		{
			retVal = true;

			if (isDown)
			{
				// If we're ignoring this key, don't continue but don't ignore the next one (mouse up)
				if (mIgnoreMouseKey)
				{
					mIgnoreMouseKey = false;
					return true;
				}

				// If this is a sticky button and it's not currently pressed, ignore the next mouse key
				mIgnoreMouseKey = (mSticky && !(mState & State::Pressed));

				// Update the button's state to be pressed and highlighted
				SetState(State::Pressed | State::Highlighted, true);
			}
			else
			{
				if (mRegion.Contains(pos))
				{
					// If we should ignore this key, don't continue
					if (mIgnoreMouseKey) return true;

					// Otherwise set the button's state to be not pressed and continue
					SetState(State::Pressed, false);
				}
				else
				{
					// Don't ignore the next key
					mIgnoreMouseKey = false;

					// Not in region - the button is not pressed or highlighted
					SetState(State::Pressed | State::Highlighted, false);
				}
			}
		}
		UIWidget::OnKeyPress(pos, key, isDown);
	}
	return retVal;
}