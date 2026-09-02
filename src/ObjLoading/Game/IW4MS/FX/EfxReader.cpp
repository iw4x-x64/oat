#include "EfxReader.h"

#include <charconv>
#include <format>
#include <sstream>

namespace fx
{
    namespace
    {
        [[nodiscard]] bool IsSeparator(const char c)
        {
            return c == '{' || c == '}' || c == ';';
        }
    } // namespace

    EfxReader::EfxReader(std::istream& stream)
        : m_index(0u)
    {
        Tokenize(stream);
    }

    void EfxReader::Tokenize(std::istream& stream)
    {
        std::ostringstream contents;
        contents << stream.rdbuf();
        const auto text = contents.str();

        for (auto i = 0u; i < text.size();)
        {
            const auto c = text[i];

            if (static_cast<unsigned char>(c) <= ' ')
            {
                i++;
                continue;
            }

            if (IsSeparator(c))
            {
                m_words.emplace_back(1u, c);
                m_word_was_quoted.push_back(false);
                i++;
                continue;
            }

            if (c == '"')
            {
                const auto end = text.find('"', i + 1u);
                if (end == std::string::npos)
                    throw EfxParseException("Unterminated string");

                m_words.emplace_back(text, i + 1u, end - i - 1u);
                m_word_was_quoted.push_back(true);
                i = end + 1u;
                continue;
            }

            auto end = i;
            while (end < text.size() && static_cast<unsigned char>(text[end]) > ' ' && !IsSeparator(text[end]) && text[end] != '"')
                end++;

            m_words.emplace_back(text, i, end - i);
            m_word_was_quoted.push_back(false);
            i = end;
        }
    }

    bool EfxReader::AtEnd() const
    {
        return m_index >= m_words.size();
    }

    const std::string& EfxReader::Peek() const
    {
        if (AtEnd())
            throw EfxParseException("Unexpected end of file");

        return m_words[m_index];
    }

    const std::string& EfxReader::Next()
    {
        const auto& word = Peek();
        m_index++;

        return word;
    }

    void EfxReader::Expect(const std::string& expected)
    {
        const auto& word = Next();
        if (word != expected)
            throw EfxParseException(std::format("Expected \"{}\" but found \"{}\"", expected, word));
    }

    bool EfxReader::NextIsString() const
    {
        if (AtEnd())
            return false;

        return m_word_was_quoted[m_index];
    }

    float EfxReader::NextFloat()
    {
        const auto& word = Next();

        float value;
        const auto* end = word.data() + word.size();
        const auto result = std::from_chars(word.data(), end, value);
        if (result.ec != std::errc() || result.ptr != end)
            throw EfxParseException(std::format("Expected a number but found \"{}\"", word));

        return value;
    }

    int EfxReader::NextInt()
    {
        const auto& word = Next();

        const auto negative = !word.empty() && word[0] == '-';
        const auto* begin = word.data() + (negative ? 1u : 0u);
        const auto* end = word.data() + word.size();

        auto base = 10;
        if (end - begin > 2 && begin[0] == '0' && (begin[1] == 'x' || begin[1] == 'X'))
        {
            begin += 2;
            base = 16;
        }

        unsigned value;
        const auto result = std::from_chars(begin, end, value, base);
        if (result.ec != std::errc() || result.ptr != end)
            throw EfxParseException(std::format("Expected a whole number but found \"{}\"", word));

        const auto asInt = static_cast<int>(value);

        return negative ? -asInt : asInt;
    }

    void EfxReader::EndStatement()
    {
        while (!AtEnd())
        {
            if (Next() == ";")
                return;
        }

        throw EfxParseException("Statement does not end");
    }
} // namespace fx
