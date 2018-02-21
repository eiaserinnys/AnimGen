#pragma once

#include <cmath>

//------------------------------------------------------------------------------
class Coefficient {
public:
	Coefficient() {}
	explicit Coefficient(double v) { Set(v); }

	Coefficient& operator = (double v) { Set(v); return *this; }
	Coefficient& operator *= (double v) { Set(w * v); return *this; }
	Coefficient& operator /= (double v) { Set(w / v); return *this; }

	bool operator <= (double v) const { return w <= v; }

	void Set(double v) { w = v; w_sqrt = std::sqrt(v); }

	operator double() const { return w; }
	double Sqrt() const { return w_sqrt; }

private:
	double w = 1;
	double w_sqrt = 1;
};
