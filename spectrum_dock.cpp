/*
***************************************************************************
*
* Author: Teunis van Beelen, Andras Fuchs
*
* Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015 Teunis van Beelen
*
* Email: teuniz@gmail.com, andras.fuchs@gmail.com
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



#include "spectrum_dock.h"


#define SPECT_LOG_MINIMUM 0.000001
#define SPECT_LOG_MINIMUM_LOG (-5)



UI_SpectrumDockWindow::UI_SpectrumDockWindow(QWidget *w_parent)
{
  busy = 0;

  dashboard = 0;

  signalcomp = NULL;

  signal_nr = -1;

  set_settings = 0;

  mainwindow = (UI_Mainwindow *)w_parent;

  spectrum_color = mainwindow->spectrum_colorbar;

  SpectrumDialog = new QDialog;

  if(mainwindow->spectrumdock_sqrt)
  {
    dock = new QDockWidget("Power Spectrum", w_parent);
  }
  else
  {
    dock = new QDockWidget("Amplitude Spectrum", w_parent);
  }

  dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  dock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
  dock->setMinimumHeight(300);
  if(dashboard)
  {
    dock->setWidget(SpectrumDialog);
  }

  histogramView = new SignalCurve;

  histogramView->create_button("to Text");

  if(!dashboard)
  {
    dock->setWidget(histogramView);
  }


  scaleButtonGroup = new QButtonGroup();
  scaleButtonGroup->addButton(new QRadioButton("Linear Amplitude"), 0);
  scaleButtonGroup->addButton(new QRadioButton("Logarithmic Amplitude"), 1);
  scaleButtonGroup->addButton(new QRadioButton("Linear Power"), 2);
  QRadioButton *rb = new QRadioButton("Logarithmic Power");
  rb->setChecked(true);
  scaleButtonGroup->addButton(rb, 3);

  for(QAbstractButton *btn : scaleButtonGroup->buttons())
  {
    QObject::connect(btn, SIGNAL(clicked(bool)), this, SLOT(scaleButtonClicked(bool)));
  }


  colorBarCheckBox = new QCheckBox;
  colorBarCheckBox->setMinimumSize(50, 20);
  colorBarCheckBox->setText("Show colorbars");
  colorBarCheckBox->setTristate(false);

  comparisonModeCheckBox = new QCheckBox;
  comparisonModeCheckBox->setMinimumSize(50, 20);
  comparisonModeCheckBox->setText("Compare mode");
  comparisonModeCheckBox->setTristate(false);

  vlayout2 = new QVBoxLayout();
  vlayout2->setSpacing(10);
  vlayout2->addStretch(100);
  vlayout2->addWidget(colorBarCheckBox);
  vlayout2->addWidget(comparisonModeCheckBox);

  QVBoxLayout *scaleButtonGroupLayout = new QVBoxLayout();
  for (int i=0; i<scaleButtonGroup->buttons().count(); i++)
  {
    scaleButtonGroupLayout->addWidget(scaleButtonGroup->buttons()[i]);
  }

  QGroupBox *scaleGroupBox = new QGroupBox();
  scaleGroupBox->setLayout(scaleButtonGroupLayout);
  vlayout2->addWidget(scaleGroupBox);



  QHBoxLayout *signalListHeaderLayout = new QHBoxLayout();

  QLabel *lbl = new QLabel();
  lbl->setObjectName("lblSingalNames");
  lbl->setMinimumWidth(150);
  lbl->setMaximumWidth(150);
  lbl->setText("Signal name");
  signalListHeaderLayout->addWidget(lbl);

  timeFrameLabels.clear();
  for (int i=1; i<4; i++) {
    QLabel* lb = new QLabel();
    lb->setObjectName("lblTimeFrame" + QString::number(i));
    lb->setMinimumWidth(50);
    lb->setMaximumWidth(50);
    lb->setText(QString::number(i) + " sec");
    signalListHeaderLayout->addWidget(lb);
    timeFrameLabels.append(lb);
  }
  vlayout2->addLayout(signalListHeaderLayout);



  hlayout1 = new QHBoxLayout();
  hlayout1->setSpacing(20);
  hlayout1->addLayout(vlayout2);
  if(dashboard)
  {
    hlayout1->addWidget(histogramView, 100);
  }


  SpectrumDialog->setLayout(hlayout1);

  t1 = new QTimer(this);
  t1->setSingleShot(true);

  QObject::connect(t1,              SIGNAL(timeout()),              this, SLOT(update_curve()));
  QObject::connect(colorBarCheckBox,  SIGNAL(toggled(bool)),          this, SLOT(colorBarButtonClicked(bool)));
  QObject::connect(comparisonModeCheckBox,  SIGNAL(toggled(bool)),          this, SLOT(comparisonModeClicked(bool)));
  QObject::connect(histogramView,          SIGNAL(extra_button_clicked()), this, SLOT(print_to_txt()));
  QObject::connect(histogramView,          SIGNAL(dashBoardClicked()),     this, SLOT(setdashboard()));
}

void UI_SpectrumDockWindow::addSignalWidgets(QBoxLayout *parentLayout, int histogramSignalGroupIndex)
{    
  HistogramSignalGroup *signalGroup = signalMatrix[histogramSignalGroupIndex];

  QString signalListLayoutName = "sllayout" + QString::number(histogramSignalGroupIndex);

  // TODO: this is temporary, because I couldn't delete the old widget
  QHBoxLayout* oldLayout = parentLayout->findChild<QHBoxLayout*>(signalListLayoutName);
  if (oldLayout)
  {
    return;
  }

  QHBoxLayout *signalListLayout = new QHBoxLayout();
  signalListLayout->setObjectName(signalListLayoutName);

  QCheckBox *chk = new QCheckBox();
  chk->setObjectName("chk" + QString::number(histogramSignalGroupIndex * 100));
  chk->setMinimumWidth(150);
  chk->setMaximumWidth(150);
  chk->setChecked(signalGroup->enabled);
  chk->setText(signalGroup->base->GetDisplayName());
  //chk->setStyleSheet("background-color: red;");
  signalListLayout->addWidget(chk);

  connect(chk, SIGNAL(stateChanged(int)), this, SLOT(signalGroupEnabledCheckBoxStateChanged(int)));



  for (int i=0; i<signalGroup->fft.count(); ++i) {
    QCheckBox* cb = new QCheckBox();
    cb->setObjectName("chk" + QString::number(histogramSignalGroupIndex * 100 + i));
    cb->setMinimumWidth(50);
    cb->setMaximumWidth(50);
    cb->setChecked(signalGroup->fft_enabled[i]);
    cb->setToolTip(signalGroup->fft[i]->GetDisplayName());
    signalListLayout->addWidget(cb);

    connect(cb, SIGNAL(stateChanged(int)), this, SLOT(signalGroupFFTEnabledCheckBoxStateChanged(int)));
  }

  if (oldLayout)
  {
    oldLayout->setEnabled(false);
    parentLayout->removeItem(oldLayout);
    delete oldLayout;
  }

  parentLayout->addLayout(signalListLayout);
}

void UI_SpectrumDockWindow::signalGroupEnabledCheckBoxStateChanged(int state)
{
  QCheckBox* chk = dynamic_cast<QCheckBox*>(sender());
  int histogramSignalGroupIndex = chk->objectName().remove(0,3).toInt() / 100;

  HistogramSignalGroup *signalGroup = signalMatrix[histogramSignalGroupIndex];
  signalGroup->enabled = (state == 2);

  update_curve();
}

void UI_SpectrumDockWindow::signalGroupFFTEnabledCheckBoxStateChanged(int state)
{
  QCheckBox* chk = dynamic_cast<QCheckBox*>(sender());
  int histogramSignalGroupIndex = chk->objectName().remove(0,3).toInt() / 100;
  int fftIndex = chk->objectName().remove(0,3).toInt() % 100;

  HistogramSignalGroup *signalGroup = signalMatrix[histogramSignalGroupIndex];
  signalGroup->fft_enabled[fftIndex] = (state == 2);

  update_curve();
}

void UI_SpectrumDockWindow::scaleButtonClicked(bool checked)
{
  if (!checked) return;

  SignalType newMode = SignalType::FFT;

  for(int i=0; i<scaleButtonGroup->buttons().count(); i++)
  {
    if (scaleButtonGroup->buttons()[i]->isChecked())
    {
      switch (i)
        {
        case 0:
          break;
        case 1:
          newMode = newMode | SignalType::LogScale;
          break;
        case 2:
          newMode = newMode | SignalType::SquareRoot;
          break;
        case 3:
          newMode = newMode | SignalType::LogScale | SignalType::SquareRoot;
          break;
        default:
          return;
        }
      }
  }

  changeSignals(base_samples, newMode);
}

void UI_SpectrumDockWindow::setsettings(struct spectrumdocksettings sett)
{
  settings = sett;

  set_settings = 1;
}


void UI_SpectrumDockWindow::getsettings(struct spectrumdocksettings *sett)
{
  sett->signalnr = signal_nr;

  if(colorBarCheckBox->isChecked() == true)
  {
    sett->colorbar = 1;
  }
  else
  {
    sett->colorbar = 0;
  }

  sett->maxvalue = 0; //maxvalue;
}



void UI_SpectrumDockWindow::setdashboard()
{
  if(dashboard)
  {
    dashboard = 0;
    dock->setWidget(histogramView);
  }
  else
  {
    dashboard = 1;
    hlayout1->addWidget(histogramView, 100);
    dock->setWidget(SpectrumDialog);
  }
}


void UI_SpectrumDockWindow::print_to_txt()
{
  // TODO: rewrite TXT printer

  /*
  int i;

  char str[1024],
       path[MAX_PATH_LENGTH];

  FILE *outputfile;


  path[0] = 0;
  if(mainwindow->recent_savedir[0]!=0)
  {
    strcpy(path, mainwindow->recent_savedir);
    strcat(path, "/");
  }
  strcat(path, "spectrum.txt");

  strcpy(path, QFileDialog::getSaveFileName(0, "Export to text (ASCII)", QString::fromLocal8Bit(path), "Text files (*.txt *.TXT)").toLocal8Bit().data());

  if(!strcmp(path, ""))
  {
    return;
  }

  get_directory_from_path(mainwindow->recent_savedir, path, MAX_PATH_LENGTH);

  outputfile = fopen(path, "wb");
  if(outputfile == NULL)
  {
    snprintf(str, 1024, "Can not open file %s for writing.", path);
    QMessageBox messagewindow(QMessageBox::Critical, "Error", QString::fromLocal8Bit(str));
    messagewindow.exec();
    return;
  }

  sprintf(str, "FFT Power Spectral Density (Power/%fHz)\n", 1.0 / fft_ts1->GetHorizontalDensity());
  remove_trailing_zeros(str);
  fprintf(outputfile, "%s", str);
  fprintf(outputfile, "Signal: %s\n", signalcomp->signallabel);
  sprintf(str, "FFT blocksize: %i\n", mainwindow->maxdftblocksize);
  sprintf(str + strlen(str), "FFT resolution: %f Hz\n", 1.0 / fft_ts1->GetHorizontalDensity());
  sprintf(str + strlen(str), "Data Samples: %i\n", base_samples->GetValues().count());
  sprintf(str + strlen(str), "Power Samples: %i\n", fft_ts1->GetValues().count());
  sprintf(str + strlen(str), "Samplefrequency: %f Hz\n", (double)signalcomp->edfhdr->edfparam[signalcomp->edfsignal[0]].smp_per_record / ((double)signalcomp->edfhdr->long_data_record_duration / TIME_DIMENSION));
  remove_trailing_zeros(str);
  fprintf(outputfile, "%s", str);

  for(i=0; i<fft_ts1->GetValues().count(); i++)
  {
    fprintf(outputfile, "%.16f\t%.16f\n", (1.0 / fft_ts1->GetHorizontalDensity()) * i, fft_ts1->GetValues()[i]);
  }

  fclose (outputfile);
  */
}


void UI_SpectrumDockWindow::colorBarButtonClicked(bool value)
{
  if(value == true)
  {
    histogramView->enableSpectrumColors(spectrum_color);
  }
  else
  {
    histogramView->disableSpectrumColors();
  }
}

void UI_SpectrumDockWindow::comparisonModeClicked(bool value)
{
  if(value == true)
  {
    comparisonMode = true;
    histogramView->enableComparisonMode();
  }
  else
  {
    comparisonMode = false;
    histogramView->disableComparisonMode();
  }
}

void UI_SpectrumDockWindow::changeSignals(Signal* signal, SignalType newMode)
{
  // backward compatibility
  mainwindow->spectrumdock_sqrt = ((newMode & SignalType::SquareRoot) == SignalType::SquareRoot);
  mainwindow->spectrumdock_vlog = ((newMode & SignalType::LogScale) == SignalType::LogScale);


  QColor signalColor = base_samples->GetPen().color();

  // generate the new fft signals
  Signal *fft_ts1 = changeMode(newMode, signal->GetName(), signal->GetAlias(), signal->GetVerticalUnit());
  fft_ts1->SetPen(QPen(QColor(signalColor.red(), signalColor.green(), signalColor.blue(), signalColor.alpha() * 0.6), 1.0, Qt::DotLine));

  Signal *fft_ts8 = new Signal(fft_ts1->GetId() + "/8", fft_ts1->GetName() + "/8", fft_ts1->GetAlias() + "/8", QVector<double>(), fft_ts1->GetHorizontalName(), fft_ts1->GetHorizontalUnit(), fft_ts1->GetHorizontalDensity(), fft_ts1->GetVerticalName(), fft_ts1->GetVerticalUnit(), fft_ts1->GetVerticalDensity(), fft_ts1->GetType());
  fft_ts8->SetPen(QPen(QColor(signalColor.red(), signalColor.green(), signalColor.blue(), signalColor.alpha() * 0.8), 1.0, Qt::DashDotLine));

  Signal *fft_ts64 = new Signal(fft_ts1->GetId() + "/64", fft_ts1->GetName() + "/64", fft_ts1->GetAlias() + "/64", QVector<double>(), fft_ts1->GetHorizontalName(), fft_ts1->GetHorizontalUnit(), fft_ts1->GetHorizontalDensity(), fft_ts1->GetVerticalName(), fft_ts1->GetVerticalUnit(), fft_ts1->GetVerticalDensity(), fft_ts1->GetType());
  fft_ts64->SetPen(QPen(QColor(signalColor.red(), signalColor.green(), signalColor.blue(), signalColor.alpha() * 1.0), 1.0, Qt::SolidLine));

  signalMatrix[0]->length_in_seconds.clear();
  signalMatrix[0]->fft_enabled.clear();
  signalMatrix[0]->fft.clear();

  signalMatrix[0]->length_in_seconds.append(1.0);
  signalMatrix[0]->fft_enabled.append(true);
  signalMatrix[0]->fft.append(fft_ts1);

  signalMatrix[0]->length_in_seconds.append(8.0);
  signalMatrix[0]->fft_enabled.append(true);
  signalMatrix[0]->fft.append(fft_ts8);

  signalMatrix[0]->length_in_seconds.append(64.0);
  signalMatrix[0]->fft_enabled.append(true);
  signalMatrix[0]->fft.append(fft_ts64);

  addSignalWidgets(vlayout2, 0);

  // add signals to the histogram
  histogramView->removeSignal();
  histogramView->addSignal(signalMatrix[0]->fft[0]);
  histogramView->addSignal(signalMatrix[0]->fft[1]);
  histogramView->addSignal(signalMatrix[0]->fft[2]); 

  this->rescan();
}


void UI_SpectrumDockWindow::init(int signal_num)
{

  if(signal_num < 0)
  {
    signalcomp = NULL;
  }
  else
  {
    signal_nr = signal_num;

    signalcomp = mainwindow->signalcomp[signal_num];

    QString base_unit = signalcomp->physdimension;
    QString signal_name = signalcomp->signallabel;
    QString signal_alias = signalcomp->alias;

    base_samples = new Signal("ADC", signal_name, signal_alias, QVector<double>(), "Time", "seconds", (double)signalcomp->edfhdr->edfparam[signalcomp->edfsignal[0]].smp_per_record / ((double)signalcomp->edfhdr->long_data_record_duration / TIME_DIMENSION), "Voltage", base_unit, 1.0, SignalType::ADC);
    base_samples->SetPen(QPen(static_cast<Qt::GlobalColor>(signalcomp->color)));

    signalMatrix.clear();
    HistogramSignalGroup *hsg = new HistogramSignalGroup(base_samples);
    signalMatrix.append(hsg);

    dock->setWindowTitle("Histogram of " + base_samples->GetDisplayName());

    changeSignals(base_samples, SignalType::FFT | SignalType::SquareRoot | SignalType::LogScale);

    dock->show(); 
  }
}


void UI_SpectrumDockWindow::rescan()
{
  t1->start(1);
}


void UI_SpectrumDockWindow::clear()
{
  int i;

  signalcomp = NULL;


  if(spectrum_color != NULL)
  {
    for(i=0; i < spectrum_color->items; i++)
    {
      spectrum_color->value[i] = 0.0;
    }
  }

  histogramView->clear();

  signal_nr = -1;
}


void UI_SpectrumDockWindow::update_curve()
{
  int i, j, n,
      dftblocksize,
      fft_outputbufsize;

  char str[512];


  if(signalcomp == NULL)
  {
    return;
  }

  if(busy)
  {
    return;
  }

  busy = 1;

  histogramView->setUpdatesEnabled(false);

  {
  long long buf_samples_count = signalcomp->samples_on_screen;

  if(signalcomp->samples_on_screen > signalcomp->sample_stop)
  {
    buf_samples_count = signalcomp->sample_stop;
  }

  buf_samples_count -= signalcomp->sample_start;

  if((buf_samples_count < 10) || (mainwindow->viewbuf == NULL))
  {
    histogramView->setUpdatesEnabled(true);

    busy = 0;

    if(spectrum_color != NULL)
    {
      for(i=0; i < spectrum_color->items; i++)
      {
        spectrum_color->value[i] = 0.0;
      }
    }

    histogramView->clear();

    return;
  }
  }


  // ---------- get the FFT input data from the original samples
  base_samples->SetValues(compileSignalFromRawData(signalcomp, mainwindow->viewbuf));
  QVector<double> buffer_of_samples = base_samples->GetValues();


  dftblocksize = mainwindow->maxdftblocksize;

  if(dftblocksize > buffer_of_samples.count())
  {
    dftblocksize = buffer_of_samples.count();
  }

  if(dftblocksize & 1)
  {
    dftblocksize--;
  }

  double samplefreq = (double)signalcomp->edfhdr->edfparam[signalcomp->edfsignal[0]].smp_per_record / ((double)signalcomp->edfhdr->long_data_record_duration / TIME_DIMENSION);
  double freqstep = samplefreq / (double)dftblocksize;

  fft_outputbufsize = dftblocksize / 2;



  // ---------- FFT CALCULATION
  QVector<double> comparisonFFTData = QVector<double>();

  for (int j=0; j<signalMatrix.count(); j++)
  {
    comparisonFFTData.clear();

    for (int i=0; i<signalMatrix[j]->fft.count(); i++)
    {
        QVector<double> fft_values = QVector<double>();
        if (signalMatrix[j]->enabled && signalMatrix[j]->fft_enabled[i])
          {
            FFTCalculationResult* fftCalculationResult = calculateFFT(buffer_of_samples, fft_outputbufsize, dftblocksize, samplefreq, signalMatrix[j]->fft[i]->SIGNAL_NA_VALUE);
            emit fftCalculated(signalMatrix[j], signalMatrix[j]->fft[i], fftCalculationResult);
            signalMatrix[j]->length_in_seconds[i] = fftCalculationResult->DFTBlockCount * fftCalculationResult->SampleLengthInSeconds;

            if (fftCalculationResult->DFTBlockCount == 0)
            {
              timeFrameLabels[i]->setText("---");
            } else {
              timeFrameLabels[i]->setText(QString("%L1").arg(signalMatrix[j]->length_in_seconds[i], 0, 'f', 1) + " sec");
            }

            if (!comparisonMode)
            {
                fft_values = fftCalculationResult->FFTData;
            }
            else
            {
              if (comparisonFFTData.count() == 0)
              {
                // that's the first FFT, so we are going to compare everything to this
                comparisonFFTData = fftCalculationResult->FFTData;
              }

              for (int k=0; k<comparisonFFTData.count(); k++)
              {
                if ((comparisonFFTData[k] != signalMatrix[j]->fft[i]->SIGNAL_NA_VALUE)
                    && (fftCalculationResult->FFTData[k] != signalMatrix[j]->fft[i]->SIGNAL_NA_VALUE))
                {
                  fft_values.append(fftCalculationResult->FFTData[k] - comparisonFFTData[k]);
                }
                else
                {
                  fft_values.append(signalMatrix[j]->fft[i]->SIGNAL_NA_VALUE);
                }
              }
            }

            // we need to transform the values if they are logarithmic and/or power values
            fft_values = transformFFTValues(fft_values, signalMatrix[j]->fft[i]->SIGNAL_NA_VALUE, freqstep);
          } else {
            fft_values = QVector<double>(fft_outputbufsize, signalMatrix[j]->fft[i]->SIGNAL_NA_VALUE);
          }
        signalMatrix[j]->fft[i]->SetHorizontalDensity(1.0/freqstep);
        signalMatrix[j]->fft[i]->SetValues(fft_values);

        buffer_of_samples.remove(0, 3 * (buffer_of_samples.count() / 4));
    }
  }

// TODO: I couldn't figure out why is this here
//  if(signalcomp->ecg_filter == NULL)
//  {
//    fft_values[0] /= 2.0;  // DC!
//  }
//  else
//  {
//    fft_values[0] = 0.0;  // Remove DC because heart rate is always a positive value
//  }


  // set the title of the histogram
  sprintf(str, "FFT resolution: %f Hz   %i samples", freqstep, dftblocksize);

  remove_trailing_zeros(str);

  histogramView->setHeaderText(str);


  QVector<double> main_signal_values = signalMatrix[0]->fft[0]->GetValues();

  // calculate the colorbar values
  if(spectrum_color != NULL)
  {
    if(spectrum_color->items > 0)
    {
      spectrum_color->value[0] = 0.0;

      n = 0;

      for(j=0; j<main_signal_values.count(); j++)
      {
        if(((freqstep * j) + (freqstep * 0.5)) < spectrum_color->freq[0])
        {
          if(spectrum_color->method == 0)  // sum
          {
            spectrum_color->value[0] += main_signal_values[j];
          }

          if(spectrum_color->method == 1)  // peak
          {
            if(spectrum_color->value[0] < main_signal_values[j])
            {
              spectrum_color->value[0] = main_signal_values[j];
            }
          }

          if(spectrum_color->method == 2)  // average
          {
            spectrum_color->value[0] += main_signal_values[j];

            n++;
          }
        }
      }

      if(spectrum_color->method == 2)  // average
      {
        if(n)
        {
          spectrum_color->value[0] /= n;
        }
      }
    }

    for(i=1; i < spectrum_color->items; i++)
    {
      spectrum_color->value[i] = 0.0;

      n = 0;

      for(j=0; j<main_signal_values.count(); j++)
      {
        if((((freqstep * j) + (freqstep * 0.5)) > spectrum_color->freq[i-1]) && (((freqstep * j) + (freqstep * 0.5)) < spectrum_color->freq[i]))
        {
          if(spectrum_color->method == 0)  // sum
          {
            spectrum_color->value[i] += main_signal_values[j];
          }

          if(spectrum_color->method == 1)  // peak
          {
            if(spectrum_color->value[i] < main_signal_values[j])
            {
              spectrum_color->value[i] = main_signal_values[j];
            }
          }

          if(spectrum_color->method == 2)  // average
          {
            spectrum_color->value[i] += main_signal_values[j];

            n++;
          }
        }
      }

      if(spectrum_color->method == 2)  // average
      {
        if(n)
        {
          spectrum_color->value[i] /= n;
        }
      }
    }
  }


    // load the settings back from the montage
  if(set_settings)
  {
    set_settings = 0;

    if(settings.colorbar > 0)
    {
      colorBarCheckBox->setChecked(true);
    }
    else
    {
      colorBarCheckBox->setChecked(false);
    }
  }

  histogramView->setUpdatesEnabled(true);

  busy = 0;
}

QVector<double> UI_SpectrumDockWindow::compileSignalFromRawData(signalcompblock *signalcomp, char *viewbuf)
{
    union {
            unsigned int one;
            signed int one_signed;
            unsigned short two[2];
            signed short two_signed[2];
            unsigned char four[4];
          } var;


    QVector<double> result = QVector<double>();

    for(long long s=signalcomp->sample_start; s<signalcomp->samples_on_screen; s++)
    {
      if(s>signalcomp->sample_stop)  break;

      double dig_value = 0.0;
      long long s2 = s + signalcomp->sample_timeoffset - signalcomp->sample_start;


      double f_tmp = 0.0;

      for(int j=0; j<signalcomp->num_of_signals; j++)
      {
        if(signalcomp->edfhdr->bdf)
        {
          var.two[0] = *((unsigned short *)(
            viewbuf
            + signalcomp->viewbufoffset
            + (signalcomp->edfhdr->recordsize * (s2 / signalcomp->edfhdr->edfparam[signalcomp->edfsignal[j]].smp_per_record))
            + signalcomp->edfhdr->edfparam[signalcomp->edfsignal[j]].buf_offset
            + ((s2 % signalcomp->edfhdr->edfparam[signalcomp->edfsignal[j]].smp_per_record) * 3)));

          var.four[2] = *((unsigned char *)(
            viewbuf
            + signalcomp->viewbufoffset
            + (signalcomp->edfhdr->recordsize * (s2 / signalcomp->edfhdr->edfparam[signalcomp->edfsignal[j]].smp_per_record))
            + signalcomp->edfhdr->edfparam[signalcomp->edfsignal[j]].buf_offset
            + ((s2 % signalcomp->edfhdr->edfparam[signalcomp->edfsignal[j]].smp_per_record) * 3)
            + 2));

          if(var.four[2]&0x80)
          {
            var.four[3] = 0xff;
          }
          else
          {
            var.four[3] = 0x00;
          }

          f_tmp = var.one_signed;
        }

        if(signalcomp->edfhdr->edf)
        {
          f_tmp = *(((short *)(
            viewbuf
            + signalcomp->viewbufoffset
            + (signalcomp->edfhdr->recordsize * (s2 / signalcomp->edfhdr->edfparam[signalcomp->edfsignal[j]].smp_per_record))
            + signalcomp->edfhdr->edfparam[signalcomp->edfsignal[j]].buf_offset))
            + (s2 % signalcomp->edfhdr->edfparam[signalcomp->edfsignal[j]].smp_per_record));
        }

        f_tmp += signalcomp->edfhdr->edfparam[signalcomp->edfsignal[j]].offset;
        f_tmp *= signalcomp->factor[j];

        dig_value += f_tmp;
      }

      // apply the filters
      if(signalcomp->spike_filter)
      {
        if(s==signalcomp->sample_start)
        {
          spike_filter_restore_buf(signalcomp->spike_filter);
        }

        dig_value = run_spike_filter(dig_value, signalcomp->spike_filter);
      }

      for(int k=0; k<signalcomp->filter_cnt; k++)
      {
        dig_value = first_order_filter(dig_value, signalcomp->filter[k]);
      }

      for(int k=0; k<signalcomp->ravg_filter_cnt; k++)
      {
        if(s==signalcomp->sample_start)
        {
          ravg_filter_restore_buf(signalcomp->ravg_filter[k]);
        }

        dig_value = run_ravg_filter(dig_value, signalcomp->ravg_filter[k]);
      }

      for(int k=0; k<signalcomp->fidfilter_cnt; k++)
      {
        if(s==signalcomp->sample_start)
        {
          memcpy(signalcomp->fidbuf[k], signalcomp->fidbuf2[k], fid_run_bufsize(signalcomp->fid_run[k]));
        }

        dig_value = signalcomp->fidfuncp[k](signalcomp->fidbuf[k], dig_value);
      }

      if(signalcomp->ecg_filter != NULL)
      {
        if(s==signalcomp->sample_start)
        {
          ecg_filter_restore_buf(signalcomp->ecg_filter);
        }

        dig_value = run_ecg_filter(dig_value, signalcomp->ecg_filter);
      }

      result.append(dig_value * signalcomp->edfhdr->edfparam[signalcomp->edfsignal[0]].bitvalue);
    }

    return result;
}

FFTCalculationResult* UI_SpectrumDockWindow::calculateFFT(QVector<double> buf_samples, int fft_outputbufsize, int dftblocksize, double samplefreq, double SIGNAL_NA_VALUE)
{
    FFTCalculationResult* result = new FFTCalculationResult();

    kiss_fftr_cfg cfg;
    QVector<double> fftdata = QVector<double>(fft_outputbufsize);

    cfg = kiss_fftr_alloc(dftblocksize, 0, NULL, NULL);

    int dftblocks = 0;
    int start_offset = buf_samples.count();
    while (start_offset - dftblocksize >= 0)
    {
        start_offset -= dftblocksize;
        dftblocks++;

        kiss_fftr_thread_func(cfg, buf_samples.data() + start_offset, &fftdata);

        // TODO: multithread support
        // std::thread t(&UI_SpectrumDockWindow::kiss_fftr_thread_func, this, cfg, buf_samples.data() + start_offset, &result);
        // t.join();
    }

    if (start_offset > 0)
    {
        // there are some samples left, we need to FFT the rest
        // NOTE: there will be some overlap on the samples, so I'm not sure this is a good idea

        start_offset = 0;
        //dftblocks++;

        //kiss_fftr_thread_func(cfg, buf_samples.data() + start_offset, &result);
    }


    for(int i=0; i<fftdata.count(); i++)
    {
        if (dftblocks > 0)
        {
          fftdata[i] /= (dftblocks);
          fftdata[i] /= samplefreq;
        }
        else
        {
          fftdata[i] = SIGNAL_NA_VALUE;
        }
    }


    free(cfg);


    //double samples_length_in_seconds = (double)buf_samples.count() / samplefreq;
    double samples_length_in_seconds = (double)dftblocksize / samplefreq;
    double freqstep = samplefreq / (double)dftblocksize;
    double freq_band = 0.0;

    // there are two limitations to the FFT algorithm
    // A, we can't meassure the intensity of a signal with a higher frequency then the half of the sample rate
    freq_band = freqstep * fftdata.count();
    for (int i=fftdata.count()-1; (freq_band<(samplefreq/2)) && (i>=0); i--)
    {
        fftdata[i] = SIGNAL_NA_VALUE;
        freq_band -= freqstep;
    }

    // B, the meassurement of the signals with wavelength less than the sample will be inaccurate
    freq_band = 0.0;
    for (int i=0; (freq_band<(1/samples_length_in_seconds)) && (i<fftdata.count()); i++)
    {
        fftdata[i] = SIGNAL_NA_VALUE;
        freq_band += freqstep;
    }


    result->Samples = buf_samples;
    result->FFTData = fftdata;

    result->DFTBlockSize = dftblocksize;
    result->DFTBlockCount = dftblocks;

    result->StartIndex = start_offset;
    result->EndIndex = buf_samples.count();

    result->SampleFrequency = samplefreq;
    result->FrequencyStep = freqstep;
    result->SampleLengthInSeconds = samples_length_in_seconds;

    return result;
}

void UI_SpectrumDockWindow::kiss_fftr_thread_func(kiss_fftr_cfg st, const kiss_fft_scalar *timedata, QVector<double> *result)
{
  kiss_fft_cpx *kiss_fftbuf;

  kiss_fftbuf = (kiss_fft_cpx *)malloc((result->count() + 1) * sizeof(kiss_fft_cpx));
  if(kiss_fftbuf == NULL)
  {
    throw std::runtime_error(std::string("The system was not able to provide enough resources (memory) to perform the requested action."));
  }

  kiss_fftr(st, timedata, kiss_fftbuf);
  for(int i=0; i<result->count(); i++)
  {
    (*result)[i] += (((kiss_fftbuf[i].r * kiss_fftbuf[i].r) + (kiss_fftbuf[i].i * kiss_fftbuf[i].i)) / result->count());
  }

  free(kiss_fftbuf);
}

Signal* UI_SpectrumDockWindow::changeMode(SignalType historgramMode, QString signalName, QString signalAlias, QString baseUnit)
{
    Signal *result;

    this->historgramMode = historgramMode;

    QString verticalRulerName = "";

    if ((historgramMode & SignalType::FFT) != SignalType::FFT) return result;

    signalName += "-FFT";

    if ((historgramMode & SignalType::SquareRoot) == SignalType::SquareRoot)
    {
        signalName += "-SQRT";
        verticalRulerName = "Power";
        baseUnit = "(" + baseUnit + ")^2/Hz";
    } else {
        verticalRulerName = "Amplitude";
    }

    if ((historgramMode & SignalType::LogScale) == SignalType::LogScale)
    {
        signalName += "-VLOG";
        baseUnit = "log(" + baseUnit + ")";
    }

    result = new Signal("FFT", signalName, signalAlias, QVector<double>(), "Frequency", "Hz", 1.0, verticalRulerName, baseUnit, 1.0, historgramMode);

    return result;
}

double UI_SpectrumDockWindow::transformFFTValue(double value, double SIGNAL_NA_VALUE, double freqstep)
{
    if (value == SIGNAL_NA_VALUE) return value;

    if ((historgramMode & SignalType::SquareRoot) == SignalType::SquareRoot)
    {
        value = sqrt(value * freqstep);
    }

    if ((historgramMode & SignalType::LogScale) == SignalType::LogScale)
    {
        value = (value > SPECT_LOG_MINIMUM ? log10(value) : log10(SPECT_LOG_MINIMUM));
    }

    return value;
}

QVector<double> UI_SpectrumDockWindow::transformFFTValues(QVector<double> fft_values, double SIGNAL_NA_VALUE, double freqstep)
{
  QVector<double> result = QVector<double>();

  for(int i=0; i<fft_values.count(); i++)
  {
      if (fft_values[i] == SIGNAL_NA_VALUE)
      {
        result.append(SIGNAL_NA_VALUE);
      } else {
        result.append(transformFFTValue(fft_values[i], SIGNAL_NA_VALUE, freqstep));
      }
  }

  return result;
}



UI_SpectrumDockWindow::~UI_SpectrumDockWindow()
{
  delete SpectrumDialog;
}

