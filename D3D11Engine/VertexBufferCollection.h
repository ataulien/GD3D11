#pragma once
#include "pch.h"
#include "BaseVertexBuffer.h"
#include "Engine.h"
#include "BaseGraphicsEngine.h"

class BaseVertexBuffer;
template <typename T>
class VertexBufferCollection
{
public:
	VertexBufferCollection(void)
	{
		Buffer = NULL;
	}

	~VertexBufferCollection(void)
	{
		delete Buffer;
	}

	/** Adds more data to the buffer. Returns offset in units. */
	unsigned int AddData(T* data, unsigned int num)
	{
		for(unsigned int i=0;i<num;i++)
			TempDataStorage.push_back(data[i]);

		return TempDataStorage.size() - num;
	}

	/** Adds more data to the buffer. Returns offset in units. */
	unsigned int AddDataDeref(const T** data, unsigned int num)
	{
		for(unsigned int i=0;i<num;i++)
			TempDataStorage.push_back(*data[i]);

		return TempDataStorage.size() - num;
	}

	/** Adds more data to the buffer. Returns offset in units. */
	unsigned int AddDataDerefLock(const T** data, unsigned int num)
	{
		static std::mutex m;

		m.lock();
		for(unsigned int i=0;i<num;i++)
			TempDataStorage.push_back(*data[i]);

		int pos = TempDataStorage.size() - num;
		m.unlock();

		return pos;
	}

	/** Adds more data to the buffer. Returns offset in units. */
	template<typename S>
	unsigned int AddDataCast(S* data, unsigned int num)
	{
		for(unsigned int i=0;i<num;i++)
			TempDataStorage.push_back((T)data[i]);

		return TempDataStorage.size() - num;
	}

	/** Has to be called after all the data has been placed into the collection */
	void Construct(BaseVertexBuffer::EBindFlags bindFlags=BaseVertexBuffer::B_VERTEXBUFFER, 
		BaseVertexBuffer::EUsageFlags usage=BaseVertexBuffer::U_IMMUTABLE, 
		BaseVertexBuffer::ECPUAccessFlags cpuAccess=BaseVertexBuffer::CA_NONE)
	{
		if(!Buffer || Buffer->GetSizeInBytes() < TempDataStorage.size() * sizeof(T))
		{
			if(TempDataStorage.empty())
				return;

			delete Buffer;
			Engine::GraphicsEngine->CreateVertexBuffer(&Buffer);
			XLE(Buffer->Init(&TempDataStorage[0],
				TempDataStorage.size() * sizeof(T),
				bindFlags,
				usage,
				cpuAccess));
		}
		else
		{
			Buffer->UpdateBuffer(&TempDataStorage[0], TempDataStorage.size() * sizeof(T));
		}

		TempDataStorage.clear();

		if(usage == BaseVertexBuffer::U_IMMUTABLE)
			TempDataStorage.shrink_to_fit(); // Clear memory as well if we never want to update the buffer
	}

	/** Returns the buffer the data is in */
	BaseVertexBuffer* GetBuffer()
	{
		return Buffer;
	}

private:
	/** Buffer big enough to hold the data given to this */
	BaseVertexBuffer* Buffer;

	/** Temporary vector for collecting ingoing data */
	std::vector<T> TempDataStorage;
};

