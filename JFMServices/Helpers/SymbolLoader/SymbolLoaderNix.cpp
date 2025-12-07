#include "SymbolLoaderNix.h"
#include <dlfcn.h>
#include "utils.hpp"

namespace utils
{
SymbolLoaderNix::SymbolLoaderNix(const char *libraryPath)
    : SymbolLoader(libraryPath)
{
    JFM_ASSERT(libraryPath != nullptr);
    m_handle = dlopen(libraryPath, RTLD_GLOBAL | RTLD_NOW);
    JFM_ASSERT(m_handle != nullptr);
}

void *SymbolLoaderNix::loadSymbol(const char *symbolName)
{
    JFM_ASSERT(m_handle);
    return dlsym(m_handle, symbolName);
}

void *SymbolLoaderNix::getProcAddress(const char *symbolName)
{
    return m_symbolTable[symbolName];
}
} // namespace utils
