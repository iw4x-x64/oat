#pragma once

#include "Parsing/Commands/Impl/CommandsParser.h"

class SequenceLayout final : public CommandsParser::sequence_t
{
public:
    SequenceLayout();

protected:
    void ProcessMatch(CommandsParserState* state, SequenceResult<CommandsParserValue>& result) const override;
};
