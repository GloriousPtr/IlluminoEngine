#pragma once

#include "half.hpp"

namespace IlluminoEngine
{
	using namespace half_float::literal;

	class Half
	{
	public:
		Half() = default;
		Half(float f) { m_Data = f; }
		Half(double d) { m_Data = d; }

		operator half_float::half() const { return m_Data; }
		operator float() const { return m_Data; }
		operator double() const { return m_Data; }

	private:
		half_float::half m_Data = 0.0_h;
	};
}
