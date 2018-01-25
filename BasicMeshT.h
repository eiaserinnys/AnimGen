#pragma once

#include <Utility.h>

//------------------------------------------------------------------------------
template <
	typename BaseClass,
	typename PositionType,
	typename IndexType>
class BasicMeshT : public BaseClass {
public:
	virtual void Append(typename BaseClass::MeshType* mesh)
	{
		UINT16 pivot = (UINT16)pos.size();
		AppendStream(pos, mesh->Vertices());

		ind.reserve(ind.size() + mesh->Indices().second);

		if (pivot > 0)
		{
			for (UINT i = 0; i < mesh->Indices().second; ++i)
			{
				ind.push_back(mesh->Indices().first[i] + pivot);
			}
		}
		else
		{
			AppendStream(ind, mesh->Indices());
		}
	}

	const std::pair<const PositionType*, UINT> Vertices() const
	{ 
		return StreamContent(pos); 
	}
		
	const std::pair<const IndexType*, UINT> Indices() const 
	{ 
		return StreamContent(ind); 
	}

protected:
	template <typename ElementType>
	void AppendStream(
		std::vector<ElementType>& dst,
		const std::pair<const ElementType*, UINT>& src)
	{
		if (src.first != nullptr&& src.second > 0)
		{
			dst.reserve(dst.size() + src.second);
			dst.insert(dst.end(), src.first, src.first + src.second);
		}
	}

	template <typename ElementType>
	std::pair<const ElementType*, UINT> StreamContent(
		const std::vector<ElementType>& stream) const
	{
		return std::make_pair(
			stream.empty() ? nullptr : &stream[0],
			(UINT)stream.size());
	}

protected:
	std::vector<PositionType> pos;
	std::vector<IndexType> ind;
};
