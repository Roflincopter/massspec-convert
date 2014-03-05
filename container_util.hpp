#pragma once

#include <algorithm>
#include <iterator>
#include <iostream>

template <typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T> const& vec)
{
	std::copy(vec.cbegin(), vec.cend() - 1, std::ostream_iterator<T>(os, ", "));
	os << vec.back();
	return os;
}