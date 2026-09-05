#pragma once

#include <cstddef>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

class MemoryManager
{
public:
    MemoryManager();
    virtual ~MemoryManager();
    MemoryManager(const MemoryManager& other) = delete;
    MemoryManager(MemoryManager&& other) noexcept = default;
    MemoryManager& operator=(const MemoryManager& other) = delete;
    MemoryManager& operator=(MemoryManager&& other) noexcept = default;

    void* AllocRaw(std::size_t size);
    char* Dup(const char* str);
    const char* DupShared(const std::string& str);

    template<typename T> std::add_pointer_t<T> Alloc(const std::size_t count = 1u)
    {
        return static_cast<std::add_pointer_t<T>>(AllocRaw(sizeof(T) * count));
    }

    void Free(const void* data);

protected:
    std::vector<void*> m_allocations;

private:
    std::unordered_map<std::string, const char*> m_shared_strings;
};
