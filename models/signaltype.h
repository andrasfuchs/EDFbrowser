#ifndef SIGNALTYPE
#define SIGNALTYPE

enum class SignalType { Unknown = 0, LogScale = 1, SquareRoot = 2, FFT = 4, ZScore = 8, BMP = 16, ADC = 32 };

inline SignalType operator|(SignalType a, SignalType b)
{return static_cast<SignalType>(static_cast<int>(a) | static_cast<int>(b));}

inline SignalType operator&(SignalType a, SignalType b)
{return static_cast<SignalType>(static_cast<int>(a) & static_cast<int>(b));}

#endif // SIGNALTYPE

