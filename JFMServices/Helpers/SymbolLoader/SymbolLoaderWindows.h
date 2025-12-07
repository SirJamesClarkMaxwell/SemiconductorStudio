#pragma once

#include "SymbolLoader.h"

namespace utils
{
struct SymbolLoaderWindows : public SymbolLoader {
public:
    SymbolLoaderWindows(const char *path);
    void *getProcAddress(const char *symbolName) override;
private:
    void *loadSymbol(const char *symbolName) override;
};
} // namespace utils
