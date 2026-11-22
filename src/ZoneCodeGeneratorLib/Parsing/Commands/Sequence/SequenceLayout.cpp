#include "SequenceLayout.h"

#include "Parsing/Commands/Matcher/CommandsCommonMatchers.h"
#include "Parsing/Commands/Matcher/CommandsMatcherFactory.h"

namespace
{
    static constexpr auto CAPTURE_ACTION_NAME = 1;
    static constexpr auto CAPTURE_TYPE = 2;
} // namespace

SequenceLayout::SequenceLayout()
{
    const CommandsMatcherFactory create(this);

    AddLabeledMatchers(CommandsCommonMatchers::Typename(this), CommandsCommonMatchers::LABEL_TYPENAME);
    AddMatchers({
        create.Keyword("set"),
        create.Keyword("layout"),
        create.Or({
            create.And({
                create.Label(CommandsCommonMatchers::LABEL_TYPENAME).Capture(CAPTURE_TYPE),
                create.Identifier().Capture(CAPTURE_ACTION_NAME),
            }),
            create.Identifier().Capture(CAPTURE_ACTION_NAME),
        }),
        create.Char(';'),
    });
}

void SequenceLayout::ProcessMatch(CommandsParserState* state, SequenceResult<CommandsParserValue>& result) const
{
    StructureInformation* type;
    const auto& actionNameToken = result.NextCapture(CAPTURE_ACTION_NAME);

    if (result.HasNextCapture(CAPTURE_TYPE))
    {
        const auto& typeNameToken = result.NextCapture(CAPTURE_TYPE);
        std::vector<MemberInformation*> memberChain;

        if (!state->GetTypenameAndMembersFromTypename(typeNameToken.TypeNameValue(), type, memberChain))
            throw ParsingException(typeNameToken.GetPos(), "Unknown type");

        if (!memberChain.empty())
        {
            memberChain.back()->m_layout_action = std::make_unique<CustomAction>(actionNameToken.IdentifierValue(), std::vector<DataDefinition*>());
            return;
        }
    }
    else if (state->GetInUse() != nullptr)
    {
        type = state->GetInUse();
    }
    else
        throw ParsingException(actionNameToken.GetPos(), "No type is used. Therefore one needs to be specified directly.");

    type->m_layout_action = std::make_unique<CustomAction>(actionNameToken.IdentifierValue(), std::vector<DataDefinition*>());
}
