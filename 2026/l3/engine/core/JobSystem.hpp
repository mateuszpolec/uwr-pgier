#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <vector>

#include <thread>
#include <mutex>

#include "engine/core/NonCopyable.hpp"

namespace engine::core
{
	using job_id_t = uint32_t;
	constexpr static job_id_t INVALID_JOB_ID = UINT32_MAX;
	constexpr static uint32_t MAX_IDLE_SPINS = 64;
	constexpr static uint32_t MAX_IDLE_YIELDS = 64;

	class CJobSystem : public engine::core::NonCopyable
	{
		public:

			CJobSystem(uint32_t _num_threads);
			~CJobSystem();

			template<typename T>
			job_id_t Submit(std::function<T()> _job, std::function<void(T)> _callback)
			{
				auto type_erased_job = [job = std::move(_job), callback = std::move(_callback)]()
				{
					T result = job();
					return[callback = std::move(callback), result = std::move(result)]() mutable -> std::function<void()>
					{
							if (callback)
							{
								callback(std::move(result));
							}
						};
				};

				return SubmitJob(std::move(type_erased_job));
			}

			job_id_t Submit(std::function<void()> _job, std::function<void()> _callback)
			{
				auto type_erased_job = [job = std::move(_job), callback = std::move(_callback)]() -> std::function<void()>
				{
					job();
					return std::move(callback);
				};

				return SubmitJob(std::move(type_erased_job));
			}

			void Frame();
			void WaitForAll();

			[[nodiscard]] bool IsIdle() const;
			[[nodiscard]] uint32_t GetWorkersCount() const { return static_cast<uint32_t>(m_WorkerThreads.size()); }
		
		private:

			struct SJob
			{
				job_id_t ID;
				std::function<std::function<void()>()> Job;
			};

		private:

			job_id_t SubmitJob(std::function<std::function<void()>()> _job);
			bool TryPopJob(SJob& _job);

			bool WaitForWork();
			void Work();

			mutable std::mutex m_Mutex;
			std::condition_variable m_WorkAvailable;
			std::deque<SJob> m_PendingJobs;
			std::deque<std::function<void()>> m_CompletedJobs;
			std::vector<std::thread> m_WorkerThreads;

			uint32_t m_NumActiveJobs = 0;
			job_id_t m_NextJobId = 0;
			bool m_StopRequested = false;
	};
}