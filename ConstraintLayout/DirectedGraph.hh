#pragma once
#include <vector>
#include <utility>
#include <limits>
#include "PriorityQueue.hh"

template<
	class T,
	template<class, class ...> class Container = std::vector,
	class SizeType = std::size_t,
	class WeightType = std::ptrdiff_t
>
class DirectedGraph
{
	template<class, template<class, class ...> class, class, class>
	friend class DirectedGraph;

public:
	struct Edge {
		friend DirectedGraph;
	protected:
		SizeType _source;
	public:
		SizeType destination;
		WeightType weight;
		Edge()
			: _source(), destination(), weight()
		{ }
		Edge(SizeType source, SizeType destination, WeightType weight = 0)
			: _source(source), destination(destination), weight(weight)
		{ }
		inline SizeType Source() { return _source; }
	};
	typedef Container<Edge>			EdgeContainerType;
	typedef const Container<Edge>	ConstEdgeContainerType;
	typedef T						ValueType;
	typedef Container<ValueType>	ValueContainerType;
	struct Vertex {
		friend DirectedGraph;
	protected:
		SizeType _index;
	public:
		ValueType value;
		Vertex()
			: _index(), value()
		{ }
		Vertex(SizeType index, const ValueType & value)
			: _index(index), value(value)
		{ }
		Vertex(SizeType index, ValueType && value)
			: _index(index), value(std::move(value))
		{ }
		inline SizeType Index() { return _index; }
	};
	typedef Container<Vertex>								VertexContainerType;
	typedef typename VertexContainerType::reference			Reference;
	typedef typename VertexContainerType::const_reference	ConstReference;

	static constexpr SizeType InvalidVertex	= std::numeric_limits<SizeType>::max();
	static constexpr WeightType Infinity	= std::numeric_limits<WeightType>::max();

protected:
	VertexContainerType				_vertices;
	Container<EdgeContainerType>	_edges;
	SizeType						_edgesSize;

	inline void _AllocateEdges()
	{
		_edges = Container<EdgeContainerType>(_vertices.size());
	}
	inline void _AddVertex(const ValueType & value)
	{
		_vertices.push_back(Vertex(_vertices.size(), value));
	}
	inline void _AddVertex(ValueType && value)
	{
		_vertices.push_back(Vertex(_vertices.size(), std::move(value)));
	}
	inline void _AddVertices(const ValueContainerType & vertices)
	{
		for (auto & vertex : vertices)
			_AddVertex(vertex);
	}
	inline void _AddVertices(ValueContainerType && vertices)
	{
		for (auto && vertex : vertices)
			_AddVertex(std::move(vertex));
	}
	inline void _AddEdge(const Edge & edge)
	{
		_edges[edge._source].push_back(edge);
		_edgesSize++;
	}

	inline void _AddEdges(const EdgeContainerType & edges)
	{
		for (auto & edge : edges)
			_AddEdge(edge);
	}

	void _Search(SizeType idx, Container<bool> & visited, Container<SizeType> & rtn)
	{
		if (visited[idx])
			return;
		visited[idx] = true;
		for(auto & i : _edges[idx])
			if (!visited[i.destination])
				_Search(i.destination, visited, rtn);
		rtn.push_back(idx);
	}
public:
	DirectedGraph()
		: DirectedGraph(0)
	{ }

	DirectedGraph(
		SizeType num_vertices,
		const EdgeContainerType & edges = EdgeContainerType()
	)
		: _edges(num_vertices), _edgesSize(0)
	{
		for (SizeType i = 0; i < num_vertices; i++)
			_AddVertex(ValueType());
		_AddEdges(edges);
	}
	
	DirectedGraph(
		const ValueContainerType & vertices,
		const EdgeContainerType & edges = EdgeContainerType()
	)
		: _edges(vertices.size()), _edgesSize(0)
	{
		_AddVertices(vertices);
		_AddEdges(edges);
	}

	DirectedGraph(
		ValueContainerType && vertices,
		const EdgeContainerType & edges = EdgeContainerType()
	)
		: _edges(vertices.size()), _edgesSize(0)
	{
		_AddVertices(std::move(vertices));
		_AddEdges(edges);
	}
	
	DirectedGraph(
		const DirectedGraph & other
	)
		: _vertices(other._vertices), _edges(other._edges), _edgesSize(other._edgesSize)
	{ }

	DirectedGraph(
		DirectedGraph && other
	)
		: _vertices(std::move(other._vertices)), _edges(std::move(other._edges)),
		_edgesSize(other._edgesSize)
	{
		other._edgesSize = 0;
	}

	~DirectedGraph() { }

	DirectedGraph & operator= (const DirectedGraph & other)
	{
		_vertices = other._vertices;
		_edges = other._edges;
		return *this;
	}

	DirectedGraph & operator= (DirectedGraph && other)
	{
		_vertices = std::move(other._vertices);
		_edges = std::move(other._edges);
		return *this;
	}

	inline ConstReference VertexAt(SizeType idx) const { return _vertices.at(idx); }
	inline Reference VertexAt(SizeType idx) { return _vertices.at(idx); }
	inline SizeType VerticesSize() const { return _vertices.size(); }
	inline bool VerticesEmpty() const { return VerticesSize() == 0; }
	inline SizeType EdgesSize() const { return _edgesSize; }
	inline bool EdgesEmpty() const { return _edgesSize == 0; }
	
	inline void PushVertex(const ValueType & new_vertex)
	{
		_edges.push_back(EdgeContainerType());
		_vertices.push_back(Vertex(_vertices.size(), new_vertex));
	}
	inline void PushVertex(ValueType && new_vertex)
	{
		_edges.push_back(EdgeContainerType());
		_vertices.push_back(Vertex(_vertices.size(), std::move(new_vertex)));
	}
	inline void PushEdge(const Edge & edge) { _AddEdge(edge); }

	inline void PopVertex() { _vertices.pop_back(); }
	inline ConstEdgeContainerType & EdgesFrom(SizeType idx) const { return _edges.at(idx); }
	inline EdgeContainerType & EdgesFrom(SizeType idx) { return _edges.at(idx); }

	inline auto begin() { return _vertices.begin(); }
	inline auto end() { return _vertices.end(); }
	inline auto rbegin() { return _vertices.rbegin(); }
	inline auto rend() { return _vertices.rend(); }
	inline auto cbegin() { return _vertices.cbegin(); }
	inline auto cend() { return _vertices.cend(); }
	inline auto crbegin() { return _vertices.crbegin(); }
	inline auto crend() { return _vertices.crend(); }

	DirectedGraph Tranpose()
	{
		DirectedGraph rtn;
		rtn._vertices = _vertices;
		rtn._AllocateEdges();
		for (auto & edges : _edges) for (auto & edge : edges)
			rtn._AddEdge(Edge(edge.destination, edge._source, edge.weight));
		return rtn;
	}
	
	Container<SizeType> Search(SizeType source = 0)
	{
		if (VerticesEmpty())
			return Container<SizeType>();
		Container<SizeType> rtn;
		Container<bool> visited(VerticesSize());
		_Search(source, visited, rtn);
		for (SizeType i = 0; i < VerticesSize(); i++)
		{
			_Search(i, visited, rtn);
		}
		return rtn;
	}

	struct DijkstraType
	{
		Container<SizeType> path;
		WeightType weight;
	};

	template< template<class, class ...> class QueueType = PriorityQueue>
	DijkstraType Dijkstra(SizeType source, SizeType destination)
	{
		SizeType size = VerticesSize();
		Container<SizeType> parents(size);
		Container<WeightType> weights(size);
		struct DijkstraInfo
		{
			SizeType position;
			WeightType weight;
		};
		for (SizeType i = 0; i < size; i++)
		{
			parents[i] = InvalidVertex;
			weights[i] = Infinity;
		}
		weights[source] = 0;

		std::function<bool(const DijkstraInfo &, const DijkstraInfo &)> comp =
			[](const DijkstraInfo & l, const DijkstraInfo & r)
			{ return l.weight > r.weight; };

		QueueType<DijkstraInfo, std::vector<DijkstraInfo>, decltype(comp)> queue(comp);
		queue.Push({ source, 0 });

		while (!queue.Empty())
		{
			DijkstraInfo info = queue.Top(); queue.Pop();
			if (info.weight > weights[info.position])
				continue;

			for (auto & i : _edges[info.position])
			{
				if (weights[i.destination] > info.weight + i.weight)
				{
					weights[i.destination] = info.weight + i.weight;
					parents[i.destination] = info.position;
					queue.Push({ i.destination, weights[i.destination] });
				}
			}
		}

		if (weights[destination] == Infinity)
			return { Container<SizeType>(), Infinity };

		Container<SizeType> rtn_tmp;
		SizeType current = destination;
		while (current != source)
		{
			rtn_tmp.push_back(current);
			current = parents[current];
		}
		Container<SizeType> rtn { source };
		for (auto it = rtn_tmp.rbegin(); it != rtn_tmp.rend(); it++)
			rtn.push_back(*it);

		return { rtn, weights[destination] };
	}

	struct BellmanFordType
	{
		Container<SizeType> parents;
		Container<WeightType> weights;
		bool hasNegativeCycle;
	};
	
	BellmanFordType BellmanFord(SizeType source)
	{
		SizeType size = VerticesSize();
		Container<SizeType> parents(size);
		Container<WeightType> weights(size);

		for (SizeType i = 0; i < size; i++)
		{
			parents[i] = InvalidVertex;
			weights[i] = Infinity;
		}
		weights[source] = 0;

		for (SizeType _iter = 0; _iter < size - 1; _iter++)
		{
			for (auto & edges : _edges) for (auto & i : edges)
			{
				if (weights[i._source] != Infinity
					&& weights[i.destination] > weights[i._source] + i.weight)
				{
					weights[i.destination] = weights[i._source] + i.weight;
					parents[i.destination] = i._source;
				}
			}
		}

		bool has_loop = false;
		for (auto & edges : _edges) for (auto & i : edges)
		{
			if (weights[i._source] != Infinity
				&& weights[i.destination] > weights[i._source] + i.weight)
			{
				has_loop = true;
				break;
			}
		}

		return { parents, weights, has_loop };
	}

	Container<Container<WeightType>> FloydWarshall()
	{
		Container<Container<WeightType>> rtn(VerticesSize(),
			Container<WeightType>(VerticesSize()));

		for (SizeType i = 0; i < VerticesSize(); i++)
			for (SizeType j = 0; j < VerticesSize(); j++)
				rtn[i][j] = Infinity;

		for (auto & edges : _edges) for (auto & i : edges)
			rtn[i._source][i.destination] = i.weight;

		for (SizeType i = 0; i < VerticesSize(); i++)
			rtn[i][i] = 0;
		
		for (SizeType i = 0; i < VerticesSize(); i++)
			for (SizeType j = 0; j < VerticesSize(); j++)
				for (SizeType k = 0; k < VerticesSize(); k++)
					if (rtn[i][j] != Infinity && rtn[j][k] != Infinity
						&& rtn[i][k] > rtn[i][j] + rtn[j][k])
						rtn[i][k] = rtn[i][j] + rtn[j][k];

		return rtn;
	}

	typedef DirectedGraph<Container<ValueType>, Container, SizeType, WeightType> StronglyConnectedType;

	StronglyConnectedType StronglyConnected()
	{
		StronglyConnectedType rtn;
		auto dfs = Search();
		auto trn = Tranpose();
		Container<SizeType> group(VerticesSize());
		Container<bool> visited(VerticesSize());
		for (auto it = dfs.rbegin(); it != dfs.rend(); it++)
		{
			Container<SizeType> vert;
			trn._Search(*it, visited, vert);
			if (!vert.empty())
			{
				SizeType vert_idx = rtn.VerticesSize();
				Container<ValueType> vert_value;
				for (auto & i : vert)
					vert_value.push_back(_vertices[i].value);
				rtn.PushVertex(std::move(vert_value));
				for (auto & i : vert)
					group[i] = vert_idx;
			}
		}
		rtn._AllocateEdges();
		for (auto & edges : _edges)
		{
			for (auto & i : edges)
			{
				SizeType sourceGroup = group[i._source];
				SizeType destinationGroup = group[i.destination];
				WeightType weight = i.weight;
				rtn.PushEdge(typename StronglyConnectedType::Edge(sourceGroup, destinationGroup, weight));
			}
		}
		return rtn;
	}
	
	void Clear()
	{
		_vertices.clear();
		_edges.clear();
		_edgesSize = 0;
	}

};