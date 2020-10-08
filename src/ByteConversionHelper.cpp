#include "ByteConversionHelper.h"

vbytes to_bytes(const std::string& str)
{
	char buffer[2048];	
	strcpy(buffer, str.data());

	return to_bytes(buffer, str.size());	
}

std::string& from_bytes(const vbytes& vbytes, std::string& str)
{
	auto size = vbytes.size();
	char buffer[2048];

	from_bytes(vbytes, buffer);	

	str = std::string(buffer, size);

	return str;
}