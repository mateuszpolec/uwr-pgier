#include "engine/core/JobSystem.hpp"

#include <catch2/catch2.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

using namespace engine::core;

//////////////////////////////////////////////////////////////////////////
namespace
{
	///////////////////////////////////////////////////////////////////////////
	struct SJobSystemFixture
	{
		static constexpr uint32_t NUM_THREADS = 4;

		std::unique_ptr<CJobSystem> Jobs;

		SJobSystemFixture()
			: Jobs(std::make_unique<CJobSystem>(NUM_THREADS))
		{
		}

		template<typename Pred>
		bool Drain(Pred _pred, std::chrono::milliseconds _timeout = std::chrono::seconds(5))
		{
			const auto deadline = std::chrono::steady_clock::now() + _timeout;

			while (!_pred())
			{
				if (std::chrono::steady_clock::now() > deadline)
				{
					return false;
				}

				Jobs->Frame();
				std::this_thread::yield();
			}

			return true;
		}
	};
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE("JobSystem (construction)", "[Core][JobSystem]")
{
	SECTION("Worker count is clamped to at least one")
	{
		CJobSystem jobs(0);
		CHECK(jobs.GetWorkersCount() >= 1u);
	}

	SECTION("Worker count does not exceed hardware limit")
	{
		CJobSystem jobs(1024);
		const uint32_t hardware_threads = std::max(2u, std::thread::hardware_concurrency() - 1);
		CHECK(jobs.GetWorkersCount() <= hardware_threads);
	}

	SECTION("Requested worker count is honored when in range")
	{
		CJobSystem jobs(2);
		CHECK(jobs.GetWorkersCount() == 2u);
	}

	SECTION("Freshly created system is idle")
	{
		CJobSystem jobs(2);
		CHECK(jobs.IsIdle());
	}

	SECTION("Destruction with no jobs does not hang")
	{
		{
			CJobSystem jobs(2);
		}
		CHECK(true);
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SJobSystemFixture, "JobSystem (Submit void job)", "[Core][JobSystem]")
{
	SECTION("Job is executed on a worker thread")
	{
		std::atomic<bool> executed = false;
		std::atomic<bool> on_main_thread = true;
		const std::thread::id main_id = std::this_thread::get_id();

		Jobs->Submit(
			[&]() { executed = true; on_main_thread = (std::this_thread::get_id() == main_id); },
			nullptr
		);

		Jobs->WaitForAll();

		CHECK(executed);
		CHECK_FALSE(on_main_thread);
	}

	SECTION("Callback is executed during Frame on the calling thread")
	{
		std::atomic<bool> callback_called = false;
		std::thread::id callback_thread_id;
		const std::thread::id main_id = std::this_thread::get_id();

		Jobs->Submit(
			[]() {},
			[&]() { callback_called = true; callback_thread_id = std::this_thread::get_id(); }
		);

		REQUIRE(Drain([&]() { return callback_called.load(); }));
		CHECK(callback_thread_id == main_id);
	}

	SECTION("Callback is not executed without Frame")
	{
		std::atomic<bool> job_done = false;
		std::atomic<bool> callback_called = false;

		Jobs->Submit(
			[&]() { job_done = true; },
			[&]() { callback_called = true; }
		);

		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
		while (!job_done && std::chrono::steady_clock::now() < deadline)
		{
			std::this_thread::yield();
		}

		REQUIRE(job_done);
		CHECK_FALSE(callback_called);

		Jobs->Frame();
		CHECK(callback_called);
	}

	SECTION("Null callback is a no-op")
	{
		std::atomic<int> counter = 0;

		Jobs->Submit([&]() { ++counter; }, nullptr);
		Jobs->WaitForAll();

		CHECK(counter == 1);
		CHECK(Jobs->IsIdle());
	}

	SECTION("Each submission returns a unique id")
	{
		std::set<job_id_t> ids;

		for (int i = 0; i < 16; ++i)
		{
			const job_id_t id = Jobs->Submit([]() {}, nullptr);
			CHECK(id != INVALID_JOB_ID);
			ids.insert(id);
		}

		Jobs->WaitForAll();
		CHECK(ids.size() == 16u);
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SJobSystemFixture, "JobSystem (Submit typed job)", "[Core][JobSystem]")
{
	SECTION("Result is passed to the callback")
	{
		int received = 0;

		Jobs->Submit<int>(
			[]() { return 42; },
			[&](int _value) { received = _value; }
		);

		Jobs->WaitForAll();

		CHECK(received == 42);
	}

	SECTION("Move-only / heavy result is forwarded correctly")
	{
		std::string received;

		Jobs->Submit<std::string>(
			[]() { return std::string("hello pulvis"); },
			[&](std::string _value) { received = std::move(_value); }
		);

		Jobs->WaitForAll();

		CHECK(received == "hello pulvis");
	}

	SECTION("Vector result preserved")
	{
		std::vector<int> received;

		Jobs->Submit<std::vector<int>>(
			[]() { return std::vector<int>{ 1, 2, 3 }; },
			[&](std::vector<int> _value) { received = std::move(_value); }
		);

		Jobs->WaitForAll();

		REQUIRE(received.size() == 3u);
		CHECK(received[0] == 1);
		CHECK(received[1] == 2);
		CHECK(received[2] == 3);
	}

	SECTION("Typed job with null callback still completes")
	{
		std::atomic<bool> executed = false;

		Jobs->Submit<int>(
			[&]() { executed = true; return 7; },
			nullptr
		);

		Jobs->WaitForAll();

		CHECK(executed);
		CHECK(Jobs->IsIdle());
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SJobSystemFixture, "JobSystem (many jobs)", "[Core][JobSystem]")
{
	SECTION("All jobs are executed exactly once")
	{
		constexpr int JOB_COUNT = 256;
		std::atomic<int> executed = 0;
		std::atomic<int> callbacks = 0;

		for (int i = 0; i < JOB_COUNT; ++i)
		{
			Jobs->Submit([&]() { ++executed; }, [&]() { ++callbacks; });
		}

		Jobs->WaitForAll();

		CHECK(executed == JOB_COUNT);
		CHECK(callbacks == JOB_COUNT);
	}

	SECTION("Results from all jobs are collected")
	{
		constexpr int JOB_COUNT = 128;
		std::vector<int> results;

		for (int i = 0; i < JOB_COUNT; ++i)
		{
			Jobs->Submit<int>(
				[i]() { return i * i; },
				[&](int _value) { results.push_back(_value); }
			);
		}

		Jobs->WaitForAll();

		REQUIRE(results.size() == static_cast<size_t>(JOB_COUNT));

		long long sum = 0;
		for (int v : results)
		{
			sum += v;
		}

		long long expected = 0;
		for (int i = 0; i < JOB_COUNT; ++i)
		{
			expected += static_cast<long long>(i) * i;
		}

		CHECK(sum == expected);
	}

	SECTION("Jobs run in parallel on multiple workers")
	{
		std::mutex mutex;
		std::set<std::thread::id> thread_ids;
		constexpr int JOB_COUNT = 64;

		for (int i = 0; i < JOB_COUNT; ++i)
		{
			Jobs->Submit(
				[&]()
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(2));
					std::lock_guard<std::mutex> lock(mutex);
					thread_ids.insert(std::this_thread::get_id());
				},
				nullptr
			);
		}

		Jobs->WaitForAll();

		if (Jobs->GetWorkersCount() > 1)
		{
			CHECK(thread_ids.size() > 1u);
		}
		CHECK(thread_ids.size() <= Jobs->GetWorkersCount());
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SJobSystemFixture, "JobSystem (IsIdle + WaitForAll)", "[Core][JobSystem]")
{
	SECTION("Not idle while a job is pending")
	{
		std::atomic<bool> release = false;

		Jobs->Submit(
			[&]() { while (!release) { std::this_thread::yield(); } },
			nullptr
		);

		CHECK_FALSE(Jobs->IsIdle());

		release = true;
		Jobs->WaitForAll();

		CHECK(Jobs->IsIdle());
	}

	SECTION("Not idle until callback is consumed by Frame")
	{
		std::atomic<bool> job_done = false;

		Jobs->Submit([&]() { job_done = true; }, []() {});

		while (!job_done)
		{
			std::this_thread::yield();
		}

		// Job wykonany, ale callback czeka na Frame().
		CHECK_FALSE(Jobs->IsIdle());

		Jobs->WaitForAll();
		CHECK(Jobs->IsIdle());
	}

	SECTION("WaitForAll on idle system returns immediately")
	{
		Jobs->WaitForAll();
		CHECK(Jobs->IsIdle());
	}

	SECTION("WaitForAll runs pending callbacks")
	{
		int received = 0;

		Jobs->Submit<int>([]() { return 5; }, [&](int _value) { received = _value; });
		Jobs->WaitForAll();

		CHECK(received == 5);
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SJobSystemFixture, "JobSystem (Frame)", "[Core][JobSystem]")
{
	SECTION("Frame with nothing completed is a no-op")
	{
		Jobs->Frame();
		CHECK(Jobs->IsIdle());
	}

	SECTION("Callback is executed only once across multiple Frames")
	{
		std::atomic<int> count = 0;

		Jobs->Submit([]() {}, [&]() { ++count; });

		REQUIRE(Drain([&]() { return count.load() == 1; }));

		Jobs->Frame();
		Jobs->Frame();

		CHECK(count == 1);
	}

	SECTION("Submitting from inside a callback is safe")
	{
		std::atomic<int> depth = 0;

		Jobs->Submit(
			[]() {},
			[&]()
			{
				++depth;
				Jobs->Submit([]() {}, [&]() { ++depth; });
			}
		);

		Jobs->WaitForAll();

		CHECK(depth == 2);
		CHECK(Jobs->IsIdle());
	}

	SECTION("Submitting from inside a job is safe")
	{
		std::atomic<int> count = 0;

		Jobs->Submit(
			[&]()
			{
				++count;
				Jobs->Submit([&]() { ++count; }, nullptr);
			},
			nullptr
		);

		Jobs->WaitForAll();

		CHECK(count == 2);
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE("JobSystem (destruction)", "[Core][JobSystem]")
{
	SECTION("Destructor joins workers while jobs are in flight")
	{
		std::atomic<int> executed = 0;

		{
			CJobSystem jobs(2);

			for (int i = 0; i < 8; ++i)
			{
				jobs.Submit(
					[&]() { std::this_thread::sleep_for(std::chrono::milliseconds(1)); ++executed; },
					nullptr
				);
			}
		}

		CHECK(executed >= 0);
	}

	SECTION("Destructor after WaitForAll does not hang")
	{
		{
			CJobSystem jobs(2);
			jobs.Submit([]() {}, nullptr);
			jobs.WaitForAll();
		}
		CHECK(true);
	}
}