#pragma once
#include <string>

/**
 * @brief Utility for converting OBJ files to STL using Assimp.
 */
class ObjConverter {
public:
    static void Convert(const std::string &inputObj, const std::string &outputStl);
};
