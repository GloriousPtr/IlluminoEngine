#pragma once

#include <memory>

namespace IlluminoEngine
{
	class String
	{
	public:
		String() = default;

		String(const char* string)
		{
			m_Size = strlen(string);
			m_Data = new char[m_Size + 1];
			memcpy(m_Data, string, m_Size);
			m_Data[m_Size] = '\0';
		}

		~String()
		{
			delete m_Data;
		}

		String(const String& other)
		{
			m_Size = other.m_Size;
			m_Data = new char[m_Size + 1];
			memcpy(m_Data, other.m_Data, m_Size);
			m_Data[m_Size] = '\0';
		}

		String(String&& other)
		{
			m_Size = other.m_Size;
			m_Data = other.m_Data;

			other.m_Size = 0;
			other.m_Data = nullptr;
		}

		operator const char*() const { return m_Data; } 

	private:
		char* m_Data;
		uint32_t m_Size;
	};
}
