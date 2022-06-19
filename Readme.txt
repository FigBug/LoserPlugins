VST Plug-In Bundle
(C) 2008
READ ME

--------------------------------------------------------------------------------

Content:
- 0xMaximizer (.dll/.so)
- 3BandEQ (.dll/.so)
- Delay (.dll/.so)
- Dither (.dll/.so)
- DVC (.dll/.so)
- DVCLite (.dll/.so)
- DVCMaster (.dll/.so)
- Exciter (.dll/.so)
- MSDecoder (.dll/.so)
- MSEncoder (.dll/.so)
- ParaEQ (.dll/.so)
- ParaEQLite (.dll/.so)
- SP7Limiter (.dll/.so)
- SimpleVerb (.dll/.so)
- Stereo (.dll/.so)
- TransientShaper (.dll/.so)


--------------------------------------------------------------------------------

0-X Maximizer (VST PLUG-IN)
(C) 2007-2008
MANUAL

A device that maximizes the audio  signal  by  scaling  the part of the waveform
between zero crossings  to  the  set  ceiling dB value. Has the same effect as a
clipper, but mentaince the roundness of the waveform.

(1) Threshold:  Controls the level above which the signal gets affected.
(2) Release: Controls the time it takes for the effect to recover.
(3) Ceiling: Sets the maximum allowed output value.

TIPS:
- Sounds good in its range.
- Will distort horrible when pushed too far.
- Only scrape of some dBs (use like a clipper)


--------------------------------------------------------------------------------

3-Band EQ (VST PLUG-IN)
(C) 2006-2008
MANUAL

A simple 3 band track EQ. The frequencies function as crossover frequencies.

(1) Low: Controls the level of the low band.
(2) Low-Mid Frequency: Sets the crossover frquency between low and mid band.
(3) Mid: Controls the level of the mid band.
(4) Mid-high Frequency: Sets the crossover frquency between mid and high band.
(5) High: Controls the level of the high band.
(6) Output: Sets the output level.

TIPS:
- Great for overall sound changes.
- No surgical EQ!


--------------------------------------------------------------------------------

Delay (VST PLUG-IN)
(C) 2006-2008
MANUAL

A simple Delay.

(1) Delay: Controls length of the delay.
(2) Mix In: The level with which the audio signal gets mix into the delay line.
(3) Feedback: Amount of feedback.
(4) Low-Pass: Frequency of the lowpass filter.
(5) High-Pass: Frequency of the highpass filter.
(6) Trim: Selects when the filtering should take place:
               MIX: Only once the signal gets mixed into the delay line.
               FBQ: Everytime the signal passes the feedback loop.
(7) Dry: Level of the dry signal.
(8) Wet: Level of the wet signal.
(9) Output: Level of the overall output.

TIPS:
- Set Low-Pass to approx. 1k and Trim to FBQ to get a tape delay like FX.


--------------------------------------------------------------------------------

Dither (VST PLUG-IN)
(C) 2007-2008
MANUAL

Dither plug-in.

(1) Bit Depth: Sets the bit depth of the output.
(2) Dither: Selects the type of dither noise to use.
(3) Dither Shaping: Sets the frequency shaping of the dither noise (HP=Highpass)
(4) Dither Amplitude: Sets the amplitude of the dither noise in LSBs.
(5) DC Shift: Lets you shift the dither noise.
(6) Noise Shaping: Toggles the noise shaping (1st order).

TIPS:
- Use as the last plug-in before mixdown.
- Do no apply further procession after the dither!
- If  the  mixdown  is  going  to  be  exported to another app (or the same) for
  further procession,  it is recommanded to use flat dither (i.e. Dither Shaping
  and Noise Shaping set to NO).


--------------------------------------------------------------------------------

DVC (VST PLUG-IN)
(C) 2007-2008
MANUAL

Compressor.

(1) Threshold: Sets the level above which the compression sets in.
(2) Ratio: Sets the amount of gain reduction
(3) Knee: Sets the size of the transition between compression and no compression
(4) Attack Start: Sets  when  the  compressor  should start to react. A negative
    value indicates lookahead  (i.e. it reacts the set time before the threshold
    is reached),  while a positive value indicates a delayed attack  ( i.e.  the
    signal  needs  to  be  over threshold for the set amount of time in order for
    compressor to start reacting).
(5) Attack: Sets the time it takes the compressor to reach its full effect.
(6) Release: Sets  the  time  it takes the compressor to go back to uncompressed
    state.
(7) Envelope Detector: Selects the type of envelope detector/character to use.
(8) Smoothen Envelope: Amount of smoothing that is applied to the envelope (does
    not applie to the DISTR (distortion) envelope).
(9) RMS Size: The size of the RMS window.
(10) Low-Pass: Sets the frequency of the lowpass filter of the detector.
(11) High-Pass: Sets the frequency of the highpass filter of the detector.
(12) Listen: Lets you choose to which source to listen.
(13) Feed: Sets the feed to the detector.
(14) Make-Up: Toggles auto make up.
(15) Output: Sets the output level.
(16) Dry Mix:  Sets  the  level  of  the  dry signal that gets mixed back to the
     processed signal.

TIPS:
- Use the DISTR Envelope Detector to moddle distortion units.
- Set the High-Pass to 120Hz to prevent pumping.


--------------------------------------------------------------------------------

DVC LITE (VST PLUG-IN)
(C) 2007-2008
MANUAL

Simple track compressor compressor.

(1) Threshold: Sets the level above which the compression sets in.
(2) Ratio: Sets the amount of gain reduction
(3) Attack: Sets the time it takes the compressor to reach its full effect.
(4) Release: Sets  the  time  it takes the compressor to go back to uncompressed
    state.
(5) Make-Up: Toggles auto make up.
(6) Output: Sets the output level.


--------------------------------------------------------------------------------

DVC MASTER (VST PLUG-IN)
(C) 2007-2008
MANUAL

Advanced modeling compressor.

(1) Threshold: Sets the level above which the compression sets in.
(2) Ratio: Sets the amount of gain reduction
(3) Knee: Sets the size of the transition between compression and no compression
(4) Attack Start: Sets  when  the  compressor  should start to react. A negative
    value indicates lookahead  (i.e. it reacts the set time before the threshold
    is reached),  while a positive value indicates a delayed attack  ( i.e.  the
    signal  needs  to  be  over threshold for the set amount of time in order for
    compressor to start reacting).
(5) Attack: Sets the time it takes the compressor to reach its full effect.
(6) Attack Shape: Selects the shape of the attack curve.
(7) Release: Sets  the  time  it takes the compressor to go back to uncompressed
    state.
(8) Release Shape: Selects the shape of the release curve.
(9) Envelope Detector: Selects the type of envelope detector/character to use.
(10) Smoothen Envelope: Amount of smoothing that is applied to the envelope (does
    not applie to the DISTR (distortion) envelope).
(9) RMS Size: The size of the RMS window.
(10) Low-Pass: Sets the frequency of the lowpass filter of the detector.
(11) High-Pass: Sets the frequency of the highpass filter of the detector.
(12) Peak: Sets the frequency of the peaking filter of the detector.
(13) Width: Sets the width of the peaking filter of the detector.
(14) Gain: Sets the gain of the peaking filter of the detector.
(15) Listen: Lets you choose to which source to listen.
(16) Feed: Sets the feed to the detector.
(17) Make-Up: Toggles auto make up.
(18) Output: Sets the output level.
(19) Saturation:  Sets  the  amount of saturation. This also engages the clipper
     (in case this is set to something other than 0%)
(20) Dry Mix:  Sets  the  level  of  the  dry signal that gets mixed back to the
     processed signal.

TIPS:
- Use the DISTR Envelope Detector to moddle distortion units.
- Set the High-Pass to 120Hz to prevent pumping.
- Set Peak to 6000Hz and its Gain to +6dB to give it some nice sound.


--------------------------------------------------------------------------------

Exciter (VST PLUG-IN)
(C) 2006-2008
MANUAL

High frequency exciter.

(1) High-Pass: Extracts the signal above the frequency to be excited.
(2) Clip Boost: Boosts the signal to be excited into a hardclipper,  in order to
    generate additional harmonics.
(3) Harmonics: Generate additional harmonics via methods of waveshaping.
(4) Mix In: Mixes the excited signal back to the original signal.

TIPS:
- Use is sparingly.
- Use it only if EQing doesn't work. 




M/S Decoder (VST PLUG-IN)
(C) 2006-2008
MANUAL

Simple M/S decoder.

(1) Mid: Sets the level of the mid signal.
(2) Side: Sets the level of the mid signal.
(3) Balance: Lets you balance the stereo image.
(4) Swap Left/Right: Toggles the channel swap.


--------------------------------------------------------------------------------

M/S Encoder (VST PLUG-IN)
(C) 2006-2008
MANUAL

Simple M/S encoder. Encodes the audio stream into a mid and side channel.


--------------------------------------------------------------------------------

Parametric EQ (VST PLUG-IN)
(C) 2007-2008

and

Parametric EQ LITE (VST PLUG-IN)
(C) 2007-2008

TIPS:
- Guitar:
  * Cut around 800Hz to 1kHz  to remove trashy/cheap sound.
  * Boost at 2.5kHz to emphasize the edge and sizzle.
- Bass:
  * Boost at 1kHz to emphasize the pick attack.
  * Boost at 2.5kHz to emphasize the slap.
- Kick:
  * Boost at around 60Hz to emphasize the thump.
  * Boost at around 2.5kHz to 4kHz to emphasize the attack.
- Snare:
  * Boost at 240Hz to emphasize body and bottom.
  * Boost at 2.5kHz to emphasize the snare crack.


--------------------------------------------------------------------------------

Simple Peak-7 Limiter (VST PLUG-IN)
(C) 2007-2008
MANUAL

Peak limiter. It prevents the signal form exceeding the set maximum ceiling.

(1) Threshold:  Controls the level above which the signal gets affected.
(2) Release: Controls the time it takes for the effect to recover.
(3) Saturation: Sets the amount of saturation applied to the signal.
(4) Ceiling: Sets the maximum allowed output value.


--------------------------------------------------------------------------------

Simple Verb (VST PLUG-IN)
(C) 2007-2008
MANUAL

A simple reverberation simulator.

(1) Room Size: Sets the size of the "room" to simulate.
(2) Damping: Sets the amount of damping used,  the less damping the brighter the
    reverb will be.
(3) Pre-Delay: Sets  the  amount  of time the reverberation is delayed after the
    dry signal.
(4) Low-Pass: Sets the frequency for the lowpass.
(5) High-Pass: Sets the frequency for the highpass.
(6) Dry: Level of the dry signal.
(7) Wet: Level of the wet signal.


--------------------------------------------------------------------------------

Stereo Processor (VST PLUG-IN)
(C) 2007-2008
MANUAL

A  stereo  imaging  plug-in,  that allows for correction and manipulation of the
stereo field.

(1+7) Width: Sets the width of the stereo image.
(2+6) Center: Sets the level of the center channel of the stereo image.
(3+5) Pan: Adjust the panning of the signal.
(4) Rotation: Lets you rotate the stereo image.
(8) Output: Sets the output level.


--------------------------------------------------------------------------------

Transient Shaper (VST PLUG-IN)
(C) 2007-2008
MANUAL

(1) Attack:  Controls the level of the attack portion of the signal's transient.
(2) Sustain: Controls the level of the sustain part of the signal's transient.
(3) Smooth: Amount of smoothing applied to the output gain envelope.
(4) Output: Sets the output level.

TIPS:
- Best used on drums.


--------------------------------------------------------------------------------

VST is a trademark of Steinberg Media Technologies GmbH.

Thanks to:
Asseca (http://www.asseca.com/) for hosting.
cern.th.skei (http://cern.linux.vst.googlepages.com/) for linux compilation.

--------------------------------------------------------------------------------
