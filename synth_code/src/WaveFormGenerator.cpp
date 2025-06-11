#include <math.h>
#include "WaveFormGenerator.h"

WaveFormGenerator::WaveFormGenerator(int sample_rate, int frequency, float magnitude) : m_envelope(0.1f, 0.1f, 0.5f, 0.1f)
{
    m_sample_rate = sample_rate;
    m_frequency = frequency;
    m_magnitude = 0.1; // currently magnitude is set by default i guess 
    m_wave_type =  SINE;// Default wave type
    m_current_position = 0;
}

void WaveFormGenerator::getFrames(Frame_t *frames, int number_frames)
{
    float full_wave_samples = m_sample_rate / m_frequency;
    float step_per_sample = M_TWOPI / full_wave_samples;
    float deltaTime = 1.0f / m_sample_rate; // time per sample in seconds

    for (int i = 0; i < number_frames; i++)
    {
        float phase = fmod(m_current_position, M_TWOPI);
        float value = getFromWaveType(phase);

        float envelopeLevel = m_envelope.tick(deltaTime);
        float amplitude = 16384 * envelopeLevel * value;

        frames[i].left = frames[i].right = amplitude;

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

        case SAWTOOTH:
            {
                float normalized_phase = phase / M_TWOPI;
                value = 2.0f * normalized_phase - 1.0f;
            }
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

