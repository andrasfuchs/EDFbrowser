#ifndef FFTCALCULATIONRESULT_H
#define FFTCALCULATIONRESULT_H

#include <QObject>
#include <QString>
#include <QVector>

class FFTCalculationResult : public QObject
{
  Q_OBJECT            // we need this to avoid the "undefined reference" error by the compilation because of the Qt's signals/events

  public:
    FFTCalculationResult();

    QVector<double> Samples;
    QVector<double> FFTData;

    int DFTBlockSize;
    int DFTBlockCount;

    int StartIndex;
    int EndIndex;

    int SampleFrequency;
    double FrequencyStep;
    double SampleLengthInSeconds;

signals:

  public slots:
};

#endif // FFTCALCULATIONRESULT_H
