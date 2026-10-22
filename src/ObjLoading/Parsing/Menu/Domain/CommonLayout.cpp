#include "CommonLayout.h"

using namespace menu;

bool CommonLayoutProperties::IsEmpty() const
{
    return !m_horizontal_anchor && !m_vertical_anchor && !m_offset_x && !m_offset_y && !m_width && !m_height && !m_min_width && !m_max_width && !m_min_height
           && !m_max_height && !m_aspect_ratio && !m_padding && !m_flow && !m_item_spacing && !m_wrap && !m_fit;
}
