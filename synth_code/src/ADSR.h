#ifndef ADSR_H
#define ADSR_H

class ADSR {
public:
    // Constructor
    ADSR(float attack, float decay, float sustain, float release);

    // Setters for ADSR parameters
    void setAttackTime(float attackTime);
    void setDecayTime(float decayTime);
    void setSustainLevel(float sustainLevel);
    void setReleaseTime(float releaseTime);

    // Getters for ADSR parameters
    float getAttackTime() const;
    float getDecayTime() const;
    float getSustainLevel() const;
    float getReleaseTime() const;

    // Process the envelope
    float process(float inputSample);

    // Trigger the envelope
    void noteOn();
    void noteOff();

private:
    // ADSR parameters
    float m_attackTime;
    float m_decayTime;
    float m_sustainLevel;
    float m_releaseTime;

    // Internal state
    enum class State { Idle, Attack, Decay, Sustain, Release } state;
    float currentLevel;
    float currentTime;
};

#endif // ADSR_H