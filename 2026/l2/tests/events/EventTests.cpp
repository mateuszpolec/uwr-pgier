#include "engine/core/FrameTime.hpp"
#include "engine/events/EventDispatcher.hpp"

#include <catch2/catch2.hpp>

#include <memory>
#include <string>
#include <vector>

using namespace engine::events;

//////////////////////////////////////////////////////////////////////////
namespace
{
///////////////////////////////////////////////////////////////////////////
	struct SEmptyEvent : public TEvent<SEmptyEvent>
	{
	};

///////////////////////////////////////////////////////////////////////////
	struct SIntEvent : public TEvent<SIntEvent>
	{
		int Value = 0;
		SIntEvent() = default;
		SIntEvent(int _value) : Value(_value) {}
	};

///////////////////////////////////////////////////////////////////////////
	struct SFloatEvent : public TEvent<SFloatEvent>
	{
		float X = 0.0f;
		float Y = 0.0f;
		SFloatEvent() = default;
		SFloatEvent(float _x, float _y) : X(_x), Y(_y) {}
	};

///////////////////////////////////////////////////////////////////////////
	struct SStringEvent : public TEvent<SStringEvent>
	{
		std::string Text;
		SStringEvent() = default;
		SStringEvent(const std::string& _text) : Text(_text) {}
	};

	///////////////////////////////////////////////////////////////////////////
	struct SEventDispatcherFixture
	{
		static constexpr float FIXED_DT = 1.0f / 60.0f;

		engine::core::SFrameTime Time;
		std::shared_ptr<CEventDispatcher> Dispatcher;

		SEventDispatcherFixture()
			: Dispatcher(std::make_shared<CEventDispatcher>(Time))
		{
			Time.Advance(FIXED_DT);
		}

		void Tick(float _dt = FIXED_DT)
		{
			Time.Advance(_dt + std::numeric_limits<float>::epsilon()); // Try to avoid floating point precision issues
			Dispatcher->Frame();
		}

		void Tick(uint32_t _frames, float _dt = FIXED_DT)
		{
			for (uint32_t i = 0; i < _frames; ++i)
			{
				Tick(_dt);
			}
		}
	};
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE("EventDispatcher (static event id)", "[Events][EventDispatcher]")
{
	SECTION("Different event types have different ids")
	{
		CHECK(SEmptyEvent::GetStaticEventId() != SIntEvent::GetStaticEventId());
		CHECK(SIntEvent::GetStaticEventId() != SFloatEvent::GetStaticEventId());
		CHECK(SFloatEvent::GetStaticEventId() != SStringEvent::GetStaticEventId());
	}

	SECTION("Same event type always returns same id")
	{
		CHECK(SEmptyEvent::GetStaticEventId() == SEmptyEvent::GetStaticEventId());
		CHECK(SIntEvent::GetStaticEventId() == SIntEvent::GetStaticEventId());
		CHECK(SFloatEvent::GetStaticEventId() == SFloatEvent::GetStaticEventId());
		CHECK(SStringEvent::GetStaticEventId() == SStringEvent::GetStaticEventId());
	}

	SECTION("Instance GetEventId matches static")
	{
		SIntEvent event(42);
		CHECK(event.GetEventId() == SIntEvent::GetStaticEventId());

		SFloatEvent floatEvent(3.14f, 2.71f);
		CHECK(floatEvent.GetEventId() == SFloatEvent::GetStaticEventId());
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SEventDispatcherFixture, "EventDispatcher (Subscribe + Submit)", "[Events][EventDispatcher]")
{
	SECTION("Single subscriber receives submitted event")
	{
		int received = 0;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { received = _event.Value; }
		);

		Dispatcher->Submit<SIntEvent>(42);

		CHECK(received == 42);
	}

	SECTION("Multiple subscribers all receive the event")
	{
		int count = 0;

		CEventHandle h1 = Dispatcher->Subscribe<SIntEvent>([&](const SIntEvent&) { ++count; });
		CEventHandle h2 = Dispatcher->Subscribe<SIntEvent>([&](const SIntEvent&) { ++count; });
		CEventHandle h3 = Dispatcher->Subscribe<SIntEvent>([&](const SIntEvent&) { ++count; });

		Dispatcher->Submit<SIntEvent>(0);

		CHECK(count == 3);
	}

	SECTION("Subscriber receives correct data")
	{
		float rx = 0.0f;
		float ry = 0.0f;

		CEventHandle handle = Dispatcher->Subscribe<SFloatEvent>(
			[&](const SFloatEvent& _event) { rx = _event.X; ry = _event.Y; }
		);

		Dispatcher->Submit<SFloatEvent>(1.5f, 2.5f);

		CHECK(rx == 1.5f);
		CHECK(ry == 2.5f);
	}

	SECTION("Submit with no subscribers is a no-op")
	{
		Dispatcher->Submit<SIntEvent>(123);
		CHECK(true);
	}

	SECTION("String event data preserved")
	{
		std::string received;

		CEventHandle handle = Dispatcher->Subscribe<SStringEvent>(
			[&](const SStringEvent& _event) { received = _event.Text; }
		);

		Dispatcher->Submit<SStringEvent>(std::string("hello pulvis"));

		CHECK(received == "hello pulvis");
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SEventDispatcherFixture, "EventDispatcher (multiple event types)", "[Events][EventDispatcher]")
{
	SECTION("Different types dispatched independently")
	{
		int int_received = 0;
		float float_received = 0.0f;

		CEventHandle h1 = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { int_received = _event.Value; }
		);

		CEventHandle h2 = Dispatcher->Subscribe<SFloatEvent>(
			[&](const SFloatEvent& _event) { float_received = _event.X; }
		);

		Dispatcher->Submit<SIntEvent>(7);

		CHECK(int_received == 7);
		CHECK(float_received == 0.0f);

		Dispatcher->Submit<SFloatEvent>(3.14f, 0.0f);

		CHECK(float_received == 3.14f);
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SEventDispatcherFixture, "EventDispatcher (Unsubscribe via handle)", "[Events][EventDispatcher]")
{
	SECTION("Reset handle stops receiving events")
	{
		int received = 0;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { received = _event.Value; }
		);

		Dispatcher->Submit<SIntEvent>(10);
		CHECK(received == 10);

		handle.Reset();
		Dispatcher->Submit<SIntEvent>(20);

		CHECK(received == 10);
	}

	SECTION("Handle destructor unsubscribes")
	{
		int received = 0;

		{
			CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
				[&](const SIntEvent& _event) { received = _event.Value; }
			);

			Dispatcher->Submit<SIntEvent>(5);
			CHECK(received == 5);
		}

		Dispatcher->Submit<SIntEvent>(99);
		CHECK(received == 5);
	}

	SECTION("Only the unsubscribed handle stops, others continue")
	{
		int a = 0;
		int b = 0;

		CEventHandle h1 = Dispatcher->Subscribe<SIntEvent>([&](const SIntEvent& _event) { a = _event.Value; });
		CEventHandle h2 = Dispatcher->Subscribe<SIntEvent>([&](const SIntEvent& _event) { b = _event.Value; });

		Dispatcher->Submit<SIntEvent>(1);
		CHECK(a == 1);
		CHECK(b == 1);

		h1.Reset();
		Dispatcher->Submit<SIntEvent>(2);

		CHECK(a == 1);
		CHECK(b == 2);
	}

	SECTION("Handle outliving dispatcher is safe to reset")
	{
		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>([](const SIntEvent&) {});

		Dispatcher.reset();

		handle.Reset();
		CHECK(!handle.IsValid());
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SEventDispatcherFixture, "EventDispatcher (handle validity)", "[Events][EventDispatcher]")
{
	SECTION("Newly created handle is valid")
	{
		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>([](const SIntEvent&) {});
		CHECK(handle.IsValid());
	}

	SECTION("Default handle is not valid")
	{
		CEventHandle handle;
		CHECK(!handle.IsValid());
	}

	SECTION("Reset handle is no longer valid")
	{
		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>([](const SIntEvent&) {});

		handle.Reset();
		CHECK(!handle.IsValid());
	}

	SECTION("Moved-from handle is not valid")
	{
		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>([](const SIntEvent&) {});

		CEventHandle moved = std::move(handle);
		CHECK(!handle.IsValid());
		CHECK(moved.IsValid());
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SEventDispatcherFixture, "EventDispatcher (QueueWithFrameDelay + Frame)", "[Events][EventDispatcher]")
{
	SECTION("Queued event submits after one frame")
	{
		int received = 0;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { received = _event.Value; }
		);

		Dispatcher->QueueWithFrameDelay<SIntEvent>(1, 42);
		CHECK(received == 0);

		Tick();
		CHECK(received == 42);
	}

	SECTION("Delayed event submits after N frames")
	{
		int received = 0;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { received = _event.Value; }
		);

		Dispatcher->QueueWithFrameDelay<SIntEvent>(3, 99);

		Tick();
		CHECK(received == 0);

		Tick();
		CHECK(received == 0);

		Tick();
		CHECK(received == 99);
	}

	SECTION("Frame without time advance does not dispatch")
	{
		int received = 0;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { received = _event.Value; }
		);

		Dispatcher->QueueWithFrameDelay<SIntEvent>(1, 42);

		Dispatcher->Frame();
		Dispatcher->Frame();
		CHECK(received == 0);

		Tick();
		CHECK(received == 42);
	}

	SECTION("Multiple queued events submit in correct frame")
	{
		std::vector<int> results;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { results.push_back(_event.Value); }
		);

		Dispatcher->QueueWithFrameDelay<SIntEvent>(0, 5);
		Dispatcher->QueueWithFrameDelay<SIntEvent>(1, 1);
		Dispatcher->QueueWithFrameDelay<SIntEvent>(2, 2);
		Dispatcher->QueueWithFrameDelay<SIntEvent>(1, 3);

		// Zero delay: dispatched at the next Frame() of the current frame.
		Dispatcher->Frame();
		REQUIRE(results.size() == 1);
		CHECK(results[0] == 5);

		Tick();
		REQUIRE(results.size() == 3);
		CHECK(results[1] == 1);
		CHECK(results[2] == 3);

		Tick();
		REQUIRE(results.size() == 4);
		CHECK(results[3] == 2);
	}

	SECTION("Queued event is dispatched only once")
	{
		int count = 0;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>([&](const SIntEvent&) { ++count; });

		Dispatcher->QueueWithFrameDelay<SIntEvent>(1, 0);

		Tick();
		CHECK(count == 1);
	}

	SECTION("Frame with empty queue is a no-op")
	{
		Tick();
		CHECK(true);
	}

	SECTION("Queue with no subscribers does not crash")
	{
		Dispatcher->QueueWithFrameDelay<SIntEvent>(1, 42);
		Tick();
		CHECK(true);
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SEventDispatcherFixture, "EventDispatcher (QueueWithTimeDelay + Frame)", "[Events][EventDispatcher]")
{
	SECTION("Not dispatched before delay elapses")
	{
		int received = 0;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { received = _event.Value; }
		);

		Dispatcher->QueueWithTimeDelay<SIntEvent>(0.5f, 7);

		Tick(0.25f);
		CHECK(received == 0);
	}

	SECTION("Dispatched once delay elapses")
	{
		int received = 0;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { received = _event.Value; }
		);

		Dispatcher->QueueWithTimeDelay<SIntEvent>(0.5f, 7);

		Tick(0.5f);
		CHECK(received == 7);
	}

	SECTION("Accumulates across many small frames")
	{
		int received = 0;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { received = _event.Value; }
		);

		Dispatcher->QueueWithTimeDelay<SIntEvent>(1.0f, 7);

		Tick(0.5f);   // 0.5 s
		CHECK(received == 0);

		Tick(1.1f);   // > 1.0 s
		CHECK(received == 7);
	}

	SECTION("Respects TimeScale (paused game does not dispatch)")
	{
		int received = 0;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { received = _event.Value; }
		);

		Dispatcher->QueueWithTimeDelay<SIntEvent>(1.0f, 7);

		Time.TimeScale = 0.0f;
		Tick(5.0f);
		CHECK(received == 0);

		Time.TimeScale = 1.0f;
		Tick(1.0f);
		CHECK(received == 7);
	}

	SECTION("Time-delayed events dispatch in chronological order")
	{
		std::vector<int> results;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { results.push_back(_event.Value); }
		);

		Dispatcher->QueueWithTimeDelay<SIntEvent>(0.3f, 3);
		Dispatcher->QueueWithTimeDelay<SIntEvent>(0.1f, 1);
		Dispatcher->QueueWithTimeDelay<SIntEvent>(0.2f, 2);

		Tick(0.1f);
		REQUIRE(results.size() == 1);
		CHECK(results[0] == 1);

		Tick(0.1f);
		REQUIRE(results.size() == 2);
		CHECK(results[1] == 2);

		Tick(0.1f);
		REQUIRE(results.size() == 3);
		CHECK(results[2] == 3);
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SEventDispatcherFixture, "EventDispatcher (Clear)", "[Events][EventDispatcher]")
{
	SECTION("Clear removes all listeners")
	{
		int received = 0;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { received = _event.Value; }
		);

		Dispatcher->Clear();
		Dispatcher->Submit<SIntEvent>(42);

		CHECK(received == 0);
	}

	SECTION("Clear removes queued events")
	{
		int received = 0;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { received = _event.Value; }
		);

		Dispatcher->QueueWithFrameDelay<SIntEvent>(1, 99);
		Dispatcher->Clear();
		Tick();

		CHECK(received == 0);
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SEventDispatcherFixture, "EventDispatcher (multiple submit calls)", "[Events][EventDispatcher]")
{
	SECTION("Each submit delivers to subscriber")
	{
		std::vector<int> results;

		CEventHandle handle = Dispatcher->Subscribe<SIntEvent>(
			[&](const SIntEvent& _event) { results.push_back(_event.Value); }
		);

		for (int i = 0; i < 10; ++i)
		{
			Dispatcher->Submit<SIntEvent>(i);
		}

		REQUIRE(results.size() == 10);
		for (int i = 0; i < 10; ++i)
		{
			CHECK(results[i] == i);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SEventDispatcherFixture, "EventDispatcher (empty event)", "[Events][EventDispatcher]")
{
	SECTION("Empty event submits and is received")
	{
		bool received = false;

		CEventHandle handle = Dispatcher->Subscribe<SEmptyEvent>(
			[&](const SEmptyEvent&) { received = true; }
		);

		Dispatcher->Submit<SEmptyEvent>();

		CHECK(received);
	}
}

//////////////////////////////////////////////////////////////////////////
TEST_CASE_METHOD(SEventDispatcherFixture, "EventDispatcher (handle move semantics)", "[Events][EventDispatcher]")
{
	SECTION("Move-assigned handle works correctly")
	{
		int received = 0;

		CEventHandle handle;

		{
			CEventHandle temp = Dispatcher->Subscribe<SIntEvent>(
				[&](const SIntEvent& _event) { received = _event.Value; }
			);

			handle = std::move(temp);
		}

		Dispatcher->Submit<SIntEvent>(77);
		CHECK(received == 77);

		handle.Reset();
		Dispatcher->Submit<SIntEvent>(88);
		CHECK(received == 77);
	}
}