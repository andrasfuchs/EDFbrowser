/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015 Teunis van Beelen
*
* Email: teuniz@gmail.com
*
***************************************************************************
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***************************************************************************
*/



#ifndef SPECTRUMANALYZERDOCK_H
#define SPECTRUMANALYZERDOCK_H



#include <QtGlobal>
#include <QApplication>
#include <QDialog>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QSlider>
#include <QLabel>
#include <QProgressDialog>
#include <QCheckBox>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QButtonGroup>
#include <QRadioButton>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "global.h"
#include "mainwindow.h"
#include "signalcurve.h"
#include "popup_messagewindow.h"
#include "utils.h"
#include "filter.h"
#include "spike_filter.h"
#include "ravg_filter.h"
#include "flywheel.h"

#include "models/signal.h"
#include "models/signalcompblock.h"
#include "models/signaltype.h"

#include "third_party/fidlib/fidlib.h"
#include "third_party/kiss_fft/kiss_fftr.h"



class UI_Mainwindow;


struct spectrumdocksettings{
                             int signalnr;
                             int span;
                             int center;
                             int amp;
                             int log_min_sl;
                             int wheel;
                             int log;
                             int sqrt;
                             int colorbar;
                             double maxvalue;
                             double maxvalue_sqrt;
                             double maxvalue_vlog;
                             double maxvalue_sqrt_vlog;
                             double minvalue_vlog;
                             double minvalue_sqrt_vlog;
                           };




class UI_SpectrumDockWindow : public QObject
{
  Q_OBJECT

public:
  UI_SpectrumDockWindow(QWidget *parent);

  ~UI_SpectrumDockWindow();

  UI_Mainwindow *mainwindow;

  QDialog *SpectrumDialog;

  QDockWidget *dock;

  struct signalcompblock *signalcomp;

  struct spectrum_markersblock *spectrum_color;

void rescan();
void init(int);
void clear();
void getsettings(struct spectrumdocksettings *);
void setsettings(struct spectrumdocksettings);

private:
QVector<double> calculateFFT(QVector<double> buf_samples, int fft_outputbufsize, int dftblocksize, int *dftblocks, double samplefreq, double SIGNAL_NA_VALUE);
QVector<double> compileSignalFromRawData(signalcompblock *signalcomp, char *viewbuf);
Signal* changeMode(SignalType historgramMode, QString signalName, QString signalAlias, QString baseUnit);
double transformFFTValue(double value, double SIGNAL_NA_VALUE, double freqstep);

private:

  QWidget *parentwindow;

  QHBoxLayout *hlayout1;

  QVBoxLayout *vlayout2;

  SignalCurve *histogramView;

  QTimer *t1;

  QCheckBox *colorBarCheckBox;

  QButtonGroup *scaleButtonGroup;


  int spectrumdock_sqrt,
      spectrumdock_vlog,
      dashboard,
      init_maxvalue,
      signal_nr,
      set_settings;

  volatile int busy;

  double maxvalue;

  Signal    *base_samples,
            *fft_ts1,
            *fft_ts8,
            *fft_ts64;

  struct spectrumdocksettings settings;

  SignalType historgramMode = SignalType::FFT | SignalType::LogScale | SignalType::SquareRoot;

private slots:

void update_curve();
void changeSignals(Signal* signal, SignalType newMode);
void colorBarButtonClicked(bool);
void print_to_txt();
void setdashboard();
void scaleButtonClicked(bool checked);

};


#endif



















