#pragma once

#include "Base64.h"
#include "Utils/MemoryManager.h"

#include <cstddef>
#include <format>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace world
{
    class LoadException final : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    template<size_t Size> void Vec(const nlohmann::json& jVec, float (&v)[Size])
    {
        const auto& jArray = jVec.get_ref<const nlohmann::json::array_t&>();
        if (jArray.size() != Size)
            throw LoadException(std::format("Expected vector of {} components but found {}", Size, jArray.size()));

        for (auto i = 0u; i < Size; i++)
            jArray[i].get_to(v[i]);
    }

    template<typename ElementType, typename TransformFn>
    [[nodiscard]] ElementType* Array(MemoryManager& memory, const nlohmann::json& jArray, TransformFn&& transform)
    {
        const auto& jElements = jArray.get_ref<const nlohmann::json::array_t&>();
        if (jElements.empty())
            return nullptr;

        auto* elements = memory.Alloc<ElementType>(jElements.size());
        for (auto i = 0u; i < jElements.size(); i++)
            transform(jElements[i], elements[i]);

        return elements;
    }

    template<typename ElementType> [[nodiscard]] ElementType* Array(MemoryManager& memory, const nlohmann::json& jArray)
    {
        return Array<ElementType>(memory,
                                  jArray,
                                  [](const nlohmann::json& jElement, ElementType& element)
                                  {
                                      jElement.get_to(element);
                                  });
    }

    template<typename ElementType, typename TransformFn>
    void ArrayOfSize(const nlohmann::json& jArray, ElementType* elements, const size_t count, TransformFn&& transform)
    {
        const auto& jElements = jArray.get_ref<const nlohmann::json::array_t&>();
        if (jElements.size() != count)
            throw LoadException(std::format("Expected array of {} entries but found {}", count, jElements.size()));

        for (auto i = 0u; i < count; i++)
            transform(jElements[i], elements[i]);
    }

    template<typename ElementType> void ArrayOfSize(const nlohmann::json& jArray, ElementType* elements, const size_t count)
    {
        ArrayOfSize(jArray,
                    elements,
                    count,
                    [](const nlohmann::json& jElement, ElementType& element)
                    {
                        jElement.get_to(element);
                    });
    }

    [[nodiscard]] inline void* Base64(MemoryManager& memory, const nlohmann::json& jData, const size_t expectedSize)
    {
        const auto& encoded = jData.get_ref<const std::string&>();
        if (encoded.empty() || expectedSize == 0u)
            return nullptr;

        auto* data = memory.AllocRaw(expectedSize);
        const auto decodedSize = base64::DecodeBase64(encoded.data(), encoded.size(), data, expectedSize);
        if (decodedSize != expectedSize)
        {
            memory.Free(data);
            throw LoadException(std::format("Expected {} bytes of base64 data but found {}", expectedSize, decodedSize));
        }

        return data;
    }

    template<size_t Size> void ReadIfPresent(const nlohmann::json& jParent, const char* key, unsigned char (&bytes)[Size])
    {
        const auto jBytes = jParent.find(key);
        if (jBytes == jParent.end())
            return;

        ArrayOfSize(*jBytes, bytes, Size);
    }
} // namespace world
