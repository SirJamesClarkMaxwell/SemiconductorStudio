#pragma once

#include "SymbolLoader.h"

namespace utils
{
struct SymbolLoaderNix : public SymbolLoader {
public:
    SymbolLoaderNix(const char *path);
    void *getProcAddress(const char *symbolName) override;
private:
    void *loadSymbol(const char *symbolName) override;
};
} // namespace utils
