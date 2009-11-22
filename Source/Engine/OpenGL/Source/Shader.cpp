#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// To keep track of currently active program
//============================================================================================================

uint g_activeProgram = 0;
uint g_activeLightCount = 0;

//============================================================================================================
// Retrieves the information log for the specified shader
//============================================================================================================

void GetShaderInfoLog(uint shader, String& out)
{
	out.Clear();

	int logLength (0);

	glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 0)
	{
		out.Resize(logLength);
		int charsWritten (0);
		glGetShaderInfoLog (shader, logLength, &charsWritten, (char*)out.GetBuffer());
	}
}

//============================================================================================================
// Retrieves the information log for the specified program
//============================================================================================================

void GetProgramInfoLog(uint shader, String& out)
{
	out.Clear();

	int logLength (0);

	glGetProgramiv (shader, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 0)
	{
		out.Resize(logLength);
		int charsWritten (0);
		glGetProgramInfoLog (shader, logLength, &charsWritten, (char*)out.GetBuffer());
	}
}

//============================================================================================================
// Splits the info log string into separate lines
//============================================================================================================

void SplitInfoLog(const String& infoLog, Array<String>& lines)
{
	String debug(infoLog), left, right;

	while ( debug.Split(left, '\n', right) ||
			debug.Split(left, '.',  right) )
	{
		// Skip the space
		lines.Expand() = (left[0] == ' ') ? &left[1] : left.GetBuffer();

		if ( right.IsValid() )
		{
			debug = (right[0] == ' ') ? &right[1] : right.GetBuffer();
		}
		else debug.Clear();
	}

	if ( debug.IsValid() ) lines.Expand() = debug;
}

//============================================================================================================
// Prints the shader log information
//============================================================================================================

void PrintInfoLog(const String& infoLog)
{
	Array<String> lines;
	SplitInfoLog(infoLog, lines);

	for (uint i = 0; i < lines.GetSize(); ++i)
	{
		System::Log("          - %s", lines[i].GetBuffer());
	}
}

//============================================================================================================
// Compiles the shader and retrieves whether the operation was successful, with an error log
//============================================================================================================

bool CompileShader (uint shader, String& out)
{
	int retVal (0);
	glCompileShader(shader);
	CHECK_GL_ERROR;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &retVal);
	GetShaderInfoLog(shader, out);
	CHECK_GL_ERROR;
	return (retVal == GL_TRUE);
}

//============================================================================================================
// Links the program together and returns whether the operation was successful, with an error log
//============================================================================================================

bool LinkProgram (uint program, String& out)
{
	int retVal (0);

	if (program != 0)
	{
		// R5 shader attributes should be automatically bound to be associated with their IGraphics counterparts
		glBindAttribLocation(program, IGraphics::Attribute::Position,		"R5_position");
		glBindAttribLocation(program, IGraphics::Attribute::Tangent,		"R5_tangent");
		glBindAttribLocation(program, IGraphics::Attribute::Normal,			"R5_normal");
		glBindAttribLocation(program, IGraphics::Attribute::Color,			"R5_color");
		glBindAttribLocation(program, IGraphics::Attribute::SecondaryColor,	"R5_secondaryColor");
		glBindAttribLocation(program, IGraphics::Attribute::FogCoord,		"R5_fogCoord");
		glBindAttribLocation(program, IGraphics::Attribute::BoneWeight,		"R5_boneWeight");
		glBindAttribLocation(program, IGraphics::Attribute::BoneIndex,		"R5_boneIndex");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord0,		"R5_texCoord0");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord1,		"R5_texCoord1");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord2,		"R5_texCoord2");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord3,		"R5_texCoord3");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord4,		"R5_texCoord4");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord5,		"R5_texCoord5");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord6,		"R5_texCoord6");
		glBindAttribLocation(program, IGraphics::Attribute::TexCoord7,		"R5_texCoord7");

		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &retVal);
		GetProgramInfoLog(program, out);
	}
	return (retVal == GL_TRUE);
}

//============================================================================================================
// Creates a shader of specified type using the specified source
//============================================================================================================

uint CreateShader (const String& name, const String& code, uint type)
{
	uint shader = glCreateShader(type);
	ASSERT( shader != 0, glGetErrorString() );

	// Set the shader source
	const char* src = code.GetBuffer();
	glShaderSource(shader, 1, &src, 0);
	CHECK_GL_ERROR;

	// Compile the shader
	String debug;
	if ( CompileShader(shader, debug) )
	{
#ifdef _DEBUG
		System::Log( "          - Compiled '%s'", name.GetBuffer());
#endif
	}
	else
	{
		// Something went wrong -- log the error
#ifdef _DEBUG
		System::Log( "          - FAILED to compile '%s'", name.GetBuffer() );
#else
		System::Log( "[SHADER]  FAILED to compile '%s'", name.GetBuffer() );
#endif
		PrintInfoLog(debug);
		glDeleteShader(shader);
		shader = 0;
	}
	return shader;
}

//============================================================================================================
// Attach a set of shaders to the program, then link them together
//============================================================================================================

bool AttachAndLink( String& debug, uint& program, uint& vertex, uint& pixel )
{
	if ( program == 0 )
	{
		program = glCreateProgram();
		ASSERT( program != 0, glGetErrorString() );
		CHECK_GL_ERROR;
	}

	if (vertex) glAttachShader(program, vertex);
	if (pixel)  glAttachShader(program, pixel);
	CHECK_GL_ERROR;

	if ( LinkProgram(program, debug) )
	{
		glUseProgram( g_activeProgram = program );
		CHECK_GL_ERROR;
		return true;
	}

	glDeleteProgram(program);
	program = 0;
	return false;
}

//============================================================================================================
// Detach and delete a specific shader
//============================================================================================================

inline void DetachAndDelete(uint program, uint& shader)
{
	if (shader != 0)
	{
		if (program != 0) glDetachShader(program, shader);
		glDeleteShader(shader);
		shader = 0;
	}
}

//============================================================================================================
// Detach the specified shader
//============================================================================================================

inline void DetachShader (uint program, uint shader)
{
	if (program != 0 && shader != 0)
	{
		glDetachShader(program, shader);
	}
}

//============================================================================================================
// Finds a shader file given the filename and extension
//============================================================================================================

String FindShader (const String& file, const char* extension)
{
	String path (file);

	if (System::FileExists(path)) return path;

	path = file + extension;
	if (System::FileExists(path)) return path;

	if (!path.BeginsWith("Shaders/")) path = String("Shaders/") + path;
	if (System::FileExists(path)) return path;

	path = String("Shaders/") + System::GetFilenameFromPath(file);
	if (System::FileExists(path)) return path;

	path << extension;
	if (System::FileExists(path)) return path;

	path = "Resources/" + path;
	if (System::FileExists(path)) return path;

	return "";
}

//============================================================================================================
// Sets a uniform integer value in a shader
//============================================================================================================

bool SetUniform1i (uint program, const char* name, int val)
{
	if (program)
	{
		int loc = glGetUniformLocation(program, name);
		if (loc != -1)
		{
#ifdef _DEBUG
			System::Log("          - Found constant uniform '%s' [%u]", name, loc);
#endif
			glUniform1i(loc, val);
			CHECK_GL_ERROR;
			return true;
		}
	}
	return false;
}

//============================================================================================================
// Updates a uniform entry
//============================================================================================================

bool UpdateUniform (const GLShader::UniformEntry& entry)
{
	if (entry.mGlID == -2)
	{
		entry.mGlID = glGetUniformLocation(g_activeProgram, entry.mName.GetBuffer());
		CHECK_GL_ERROR;

#ifdef _DEBUG
		if (entry.mGlID != -1)
		{
			System::Log("          - Found uniform '%s' [%u]",
				entry.mName.GetBuffer(), entry.mGlID);
		}
#endif
	}
#ifdef _DEBUG
	else
	{
		uint id = glGetUniformLocation(g_activeProgram, entry.mName.GetBuffer());
		ASSERT(entry.mGlID == id, "The uniform shifted!");
	}
#endif
	
	if (entry.mGlID != -1)
	{
		if (entry.mDelegate)
		{
			Uniform uni;
			uni.mType = Uniform::Type::Invalid;
			entry.mDelegate(entry.mName, uni);

			switch (uni.mType)
			{
			case Uniform::Type::Float1:
				glUniform1f(entry.mGlID, uni.mVal[0]);
				break;
			case Uniform::Type::Float2:
				glUniform2f(entry.mGlID, uni.mVal[0], uni.mVal[1]);
				break;
			case Uniform::Type::Float3:
				glUniform3f(entry.mGlID, uni.mVal[0], uni.mVal[1], uni.mVal[2]);
				break;
			case Uniform::Type::Float4:
				glUniform4f(entry.mGlID, uni.mVal[0], uni.mVal[1], uni.mVal[2], uni.mVal[3]);
				break;
			case Uniform::Type::Float9:
				glUniformMatrix3fv(entry.mGlID, 1, 0, uni.mVal);
				break;
			case Uniform::Type::Float16:
				glUniformMatrix4fv(entry.mGlID, 1, 0, uni.mVal);
				break;
			case Uniform::Type::ArrayFloat1:
				glUniform1fv(entry.mGlID, uni.mElements, (float*)uni.mPtr);
				break;
			case Uniform::Type::ArrayFloat2:
				glUniform2fv(entry.mGlID, uni.mElements, (float*)uni.mPtr);
				break;
			case Uniform::Type::ArrayFloat3:
				glUniform3fv(entry.mGlID, uni.mElements, (float*)uni.mPtr);
				break;
			case Uniform::Type::ArrayFloat4:
				glUniform4fv(entry.mGlID, uni.mElements, (float*)uni.mPtr);
				break;
			case Uniform::Type::ArrayFloat9:
				glUniformMatrix3fv(entry.mGlID, uni.mElements, 0, (float*)uni.mPtr);
				break;
			case Uniform::Type::ArrayFloat16:
				glUniformMatrix4fv(entry.mGlID, uni.mElements, 0, (float*)uni.mPtr);
				break;
			case Uniform::Type::ArrayInt:
				glUniform1iv(entry.mGlID, uni.mElements, (int*)uni.mPtr);
				break;
			}
			CHECK_GL_ERROR;
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Determines whether the shader's source will need pre-processing
//============================================================================================================

inline bool PreprocessCheck (const String& source)
{
	return	source.Find("R5_FOR_EACH_LIGHT") != source.GetLength();
}

//============================================================================================================
// Unwraps the 'for' loops then compiles the shader (fix for shoddy GLSL support on Intel cards)
//============================================================================================================

bool PreprocessCompile (const String& name, GLShader::ShaderEntry& entry, GLShader::ShaderInfo& info, uint type, uint lights)
{
	if (entry.mGlID == 0 && entry.mStatus != GLShader::ShaderEntry::CompileStatus::Error)
	{
		if (info.mSource.IsValid())
		{
			if (info.mSpecial)
			{
				uint offset (0);
				String source (info.mSource);

				while ( (offset = source.Find("R5_FOR_EACH_LIGHT", true, offset)) != source.GetLength() )
				{
					int braces = 0;
					uint start (source.GetLength()),
								 end   (source.GetLength());

					for (uint i = offset; i < source.GetLength(); ++i)
					{
						char letter = source[i];

						// Opening brace -- potential beginning of the block
						if (letter == '{')
						{
							if (++braces == 1) start = i;
						}
						// Closing brace -- potential end of the block
						else if (letter == '}')
						{
							if (--braces == 0)
							{
								end = i;
								break;
							}
						}
					}

					if ( start == source.GetLength() || end == source.GetLength() )
					{
						entry.mStatus = GLShader::ShaderEntry::CompileStatus::Error;
#ifdef _DEBUG
						System::Log("          - ERROR! Mismatched brackets found while compiling '%s'", name.GetBuffer());
#else
						System::Log("[SHADER]  ERROR! Mismatched brackets found while compiling '%s'", name.GetBuffer());
#endif
						return false;
					}

					// Move up the offset to the end of the block
					offset = end+1;
					String block, left, right, copy;
					source.GetString(left, 0, start);
					source.GetString(block, start, offset);
					source.GetString(right, offset);

					// Beginning of the code
					source = left;

					// Block entries for every light
					for (uint i = 0; i < lights; ++i)
					{
						if (i == 0)
						{
							source << block;
						}
						else
						{
							copy = block;
							String index;
							index.Set("[%u]", i);
							copy.Replace("[0]", index);
							source << copy;
						}
					}

					// Finish by adding the rest of the code
					offset = source.GetLength();
					source << right;
				}

				// Attempt to create the shader
				entry.mGlID = ::CreateShader(name, source, type == GLShader::Type::Vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
			}
			else // not marked as 'special'
			{
				// Attempt to create the shader
				entry.mGlID = ::CreateShader(name, info.mSource, type == GLShader::Type::Vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
			}
		}

		// Remember the result
		entry.mStatus = (entry.mGlID == 0) ?
			GLShader::ShaderEntry::CompileStatus::Error :
			GLShader::ShaderEntry::CompileStatus::Success;
	}

	// Return whether the shader was compiled successfully
	return (entry.mStatus == GLShader::ShaderEntry::CompileStatus::Success);
}

//============================================================================================================
// Actual shader class begins here...
//============================================================================================================

GLShader::GLShader (const String& name) :
	mName			(name),
	mFlag			(0),
	mIsDirty		(false),
	mSerializable	(false),
	mLast			(g_caps.mMaxLights + 1),
	mUpdateStamp	(0)
{
	mVertexInfo.mPath   = ::FindShader(name, ".vert");
	mFragmentInfo.mPath = ::FindShader(name, ".frag");

	SetSourcePath (mVertexInfo.mPath,	Type::Vertex);
	SetSourcePath (mFragmentInfo.mPath, Type::Fragment);

	// If the path was deduced from the shader's name, don't serialize this shader
	mSerializable = mVertexInfo.mPath.IsEmpty() && mFragmentInfo.mPath.IsEmpty();
}

//============================================================================================================
// Non thread-safe version of Release(), for internal use only
//============================================================================================================

void GLShader::_InternalRelease (bool clearUniforms)
{
	mFlag = 0;
	mLast = g_caps.mMaxLights + 1;

	// Detach and release all shaders
	for (uint i = mPrograms.GetSize(); i > 0; )
	{
		ProgramEntry& entry (mPrograms[--i]);
		uint& pid = entry.mProgram.mGlID;

		if (pid != 0)
		{
			if (mVertexInfo.mSpecial || i == 0)		::DetachAndDelete( pid, mPrograms[i].mVertex.mGlID );
			else									::DetachShader   ( pid, mPrograms[0].mVertex.mGlID );

			if (mFragmentInfo.mSpecial || i == 0)	::DetachAndDelete( pid, mPrograms[i].mFragment.mGlID );
			else									::DetachShader   ( pid, mPrograms[0].mFragment.mGlID );

			glDeleteProgram(pid);
			pid = 0;
		}

		// Reset all uniforms back to "not found" state
		Array<UniformEntry>& uniforms (entry.mUniforms);

		uniforms.Lock();
		{
			if (clearUniforms) uniforms.Clear();
			else for (uint i = 0; i < uniforms.GetSize(); ++i)
				uniforms[i].mGlID = -2;
		}
		uniforms.Unlock();
	}
}

//============================================================================================================
// Releases the source for the shader and marks it as dirty so it's released next frame
//============================================================================================================

void GLShader::Release()
{
	mLock.Lock();
	{
		mVertexInfo.Clear();
		mFragmentInfo.Clear();
		mIsDirty = true;
	}
	mLock.Unlock();
}

//============================================================================================================
// Activates the shader compiled for the specified number of lights
//============================================================================================================

uint GLShader::_Activate (uint activeLightCount, bool updateUniforms)
{
	CHECK_GL_ERROR;

	// Release the shaders if necessary
	if (mIsDirty)
	{
		mIsDirty = false;
		_InternalRelease(false);
	}

	// Ensure that we have the correct number of programs
	uint maxEntries = g_caps.mMaxLights + 1;
	if (mPrograms.GetSize() < maxEntries)
		mPrograms.ExpandTo(maxEntries);

	// Just in case, never exceed the maximum number of lights
	if (activeLightCount > maxEntries)
		activeLightCount = maxEntries;

	// If neither the vertex shader nor the fragment shader require special
	// preprocessing, always use the shader compiled for no lights.
	if (!mVertexInfo.mSpecial && !mFragmentInfo.mSpecial)
		activeLightCount = 0;

	// When shaders fail to link, they decrement the 'mLast' variable, in essence
	// forcing the next activation to try to compile this shader with less lights.
	if (activeLightCount > mLast)
		activeLightCount = mLast;

	// Find the best match in the shader list
	for (uint i = activeLightCount; ; --i)
	{
		ProgramEntry& entry	  ( mPrograms[i] );
		ShaderEntry& program  ( entry.mProgram );
		ShaderEntry& fragment ( mFragmentInfo.mSpecial ? entry.mFragment : mPrograms[0].mFragment );
		ShaderEntry& vertex	  (   mVertexInfo.mSpecial ? entry.mVertex   : mPrograms[0].mVertex   );

		// If the program hasn't been compiled, do it now
		if (program.mStatus == ShaderEntry::CompileStatus::Unknown && program.mGlID == 0)
		{
			int success (0);

#ifdef _DEBUG
			System::Log("[SHADER]  Compiling '%s' for %u lights", mName.GetBuffer(), i);
#endif

			// Compile the vertex shader
			if (vertex.mStatus == ShaderEntry::CompileStatus::Unknown && mVertexInfo.mSource.IsValid())
			{
				::PreprocessCompile(mVertexInfo.mPath.IsValid() ? mVertexInfo.mPath :
					(mName + " (Vertex)"), vertex, mVertexInfo,
					Type::Vertex, mVertexInfo.mSpecial ? i : 0);

				if (vertex.mStatus == ShaderEntry::CompileStatus::Error && vertex.mGlID != 0)
				{
					glDeleteShader(vertex.mGlID);
					vertex.mGlID = 0;
				}
			}
#ifdef _DEBUG
			else System::Log("          - No vertex shader");
#endif

			// Compile the fragment shader
			if (fragment.mStatus == ShaderEntry::CompileStatus::Unknown && mFragmentInfo.mSource.IsValid())
			{
				::PreprocessCompile(mFragmentInfo.mPath.IsValid() ? mFragmentInfo.mPath :
					(mName + " (Fragment)"), fragment, mFragmentInfo, Type::Fragment,
					mFragmentInfo.mSpecial ? i : 0);

				if (fragment.mStatus == ShaderEntry::CompileStatus::Error && fragment.mGlID != 0)
				{
					glDeleteShader(fragment.mGlID);
					fragment.mGlID = 0;
				}
			}
#ifdef _DEBUG
			else System::Log("          - No fragment shader");
#endif

			// Add up the results
			if		(vertex.mStatus	== ShaderEntry::CompileStatus::Success)	++success;
			else if (vertex.mStatus	== ShaderEntry::CompileStatus::Error)	success -= 100;
			if		(fragment.mStatus	== ShaderEntry::CompileStatus::Success) ++success;
			else if (fragment.mStatus	== ShaderEntry::CompileStatus::Error)	success -= 100;

			// Only continue if there is something to work with
			if (success > 0)
			{
				String debug;

				// Try to link the shaders together -- this step may fail if shaders exceed
				// the allowed resources (number of variables, for example)
				if ( ::AttachAndLink(debug, program.mGlID, vertex.mGlID, fragment.mGlID) )
				{
					updateUniforms = true;
					program.mStatus = ShaderEntry::CompileStatus::Success;

					// Set the values of texture units in the shader
					::SetUniform1i(program.mGlID, "R5_texture0", 0);
					::SetUniform1i(program.mGlID, "R5_texture1", 1);
					::SetUniform1i(program.mGlID, "R5_texture2", 2);
					::SetUniform1i(program.mGlID, "R5_texture3", 3);
					::SetUniform1i(program.mGlID, "R5_texture4", 4);
					::SetUniform1i(program.mGlID, "R5_texture5", 5);
					::SetUniform1i(program.mGlID, "R5_texture6", 6);
					::SetUniform1i(program.mGlID, "R5_texture7", 7);

					CHECK_GL_ERROR;
#ifdef _DEBUG
					System::Log("          - Linked successfully");
					System::Log("          - Linker Log:\n%s", debug.GetBuffer());
#endif
				}
				else // Linking failed -- resources may have been exceeded
				{
					vertex.mStatus   = ShaderEntry::CompileStatus::Error;
					fragment.mStatus = ShaderEntry::CompileStatus::Error;
					program.mStatus  = ShaderEntry::CompileStatus::Error;

					// Delete the shader if it has been preprocessed, otherwise only detach it
					if (  mVertexInfo.mSpecial || i == 0)	::DetachAndDelete( program.mGlID, vertex.mGlID );
					else									::DetachShader	 ( program.mGlID, vertex.mGlID );
					if (mFragmentInfo.mSpecial || i == 0)	::DetachAndDelete( program.mGlID, fragment.mGlID );
					else									::DetachShader	 ( program.mGlID, fragment.mGlID );

#ifdef _DEBUG
					System::Log("          - FAILED to link, will not be used");
					System::Log("          - Linker Log:\n%s", debug.GetBuffer());
#else
					if (i == 0)
					{
						System::Log("[SHADER]  FAILED to link '%s'", mName.GetBuffer());
						System::Log("          - Linker Log:\n%s", debug.GetBuffer());
					}
#endif
					if (mLast > i) mLast = i;
					g_activeProgram = 0;
				}
			}
			else
			{
				// Some compiling error occured -- exit prematurely
				vertex.mStatus   = ShaderEntry::CompileStatus::Error;
				fragment.mStatus = ShaderEntry::CompileStatus::Error;
				program.mStatus  = ShaderEntry::CompileStatus::Error;

				// Mark the most basic shader as having failed as well
				mPrograms[mLast=0].mProgram.mStatus = ShaderEntry::CompileStatus::Error;

				if (vertex.mGlID)
				{
					glDeleteShader(vertex.mGlID);
					vertex.mGlID = 0;
				}

				if (fragment.mGlID)
				{
					glDeleteShader(fragment.mGlID);
					fragment.mGlID = 0;
				}

				CHECK_GL_ERROR;
				ASSERT(success == 0, "Shader compilation failure");

				// No need to keep the defective source code
				mVertexInfo.mSource.Release();
				mFragmentInfo.mSource.Release();

				// Compiling errors indicate a problem with the source code,
				// so there is no need to continuing beyond this point.
				g_activeProgram = 0;
				break;
			}
		}

		if (program.mStatus == ShaderEntry::CompileStatus::Success)
		{
			// Remember the number of lights this program was activated for
			g_activeLightCount = activeLightCount;

			// Use the program if it hasn't been used already
			if (g_activeProgram != program.mGlID)
			{
				updateUniforms = true;
				glUseProgram( g_activeProgram = program.mGlID );
				CHECK_GL_ERROR;
			}

			// Update uniform callbacks every millisecond as well, since the same shader
			// can stay enabled from one frame into the next.
			ulong current = Time::GetMilliseconds();
			if (current - mUpdateStamp > 1) updateUniforms = true;

			// List of registered uniforms
			Array<UniformEntry>& uniforms (entry.mUniforms);

			// Update all registered uniforms
			if (updateUniforms && program.mGlID != 0 && uniforms.GetSize() > 0)
			{
				mUpdateStamp = current;

				for (uint u = uniforms.GetSize(); u > 0; )
				{
					const UniformEntry& entry = uniforms[--u];

					if (entry.mGlID != -1)
					{
						::UpdateUniform(entry);
					}
				}
			}
			// Success -- exit the function
			return i;
		}

		// If the end of entries has been reached, break out
		if (i == 0) break;
	}

	// If this point was reached, the shader program is invalid
	if (g_activeProgram != 0)
	{
		glUseProgram(g_activeProgram = 0);
		g_activeLightCount = 0;
	}
	return -1;
}

//============================================================================================================
// Sets a custom flag for the shader that can be used to cache "is this uniform present?" type states
//============================================================================================================

void GLShader::SetCustomFlag (uint index, bool value)
{
	ASSERT(index < 32, "Only up to 32 flags are supported");

	if (index < 32)
	{
		if (value)
		{
			mFlag |= (1 << index);
		}
		else
		{
			mFlag &= ~(1 << index);
		}
	}
}

//============================================================================================================
// Safely activates the shader
//============================================================================================================

uint GLShader::Activate (uint activeLightCount, bool forceUpdateUniforms) const
{
	mLock.Lock();
	uint retVal = (const_cast<GLShader*>(this))->_Activate(activeLightCount, forceUpdateUniforms);
	mLock.Unlock();
	return retVal;
}

//============================================================================================================
// Deactivates the active shader
//============================================================================================================

void GLShader::Deactivate() const
{
	if ( g_activeProgram != 0 )
	{
		glUseProgram( g_activeProgram = 0 );
		g_activeLightCount = 0;
	}
}

//============================================================================================================
// Registers a uniform variable
//============================================================================================================

void GLShader::RegisterUniform (const String& name, const SetUniformDelegate& fnct, uint id)
{
	uint maxEntries = g_caps.mMaxLights + 1;
	if (mPrograms.GetSize() < maxEntries)
		mPrograms.ExpandTo(maxEntries);

	for (uint i = 0; i < mPrograms.GetSize(); ++i)
	{
		mPrograms[i].RegisterUniform(name, fnct, id);
	}
}

//============================================================================================================

void GLShader::ProgramEntry::RegisterUniform (const String& name, const SetUniformDelegate& fnct, uint id)
{
	mUniforms.Lock();
	{
		for (uint i = 0; i < mUniforms.GetSize(); ++i)
		{
			if (mUniforms[i].mName == name)
			{
				mUniforms[i].mId = id;
				mUniforms[i].mDelegate = fnct;
				mUniforms.Unlock();
				return;
			}
		}

		UniformEntry& entry = mUniforms.Expand();
		entry.mId			= id;
		entry.mName		= name;
		entry.mDelegate	= fnct;
	}
	mUniforms.Unlock();
}

//============================================================================================================
// Force-updates the value of the specified uniform
//============================================================================================================

bool GLShader::UpdateUniform (uint id, const SetUniformDelegate& fnct) const
{
	if (g_activeProgram != 0)
	{
		if (mPrograms[g_activeLightCount].mProgram.mGlID == g_activeProgram)
		{
			return mPrograms[g_activeLightCount].UpdateUniform(id, fnct);
		}
#ifdef _DEBUG
		else ASSERT(false, "Calling IShader::UpdateUniform on a shader that's not currently active!");
#endif
	}
	return true;
}

//============================================================================================================

bool GLShader::ProgramEntry::UpdateUniform (uint id, const SetUniformDelegate& fnct) const
{
	mUniforms.Lock();
	{
		for (uint i = mUniforms.GetSize(); i > 0; )
		{
			const UniformEntry& entry = mUniforms[--i];

			if (entry.mId == id)
			{
				if (entry.mGlID != -1)
				{
					if (fnct) entry.mDelegate = fnct;
					::UpdateUniform(entry);
				}
				mUniforms.Unlock();
				return true;
			}
		}
	}
	mUniforms.Unlock();
	return false;
}

//============================================================================================================
// Directly sets the source code for the shader
//============================================================================================================

void GLShader::SetSourceCode (const String& code, uint type)
{
	mLock.Lock();
	{
		mIsDirty = true;
		mSerializable = false;

		if (type == Type::Vertex)
		{
			mVertexInfo.mPath.Clear();
			mVertexInfo.mSource = code;
		}
		else
		{
			mFragmentInfo.mPath.Clear();
			mFragmentInfo.mSource = code;
		}

		// Serialization should only happen if the paths are valid
		mSerializable = mVertexInfo.mPath.IsValid() || mFragmentInfo.mPath.IsValid();
	}
	mLock.Unlock();
}

//============================================================================================================
// Sets the path where the shader's source code can be found
//============================================================================================================

void GLShader::SetSourcePath (const String& path, uint type)
{
	mLock.Lock();
	{
		mIsDirty = true;

		if (type == Type::Vertex)
		{
			if (mVertexInfo.mSource.Load(path))
			{
				 mVertexInfo.mPath = path;
				 mVertexInfo.mSpecial = ::PreprocessCheck(mVertexInfo.mSource);
			}
			else mVertexInfo.Clear();
		}
		else
		{
			if (mFragmentInfo.mSource.Load(path))
			{
				 mFragmentInfo.mPath = path;
				 mFragmentInfo.mSpecial = ::PreprocessCheck(mFragmentInfo.mSource);
			}
			else mFragmentInfo.Clear();
		}

		// Serialization should only happen if the paths are valid
		mSerializable = mVertexInfo.mPath.IsValid() || mFragmentInfo.mPath.IsValid();
	}
	mLock.Unlock();
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool GLShader::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	if (!IsValid() || forceUpdate)
	{
		mLock.Lock();
		{
			mSerializable = true;

			for (uint i = 0; i < root.mChildren.GetSize(); ++i)
			{
				const TreeNode& node  = root.mChildren[i];
				const String&	tag   = node.mTag;
				const Variable&	value = node.mValue;

				if (tag == "Serializable")
				{
					value >> mSerializable;
				}
				else if (value.IsString())
				{
					if		(tag == "Vertex")		SetSourcePath(value.AsString(), Type::Vertex);
					else if (tag == "Fragment")		SetSourcePath(value.AsString(), Type::Fragment);
				}
			}
		}
		mLock.Unlock();
	}
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool GLShader::SerializeTo (TreeNode& root) const
{
	if ( mSerializable && (mVertexInfo.mPath.IsValid() || mFragmentInfo.mPath.IsValid()) )
	{
		TreeNode& node = root.AddChild("Shader", mName);

		if (mVertexInfo.mPath.IsValid())	node.AddChild("Vertex", mVertexInfo.mPath);
		if (mFragmentInfo.mPath.IsValid())  node.AddChild("Fragment", mFragmentInfo.mPath);
	}
	return true;
}