// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.


#ifndef __SLATED3DVERTEXBUFFER_H__
#define __SLATED3DVERTEXBUFFER_H__

/** 
 * Vertex buffer containing all Slate vertices
 */
class FSlateD3DVertexBuffer
{
public:
	FSlateD3DVertexBuffer();
	~FSlateD3DVertexBuffer();

	/** Initializes the vertex buffers RHI resource. */
	void CreateBuffer( uint32 InStride );

	/** Releases the vertex buffers RHI resource. */
	void DestroyBuffer();

	/** Returns the size of the buffer in bytes. */
	uint32 GetBufferSize() const { return BufferSize; }

	/** Resizes the buffer to the passed in size.  Preserves internal data*/
	void ResizeBuffer( uint32 NewSize );

	void* Lock( uint32 Offset );
	void Unlock();

	TRefCountPtr<ID3D11Buffer> GetResource() { return Buffer; }
private:
	/** The size of the buffer in bytes. */
	uint32 BufferSize;
	uint32 Stride;
	TRefCountPtr<ID3D11Buffer> Buffer;
	/** Hidden copy methods. */
	FSlateD3DVertexBuffer( const FSlateD3DVertexBuffer& );
	void operator=(const FSlateD3DVertexBuffer& );
};

#endif //__SLATED3DVERTEXBUFFER_H__