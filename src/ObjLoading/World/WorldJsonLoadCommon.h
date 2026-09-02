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

    template<typename ElementType, size_t Size, typename TransformFn>
    void ArrayOfSize(const nlohmann::json& jArray, ElementType (&elements)[Size], TransformFn&& transform)
    {
        const auto& jElements = jArray.get_ref<const nlohmann::json::array_t&>();
        if (jElements.size() != Size)
            throw LoadException(std::format("Expected array of {} entries but found {}", Size, jElements.size()));

        for (auto i = 0u; i < Size; i++)
            transform(jElements[i], elements[i]);
    }

    template<typename ElementType, size_t Size> void ArrayOfSize(const nlohmann::json& jArray, ElementType (&elements)[Size])
    {
        ArrayOfSize(jArray,
                    elements,
                    [](const nlohmann::json& jElement, ElementType& element)
                    {
                        jElement.get_to(element);
                    });
    }

    template<typename BoundsType> void Bounds(const nlohmann::json& jBounds, BoundsType& bounds)
    {
        Vec(jBounds.at("midPoint"), bounds.midPoint.v);
        Vec(jBounds.at("halfSize"), bounds.halfSize.v);
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

    struct Base64Data
    {
        void* data;
        size_t size;
    };

    [[nodiscard]] inline Base64Data Base64(MemoryManager& memory, const nlohmann::json& jData)
    {
        const auto& encoded = jData.get_ref<const std::string&>();
        if (encoded.empty())
            return {nullptr, 0u};

        const auto size = base64::GetBase64DecodeOutputLength(encoded.data(), encoded.size());

        return {Base64(memory, jData, size), size};
    }

    template<size_t Size> void ReadIfPresent(const nlohmann::json& jParent, const char* key, unsigned char (&bytes)[Size])
    {
        const auto jBytes = jParent.find(key);
        if (jBytes == jParent.end())
            return;

        ArrayOfSize(*jBytes, bytes);
    }
} // namespace world
