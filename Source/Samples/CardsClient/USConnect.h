#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Connect script is attached to a button and handles all network-related communication
// Author: Michael Lyashenko
//============================================================================================================

class USConnect : public UIScript
{
	UIButton*		mButton;
	UIInput*		mAddress;
	UIInput*		mName;
	UIButton*		mPlay;
	UIWidget*		mTable;
	UIHighlight*	mHand;
	Network			mNet;
	bool			mIsConnected;
	bool			mMyTurn;
	Memory			mIn;
	Memory			mOut;
	TreeNode		mRoot;

	USConnect() : mButton(0), mAddress(0), mName(0), mPlay(0), mTable(0), mHand(0), mIsConnected(false),
		mMyTurn(false) {}

public:

	R5_DECLARE_INHERITED_CLASS(USConnect, UIScript, UIScript);

	// Ensure the script is attached to a button
	virtual void OnInit();

	// Remove bound callbacks
	virtual void OnDestroy();

	// Connect to the server
	virtual void OnKeyPress (const Vector2i& pos, byte key, bool isDown);

	// Send out messages when they arrive
	virtual void OnUpdate (bool areaChanged);

public:

	// New connection notification
	void OnConnect (const Network::Address& addr, uint socketId, VoidPtr& ptr, const String& msg);

	// Connection has been closed notification
	void OnClose (const Network::Address& addr, uint socketId, VoidPtr& ptr);

	// Receive new packet
	void OnReceive (const Network::Address& addr, uint socketId, VoidPtr& ptr, const byte* data, uint size, Thread::IDType threadId);

	// Error notification
	void OnError (const Network::Address& addr, uint socketId, VoidPtr& ptr, const char* message);

public:

	void LockUI()	{ mWidget->GetUI()->Lock(); }
	void UnlockUI()	{ mWidget->GetUI()->Unlock(); }

	// Gets the table widget
	UIWidget* GetTable() { if (mTable == 0) mTable = mWidget->GetUI()->FindWidget<UIWidget>("Table"); return mTable; }

	// Sets the hand's values given the specified TreeNode with an array
	void SetHand (const TreeNode& root);

	// Locks the buffer, starts the send process
	void BeginSend();

	// Sends the packet to the specified socket
	void EndSend();

	// Callback triggered when the Play button is pressed
	void OnPlay (UIWidget* widget, const Vector2i& pos, byte key, bool isDown);

	// Process the player's packet
	bool ProcessPacket (const byte* buffer, uint size);
};