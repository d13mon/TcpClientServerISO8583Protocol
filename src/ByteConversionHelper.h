#ifndef BYTECONVERSION_H
#define BYTECONVERSION_H

#include <array>
#include <vector>
#include <string>
#include <algorithm>
#include <type_traits>
#include <iostream>

using byte_ = unsigned char;
using vbytes = std::vector<byte_>;


template<typename T>
vbytes to_bytes(const T& object, size_t size = 0)
{
	auto size_ = (size == 0) ? sizeof(T) : size;

	vbytes bytes;
	bytes.resize(size_);

	const byte_* begin = reinterpret_cast<const byte_*>(std::addressof(object));
	const byte_* end = begin + size_;
	std::copy(begin, end, std::begin(bytes));

	return bytes;
}

vbytes to_bytes(const std::string& str);


template<typename T>
T& from_bytes(const vbytes& vbytes, T& object)
{
    //NOTE: http://en.cppreference.com/w/cpp/types/is_trivially_copyable
	static_assert(std::is_trivially_copyable<T>::value, "Not a TriviallyCopyable type");

	byte_* begin_object = reinterpret_cast<byte_*>(std::addressof(object));
	std::copy(std::begin(vbytes), std::end(vbytes), begin_object);

	return object;
}

std::string& from_bytes(const vbytes& vbytes, std::string& str);





#endif