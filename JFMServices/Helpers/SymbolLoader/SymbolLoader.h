#pragma once

#include <map>
#include <vector>

namespace utils
{
enum class PlatformType
{
    Nix,
    Windows,
    Count
};

struct SymbolLoader {
public:
    static SymbolLoader *Create(const char *libraryPath);

    virtual void *getProcAddress(const char *symbolName) = 0;

protected:
    SymbolLoader(const char *libraryPath)
        : m_libraryPath(libraryPath)
    {
    }

    virtual void *loadSymbol(const char *symbolName) = 0;

    const char *m_libraryPath;
    void *m_handle;

    std::map<const char *, void *> m_symbolTable;
    std::vector<const char *> m_symbolNames =
    {
        "calculatorGpuCall"
    };

private:
    void loadSymbolTable();
};
} // namespace utils
