#pragma once

#include <memory>

#include <Bootil/Bootil.h>
#include <Bootil/Types/Buffer.h>

class Buffer
{
public:
	Buffer()
		: m_pBuffer(std::make_unique<Bootil::AutoBuffer>())
	{
	}

	explicit Buffer(int initialSize)
		:	m_pBuffer(std::make_unique<Bootil::AutoBuffer>(initialSize))
	{
	}

	Buffer(Buffer&&) noexcept = default;
	Buffer& operator=(Buffer&&) noexcept = default;

	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	Bootil::AutoBuffer& Get()
	{
		return *m_pBuffer;
	}

	const Bootil::AutoBuffer& Get() const
	{
		return *m_pBuffer;
	}

	void Write(const void* data, unsigned int size)
	{
		m_pBuffer->Write(data, size);
	}

	void Clear()
	{
		m_pBuffer->Clear();
	}

	void CopyTo(Bootil::AutoBuffer& destination) const
	{
		destination.Clear();
		destination.WriteBuffer(*m_pBuffer);
	}

	unsigned int GetWritten() const
	{
		return m_pBuffer->GetWritten();
	}

	void* GetBase() const
	{
		return m_pBuffer->GetBase();
	}

private:
	std::unique_ptr<Bootil::AutoBuffer> m_pBuffer;
};