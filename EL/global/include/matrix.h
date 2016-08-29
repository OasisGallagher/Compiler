#pragma once
#include <map>
#include <functional>

template <class Kty1, class Kty2, class Ty>
class matrix {
public:
	typedef Kty1 first_key_type;
	typedef Kty2 second_key_type;
	typedef std::pair<first_key_type, second_key_type> key_type;
	typedef Ty value_type;

	typedef struct KeyComparer : public std::binary_function <key_type, key_type, bool > {
		bool operator () (const key_type& lhs, const key_type& rhs) const {
			if (lhs.first == rhs.first) {
				return lhs.second < rhs.second;
			}

			return lhs.first < rhs.first;
		}
	} comparer_type;

	typedef std::map<key_type, value_type, comparer_type> container_type;
	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;

public:
	value_type& at(const first_key_type& k1, const second_key_type& k2) {
		return cont_[std::make_pair(k1, k2)];
	}

	value_type& operator () (const first_key_type& k1, const second_key_type& k2) {
		return cont_[std::make_pair(k1, k2)];
	}

	iterator find(const first_key_type& k1, const second_key_type& k2) {
		return cont_.find(std::make_pair(k1, k2));
	}

	const_iterator begin() const {
		return cont_.begin();
	}

	const_iterator end() const {
		return cont_.end();
	}

	void clear() {
		cont_.clear();
	}

	std::string ToString() const;

protected:
	container_type cont_;
};
