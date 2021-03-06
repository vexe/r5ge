#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Multiple projected texture object
// Author: Michael Lyashenko
//============================================================================================================

class Decal : public Object
{
protected:

	Matrix43	mMatrix;
	IShader*	mShader;
	Color4f		mColor;
	uint		mMask;

	// Textures passed to the decal
	Array<const ITexture*> mTextures;

	// Objects should never be created manually. Use the AddObject<> template instead.
	Decal();

public:

	// Object creation
	R5_DECLARE_INHERITED_CLASS(Decal, Object, Object);

	// Changes the default drawing layer that will be used by decals
	static void SetDefaultLayer(byte layer);

	const IShader*	GetShader()	const	{ return mShader;	}
	const Color4f&	GetColor()	const	{ return mColor;	}

	void SetShader	(const String& shader);
	void SetShader	(IShader* val)			{ mShader	= val;	}
	void SetColor	(const Color4f& val)	{ mColor	= val;	}
	
	Array<const ITexture*>& GetTextureArray()	{ return mTextures;	}

protected:

	// Set the mask
	virtual void OnInit();

	// Updates the transformation matrix
	virtual void OnUpdate();

	// Fill the renderable object and visible light lists
	virtual bool OnFill (FillParams& params);

	// Draws the light on-screen if it's visible
	virtual uint OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut);

	// Serialization
	virtual void OnSerializeTo	  (TreeNode& node) const;
	virtual bool OnSerializeFrom  (const TreeNode& node);
};