#include "histogramsignalgroup.h"

HistogramSignalGroup::HistogramSignalGroup(Signal *signal)
{
  this->base = signal;
  this->enabled = true;

  this->fft = QList<Signal*>();
  this->fft_enabled = QList<bool>();
  this->length_in_seconds = QList<double>();
}

