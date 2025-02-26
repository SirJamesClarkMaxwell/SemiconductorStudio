#include "DataManager.hpp"

namespace JFMService
{
	DataManager::DataManager()
	{
		loaders[LoadingTypes::Characteristic] = std::make_shared<CharacteristicLoader>();
		loaders[LoadingTypes::MonteCarloResult] = std::make_shared<MonteCarloResultLoader>();

		dumpers[LoadingTypes::MonteCarloResult] = std::make_shared<MonteCarloResultDumper>();
	}

	void DataManager::Load(const path &path, const Callback &callback)
	{
        try
        {
			std::shared_ptr<Loader> loader = loaders.at(findLoader(path));
			LoaderOutput output = std::move(loader->Load(path));
			if (callback)
				callback(std::move(output));
        }
        catch(std::exception& e)
        {
			std::cout << e.what() << std::endl;
        }
	}

	void DataManager::Load(const std::vector<path> &paths, const VectorCallback &callbacks)
	{

		std::vector<LoaderOutput> outputs;
		auto func = [&](LoaderOutput output)
		{
			mutex.lock();
			outputs.push_back(std::move(output));
			if (callbacks)
				callbacks(std::move(outputs));
			mutex.unlock();
		};
		for (auto &path : paths)
			Load(path, func);
	}

	void DataManager::Save(const path &path, const LoaderOutput &input, const Callback &callback)
	{
		std::shared_ptr<Dumper> dumper = dumpers.at(findDumper(path));
		dumper->Save(path, input, callback);
	}

	void DataManager::Save(const std::vector<path> &paths, const std::vector<LoaderOutput> &input, const VectorCallback &callbacks)
	{
	}

	void DataManager::MoveInto(path &currentPath, const std::string &destination) { currentPath /= destination; }

	void DataManager::MoveBack(path &currentPath) { currentPath = currentPath.parent_path(); }

	std::vector<std::string> DataManager::GetDirectories(const path &path)
	{
		std::vector<std::string> paths;
		for (const auto &item : std::filesystem::directory_iterator(path))
			if (item.is_directory())
				paths.push_back(item.path().filename().string());
		return paths;
	}

	std::vector<std::string> DataManager::GetFiles(const path &path)
	{
		std::vector<std::string> paths;
		for (const auto &item : std::filesystem::directory_iterator(path))
			if (!item.is_directory())
				paths.push_back(item.path().filename().string());
		return paths;
	}
	LoadingTypes DataManager::findLoader(const std::filesystem::path &path)
	{
		// todo improve this logic later, add method in loader which will check possibility of loading this type of file
		for (const auto &[loadingType, loader] : loaders)
		{
			if (!loader->CheckExtentionCompatibility(path))
				continue;
			else
				return loadingType;
		}
	}
	LoadingTypes DataManager::findDumper(const std::filesystem::path &path)
	{
		return LoadingTypes::MonteCarloResult;
	}
}