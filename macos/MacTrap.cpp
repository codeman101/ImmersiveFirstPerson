#include <atomic>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <set>

namespace
{
std::mutex& Lock()
{
    static std::mutex lock;
    return lock;
}

std::set<uint32_t>& Seen()
{
    static std::set<uint32_t> seen;
    return seen;
}

std::atomic<bool> g_stubLogged{false};

__attribute__((noinline, visibility("hidden"))) uintptr_t ifp_mac_trap_stub()
{
    if (!g_stubLogged.exchange(true))
    {
        std::fprintf(stderr, "[ifp] trap stub called (a function with no Mac address); it returns 0\n");
    }
    return 0;
}
}

// Hidden on purpose: the plugin exports are Main, Query, Supports, and ifp_registerNatives.
extern "C" __attribute__((visibility("hidden"))) uintptr_t ifp_mac_trap_for_hash(uint32_t aHash)
{
    std::lock_guard<std::mutex> lock(Lock());
    if (Seen().insert(aHash).second)
    {
        std::fprintf(stderr, "[ifp] address hash %u has no Mac row: it resolves to the fail-safe trap stub\n", aHash);
    }
    return reinterpret_cast<uintptr_t>(&ifp_mac_trap_stub);
}
