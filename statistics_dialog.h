/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2011, 2012, 2013, 2014, 2015 Teunis van Beelen
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




#ifndef STATISTICS_DIALOGFORM1_H
#define STATISTICS_DIALOGFORM1_H



#include <QtGlobal>
#include <QApplication>
#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "ecg_filter.h"
#include "signalcurve.h"

#include "models/signalcompblock.h"


class UI_StatisticWindow : public QObject
{
  Q_OBJECT

public:

UI_StatisticWindow(struct signalcompblock *, long long);

private:

QDialog     *StatDialog;

QHBoxLayout *hlayout1,
            *hlayout1_1_1;

QVBoxLayout *vlayout1_1,
            *vlayout2_1;

QSlider     *startSlider,
            *stopSlider;

QLabel      *Label1,
            *startLabel,
            *stopLabel;

QPushButton  *pushButton1;

SignalCurve  *curve1;

int beat_cnt,
    max_val,
    start_ruler,
    end_ruler;

double bpm_distribution[300];

private slots:

void startSliderMoved(int);
void stopSliderMoved(int);

};



#endif // STATISTICS_DIALOGFORM1_H


