/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Teunis van Beelen
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




#ifndef POPUP_MESSAGEFORM1_H
#define POPUP_MESSAGEFORM1_H



#include <QtGlobal>
#include <QApplication>
#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

#include <string.h>

#include "global.h"



class UI_Messagewindow : public QObject
{
  Q_OBJECT

public:
  UI_Messagewindow(const char *, const char *, const char *buttontxt = NULL, int timeout = 0);

private:

QDialog      *MessageDialog;

QLabel       *Label1;

QPushButton  *pushButton1;

};



#endif // POPUP_MESSAGEFORM1_H


