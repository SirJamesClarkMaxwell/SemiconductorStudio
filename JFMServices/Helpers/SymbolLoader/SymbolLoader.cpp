#include "SymbolLoaderNix.h"
#include "SymbolLoaderWindows.h"
#include "utils.hpp"
#include "JunctionFitMaster.hpp"

namespace utils
{
SymbolLoader *SymbolLoader::Create(const char *libraryPath)
{
    SymbolLoader *loader = nullptr;

    switch (JunctionFitMaster::platformType)
    {
    case PlatformType::Nix:
        loader = new SymbolLoaderNix(libraryPath);
        break;
    case PlatformType::Windows:
        loader = new SymbolLoaderWindows(libraryPath);
        break;
    default:
        Unreachable();
    }
    JFM_ASSERT(loader);

    loader->loadSymbolTable();

    return loader;
}

void SymbolLoader::loadSymbolTable()
{
    for (auto symbolName : m_symbolNames)
    {
        void *symbol = loadSymbol(symbolName);
        if (symbol == nullptr)
            Err() << "Failed to load symbol : " << symbolName << "\n";

        m_symbolTable[symbolName] = symbol;
    }
}
} // namespace utils
