#include <algorithm>
#include <immintrin.h>

#include "JobSystem.hpp"

namespace engine::core
{
	CJobSystem::CJobSystem(uint32_t _num_threads)
	{
		const uint32_t hardware_threads = std::max(2u, std::thread::hardware_concurrency() - 1);
		_num_threads = std::clamp(_num_threads, 1u, hardware_threads);

		for (uint32_t i = 0; i < _num_threads; ++i)
		{
			m_WorkerThreads.emplace_back(&CJobSystem::Work, this);
		}
	}

	CJobSystem::~CJobSystem()
	{
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_StopRequested = true;
		}

		m_WorkAvailable.notify_all();

		for (auto& thread : m_WorkerThreads)
		{
			if (thread.joinable())
			{
				thread.join();
			}
		}
	}

	void CJobSystem::Frame()
	{
		std::deque<std::function<void()>> completed_jobs;
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			completed_jobs.swap(m_CompletedJobs);
		}
		
		for (auto& callback : completed_jobs)
		{
			callback();
		}

		if (!completed_jobs.empty())
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_NumActiveJobs -= static_cast<uint32_t>(completed_jobs.size());
		}
	}

	void CJobSystem::WaitForAll()
	{
		uint32_t idle_spins = 0;

		while (!IsIdle())
		{
			Frame();

			if (idle_spins < MAX_IDLE_SPINS)
			{
				++idle_spins;
				std::this_thread::yield();
			}
			else
			{
				_mm_pause();
			}
		}
	}

	bool CJobSystem::IsIdle() const
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		return m_NumActiveJobs == 0;
	}

	job_id_t CJobSystem::SubmitJob(std::function<std::function<void()>()> _job)
	{
		job_id_t job_id = INVALID_JOB_ID;
		{
			std::lock_guard<std::mutex> lock(m_Mutex);

			job_id = m_NextJobId++;
			m_PendingJobs.push_back({ job_id, std::move(_job) });
			++m_NumActiveJobs;
		}

		m_WorkAvailable.notify_one();
		return job_id;
	}

	bool CJobSystem::TryPopJob(SJob& _job)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		if (!m_PendingJobs.empty())
		{
			_job = std::move(m_PendingJobs.front());
			m_PendingJobs.pop_front();
			return true;
		}

		return false;
	}

	bool CJobSystem::WaitForWork()
	{
		for (uint32_t i = 0; i < MAX_IDLE_SPINS; ++i)
		{
			{
				std::lock_guard<std::mutex> lock(m_Mutex);
				if (!m_PendingJobs.empty() || m_StopRequested)
				{
					return !m_StopRequested;
				}
			}
			_mm_pause();
		}

		for (uint32_t i = 0; i < MAX_IDLE_YIELDS; ++i)
		{
			{
				std::lock_guard<std::mutex> lock(m_Mutex);
				if (!m_PendingJobs.empty() || m_StopRequested)
				{
					return !m_StopRequested;
				}
			}
			std::this_thread::yield();
		}

		std::unique_lock<std::mutex> lock(m_Mutex);
		m_WorkAvailable.wait(lock, [this]() { return !m_PendingJobs.empty() || m_StopRequested; });
		return !m_StopRequested;
	}

	void CJobSystem::Work()
	{
		while (true)
		{
			SJob job;
			if (!TryPopJob(job))
			{
				if (!WaitForWork())
				{
					return;
				}
				continue;
			}

			std::function<void()> callback = job.Job();

			{
				std::lock_guard<std::mutex> lock(m_Mutex);
				m_CompletedJobs.push_back(std::move(callback));
			}
		}
	}
}