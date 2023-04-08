# VSTEmotionRenderer
A VST3 Plugin that can add specific emotion to voice.
It's basicly a simplified audio vocoder which contains carrier signal itself.
The carrier signal is converted directly from .wav file to float by python scripts.
The carrier signal is looped inside the plugin so every time your sending modulator signal to it would lead to a different sound output.

TODO:
1. Maybe adding mel-filterbank to process modulator signal could lead to better sound effect(maybe like voice become clearer).
2. There may be some problems in the STFT algorithm, and it leads to audio click when buffer size is not a multiple of shift number.
   Probably because of the 'adding zero' code in STFT algorithm or something wrong in the 'overlap', 'shift' code or maybe any other place.
