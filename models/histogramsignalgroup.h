#ifndef HISTOGRAMSIGNALGROUP_H
#define HISTOGRAMSIGNALGROUP_H

#include <QWidget>
#include <QLabel>
#include <QCheckBox>

#include "models/signal.h"

class HistogramSignalGroup : public QObject
{
  Q_OBJECT
public:
  Signal *base;                     // the signal that all others are based on
  bool enabled;                     // indicates if the signal is enabled/visible
  QList<double> length_in_seconds;  // lengths of the different timescales in seconds
  QList<Signal*> fft;               // FFT signals on different timescales
  QList<bool> fft_enabled;          // is the signal enabled

public:
  HistogramSignalGroup();

  HistogramSignalGroup(Signal *signal);

signals:

public slots:
};

#endif // HISTOGRAMSIGNALGROUP_H
