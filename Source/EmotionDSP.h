/*
  ==============================================================================

    EmotionDSP.h
    Created: 30 Mar 2023 3:58:58pm
    Author:  quanjx

  ==============================================================================
*/

#pragma once
#include <array>
#include <cmath>
#include <memory>
#include <numbers>
#include <utility>

#ifndef M_PI
#define M_PI 3.14159265359
#endif


namespace DSP
{
    /** band-pass filter with any Q-factor. */
    class EmotionFilter
    {
    public:
        EmotionFilter() : b0(1.0f), b1(0.0f), b2(0.0f), a1(0.0f), a2(0.0f), state({0, 0}) {}

        void setCoefficients(double sampleRate, float frequency, float Q)
        {
            const auto omega = 2 * (float)M_PI * (frequency / sampleRate);
            const auto sin_omega = std::sin(omega);
            const auto cos_omega = std::cos(omega);
            const auto alpha = sin_omega / (2 * Q);

            const auto scale = 1 / (1 + alpha);

            b0 = alpha * scale;
            b1 = 0;
            b2 = -alpha * scale;
            a1 = -2 * cos_omega * scale;
            a2 = (1 - alpha) * scale;
        }

        void process(float *src, float *dst, int numSamples)
        {
            auto lv1 = state[0];
            auto lv2 = state[1];
            for (size_t i = 0; i < numSamples; ++i)
            {
                const auto input = src[i];
                const auto output = (b0 * input) + lv1;
                dst[i] = output;

                lv1 = (b1 * input) - (a1 * output) + lv2;
                lv2 = (b2 * input) - (a2 * output);
            }
            state[0] = lv1;
            state[1] = lv2;
        }

        float calculateRMS(const float *data, double numSamples)
        {
            if (numSamples <= 0)
            {
                return 0;
            }

            double sum = 0.0;
            for (size_t i = 0; i < numSamples; i++)
            {
                auto sample = data[i];
                sum += sample * sample;
            }

            return std::sqrt(sum / numSamples);
        }

    private:
        /** Filter coefficients. */
        float b0, b1, b2, a1, a2;
        /** Filter state between audio buffer. */
        std::array<float, 2> state;
    };

}
