#include "MemoryManager.h"

#include <cstdlib>
#include <cstring>

MemoryManager::MemoryManager() = default;

MemoryManager::~MemoryManager()
{
    for (auto* allocation : m_allocations)
        free(allocation);

    m_allocations.clear();
}

void* MemoryManager::AllocRaw(const size_t size)
{
    void* result = calloc(size, 1u);
    m_allocations.push_back(result);

    return result;
}

char* MemoryManager::Dup(const char* str)
{
#ifdef _MSC_VER
    auto* result = _strdup(str);
#else
    auto* result = strdup(str);
#endif
    m_allocations.push_back(result);

    return result;
}

void* MemoryManager::AllocSharedRaw(const void* data, const size_t size)
{
    std::string key(static_cast<const char*>(data), size);

    const auto existing = m_shared_data.find(key);
    if (existing != m_shared_data.end())
        return existing->second;

    auto* result = AllocRaw(size);
    memcpy(result, data, size);
    m_shared_data.emplace(std::move(key), result);

    return result;
}

const char* MemoryManager::DupShared(const std::string& str)
{
    const auto existing = m_shared_strings.find(str);
    if (existing != m_shared_strings.end())
        return existing->second;

    const auto* result = Dup(str.c_str());
    m_shared_strings.emplace(str, result);

    return result;
}

void MemoryManager::Free(const void* data)
{
    for (auto iAlloc = m_allocations.begin(); iAlloc != m_allocations.end(); ++iAlloc)
    {
        if (*iAlloc == data)
        {
            std::erase_if(m_shared_strings,
                          [data](const auto& shared)
                          {
                              return shared.second == data;
                          });
            std::erase_if(m_shared_data,
                          [data](const auto& shared)
                          {
                              return shared.second == data;
                          });

            free(*iAlloc);
            m_allocations.erase(iAlloc);
            return;
        }
    }
}
