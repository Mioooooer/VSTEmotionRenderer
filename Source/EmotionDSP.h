/*
  ==============================================================================

    EmotionDSP.h
    Created: 30 Mar 2023 3:58:58pm
    Author:  quanjx

  ==============================================================================
*/

#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <memory>
#include <numbers>
#include <utility>
#include "STFT/STFT.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


namespace DSP
{
    /** band-pass filter with any Q-factor. */
    class BandPassFilter
    {
    public:
        BandPassFilter() : b0(1.0f), b1(0.0f), b2(0.0f), a1(0.0f), a2(0.0f), state({0, 0}) {}

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


    private:
        /** Filter coefficients. */
        float b0, b1, b2, a1, a2;
        /** Filter state between audio buffer. */
        std::array<float, 2> state;
    };

    class MyUtils
    {
    public:
        MyUtils() {
            //mySTFT = STFT(ch,frame,shift);
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

        float calculateRMS(std::vector<double>& data, double numSamples)
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

        void complexMultiply(double a, double b, double c, double d, double& e, double& f)
        {
            e = a*c - b*d;
            f = a*d + b*c;
        }

        void applySTFT(std::vector<double>& bufferin, std::vector<double>& bufferout)
        {
            double* tempIn = new double[shift];
            double* tempOut = new double[frame + 2];
            int restNum = bufferin.size() % shift;
            if (restNum != 0)
            {
                for (int i = 0; i < (shift - restNum); i++)
                {
                    bufferin.emplace_back(0.0);
                }
            }
            
            int numShift = bufferin.size() / shift;
            for (int i = 0; i < numShift; i++)
            {
                for (int j = 0; j < shift; j++)
                {
                    tempIn[j] = bufferin[i*shift+j];
                }
                mySTFT.stft(tempIn, tempOut);
                for(int k = 0; k < (frame+2); k++)
                {
                    bufferout.emplace_back(tempOut[k]);
                }
            }
            delete[] tempIn;
            delete[] tempOut;
            
        }

        void applyISTFT(std::vector<double>& bufferin, std::vector<double>& bufferout)
        {
            // bufferout would be equal to or larger than sampleNum due to adding zero when applySTFT.
            double* tempIn = new double[frame + 2];
            double* tempOut = new double[shift];
            int numFrame = bufferin.size() / (frame+2);
            for(int i = 0; i < numFrame; i++)
            {
                for (int j = 0; j < (frame + 2); j++)
                {
                    tempIn[j] = bufferin[i*(frame + 2) + j];
                }
                mySTFT.istft(tempIn, tempOut);
                for (int k = 0; k < shift; k++)
                {
                    bufferout.emplace_back(tempOut[k]);
                }
            }
            delete[] tempIn;
            delete[] tempOut;

        }

        void getMagnitude(std::vector<double>& bufferin, std::vector<double>& bufferout)
        {
            for(int i = 0; i < bufferin.size(); i = i + 2)
            {
                bufferout.emplace_back(std::sqrt(bufferin[i]*bufferin[i] + bufferin[i+1]*bufferin[i+1]));
            }
        }

        void applyVocoder(std::vector<double>& carrierIn, std::vector<double>& modulatorIn, std::vector<double>& vocoderOut)
        {
            std::vector<double> carrierSTFT;
            std::vector<double> modulatorSTFT;
            std::vector<double> modulatorMagnitude;
            std::vector<double> multipliedSTFT;
            applySTFT(carrierIn, carrierSTFT);
            applySTFT(modulatorIn, modulatorSTFT);
            getMagnitude(modulatorSTFT, modulatorMagnitude);
            for(int i = 0; i < carrierSTFT.size(); i++)//carrierSTFT multiplied by modulatorMagnitude
            {
                multipliedSTFT.emplace_back(carrierSTFT[i] * modulatorMagnitude[i/2]);
            }
            applyISTFT(multipliedSTFT, vocoderOut);
        }
        
        

    private:
        const int ch = 1;
        //const int frame = 512;
        //const int shift = 128;
        const int frame = 1024;
        const int shift = 1024;
        STFT mySTFT = STFT(ch,frame,shift);

    };

    

}
