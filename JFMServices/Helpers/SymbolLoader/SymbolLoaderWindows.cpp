#include "SymbolLoaderWindows.h"
#include "utils.hpp"

namespace utils
{
SymbolLoaderWindows::SymbolLoaderWindows(const char *libraryPath)
    : SymbolLoader(libraryPath)
{
    JFM_ASSERT("Not implemented" == 0);
}

void *SymbolLoaderWindows::loadSymbol(const char *symbolName)
{
    JFM_ASSERT("Not implemented" == 0);
	return NULL;
}

void *SymbolLoaderWindows::getProcAddress(const char *symbolName)
{
    JFM_ASSERT("Not implemented" == 0);
    return NULL;
}
} // namespace utils
