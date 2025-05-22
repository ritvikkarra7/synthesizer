#include "ADSR.h"

ADSR::ADSR(float attack, float decay, float sustain, float release)
{
    m_attackTime = attack;
    m_decayTime = decay;
    m_sustainLevel = sustain;
    m_releaseTime = release;
    state = State::Idle;
    currentLevel = 0.0f;
    currentTime = 0.0f;
}

float ADSR::tick(float deltaTime) {
    switch (state) {
        case State::Attack:
            currentLevel += deltaTime / m_attackTime;
            if (currentLevel >= 1.0f) {
                currentLevel = 1.0f;
                state = State::Decay;
            }
            break;
        case State::Decay:
            currentLevel -= deltaTime * (1.0f - m_sustainLevel) / m_decayTime;
            if (currentLevel <= m_sustainLevel) {
                currentLevel = m_sustainLevel;
                state = State::Sustain;
            }
            break;
        case State::Sustain:
            // Hold sustain level
            break;
        case State::Release:
            currentLevel -= deltaTime * m_sustainLevel / m_releaseTime;
            if (currentLevel <= 0.0f) {
                currentLevel = 0.0f;
                state = State::Idle;
            }
            break;
        case State::Idle:
        default:
            currentLevel = 0.0f;
            break;
    }

    return currentLevel;
}


void ADSR::noteOn() {
    state = State::Attack;
}

void ADSR::noteOff() {
    state = State::Release;
}

// setters for ADSR parameters
void ADSR::setAttackTime(float time) {
    m_attackTime = time;
}

void ADSR::setDecayTime(float time) {
    m_decayTime = time;
}

void ADSR::setSustainLevel(float level) {
    m_sustainLevel = level;
}

void ADSR::setReleaseTime(float time) {
    m_releaseTime = time;
}

// getters for ADSR parameters
float ADSR::getAttackTime() const {
    return m_attackTime;
}

float ADSR::getDecayTime() const {
    return m_decayTime;
}

float ADSR::getSustainLevel() const {
    return m_sustainLevel;
}

float ADSR::getReleaseTime() const {
    return m_releaseTime;
}

float ADSR::getCurrentLevel() const {
    return currentLevel;
}
