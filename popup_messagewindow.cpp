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




#include "popup_messagewindow.h"



UI_Messagewindow::UI_Messagewindow(const char *title, const char *str, const char *buttontxt, int timeout)
{
  int i,
      len,
      max=0,
      labelwidth = 0,
      labelheight = 25;

  len = strlen(str);

  for(i=0; i<len; i++)
  {
    if(str[i]=='\n')
    {
      labelheight += 20;
      if(max>labelwidth) labelwidth = max;
      max = 0;
    }
    else
    {
      max++;
    }
  }

  if(max>labelwidth) labelwidth = max;

  labelwidth *= 6;
  labelwidth += 10;
  if(labelwidth<100)  labelwidth = 100;

  MessageDialog = new QDialog;

  MessageDialog->setMinimumSize(labelwidth + 40, labelheight + 85);
  MessageDialog->setMaximumSize(labelwidth + 40, labelheight + 85);
  MessageDialog->setWindowTitle(title);
  MessageDialog->setModal(true);
  MessageDialog->setAttribute(Qt::WA_DeleteOnClose, true);

  Label1 = new QLabel(MessageDialog);
  Label1->setGeometry(20, 20, labelwidth, labelheight);
  Label1->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  Label1->setText(str);

  pushButton1 = new QPushButton(MessageDialog);
  pushButton1->setGeometry(labelwidth - 80, labelheight + 40, 100, 25);
  if(buttontxt == NULL)
  {
    pushButton1->setText("&Close");
  }
  else
  {
    pushButton1->setText(buttontxt);
  }

  if(timeout > 100)
  {
    QTimer::singleShot(timeout, MessageDialog, SLOT(close()));
  }

  QObject::connect(pushButton1, SIGNAL(clicked()), MessageDialog, SLOT(close()));

  MessageDialog->exec();
}






