#include <stdio.h>
#include <math.h>
#include <iostream>


constexpr double TwoPi = 3.141592653589793 * 2.0;
constexpr double SampleRate = 44100.0;
constexpr double Duration = 5.0;
constexpr double SampleSpan = 1.0 / SampleRate;
constexpr int SampleCount = int(Duration / SampleSpan);


double Lerp(const double Start, const double End, const double Alpha)
{
	return Start * (1.0 - Alpha) + End * Alpha;
}


double Spline(const double Start, const double Middle, const double End, const double Alpha)
{
	return Lerp(Lerp(Start, Middle, Alpha), Lerp(Middle, End, Alpha), Alpha);
}


double NormalizeWithinRange(const double Low, const double High, const double Value)
{
	return (Value - Low) / (High - Low);
}


double SineWave(const double Time)
{
	return sin(TwoPi * Time * 440.0);
}


double SquareWave(const double Time)
{
	return floor(fmod(Time * 440.0 * 2.0, 2.0)) * -2.0 + 1.0;
}


void GenerateSine(float* Samples)
{
	for (int i = 0; i < SampleCount; ++i)
	{
		const double Time = double(i) * SampleSpan;
		Samples[i] = float(SineWave(Time));
	}

	FILE* Out;
	Out = fopen("sine.raw", "wb");
	fwrite(Samples, sizeof(float), SampleCount, Out);
	fclose(Out);
}


void GenerateSquare(float* Samples)
{
	for (int i = 0; i < SampleCount; ++i)
	{
		const double Time = double(i) * SampleSpan;
		Samples[i] = float(SquareWave(Time));
	}

	FILE* Out;
	Out = fopen("square.raw", "wb");
	fwrite(Samples, sizeof(float), SampleCount, Out);
	fclose(Out);
}


void GenerateViberatoSquare(float* Samples)
{
	double WaveStart = 0.0;
	double WaveEnd = 1.0 / 440.0;
	for (int i = 0; i < SampleCount; ++i)
	{
		const double Time = double(i) * SampleSpan;
		if (Time >= WaveEnd)
		{
			WaveStart = WaveEnd;
			const double Vibrato = sin(TwoPi * Time * 10.0) * 10.0;
			const double WaveLength = 1.0 / (440.0 + Vibrato);
			WaveEnd = WaveStart + WaveLength;
		}
		double Phase = NormalizeWithinRange(WaveStart, WaveEnd, Time) * 2.0 - 1.0;
		Samples[i] = Phase < 0.0 ? -1.0f : 1.0f;
	}

	FILE* Out;
	Out = fopen("vibrato.raw", "wb");
	fwrite(Samples, sizeof(float), SampleCount, Out);
	fclose(Out);
}


void GenerateEnvelope(float* Samples)
{
	const double AttackStartTime = 0.0;
	const double AttackEndTime = 0.1;
	const double SustainStartTime = AttackEndTime;
	const double SustainEndTime = Duration;

	const double AttackCtrl = 0.4;
	const double SustainCtrl = 0.2;

	for (int i = 0; i < SampleCount; ++i)
	{
		const double Time = double(i) * SampleSpan;
		const double SquareSample = SquareWave(Time);
		double Amplitude = 0.0;
		if (Time <= AttackEndTime)
		{
			const double Alpha = NormalizeWithinRange(AttackStartTime, AttackEndTime, Time);
			Amplitude = Spline(0.0, AttackCtrl, 1.0, Alpha);
		}
		else
		{
			const double Alpha = NormalizeWithinRange(SustainStartTime, SustainEndTime, Time);
			Amplitude = Spline(1.0, SustainCtrl, 0.0, Alpha);
		}
		Samples[i] = float(SquareSample * Amplitude);
	}

	FILE* Out;
	Out = fopen("envelope.raw", "wb");
	fwrite(Samples, sizeof(float), SampleCount, Out);
	fclose(Out);
}


int main()
{
	float Samples[SampleCount];
	GenerateSine(&Samples[0]);
	GenerateSquare(&Samples[0]);
	GenerateEnvelope(&Samples[0]);
	GenerateViberatoSquare(&Samples[0]);
	return 0;
}
