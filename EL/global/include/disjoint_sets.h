#pragma once

#include <map>

template <class Ty>
class disjoint_sets {
public:
	typedef Ty value_type;
	typedef unsigned rank_type;
	typedef std::map<value_type, value_type> ParentContainer;
	typedef std::map<value_type, rank_type> RankContainer;
	
	void make_set(const value_type& x) {
		setparent(x, x);
		setrank(x, 1);
	}

	value_type find_set(const value_type& x) {
		return path_compression(x);
	}

	void union_set(const value_type& x, const value_type& y) {
		link(find_set(x), find_set(y));
	}

	size_t count_sets() const {
		size_t r = 0;
		for(ParentContainer::const_iterator ite = parent.begin();
			ite != parent.end(); ++ite) {
			r += (size_t)(ite->first == ite->second);
		}

		return r;
	}
private:
	void link(const value_type& x, const value_type& y) {
		if(x == y) return;
		rank_type rx = getrank(x), ry = getrank(y);
		if(rx < ry)
			setparent(x, y);
		else
			setparent(y, x);

		if(rx == ry)
			setrank(x, getrank(x) + 1);
	}

	value_type path_compression(const value_type& x) {
		if(getparent(x) != x)
			setparent(x, path_compression(getparent(x)));
		return getparent(x);
	}

	void setparent(const value_type& x, const value_type y) {
		parent[x] = y;
	}

	const value_type& getparent(const value_type& x) {
		return parent[x];
	}

	void setrank(const value_type& x, rank_type r) {
		rank[x] = r;
	}

	rank_type getrank(const value_type& x) {
		return rank[x];
	}

	ParentContainer parent;
	RankContainer rank;
};
/// ****** example
/*
int main() {
	disjoint_sets<int> sets;
	sets.make_set(0);
	sets.make_set(1);
	sets.make_set(2);

	printf("found %u set(s).\n", sets.count_sets());
	sets.union_set(0, 1);
	printf("found %u set(s).\n", sets.count_sets());
}*/
/// *****
