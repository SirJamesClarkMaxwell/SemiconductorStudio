#pragma once

#include "pch.hpp"
#include "ViewData.hpp"
#include "JFMIFitting.hpp"
#include "IDataManager.hpp"

#include "Widgets.hpp"
#include "imgui_internal.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

namespace JFMApp {

	struct AppServiceBundle {
		std::shared_ptr<JFMService::FittingService::IFitting> numerics{};
		std::shared_ptr<JFMService::DataManagementService::IDataManager> dataLoader{};
	};

	class App
	{
	public:
		App() = delete;
		App(const AppServiceBundle& services);
		~App();

		App(const App&) = delete;
		App& operator=(const App&) = delete;

		App(App&&) = delete;
		App& operator=(App&&) = delete;

		void init();

		void update();
		void draw();

	private:
		void setUpCallbacks();

	private:
		Data::ViewData m_state{};
		std::shared_ptr<JFMService::FittingService::IFitting> m_numerics{};
		std::shared_ptr<JFMService::DataManagementService::IDataManager> m_dataLoader{};

		struct WorkDispatcher {
			typedef std::function<void()> WorkCb;
			struct Work {
				WorkCb cb;
				const char *name;

				Work()
				{
				}

				Work(const WorkCb &cb, const char *name)
					: cb(cb)
					, name(name)
				{
				}
			};
			std::queue<Work> works;
			std::mutex queueLock;
			std::thread workerThread;
			std::condition_variable cv;
			std::atomic<bool> calledClose;

			std::atomic<bool> currentDone;
			std::mutex currentLock;
			std::condition_variable currentCv;

			WorkDispatcher()
				: workerThread(std::thread{ &WorkDispatcher::dispatch, this })
				, calledClose(false)
				, currentDone(true)
			{
				JFM_ASSERT(workerThread.joinable() == true);
			}

			~WorkDispatcher()
			{
				workerThread.join();
			}

		private:
			void resetOnDone(const Work &w)
			{
				w.cb();
				Info() << "Processing of work: " << w.name << " finished\n";

				currentDone = true;
				currentCv.notify_one();
			}

			WorkCb wrap(const WorkCb &work, const char *name)
			{
				return std::bind(&WorkDispatcher::resetOnDone, this, Work{work, name});
			}

			void dispatch()
			{
				while (1)
				{
					Work work;
					{
						std::unique_lock<std::mutex> lock(queueLock);
						cv.wait(lock, [&]() { return (works.size() > 0) || (calledClose == true); });

						if (calledClose == true)
							break;

						work = works.front();
						works.pop();
					}

					// wait until processing of _current_ work is not finished
					{
						std::unique_lock<std::mutex> lock(currentLock);
						currentCv.wait(lock, [&]() { return (currentDone == true) || (calledClose == true); });

						if (calledClose == true)
							break;

						JFM_ASSERT(currentDone == true);
						currentDone = false;
					}

					const char *name = work.name;
					std::jthread t(work.cb);
					t.detach();
					Info() << "Started processing work: " << name << "\n";
				}
			}

		public:
			void addWork(const WorkCb &work, const char *workName)
			{
				std::lock_guard<std::mutex> lock(queueLock);
				Info() << "Scheduled new work: " << workName << "\n";

				works.push( Work(wrap(work, workName), workName) );
				cv.notify_one();
			}

			void close()
			{
				calledClose = true;
				cv.notify_one();
				currentCv.notify_one();
			}
		};
		WorkDispatcher dispatcher;
	};
}



