#include <math.h>
#include "WaveFormGenerator.h"

WaveFormGenerator::WaveFormGenerator(int sample_rate, int frequency, float magnitude)
{
    m_sample_rate = sample_rate;
    m_frequency = frequency;
    m_magnitude = magnitude;
    m_wave_type = SINE; // Default wave type
    m_current_position = 0;
}

void WaveFormGenerator::getFrames(Frame_t *frames, int number_frames)
{
    float full_wave_samples = m_sample_rate / m_frequency;
    float step_per_sample = M_TWOPI / full_wave_samples;

    for (int i = 0; i < number_frames; i++)
    {
        float phase = fmod(m_current_position, M_TWOPI);
        float value = getFromWaveType(phase);

        frames[i].left = frames[i].right = 16384 * m_magnitude * value;

        m_current_position += step_per_sample;
        if (m_current_position > M_TWOPI)
        {
            m_current_position -= M_TWOPI;
        }
    }
}

float WaveFormGenerator::getFromWaveType(float phase)
{   
    float value = 0.0f;
    switch (m_wave_type)
    {
        case SINE:
            value = sin(phase);
            break;

        case SQUARE:
            value = (phase < M_PI) ? 1.0f : -1.0f;
            break;

        case TRIANGLE:
            value = 4.0f * fabs((phase/TWO_PI) - 0.5f) - 1.0f;
            break;
        default :
            value = 0.0f; // Default case
            break;
    }

    return value; 
}
// Setters  

void WaveFormGenerator::setWaveType(WaveType type)
{
    m_wave_type = type; 
}

void WaveFormGenerator::setFrequency(int frequency)
{
    m_frequency = frequency;
}

void WaveFormGenerator::setMagnitude(float magnitude)
{
    m_magnitude = magnitude;
}

