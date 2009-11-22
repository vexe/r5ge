#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Object point light
//============================================================================================================

class PointLight : public Object
{
private:

	// The only difference from directional lights is that point lights use attenuation parameters
	struct Params : public DirectionalLight::Params
	{
		Vector3f mAtten;
		virtual uint GetLightType()	 const	{ return ILight::Type::Point; }
		virtual const Vector3f*	GetAttenParams() const	{ return &mAtten; }
	};

protected:

	Color3f		mAmbient;		// Ambient color
	Color3f		mDiffuse;		// Diffuse color
	Color3f		mSpecular;		// Specular color
	float		mBrightness;	// Light's brighness
	float		mRange;			// Range of the light
	float		mPower;			// Attenuation power
	Params		mParams;		// Outgoing parameters

public:

	PointLight();

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Point Light", PointLight, Object, Object);

private:

	// Updates appropriate fields in 'mParams'
	void _UpdateColors();
	void _UpdateAtten();

public:

	const Color3f&	GetAmbient()	const { return mAmbient;	}
	const Color3f&	GetDiffuse()	const { return mDiffuse;	}
	const Color3f&	GetSpecular()	const { return mSpecular;	}
	float			GetBrightness() const { return mBrightness;	}
	float			GetRange()		const { return mRange;		}
	float			GetPower()		const { return mPower;		}

	void SetAmbient		(const Color3f& c)	{ mAmbient	= c; _UpdateColors(); }
	void SetDiffuse		(const Color3f& c)	{ mDiffuse	= c; _UpdateColors(); }
	void SetSpecular	(const Color3f& c)	{ mSpecular	= c; _UpdateColors(); }
	void SetBrightness	(float val);
	void SetRange		(float val);
	void SetPower		(float val);

protected:

	// Update the light parameters
	virtual void OnUpdate();

	// Cull the object based on the viewing frustum
	virtual bool OnCull (CullParams& params, bool isParentVisible, bool render);

protected:

	// Serialization
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual bool OnSerializeFrom (const TreeNode& root);
};