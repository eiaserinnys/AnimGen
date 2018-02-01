#pragma once

template <typename V, int>
struct VectorDescriptor;

template <typename V>
struct VectorDescriptor<V, 2>
{
	union
	{
		struct { V x, y; };
		V m[2];
	};
};

template <typename V>
struct VectorDescriptor<V, 3>
{
	union
	{
		struct { V x, y, z; };
		V m[3];
	};
};

template <typename V>
struct VectorDescriptor<V, 4>
{
	union
	{
		struct { V x, y, z, w; };
		V m[4];
	};
};
