#include "Animation.h"

Animation::Animation(const char* Filename)
{
    m_filename = Filename;
}

Animation::~Animation()
{
}

float Animation::GetTicksPerSecond()
{
    return m_ticksPerSecond != 0 ? m_ticksPerSecond : 25.0f;;
}
