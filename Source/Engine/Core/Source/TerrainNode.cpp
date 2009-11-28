#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Should create the node's topology and update 'mBounds'
//============================================================================================================

void TerrainNode::OnFill (void* ptr, float bboxPadding)
{
	Terrain* terrain = (Terrain*)mTree;
	IGraphics* graphics = mTree->GetCore()->GetGraphics();

	if (graphics != 0)
	{
		// Passed parameters used to generate the terrain
		const Terrain::Heightmap* hm	= (const Terrain::Heightmap*)ptr;
		const float*	buffer			= hm->mBufferData;
		const uint		bufferWidth		= hm->mBufferSize.x;
		const uint		bufferHeight	= hm->mBufferSize.y;
		const uint		meshWidth		= hm->mMeshSize.x;
		const uint		meshHeight		= hm->mMeshSize.y;
		const Vector3f& terrainScale	= hm->mTerrainScale;
		const Vector3f& terrainOffset	= hm->mTerrainOffset;

		// Tile's offset and size in index space
		Vector2f indexOffset (mOffset.x * bufferWidth, mOffset.y * bufferHeight);
		Vector2f indexSize   (mSize.x   * bufferWidth, mSize.y   * bufferHeight);

		uint defaultX = Float::RoundToUInt(indexSize.x);
		uint defaultY = Float::RoundToUInt(indexSize.y);

		// Tile's first and last indices
		uint firstX = Float::RoundToUInt(indexOffset.x);
		uint firstY = Float::RoundToUInt(indexOffset.y);
		uint lastX  = Float::RoundToUInt(indexOffset.x + indexSize.x);
		uint lastY  = Float::RoundToUInt(indexOffset.y + indexSize.y);

		// Don't allow the indices to exceed the limits
		if (lastX >= bufferWidth)  lastX = bufferWidth  - 1;
		if (lastY >= bufferHeight) lastY = bufferHeight - 1;

		// Actual number of vertices is always one higher
		uint vertsX = meshWidth  + 1;
		uint vertsY = meshHeight + 1;

		Memory mem;

		// Generate all the vertices
		{
			// Offset in world space
			Vector2f worldOffset (
				mOffset.x * terrainScale.x + terrainOffset.x,
				mOffset.y * terrainScale.y + terrainOffset.y );

			// Each quad's size in world space
			Vector2f worldSize(
				(1.0f / bufferWidth ) * terrainScale.x,
				(1.0f / bufferHeight) * terrainScale.y );

			// Ratio of actual size to desired size
			Vector2f ratio ((float)(lastX - firstX) / meshWidth,
							(float)(lastY - firstY) / meshHeight);

			// Scaled value that will convert vertex iterators below into size in world space
			Vector2f scaledWorld (ratio.x * worldSize.x, ratio.y * worldSize.y);

			// Scaled value that will convert vertex iterators below into 0-1 relative offset space
			Vector2f scaledRelative (mSize.x / meshWidth, mSize.y / meshHeight);

			// Create a temporary buffer
			Vector3f* v = (Vector3f*)mem.Resize(vertsX * vertsY * 12);

			float fx, fy, wx, wy, wz;

			// Fill the buffer with vertices
			for (unsigned int y = 0; y < vertsY; ++y)
			{
				wy = worldOffset.y + scaledWorld.y * y;
				fy = mOffset.y + scaledRelative.y * y;

				for (unsigned int x = 0; x < vertsX; ++x, ++v)
				{
					wx = worldOffset.x + scaledWorld.x * x;
					fx = mOffset.x + scaledRelative.x * x;

					// Sample the buffer using bilinear filtering
					wz = Interpolation::BicubicClamp(buffer, bufferWidth, bufferHeight, fx, fy);

					// Set the vertex
					v->Set(wx, wy, wz * terrainScale.z);

					// Include this vertex in the node's bounds
					mBounds.Include(*v);
				}
			}

			// Take padding into consideration
			if (bboxPadding > 0.0f)
			{
				Vector3f min (mBounds.GetMin());
				Vector3f max (mBounds.GetMax());

				min.y -= bboxPadding;
				max.y += bboxPadding;

				mBounds.Include(min);
				mBounds.Include(max);
			}

			if (mVBO == 0) mVBO = graphics->CreateVBO();
			
			// Fill the VBO
			mVBO->Lock();
			mVBO->Set(mem.GetBuffer(), mem.GetSize(), IVBO::Type::Vertex);
			mVBO->Unlock();
		}

		// Generate the indices
		{
			mIndices = meshWidth * meshHeight * 4;
			ushort* index = (ushort*)mem.Resize(mIndices * 2);

			// Fill the index array using quad primitives
			for (unsigned int y = 0; y < meshHeight; ++y)
			{
				unsigned int y0 = vertsX * y;
				unsigned int y1 = vertsX * (y + 1);

				for (unsigned int x = 0; x < meshWidth; ++x)
				{
					*index = y1 + x;		++index;
					*index = y0 + x;		++index;
					*index = y0 + x + 1;	++index;
					*index = y1 + x + 1;	++index;
				}
			}

			if (mIBO == 0) mIBO = graphics->CreateVBO();

			mIBO->Lock();
			mIBO->Set(mem.GetBuffer(), mem.GetSize(), IVBO::Type::Index);
			mIBO->Unlock();
		}
	}
}

//============================================================================================================
// Draw the object using the specified technique
//============================================================================================================

uint TerrainNode::OnDraw (IGraphics* graphics, const ITechnique* tech, bool insideOut)
{
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position,
		mVBO, 0, IGraphics::DataType::Float, 3, 12 );

	return graphics->DrawIndices(mIBO, IGraphics::Primitive::Quad, mIndices);
}