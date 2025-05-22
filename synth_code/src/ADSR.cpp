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
