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

  flywheel_value = 1050;

  init_maxvalue = 1;

  if(mainwindow->spectrumdock_sqrt)
  {
    dock = new QDockWidget("Amplitude Spectrum", w_parent);
  }
  else
  {
    dock = new QDockWidget("Power Spectrum", w_parent);
  }

  dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  dock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
  dock->setMinimumHeight(300);
  if(dashboard)
  {
    dock->setWidget(SpectrumDialog);
  }

  curve1 = new SignalCurve;  
  curve1->setSignalColor(QColor(255,127,0));

  curve1->create_button("to Text");

  if(!dashboard)
  {
    dock->setWidget(curve1);
  }

  sqrtButton = new QCheckBox;
  sqrtButton->setMinimumSize(50, 20);
  sqrtButton->setText("Amplitude");
  sqrtButton->setTristate(false);
  if(mainwindow->spectrumdock_sqrt)
  {
    sqrtButton->setChecked(true);
  }
  else
  {
    sqrtButton->setChecked(false);
  }

  vlogButton = new QCheckBox;
  vlogButton->setMinimumSize(50, 20);
  vlogButton->setText("Log");
  vlogButton->setTristate(false);
  if(mainwindow->spectrumdock_vlog)
  {
    vlogButton->setChecked(true);
  }
  else
  {
    vlogButton->setChecked(false);
  }

  colorBarButton = new QCheckBox;
  colorBarButton->setMinimumSize(50, 20);
  colorBarButton->setText("Colorbar");
  colorBarButton->setTristate(false);

  vlayout3 = new QVBoxLayout;
  vlayout3->addStretch(100);
  vlayout3->addStretch(100);

  hlayout4 = new QHBoxLayout;
  hlayout4->addStretch(100);
  hlayout4->addLayout(vlayout3, 100);
  hlayout4->addStretch(100);

  vlayout2 = new QVBoxLayout;
  vlayout2->setSpacing(10);
  vlayout2->addStretch(100);
  vlayout2->addLayout(hlayout4, 200);
  vlayout2->addWidget(sqrtButton);
  vlayout2->addWidget(vlogButton);
  vlayout2->addWidget(colorBarButton);  

  hlayout1 = new QHBoxLayout;
  hlayout1->setSpacing(20);
  hlayout1->addLayout(vlayout2);
  if(dashboard)
  {
    hlayout1->addWidget(curve1, 100);
  }

  hlayout2 = new QHBoxLayout;
  hlayout2->setSpacing(20);
  hlayout2->addStretch(100);

  hlayout3 = new QHBoxLayout;
  hlayout3->setSpacing(20);
  hlayout3->addStretch(100);

  vlayout1 = new QVBoxLayout;
  vlayout1->setSpacing(20);
  vlayout1->addLayout(hlayout1);
  vlayout1->addLayout(hlayout2);
  vlayout1->addLayout(hlayout3);

  SpectrumDialog->setLayout(vlayout1);

  t1 = new QTimer(this);
  t1->setSingleShot(true);

  QObject::connect(t1,              SIGNAL(timeout()),              this, SLOT(update_curve()));
  QObject::connect(sqrtButton,      SIGNAL(toggled(bool)),          this, SLOT(sqrtButtonClicked(bool)));
  QObject::connect(vlogButton,      SIGNAL(toggled(bool)),          this, SLOT(vlogButtonClicked(bool)));
  QObject::connect(colorBarButton,  SIGNAL(toggled(bool)),          this, SLOT(colorBarButtonClicked(bool)));
  QObject::connect(curve1,          SIGNAL(extra_button_clicked()), this, SLOT(print_to_txt()));
  QObject::connect(curve1,          SIGNAL(dashBoardClicked()),     this, SLOT(setdashboard()));
}


void UI_SpectrumDockWindow::setsettings(struct spectrumdocksettings sett)
{
  settings = sett;

  set_settings = 1;
}


void UI_SpectrumDockWindow::getsettings(struct spectrumdocksettings *sett)
{
  sett->signalnr = signal_nr;

  if(sqrtButton->isChecked() == true)
  {
    sett->sqrt = 1;
  }
  else
  {
    sett->sqrt = 0;
  }

  if(vlogButton->isChecked() == true)
  {
    sett->log = 1;
  }
  else
  {
    sett->log = 0;
  }

  if(colorBarButton->isChecked() == true)
  {
    sett->colorbar = 1;
  }
  else
  {
    sett->colorbar = 0;
  }

  sett->maxvalue = maxvalue;

  sett->maxvalue_sqrt = maxvalue_sqrt;

  sett->maxvalue_vlog = maxvalue_vlog;

  sett->maxvalue_sqrt_vlog = maxvalue_sqrt_vlog;

  sett->minvalue_vlog = minvalue_vlog;

  sett->minvalue_sqrt_vlog = minvalue_sqrt_vlog;
}



void UI_SpectrumDockWindow::setdashboard()
{
  if(dashboard)
  {
    dashboard = 0;
    dock->setWidget(curve1);
  }
  else
  {
    dashboard = 1;
    hlayout1->addWidget(curve1, 100);
    dock->setWidget(SpectrumDialog);
  }
}


void UI_SpectrumDockWindow::print_to_txt()
{
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

  sprintf(str, "FFT Power Spectral Density (Power/%fHz)\n", 1.0 / fft->GetHorizontalDensity());
  remove_trailing_zeros(str);
  fprintf(outputfile, "%s", str);
  fprintf(outputfile, "Signal: %s\n", signalcomp->signallabel);
  sprintf(str, "FFT blocksize: %i\n", mainwindow->maxdftblocksize);
  sprintf(str + strlen(str), "FFT resolution: %f Hz\n", 1.0 / fft->GetHorizontalDensity());
  sprintf(str + strlen(str), "Data Samples: %i\n", base_samples->GetValues().count());
  sprintf(str + strlen(str), "Power Samples: %i\n", fft->GetValues().count());
  sprintf(str + strlen(str), "Samplefrequency: %f Hz\n", (double)signalcomp->edfhdr->edfparam[signalcomp->edfsignal[0]].smp_per_record / ((double)signalcomp->edfhdr->long_data_record_duration / TIME_DIMENSION));
  remove_trailing_zeros(str);
  fprintf(outputfile, "%s", str);

  for(i=0; i<fft->GetValues().count(); i++)
  {
    fprintf(outputfile, "%.16f\t%.16f\n", (1.0 / fft->GetHorizontalDensity()) * i, fft->GetValues()[i]);
  }

  fclose (outputfile);
}


void UI_SpectrumDockWindow::colorBarButtonClicked(bool value)
{
  if(value == true)
  {
    curve1->enableSpectrumColors(spectrum_color);
  }
  else
  {
    curve1->disableSpectrumColors();
  }
}


void UI_SpectrumDockWindow::sqrtButtonClicked(bool value)
{
  if(value == false)
  {
    mainwindow->spectrumdock_sqrt = 0;
  }
  else
  {
    mainwindow->spectrumdock_sqrt = 1;
  }

  changeSignals();
}


void UI_SpectrumDockWindow::vlogButtonClicked(bool value)
{
  if(value == false)
  {
    mainwindow->spectrumdock_vlog = 0;
  }
  else
  {
    mainwindow->spectrumdock_vlog = 1;
  }

  changeSignals();
}


void UI_SpectrumDockWindow::changeSignals()
{
  if ((sqrtButton->checkState() != Qt::Checked) && (vlogButton->checkState() != Qt::Checked))
  {
    mainwindow->spectrumdock_sqrt = 0;
    mainwindow->spectrumdock_vlog = 0;

    curve1->addSignal(fft);
  }
  else if ((sqrtButton->checkState() == Qt::Checked) && (vlogButton->checkState() != Qt::Checked))
  {
    mainwindow->spectrumdock_sqrt = 1;
    mainwindow->spectrumdock_vlog = 0;

    curve1->addSignal(fft_sqrt);
  }
  else if ((sqrtButton->checkState() != Qt::Checked) && (vlogButton->checkState() == Qt::Checked))
  {
    mainwindow->spectrumdock_sqrt = 0;
    mainwindow->spectrumdock_vlog = 1;

    curve1->addSignal(fft_vlog);
  }
  else if ((sqrtButton->checkState() == Qt::Checked) && (vlogButton->checkState() == Qt::Checked))
  {
    mainwindow->spectrumdock_sqrt = 1;
    mainwindow->spectrumdock_vlog = 1;

    curve1->addSignal(fft_sqrt_vlog);
  }
}


void UI_SpectrumDockWindow::init(int signal_num)
{
  init_maxvalue = 1;


  if(signal_num < 0)
  {
    signalcomp = NULL;

    viewbuf = NULL;
  }
  else
  {
    signal_nr = signal_num;

    signalcomp = mainwindow->signalcomp[signal_num];

    QString base_unit = signalcomp->physdimension;
    QString signal_name = signalcomp->signallabel;
    QString signal_alias = signalcomp->alias;

    base_samples = new Signal("ADC", signal_name, signal_alias, QVector<double>(), "Time", "seconds", (double)signalcomp->edfhdr->edfparam[signalcomp->edfsignal[0]].smp_per_record / ((double)signalcomp->edfhdr->long_data_record_duration / TIME_DIMENSION), "Voltage", base_unit, 1.0);
    base_samples->SetColor(QColor(signalcomp->color));

    fft = new Signal("FFT", signal_name + "-FFT", signal_alias + "-FFT", QVector<double>(), "Frequency", "Hz", 1.0, "Amplitude", base_unit, 1.0);
    fft_vlog = new Signal("FFT-VLOG", signal_name + "-FFT-VLOG", signal_alias + "-FFT-VLOG", QVector<double>(), "Frequency", "Hz", 1.0, "Amplitude", "log10("+base_unit+")", 1.0);
    fft_sqrt = new Signal("FFT-SQRT", signal_name + "-FFT-SQRT", signal_alias + "-FFT-SQRT", QVector<double>(), "Frequency", "Hz", 1.0, "Intensity", "("+base_unit+")^2/Hz", 1.0);
    fft_sqrt_vlog = new Signal("FFT-SQRT-VLOG", signal_name + "-FFT-SQRT-VLOG", signal_alias + "-FFT-SQRT-VLOG", QVector<double>(), "Frequency", "Hz", 1.0, "Intensity", "log(("+base_unit+")^2/Hz)", 1.0);

    viewbuf = mainwindow->viewbuf;

    changeSignals();

    dock->show();

    t1->start(1);
  }
}


void UI_SpectrumDockWindow::rescan()
{
  t1->start(1);
}


void UI_SpectrumDockWindow::clear()
{
  int i;

  init_maxvalue = 1;

  signalcomp = NULL;

  viewbuf = NULL;


  if(spectrum_color != NULL)
  {
    for(i=0; i < spectrum_color->items; i++)
    {
      spectrum_color->value[i] = 0.0;
    }
  }

  curve1->clear();

  signal_nr = -1;
}


void UI_SpectrumDockWindow::update_curve()
{
  int i, j, n,
      dftblocksize,
      dftblocks,
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

  viewbuf = mainwindow->viewbuf;

  if(viewbuf == NULL)
  {
    return;
  }

  busy = 1;

  curve1->setUpdatesEnabled(false);

  {
  long long buf_samples_count = signalcomp->samples_on_screen;

  if(signalcomp->samples_on_screen > signalcomp->sample_stop)
  {
    buf_samples_count = signalcomp->sample_stop;
  }

  buf_samples_count -= signalcomp->sample_start;

  if((buf_samples_count < 10) || (viewbuf == NULL))
  {
    curve1->setUpdatesEnabled(true);

    busy = 0;

    if(spectrum_color != NULL)
    {
      for(i=0; i < spectrum_color->items; i++)
      {
        spectrum_color->value[i] = 0.0;
      }
    }

    curve1->clear();

    return;
  }
  }


  // ---------- get the FFT input data from the original samples
  QVector<double> buffer_of_samples = getFFTInputData(signalcomp);


  dftblocksize = mainwindow->maxdftblocksize;

  if(dftblocksize & 1)
  {
    dftblocksize--;
  }

  dftblocks = 1;

  if(dftblocksize < buffer_of_samples.count())
  {
    dftblocks = buffer_of_samples.count() / dftblocksize;
  }
  else
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


  // set up the limits
  if(init_maxvalue && !set_settings)
  {
    maxvalue = 0.000001;
    maxvalue_sqrt = 0.000001;
    maxvalue_vlog = 0.000001;
    maxvalue_sqrt_vlog = 0.000001;
    minvalue_vlog = 0.0;
    minvalue_sqrt_vlog = 0.0;
  }

  if(set_settings)
  {
    maxvalue = settings.maxvalue;
    maxvalue_sqrt = settings.maxvalue_sqrt;
    maxvalue_vlog = settings.maxvalue_vlog;
    maxvalue_sqrt_vlog = settings.maxvalue_sqrt_vlog;
    minvalue_vlog = settings.minvalue_vlog;
    minvalue_sqrt_vlog = settings.minvalue_sqrt_vlog;
  }

  // ---------- FFT CALCULATION
  QVector<double> fft_values = calculateFFT(buffer_of_samples, fft_outputbufsize, dftblocksize, &dftblocks);
  QVector<double> fft_sqrt_values = QVector<double>(fft_values.count());
  QVector<double> fft_vlog_values = QVector<double>(fft_values.count());
  QVector<double> fft_sqrt_vlog_values = QVector<double>(fft_values.count());

  if(signalcomp->ecg_filter == NULL)
  {
    fft_values[0] /= 2.0;  // DC!
  }
  else
  {
    fft_values[0] = 0.0;  // Remove DC because heart rate is always a positive value
  }


  for(i=0; i<fft_values.count(); i++)
  {
      fft_values[i] /= samplefreq;

      double fft_value = fft_values[i];
      double fft_sqrt_value = sqrt(fft_value * freqstep);
      fft_sqrt_values[i] = fft_sqrt_value;

      fft_vlog_values[i] = (fft_value > SPECT_LOG_MINIMUM ? log10(fft_value) : log10(SPECT_LOG_MINIMUM));

      fft_sqrt_vlog_values[i] = (fft_sqrt_value > SPECT_LOG_MINIMUM ? log10(fft_sqrt_value) : log10(SPECT_LOG_MINIMUM));



    if(init_maxvalue && !set_settings)
    {
      if(i)  // don't use the dc-bin for the autogain of the screen
      {
        if(fft_values[i] > maxvalue)
        {
          maxvalue = fft_values[i];
        }

        if(fft_sqrt_values[i] > maxvalue_sqrt)
        {
          maxvalue_sqrt = fft_sqrt_values[i];
        }

        if(fft_vlog_values[i] > maxvalue_vlog)
        {
          maxvalue_vlog = fft_vlog_values[i];
        }

        if(fft_sqrt_vlog_values[i] > maxvalue_sqrt_vlog)
        {
          maxvalue_sqrt_vlog = fft_sqrt_vlog_values[i];
        }

        if((fft_vlog_values[i] < minvalue_vlog) && (fft_vlog_values[i] >= SPECT_LOG_MINIMUM_LOG))
        {
          minvalue_vlog = fft_vlog_values[i];
        }

        if((fft_sqrt_vlog_values[i] < minvalue_sqrt_vlog) && (fft_sqrt_vlog_values[i] >= SPECT_LOG_MINIMUM_LOG))
        {
          minvalue_sqrt_vlog = fft_sqrt_vlog_values[i];
        }
      }
    }
  }

  fft->SetHorizontalDensity(1.0/freqstep);
  fft->SetValues(fft_values);

  fft_sqrt->SetHorizontalDensity(1.0/freqstep);
  fft_sqrt->SetValues(fft_sqrt_values);

  fft_vlog->SetHorizontalDensity(1.0/freqstep);
  fft_vlog->SetValues(fft_vlog_values);

  fft_sqrt_vlog->SetHorizontalDensity(1.0/freqstep);
  fft_sqrt_vlog->SetValues(fft_sqrt_vlog_values);


  if(init_maxvalue)
  {
    if(minvalue_vlog < SPECT_LOG_MINIMUM_LOG)
      minvalue_vlog = SPECT_LOG_MINIMUM_LOG;

    if(minvalue_sqrt_vlog < SPECT_LOG_MINIMUM_LOG)
      minvalue_sqrt_vlog = SPECT_LOG_MINIMUM_LOG;
  }

  sprintf(str, "FFT resolution: %f Hz   %i blocks of %i samples", freqstep, dftblocks, dftblocksize);

  remove_trailing_zeros(str);

  curve1->setHeaderText(str);

  if(spectrum_color != NULL)
  {
    if(spectrum_color->items > 0)
    {
      spectrum_color->value[0] = 0.0;

      n = 0;

      for(j=0; j<fft_values.count(); j++)
      {
        if(((freqstep * j) + (freqstep * 0.5)) < spectrum_color->freq[0])
        {
          if(spectrum_color->method == 0)  // sum
          {
            spectrum_color->value[0] += fft_values[j];
          }

          if(spectrum_color->method == 1)  // peak
          {
            if(spectrum_color->value[0] < fft_values[j])
            {
              spectrum_color->value[0] = fft_values[j];
            }
          }

          if(spectrum_color->method == 2)  // average
          {
            spectrum_color->value[0] += fft_values[j];

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

      for(j=0; j<fft_values.count(); j++)
      {
        if((((freqstep * j) + (freqstep * 0.5)) > spectrum_color->freq[i-1]) && (((freqstep * j) + (freqstep * 0.5)) < spectrum_color->freq[i]))
        {
          if(spectrum_color->method == 0)  // sum
          {
            spectrum_color->value[i] += fft_values[j];
          }

          if(spectrum_color->method == 1)  // peak
          {
            if(spectrum_color->value[i] < fft_values[j])
            {
              spectrum_color->value[i] = fft_values[j];
            }
          }

          if(spectrum_color->method == 2)  // average
          {
            spectrum_color->value[i] += fft_values[j];

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

  if(mainwindow->spectrumdock_sqrt)
  {
    dock->setWindowTitle("Amplitude Spectrum of " + base_samples->GetAlias());
  }
  else
  {
    dock->setWindowTitle("Power Spectrum of " + base_samples->GetAlias());
  }


  if(set_settings)
  {
    set_settings = 0;

    if(settings.sqrt > 0)
    {
      sqrtButton->setChecked(true);
    }
    else
    {
      sqrtButton->setChecked(false);
    }

    if(settings.log > 0)
    {
      vlogButton->setChecked(true);
    }
    else
    {
      vlogButton->setChecked(false);
    }

    if(settings.colorbar > 0)
    {
      colorBarButton->setChecked(true);
    }
    else
    {
      colorBarButton->setChecked(false);
    }

    if((flywheel_value >= 10) && (flywheel_value <= 100000))
    {
      flywheel_value = settings.wheel;
    }
  }

  curve1->setUpdatesEnabled(true);

  busy = 0;

  init_maxvalue = 0;
}

QVector<double> UI_SpectrumDockWindow::getFFTInputData(signalcompblock *signalcomp)
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

QVector<double> UI_SpectrumDockWindow::calculateFFT(QVector<double> buf_samples, int fft_outputbufsize, int dftblocksize, int *dftblocks)
{
    kiss_fftr_cfg cfg;
    kiss_fft_cpx *kiss_fftbuf;
    QVector<double> result = QVector<double>(fft_outputbufsize);


    kiss_fftbuf = (kiss_fft_cpx *)malloc((fft_outputbufsize + 1) * sizeof(kiss_fft_cpx));
    if(kiss_fftbuf == NULL)
    {
      throw ("Error", "The system was not able to provide enough resources (memory) to perform the requested action.");
    }

    cfg = kiss_fftr_alloc(dftblocksize, 0, NULL, NULL);


    for(int j=0; j<(*dftblocks); j++)
    {
      kiss_fftr(cfg, buf_samples.data() + (j * dftblocksize), kiss_fftbuf);

      for(int i=0; i<fft_outputbufsize; i++)
      {
        result[i] += (((kiss_fftbuf[i].r * kiss_fftbuf[i].r) + (kiss_fftbuf[i].i * kiss_fftbuf[i].i)) / fft_outputbufsize);
      }
    }

    int samplesleft = buf_samples.count() % dftblocksize;
    if(samplesleft & 1)
    {
      samplesleft--;
    }

    if(samplesleft)
    {
        // we need an extra FFT block to process all samples
      kiss_fftr(cfg, buf_samples.data() + ((((*dftblocks)-1) * dftblocksize) + samplesleft), kiss_fftbuf);

      for(int i=0; i<fft_outputbufsize; i++)
      {
        result[i] += (((kiss_fftbuf[i].r * kiss_fftbuf[i].r) + (kiss_fftbuf[i].i * kiss_fftbuf[i].i)) / fft_outputbufsize);

        result[i] /= ((*dftblocks) + 1);
      }
    }
    else
    {
      for(int i=0; i<fft_outputbufsize; i++)
      {
        result[i] /= (*dftblocks);
      }
    }

    if(samplesleft)
    {
      (*dftblocks)++;
    }


    free(cfg);
    free(kiss_fftbuf);


    return result;
}

UI_SpectrumDockWindow::~UI_SpectrumDockWindow()
{
  delete SpectrumDialog;
}
