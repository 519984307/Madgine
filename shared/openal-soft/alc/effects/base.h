#ifndef EFFECTS_BASE_H
#define EFFECTS_BASE_H

#include <cstddef>

#include "alcmain.h"
#include "almalloc.h"
#include "alspan.h"
#include "atomic.h"
#include "intrusive_ptr.h"

struct EffectSlot;
struct BufferStorage;


union EffectProps {
    struct {
        // Shared Reverb Properties
        float Density;
        float Diffusion;
        float Gain;
        float GainHF;
        float DecayTime;
        float DecayHFRatio;
        float ReflectionsGain;
        float ReflectionsDelay;
        float LateReverbGain;
        float LateReverbDelay;
        float AirAbsorptionGainHF;
        float RoomRolloffFactor;
        bool DecayHFLimit;

        // Additional EAX Reverb Properties
        float GainLF;
        float DecayLFRatio;
        float ReflectionsPan[3];
        float LateReverbPan[3];
        float EchoTime;
        float EchoDepth;
        float ModulationTime;
        float ModulationDepth;
        float HFReference;
        float LFReference;
    } Reverb;

    struct {
        float AttackTime;
        float ReleaseTime;
        float Resonance;
        float PeakGain;
    } Autowah;

    struct {
        int Waveform;
        int Phase;
        float Rate;
        float Depth;
        float Feedback;
        float Delay;
    } Chorus; /* Also Flanger */

    struct {
        bool OnOff;
    } Compressor;

    struct {
        float Edge;
        float Gain;
        float LowpassCutoff;
        float EQCenter;
        float EQBandwidth;
    } Distortion;

    struct {
        float Delay;
        float LRDelay;

        float Damping;
        float Feedback;

        float Spread;
    } Echo;

    struct {
        float LowCutoff;
        float LowGain;
        float Mid1Center;
        float Mid1Gain;
        float Mid1Width;
        float Mid2Center;
        float Mid2Gain;
        float Mid2Width;
        float HighCutoff;
        float HighGain;
    } Equalizer;

    struct {
        float Frequency;
        int LeftDirection;
        int RightDirection;
    } Fshifter;

    struct {
        float Frequency;
        float HighPassCutoff;
        int Waveform;
    } Modulator;

    struct {
        int CoarseTune;
        int FineTune;
    } Pshifter;

    struct {
        float Rate;
        int PhonemeA;
        int PhonemeB;
        int PhonemeACoarseTuning;
        int PhonemeBCoarseTuning;
        int Waveform;
    } Vmorpher;

    struct {
        float Gain;
    } Dedicated;
};


struct EffectTarget {
    MixParams *Main;
    RealMixParams *RealOut;
};

struct EffectState : public al::intrusive_ref<EffectState> {
    al::span<FloatBufferLine> mOutTarget;


    virtual ~EffectState() = default;

    virtual void deviceUpdate(const ALCdevice *device) = 0;
    virtual void setBuffer(const ALCdevice* /*device*/, const BufferStorage* /*buffer*/) { }
    virtual void update(const ALCcontext *context, const EffectSlot *slot,
        const EffectProps *props, const EffectTarget target) = 0;
    virtual void process(const size_t samplesToDo, const al::span<const FloatBufferLine> samplesIn,
        const al::span<FloatBufferLine> samplesOut) = 0;
};


struct EffectStateFactory {
    virtual ~EffectStateFactory() = default;

    virtual EffectState *create() = 0;
};


EffectStateFactory *NullStateFactory_getFactory(void);
EffectStateFactory *ReverbStateFactory_getFactory(void);
EffectStateFactory *StdReverbStateFactory_getFactory(void);
EffectStateFactory *AutowahStateFactory_getFactory(void);
EffectStateFactory *ChorusStateFactory_getFactory(void);
EffectStateFactory *CompressorStateFactory_getFactory(void);
EffectStateFactory *DistortionStateFactory_getFactory(void);
EffectStateFactory *EchoStateFactory_getFactory(void);
EffectStateFactory *EqualizerStateFactory_getFactory(void);
EffectStateFactory *FlangerStateFactory_getFactory(void);
EffectStateFactory *FshifterStateFactory_getFactory(void);
EffectStateFactory *ModulatorStateFactory_getFactory(void);
EffectStateFactory *PshifterStateFactory_getFactory(void);
EffectStateFactory* VmorpherStateFactory_getFactory(void);

EffectStateFactory *DedicatedStateFactory_getFactory(void);

EffectStateFactory *ConvolutionStateFactory_getFactory(void);

#endif /* EFFECTS_BASE_H */
