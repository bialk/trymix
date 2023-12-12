#pragma once

#include <string>

namespace sV2 {

char* decodeBase64InPlace(char* begin, char* end);

std::string toBase64(char const* begin, char const* end);

}
