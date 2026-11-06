#include <atomic>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/detail/async_operation_registry.hpp"

#if UAPP_HAS_ASYNC_OPERATIONS && UAPP_OPEN62541_VER_GE(1, 5)

using opcua::detail::AsyncOperationRegistry;
using opcua::detail::AsyncOperationState;

TEST_CASE("AsyncOperationRegistry: completion runs once, entry is removed") {
    AsyncOperationRegistry registry;
    int id = 0;
    int ran = 0;
    auto state = registry.add(&id);
    CHECK(registry.post(&id, state, [&] { ++ran; }));
    CHECK(ran == 0);
    registry.drain();
    CHECK(ran == 1);
    CHECK(registry.size() == 0);
    registry.drain();
    CHECK(ran == 1);
}

TEST_CASE("AsyncOperationRegistry: cancelled operation cannot post") {
    AsyncOperationRegistry registry;
    int id = 0;
    auto state = registry.add(&id);
    registry.cancel(&id);
    CHECK(state->cancelled());
    CHECK_FALSE(registry.post(&id, state, [] {}));
}

TEST_CASE("AsyncOperationRegistry: pointer reused after cancel, stale operation is rejected") {
    AsyncOperationRegistry registry;
    int id = 0;
    int ranOld = 0;
    int ranNew = 0;
    auto oldState = registry.add(&id);
    registry.cancel(&id);
    auto newState = registry.add(&id);  // same address, new operation
    CHECK_FALSE(registry.post(&id, oldState, [&] { ++ranOld; }));  // late worker of the old one
    CHECK(registry.post(&id, newState, [&] { ++ranNew; }));
    registry.drain();
    CHECK(ranOld == 0);
    CHECK(ranNew == 1);
}

TEST_CASE("AsyncOperationRegistry: completion queued before cancel is dropped after pointer reuse"
) {
    AsyncOperationRegistry registry;
    int id = 0;
    int ranOld = 0;
    int ranNew = 0;
    auto oldState = registry.add(&id);
    CHECK(registry.post(&id, oldState, [&] { ++ranOld; }));  // worker wins the race
    registry.cancel(&id);  // server cancels before the loop runs the completion
    auto newState = registry.add(&id);  // address is reused
    registry.drain();
    CHECK(ranOld == 0);  // must not complete the new operation with old data
    CHECK(registry.size() == 1);
    CHECK(registry.post(&id, newState, [&] { ++ranNew; }));
    registry.drain();
    CHECK(ranNew == 1);
}

TEST_CASE("AsyncOperationRegistry: completions run without holding the registry lock") {
    AsyncOperationRegistry registry;
    int a = 0;
    int b = 0;
    auto stateA = registry.add(&a);
    [[maybe_unused]] auto stateB = registry.add(&b);
    CHECK(registry.post(&a, stateA, [&] {
        registry.cancel(&b);  // would deadlock if drain() held the lock
        CHECK(registry.size() == 0);
    }));
    registry.drain();
}

TEST_CASE("AsyncOperationRegistry: concurrent workers, each completion runs exactly once") {
    constexpr int n = 200;
    AsyncOperationRegistry registry;
    std::vector<int> ids(n);
    std::vector<std::atomic<int>> ran(n);
    std::vector<std::shared_ptr<opcua::detail::AsyncOperationState>> states;
    for (int i = 0; i < n; ++i) {
        ran[i] = 0;
        states.push_back(registry.add(&ids[i]));
    }
    std::vector<std::thread> workers;
    for (int t = 0; t < 4; ++t) {
        workers.emplace_back([&, t] {
            for (int i = t; i < n; i += 4) {
                registry.post(&ids[i], states[i], [&ran, i] { ++ran[i]; });
            }
        });
    }
    for (int round = 0; round < 50; ++round) {
        registry.drain();  // the "event loop" drains while workers are still posting
    }
    for (auto& w : workers) {
        w.join();
    }
    registry.drain();
    int total = 0;
    for (int i = 0; i < n; ++i) {
        total += ran[i];
        CHECK(ran[i] == 1);
    }
    CHECK(total == n);
    CHECK(registry.size() == 0);
}

TEST_CASE("AsyncOperationState: completing is not cancelling") {
    AsyncOperationState state;
    CHECK_FALSE(state.cancelled());
    CHECK(state.tryComplete());
    CHECK_FALSE(state.tryComplete());
    CHECK_FALSE(state.cancelled());
}

TEST_CASE("AsyncOperationState: cancelled operation cannot be completed") {
    AsyncOperationState state;
    state.cancel();
    CHECK(state.cancelled());
    CHECK_FALSE(state.tryComplete());
}

TEST_CASE("AsyncOperationRegistry: cancel after a completion was queued marks it cancelled") {
    AsyncOperationRegistry registry;
    int id = 0;
    int ran = 0;
    auto state = registry.add(&id);
    CHECK(state->tryComplete());
    CHECK(registry.post(&id, state, [&] { ++ran; }));
    registry.cancel(&id);
    CHECK(state->cancelled());
    registry.drain();
    CHECK(ran == 0);
}

#endif
