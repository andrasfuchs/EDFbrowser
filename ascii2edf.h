/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Teunis van Beelen
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


#ifndef UI_ASCII2EDFFORM_H
#define UI_ASCII2EDFFORM_H


#include <QtGlobal>
#include <QApplication>
#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateTimeEdit>
#include <QDateTime>
#include <QTableWidget>
#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QCursor>
#include <QCheckBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QPixmap>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "popup_messagewindow.h"
#include "xml.h"
#include "utils.h"


#define ASCII_MAX_EDF_SIGNALS 512
#define ASCII_MAX_LINE_LEN 2048




class UI_ASCII2EDFapp : public QObject
{
  Q_OBJECT

public:
  UI_ASCII2EDFapp(char *recent_dir=NULL, char *save_dir=NULL);

private:

QDialog       *ascii2edfDialog;

QLabel        *SeparatorLabel,
              *NumsignalsLabel,
              *DatastartLabel,
              *SamplefreqLabel,
              *PatientnameLabel,
              *RecordingLabel,
              *DatetimeLabel,
              *SignalsLabel,
              *autoPhysicalMaximumLabel;

QRadioButton  *edfButton,
              *bdfButton;

QGroupBox     *groupbox1;

QVBoxLayout   *vbox1;

QLineEdit     *SeparatorLineEdit,
              *PatientnameLineEdit,
              *RecordingLineEdit;

QSpinBox      *NumcolumnsSpinbox,
              *DatastartSpinbox;

QDoubleSpinBox *SamplefreqSpinbox;

QDateTimeEdit *StartDatetimeedit;

QCheckBox     *autoPhysicalMaximumCheckbox;

QTableWidget  *SignalsTablewidget;

QPushButton   *GoButton,
              *CloseButton,
              *SaveButton,
              *LoadButton;

char separator,
     *recent_opendir,
     *recent_savedir;

int edfsignals,
    startline,
    columns,
    column_enabled[ASCII_MAX_EDF_SIGNALS],
    autoPhysicalMaximum,
    edf_format;

double samplefrequency,
       datrecduration,
       sensitivity[ASCII_MAX_EDF_SIGNALS],
       value[ASCII_MAX_EDF_SIGNALS],
       physmax[ASCII_MAX_EDF_SIGNALS];


int check_input(void);

private slots:

void numofcolumnschanged(int);
void gobuttonpressed();
void savebuttonpressed();
void loadbuttonpressed();
void autoPhysicalMaximumCheckboxChanged(int);
};




#endif


