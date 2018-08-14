#pragma once
#include <vector>
#include <functional>

template<
	class T,
	class Container = std::vector<T>,
	class Compare = std::less<typename Container::value_type>
>
class PriorityQueue
{
public:
	typedef Container							ContainerType;
	typedef Compare								ValueCompare;
	typedef typename Container::value_type		ValueType;
	typedef typename Container::size_type		SizeType;
	typedef typename Container::reference		Reference;
	typedef typename Container::const_reference	ConstReference;

protected:
	Container	_c;
	Compare		_comp;

	inline SizeType _Parent(SizeType idx) { return (idx - 1) / 2; }
	inline SizeType _Left(SizeType idx) { return idx * 2 + 1; }
	inline SizeType _Right(SizeType idx) { return idx * 2 + 2; }

	inline void _Heapify(SizeType idx)
	{
		SizeType l = _Left(idx), r = _Right(idx), t = idx;
		for (;;)
		{
			SizeType size = _c.size();
			if (l < size && _comp(_c.at(idx), _c.at(l)))
				t = l;
			if (r < size && _comp(_c.at(t), _c.at(r)))
				t = r;
			if (t != idx)
			{
				std::swap(_c.at(t), _c.at(idx));
				idx = t;
				l = _Left(idx), r = _Right(idx);
			}
			else
				break;
		}
	}

	inline void _RHeapify(SizeType idx)
	{
		SizeType p = _Parent(idx);
		while (idx && _comp(_c.at(p), _c.at(idx)))
		{
			std::swap(_c.at(p), _c.at(idx));
			idx = p;
			p = _Parent(idx);
		}
	}

public:
	PriorityQueue()
		: PriorityQueue(Compare(), Container()) { }

	explicit PriorityQueue(const Compare & compare)
		: PriorityQueue(compare, Container()) { }

	PriorityQueue(
		const Compare & compare,
		const Container & cont)
		: _comp(compare), _c()
	{
		for (auto & i : cont) Push(i);
	}

	PriorityQueue(
		const PriorityQueue & other
	)
		: _comp(other._comp), _c(other._c)
	{ }

	PriorityQueue(
		PriorityQueue && other
	)
		: _comp(other._comp), _c(std::move(other._c))
	{ }

	~PriorityQueue() { }

	PriorityQueue & operator= (const PriorityQueue & other)
	{
		_c = other._c;
		_comp = other._comp;
		return *this;
	}

	PriorityQueue & operator= (PriorityQueue && other)
	{
		_c = std::move(other._c);
		_comp = other._comp;
		return *this;
	}

	inline ConstReference Top() const { return _c.front(); }
	inline Reference Top() { return _c.front(); }
	bool Empty() const { return Size() == 0; }
	SizeType Size() const { return _c.size(); }

	void Push(const ValueType & value)
	{
		_c.push_back(value);
		_RHeapify(_c.size() - 1);
	}
	void Push(ValueType && value)
	{
		_c.push_back(std::move(value));
		_RHeapify(_c.size() - 1);
	}
	void Pop()
	{
		_c.front() = _c.back();
		_c.pop_back();
		_Heapify(0);
	}

	template<class ... Args>
	void Emplace(Args && ... args);

	template<class Arg>
	inline void Emplace(Arg && arg) { Push(std::move(arg)); }

	template<class Arg, class ... Args>
	inline void Emplace(Arg && arg, Args && ... args)
	{
		Push(std::move(arg)); Emplace(std::move(args)...);
	}
};
