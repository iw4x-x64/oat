#pragma once

#include <cstddef>
#include <istream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fx
{
    class EfxParseException final : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    struct EfxCurvePoint
    {
        float time;
        std::vector<float> components;
    };

    struct EfxGraph
    {
        float scale;
        std::vector<EfxCurvePoint> base;
        std::vector<EfxCurvePoint> amplitude;
    };

    class EfxReader
    {
    public:
        explicit EfxReader(std::istream& stream);

        [[nodiscard]] bool AtEnd() const;

        [[nodiscard]] const std::string& Peek() const;

        const std::string& Next();

        void Expect(const std::string& expected);

        [[nodiscard]] bool NextIsString() const;

        float NextFloat();
        int NextInt();

        void EndStatement();

    private:
        void Tokenize(std::istream& stream);

        std::vector<std::string> m_words;
        std::vector<bool> m_word_was_quoted;
        size_t m_index;
    };
} // namespace fx
