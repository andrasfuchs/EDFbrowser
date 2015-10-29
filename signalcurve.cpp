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



#include "signalcurve.h"


SignalCurve::SignalCurve(QWidget *w_parent) : QWidget(w_parent)
{
  setAttribute(Qt::WA_OpaquePaintEvent);

  recent_savedir[0] = 0;

  BackgroundColor = QColor(0,0,0);
  RasterColor = QColor(127,127,127,191);
  SecondaryRasterColor = QColor(127,127,127,63);
  BorderColor = QColor(30,44,54);
  RulerColor = QColor(255,255,255);
  TextColor = QColor(145,145,145);
  crosshair_1_color = Qt::red;


  Marker1Pen.setStyle(Qt::DashLine);
  Marker1Pen.setColor(Qt::yellow);
  Marker2Pen.setStyle(Qt::DashLine);
  Marker2Pen.setColor(Qt::yellow);

  isHorizontalRulerVisible = 1;
  isVerticalRulerVisible = 1;
  extra_button = 0;
  extra_button_txt[0] = 0;
  use_move_events = 0;
  crosshair_1_active = 0;
  crosshair_1_moving = 0;
  crosshair_1_value = 0.0;
  crosshair_1_value_2 = 0.0;
  crosshair_1_x_position = 0;
  marker_1_position = 0.25;
  marker_1_moving = 0;
  marker_2_position = 0.75;
  marker_2_moving = 0;
  fillsurface = 0;

  cursorEnabled = true;
  printEnabled = true;
  dashBoardEnabled = true;
  updates_enabled = true;
  Marker1Enabled = false;
  Marker1MovableEnabled = false;
  Marker2Enabled = false;
  Marker2MovableEnabled = false;
  curveUpSideDown = false;
  line1Enabled = false;

  spectrum_color = NULL;

  old_w = 10000;

  // limit our refresh rate
  update_timer = new QTimer(this);
  update_timer->setInterval((long)(1000/MAX_FPS) + 1);
  connect(update_timer, SIGNAL(timeout()), this, SLOT(updateWidget()));
  update_timer->start();
}

void SignalCurve::updateWidget()
{
    if (update_pending)
    {
        update();
        update_pending = false;
    }
}

void SignalCurve::clearSignal()
{
  signals_.clear();
}

void SignalCurve::clear()
{
  signals_.clear();

  use_move_events = 0;
  crosshair_1_active = 0;
  crosshair_1_moving = 0;
  crosshair_1_value = 0.0;
  crosshair_1_value_2 = 0.0;
  crosshair_1_x_position = 0;
  marker_1_position = 0.25;
  marker_2_position = 0.75;
  Marker1Enabled = false;
  Marker1MovableEnabled = false;
  Marker2Enabled = false;
  Marker2MovableEnabled = false;
  line1Enabled = false;

  update_pending = true;
}


void SignalCurve::mousePressEvent(QMouseEvent *press_event)
{
  setFocus(Qt::MouseFocusReason);

  int m_x = press_event->x() - chartArea.x();
  int m_y = press_event->y() - chartArea.y();

  // check if the mouse is over the chart area
  if (chartArea.contains(press_event->x(), press_event->y()))
  {
      // check if the left button was pressed
      if(press_event->button()==Qt::LeftButton)
      {
          mouseLastPosition = QPoint(m_x, m_y);

        if ((printEnabled == true) && ((m_y<21)&&(m_y>3)&&(m_x>(chartArea.width() - 43))&&(m_x<(chartArea.width() - 3))))
        {
          exec_sidemenu();
          return;
        }
        else if ((dashBoardEnabled == true) && ((m_y<61)&&(m_y>43)&&(m_x>(chartArea.width() - 43))&&(m_x<(chartArea.width() - 3))))
        {
          emit dashBoardClicked();
          return;
        }
        else if ((cursorEnabled == true) && ((m_y<41)&&(m_y>23)&&(m_x>(chartArea.width() - 43))&&(m_x<(chartArea.width() - 3))))
        {
            if(crosshair_1_active)
            {
              crosshair_1_active = 0;
              crosshair_1_moving = 0;
              use_move_events = 0;
              setMouseTracking(false);
            }
            else
            {
              crosshair_1_active = 1;
              if(!crosshair_1_x_position)
              {
                crosshair_1_value = 0.0;
                crosshair_1_x_position = chartArea.width() / 2;
                crosshair_1_y_position = chartArea.height() / 2;
                mouseLastPosition = QPoint(crosshair_1_x_position, crosshair_1_y_position);
              }
            }
        }
        else if ((crosshair_1_active)
            && (((m_y<(crosshair_1_y_position + 15))&&(m_y>(crosshair_1_y_position - 25))&&(m_x>crosshair_1_x_position)&&(m_x<(crosshair_1_x_position + 65)))
                || (m_x>(crosshair_1_x_position-10)&&(m_x<(crosshair_1_x_position + 10)))))
        {
            crosshair_1_moving = 1;
            use_move_events = 1;
            setMouseTracking(true);
        }
        else if((Marker1MovableEnabled == true) && (Marker1Enabled == true))
        {
          marker_1_x_position = chartArea.width() * marker_1_position;

          if(m_x > (marker_1_x_position - 5) && (m_x < (marker_1_x_position + 5)))
          {
            marker_1_moving = 1;
            use_move_events = 1;
            setMouseTracking(true);
          }
        }
        else if((Marker2MovableEnabled == true) && (Marker2Enabled == true))
        {
          marker_2_x_position = chartArea.width() * marker_2_position;

          if(m_x > (marker_2_x_position - 5) && (m_x < (marker_2_x_position + 5)))
          {
            marker_2_moving = 1;
            use_move_events = 1;
            setMouseTracking(true);
          }
        } else {
            // if the mouse is on the chart area and not over the cursor/marker/button
            use_move_events = 1;
            setMouseTracking(true);
        }
      }
  }

  update_pending = true;
}


void SignalCurve::mouseReleaseEvent(QMouseEvent *)
{
  crosshair_1_moving = 0;
  marker_1_moving = 0;
  marker_2_moving = 0;
  use_move_events = 0;
  setMouseTracking(false);
}

void SignalCurve::mouseDoubleClickEvent(QMouseEvent *)
{
    resetRulers();

    update_pending = true;
}

void SignalCurve::resetRulers()
{
    if (signals_.count() == 0)
    {
        return;
    }

    // 1, set the horizontal axis to default, 20 pixel / Hz
    double width_change = (h_ruler_min_value - h_ruler_max_value);
    h_ruler_min_value = 0;
    h_ruler_max_value = chartArea.width() / 20;
    width_change /= (h_ruler_min_value - h_ruler_max_value);

    // 2, set the vertical values to the 90% of the minimum and 110% of the maximum value
    v_ruler_min_value = DBL_MAX;
    v_ruler_max_value = DBL_MIN;

    for (int j=0; j < signals_.count(); j++)
    {
        for(int i=0; i < signals_[j]->GetValues().count(); i++)
        {
          double value = (signals_[j]->GetValues()[i]);

          if (value == signals_[j]->SIGNAL_NA_VALUE) continue;

          if (value < v_ruler_min_value)
          {
              v_ruler_min_value = value;
          }

          if (value > v_ruler_max_value)
          {
              v_ruler_max_value = value;
          }
        }
    }

    if ((v_ruler_min_value != DBL_MAX) && (v_ruler_max_value != DBL_MIN))
    {
        v_ruler_min_value *= (v_ruler_min_value > 0 ? 0.9 : 1.1);
        v_ruler_max_value *= (v_ruler_max_value > 0 ? 1.1 : 0.9);
    }

    signal_display_start = 0;
    signal_display_length = h_ruler_max_value * signals_[0]->GetHorizontalDensity();
}

void SignalCurve::mouseMoveEvent(QMouseEvent *move_event)
{
  if(!use_move_events)
  {
    return;
  }

  QPoint mousePosition = QPoint(move_event->x() - chartArea.x(), move_event->y() - chartArea.y());

  int mouse_delta_x = (mousePosition.x() - mouseLastPosition.x());
  int mouse_delta_y = (mousePosition.y() - mouseLastPosition.y());

  mouseLastPosition = mousePosition;

  if(crosshair_1_moving)
  {
    crosshair_1_x_position += mouse_delta_x;

    if(crosshair_1_x_position<2)
    {
      crosshair_1_x_position = 2;
    }
    if(crosshair_1_x_position>chartArea.width()-2)
    {
      crosshair_1_x_position = chartArea.width() -2;
    }

    crosshair_1_y_position += mouse_delta_y;
    if(crosshair_1_y_position<25)
    {
      crosshair_1_y_position = 25;
    }
    if(crosshair_1_y_position>(chartArea.height() -25))
    {
      crosshair_1_y_position = chartArea.height() -25;
    }
  }
  else if(marker_1_moving)
  {
    marker_1_x_position += mouse_delta_x;
    if(marker_1_x_position<2)
    {
      marker_1_x_position = 2;
    }
    if(marker_1_x_position>(chartArea.width() - 2))
    {
      marker_1_x_position = chartArea.width() - 2;
    }

    marker_1_position = (double)marker_1_x_position / (double)chartArea.width();

    emit markerHasMoved();
  }
  else if(marker_2_moving)
  {
    marker_2_x_position += mouse_delta_x;
    if(marker_2_x_position<2)
    {
      marker_2_x_position = 2;
    }
    if(marker_2_x_position>(chartArea.width() - 2))
    {
      marker_2_x_position = chartArea.width() - 2;
    }

    marker_2_position = (double)marker_2_x_position / (double)chartArea.width();

    emit markerHasMoved();
  }
  else
  {
      // we need to scroll the chart

      // 0, convert the amount of movement from pixels to percent
      double value_delta_x_pct = (double)mouse_delta_x / width();
      double value_delta_y_pct = (double)mouse_delta_y / height();

      // width should remain the same when we are scrolling
      double h_ruler_width = h_ruler_max_value - h_ruler_min_value;

      // check if we are in the valid range
      double desired_signal_display_start = signal_display_start - (value_delta_x_pct * signal_display_length);

      if (desired_signal_display_start + signal_display_length >= signals_[0]->GetValues().count() - 1)
      {
          desired_signal_display_start = signals_[0]->GetValues().count() - (int)signal_display_length - 1;
          value_delta_x_pct = (signal_display_start - desired_signal_display_start) / signal_display_length;
      }

      if (desired_signal_display_start < 0)
      {
          desired_signal_display_start = 0;
          value_delta_x_pct = (signal_display_start - desired_signal_display_start) / signal_display_length;;
      }

      // 1, move the chart horizontally
      signal_display_start = desired_signal_display_start;

      // 2, move the horizontal ruler

      // move the starting point
      h_ruler_min_value -= value_delta_x_pct * h_ruler_width;

      // set the endvalue of the ruler
      h_ruler_max_value = h_ruler_min_value + h_ruler_width;

      // 3, move the chart vertically
      double value_width = v_ruler_max_value - v_ruler_min_value;

      v_ruler_min_value += value_delta_y_pct * value_width;
      v_ruler_max_value = v_ruler_min_value + value_width;
  }

  update_pending = true;
}

void SignalCurve::wheelEvent(QWheelEvent * event)
{
    // we need to zoom the chart

    // 0, convert the amount of movement from angles to percent

    // this one works only with Qt 5.5+ and it enables the x and y axis wheel usage
    //QPoint delta = event->angleDelta();
    //double zoom_x_pct = 1 + ((double)delta.x() / 5000);
    //double zoom_y_pct = 1 + (-(double)delta.y() / 5000);

    // this one works with Qt 4.8 too but only y axis wheel movement is registered
    int delta = event->delta();
    double zoom_x_pct = 1;
    double zoom_y_pct = 1 + (-(double)delta / 5000);

    // check if we are in the valid range
//    if (h_ruler_min_value + ((h_ruler_max_value - h_ruler_min_value) * zoom_y_pct * zoom_x_pct) > 256)
//    {
//        zoom_x_pct = 1;
//        zoom_y_pct = 1;
//    }

    // 1, horizontal scale
    double desired_signal_display_length = signal_display_length * zoom_y_pct * zoom_x_pct;

    if (signal_display_start + desired_signal_display_length >= signals_[0]->GetValues().count() - 1)
    {
        desired_signal_display_length = signals_[0]->GetValues().count() - (int)signal_display_start - 1;
        zoom_y_pct = desired_signal_display_length / signal_display_length;
    }

    signal_display_length = desired_signal_display_length;


    double h_ruler_width = (h_ruler_max_value - h_ruler_min_value) * zoom_y_pct * zoom_x_pct;
    h_ruler_max_value = h_ruler_min_value + h_ruler_width;

    // 2, vertical scale
    double value_width = (v_ruler_max_value - v_ruler_min_value) * zoom_y_pct;

    //min_value -= value_width_change / 2;
    v_ruler_max_value = v_ruler_min_value + value_width;

    update_pending = true;
}

void SignalCurve::shiftCursorIndexLeft(int idxs)
{
  int idx;

  double ppi;

  if(!crosshair_1_active)
  {
    return;
  }

  if(signal_display_length < 2)
  {
    return;
  }

  ppi = (double)chartArea.width() / (double)signal_display_length;

  idx = (double)crosshair_1_x_position / ppi;

  idx -= idxs;

  if(idx < 0)
  {
    idx = 0;
  }

  if(idx >= signal_display_length)
  {
    idx = signal_display_length - 1;
  }

  crosshair_1_x_position = (double)idx * ppi + (0.5 * ppi);

  update_pending = true;
}


void SignalCurve::shiftCursorIndexRight(int idxs)
{
  int idx;

  double ppi;

  if(!crosshair_1_active)
  {
    return;
  }

  if(signal_display_length < 2)
  {
    return;
  }

  ppi = (double)chartArea.width() / signal_display_length;

  idx = (double)crosshair_1_x_position / ppi;

  idx += idxs;

  if(idx < 0)
  {
    idx = 0;
  }

  if(idx >= signal_display_length)
  {
    idx = signal_display_length - 1;
  }

  crosshair_1_x_position = (double)idx * ppi + (0.5 * ppi);

  update_pending = true;
}


void SignalCurve::shiftCursorPixelsLeft(int pixels)
{
  if(!crosshair_1_active)
  {
    return;
  }

  crosshair_1_x_position -= pixels;
  if(crosshair_1_x_position<2)
  {
    crosshair_1_x_position = 2;
  }

  update_pending = true;
}


void SignalCurve::shiftCursorPixelsRight(int pixels)
{
  if(!crosshair_1_active)
  {
    return;
  }

  crosshair_1_x_position += pixels;
  if(crosshair_1_x_position>(chartArea.width() -2))
  {
    crosshair_1_x_position = chartArea.width() -2;
  }

  update_pending = true;
}


void SignalCurve::resizeEvent(QResizeEvent *resize_event)
{
  if(crosshair_1_active)
  {
    crosshair_1_x_position *= ((double)chartArea.width() / (double)(old_w - (width() - chartArea.width())));
  }

  // set the size of the chart area by setting its size compared to the widget
  chartArea = QRect(90, 50, width() - (90+30), height() - (50+70));

  QWidget::resizeEvent(resize_event);
}


void SignalCurve::exec_sidemenu()
{
  sidemenu = new QDialog(this);

  if(extra_button)
  {
    sidemenu->setMinimumSize(QSize(120, 190));
    sidemenu->setMaximumSize(QSize(120, 190));
  }
  else
  {
    sidemenu->setMinimumSize(QSize(120, 160));
    sidemenu->setMaximumSize(QSize(120, 160));
  }
  sidemenu->setWindowTitle("Print");
  sidemenu->setModal(true);
  sidemenu->setAttribute(Qt::WA_DeleteOnClose, true);

  sidemenuButton1 = new QPushButton(sidemenu);
  sidemenuButton1->setGeometry(10, 10, 100, 20);
  sidemenuButton1->setText("to printer");

#if QT_VERSION < 0x050000
  sidemenuButton2 = new QPushButton(sidemenu);
  sidemenuButton2->setGeometry(10, 40, 100, 20);
  sidemenuButton2->setText("to Postscript");
#endif

  sidemenuButton3 = new QPushButton(sidemenu);
  sidemenuButton3->setGeometry(10, 70, 100, 20);
  sidemenuButton3->setText("to PDF");

  sidemenuButton4 = new QPushButton(sidemenu);
  sidemenuButton4->setGeometry(10, 100, 100, 20);
  sidemenuButton4->setText("to Image");

  sidemenuButton5 = new QPushButton(sidemenu);
  sidemenuButton5->setGeometry(10, 130, 100, 20);
  sidemenuButton5->setText("to ASCII");

  if(extra_button)
  {
    sidemenuButton6 = new QPushButton(sidemenu);
    sidemenuButton6->setGeometry(10, 160, 100, 20);
    sidemenuButton6->setText(extra_button_txt);
  }

  QObject::connect(sidemenuButton1, SIGNAL(clicked()), this, SLOT(print_to_printer()));
#if QT_VERSION < 0x050000
  QObject::connect(sidemenuButton2, SIGNAL(clicked()), this, SLOT(print_to_postscript()));
#endif
  QObject::connect(sidemenuButton3, SIGNAL(clicked()), this, SLOT(print_to_pdf()));
  QObject::connect(sidemenuButton4, SIGNAL(clicked()), this, SLOT(print_to_image()));
  QObject::connect(sidemenuButton5, SIGNAL(clicked()), this, SLOT(print_to_ascii()));
  if(extra_button)
  {
    QObject::connect(sidemenuButton6, SIGNAL(clicked()), this, SLOT(send_button_event()));
  }

  sidemenu->exec();
}


void SignalCurve::send_button_event()
{
  emit extra_button_clicked();

  sidemenu->close();
}


void SignalCurve::create_button(const char *txt)
{
  extra_button = 1;
  strncpy(extra_button_txt, txt, 16);
  extra_button_txt[15] = 0;
}


void SignalCurve::backup_colors_for_printing(void)
{
  backup_color_2 = RasterColor;
  RasterColor = Qt::black;
  backup_color_3 = BorderColor;
  BorderColor = Qt::black;
  backup_color_4 = RulerColor;
  RulerColor = Qt::black;
  backup_color_5 = TextColor;
  TextColor = Qt::black;
  backup_color_6 = Marker1Pen.color();
  Marker1Pen.setColor(Qt::black);
}


void SignalCurve::restore_colors_after_printing(void)
{
  RasterColor = backup_color_2;
  BorderColor = backup_color_3;
  RulerColor = backup_color_4;
  TextColor = backup_color_5;
  Marker1Pen.setColor(backup_color_6);
}


void SignalCurve::print_to_ascii()
{
  int i;

  char path[SC_MAX_PATH_LEN];

  FILE *outputfile;

  path[0] = 0;
  if(recent_savedir[0]!=0)
  {
    strcpy(path, recent_savedir);
    strcat(path, "/");
  }
  strcat(path, "curve.csv");

  strcpy(path, QFileDialog::getSaveFileName(0, "Print to ASCII / CSV", QString::fromLocal8Bit(path), "ASCII / CSV files (*.csv *.CSV *.txt *.TXT)").toLocal8Bit().data());

  if(!strcmp(path, ""))
  {
    sidemenu->close();

    return;
  }

  get_directory_from_path(recent_savedir, path, SC_MAX_PATH_LEN);

  outputfile = fopen(path, "wb");
  if(path == NULL)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not open outputfile for writing.");
    messagewindow.exec();
    return;
  }

  for(i=0; i<signals_[0]->GetValues().count(); i++)
  {
    fprintf(outputfile, "%.8f\n", signals_[0]->GetValues()[i]);
  }

  fclose(outputfile);

  sidemenu->close();
}


#if QT_VERSION < 0x050000
void SignalCurve::print_to_postscript()
{
  char path[SC_MAX_PATH_LEN];

  path[0] = 0;
  if(recent_savedir[0]!=0)
  {
    strcpy(path, recent_savedir);
    strcat(path, "/");
  }
  strcat(path, "curve.ps");

  strcpy(path, QFileDialog::getSaveFileName(0, "Print to PostScript", QString::fromLocal8Bit(path), "PostScript files (*.ps *.PS)").toLocal8Bit().data());

  if(!strcmp(path, ""))
  {
    sidemenu->close();

    return;
  }

  get_directory_from_path(recent_savedir, path, SC_MAX_PATH_LEN);

  QPrinter curve_printer(QPrinter::HighResolution);

  curve_printer.setOutputFormat(QPrinter::PostScriptFormat);
  curve_printer.setOutputFileName(path);
  curve_printer.setPageSize(QPrinter::A4);
  curve_printer.setOrientation(QPrinter::Landscape);

  backup_colors_for_printing();

  QPainter paint(&curve_printer);

  drawWidget(&paint, curve_printer.pageRect().width(), curve_printer.pageRect().height(), true);

  restore_colors_after_printing();

  sidemenu->close();
}
#endif


void SignalCurve::print_to_pdf()
{
  char path[SC_MAX_PATH_LEN];

  path[0] = 0;
  if(recent_savedir[0]!=0)
  {
    strcpy(path, recent_savedir);
    strcat(path, "/");
  }
  strcat(path, "curve.pdf");

  strcpy(path, QFileDialog::getSaveFileName(0, "Print to PDF", QString::fromLocal8Bit(path), "PDF files (*.pdf *.PDF)").toLocal8Bit().data());

  if(!strcmp(path, ""))
  {
    sidemenu->close();

    return;
  }

  get_directory_from_path(recent_savedir, path, SC_MAX_PATH_LEN);

  QPrinter curve_printer(QPrinter::HighResolution);

  curve_printer.setOutputFormat(QPrinter::PdfFormat);
  curve_printer.setOutputFileName(path);
  curve_printer.setPageSize(QPrinter::A4);
  curve_printer.setOrientation(QPrinter::Landscape);

  backup_colors_for_printing();

  QPainter paint(&curve_printer);

  drawWidget(&paint, curve_printer.pageRect().width(), curve_printer.pageRect().height(), true);

  restore_colors_after_printing();

  sidemenu->close();
}


void SignalCurve::print_to_image()
{
  char path[SC_MAX_PATH_LEN];

  path[0] = 0;
  if(recent_savedir[0]!=0)
  {
    strcpy(path, recent_savedir);
    strcat(path, "/");
  }
  strcat(path, "curve.png");

  strcpy(path, QFileDialog::getSaveFileName(0, "Print to Image", QString::fromLocal8Bit(path), "PNG files (*.png *.PNG)").toLocal8Bit().data());

  if(!strcmp(path, ""))
  {
    sidemenu->close();

    return;
  }

  get_directory_from_path(recent_savedir, path, SC_MAX_PATH_LEN);

  QPixmap pixmap(width(), height());

  QPainter paint(&pixmap);

  drawWidget(&paint, width(), height());

  pixmap.save(path, "PNG", 90);

  sidemenu->close();
}


void SignalCurve::print_to_printer()
{
  QPrinter curve_printer(QPrinter::HighResolution);

  curve_printer.setOutputFormat(QPrinter::NativeFormat);
  curve_printer.setPageSize(QPrinter::A4);
  curve_printer.setOrientation(QPrinter::Landscape);

  QPrintDialog printerdialog(&curve_printer, this);
  printerdialog.setWindowTitle("Print");

  if(!(printerdialog.exec()==QDialog::Accepted))
  {
    sidemenu->close();

    return;
  }

  backup_colors_for_printing();

  QPainter paint(&curve_printer);

  drawWidget(&paint, curve_printer.pageRect().width(), curve_printer.pageRect().height(), true);

  restore_colors_after_printing();

  sidemenu->close();
}


void SignalCurve::setUpdatesEnabled(bool enabled)
{
  updates_enabled = enabled;
}


void SignalCurve::paintEvent(QPaintEvent *)
{
  if(updates_enabled == true)
  {
    QPainter paint(this);

    drawWidget(&paint, width(), height());

    old_w = width();
  }
}


void SignalCurve::drawWidget(QPainter *painter, int curve_w, int curve_h, bool is_printer)
{
  int i,
      p_precision,
      p_divisor,
      p_range,
      p_multiplier,
      p_ruler_startvalue,
      p_ruler_endvalue,
      raster_position;

  double v_sens=0.0,
         offset=0.0,
         h_step=0.0,
         pixelsPerUnit,
         sum_colorbar_value,
         p_pixels_per_unit;

  char str[128];  

  painter->setFont(QFont("Arial", 8));
  painter->fillRect(0, 0, curve_w, curve_h, BorderColor);

  // if the desired chart size is too small (smaller than 5x5)
  if((curve_w < (width()-chartArea.width() + 5)) || (curve_h < (width()-chartArea.width() + 5)))
  {
    return;
  }


  if (is_printer) {
    QPen printerPen = QPen(Qt::SolidPattern, 2, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin);
    printerPen.setColor(Qt::black);

    painter->setPen(printerPen);
    painter->setFont(QFont("Arial", 8));

  /////////////////////////////////// draw the window for the printer ///////////////////////////////////////////

    painter->drawLine(0, 0, 0, curve_h);
    painter->drawLine(0, 0, curve_w, 0);
    painter->drawLine(curve_w, curve_h, curve_w, 0);
    painter->drawLine(curve_w, curve_h, 0, curve_h);
  }


  if (((v_ruler_min_value == DBL_MAX) || (v_ruler_max_value == DBL_MIN) || (v_ruler_min_value == v_ruler_max_value))
      || ((h_ruler_min_value == DBL_MAX) || (h_ruler_max_value == DBL_MIN) || (h_ruler_min_value == h_ruler_max_value)))
  {
      return;
  }

/////////////////////////////////// draw the horizontal ruler ///////////////////////////////////////////

  if(isHorizontalRulerVisible)
  {
    calculateRulerParameters(
                  chartArea.width(), h_ruler_min_value, h_ruler_max_value,
                  &p_multiplier, &p_ruler_startvalue, &p_ruler_endvalue, &p_range, &p_pixels_per_unit, &p_divisor, &p_precision
                  );

    painter->setPen(RulerColor);
    painter->drawLine(chartArea.left(), chartArea.bottom() + 5, chartArea.right(), chartArea.bottom() + 5);

    // the minimum value on the ruler
    int horizontal_ruler_text_start_position = (int)((double)(p_ruler_startvalue - p_ruler_startvalue) * p_pixels_per_unit);
    double horizontal_start_value = (double)p_ruler_startvalue / (double)p_multiplier;
    drawTextOnRuler(painter, chartArea, horizontal_ruler_text_start_position, horizontal_start_value, p_precision, false);

    // the maximum value on the ruler
    int horizontal_ruler_text_end_position = (int)((double)(p_ruler_endvalue - p_ruler_startvalue) * p_pixels_per_unit);
    double horizontal_end_value = (double)p_ruler_endvalue / (double)p_multiplier;
    drawTextOnRuler(painter, chartArea, horizontal_ruler_text_end_position, horizontal_end_value, p_precision, false);

    for(i = (p_ruler_startvalue / p_divisor) * p_divisor; i <= p_ruler_endvalue; i += p_divisor)
    {
      if(i < p_ruler_startvalue)
      {
        continue;
      }

      int horizontal_position = (int)((double)(i - p_ruler_startvalue) * p_pixels_per_unit);
      double horizoltal_value = (double)i / (double)p_multiplier;

      if ((horizontal_position > horizontal_ruler_text_start_position + 20) && (horizontal_position < horizontal_ruler_text_end_position - 20))
      {
        drawTextOnRuler(painter, chartArea, horizontal_position, horizoltal_value, p_precision, false);
      }
    }
  }

/////////////////////////////////// draw the vertical ruler ///////////////////////////////////////////

  if(isVerticalRulerVisible)
  {
    calculateRulerParameters(
                  chartArea.height(), v_ruler_min_value, v_ruler_max_value,
                  &p_multiplier, &p_ruler_startvalue, &p_ruler_endvalue, &p_range, &p_pixels_per_unit, &p_divisor, &p_precision
                  );

    painter->setPen(RulerColor);
    painter->drawLine(chartArea.x() - 5, chartArea.y(), chartArea.x() - 5, chartArea.bottom());

    int vertical_ruler_text_start_position = (int)((double)(p_ruler_startvalue - p_ruler_startvalue) * p_pixels_per_unit);
    double vertical_start_value = (double)p_ruler_startvalue / (double)p_multiplier;
    drawTextOnRuler(painter, chartArea, vertical_ruler_text_start_position, vertical_start_value, p_precision, true, curveUpSideDown);

    int vertical_ruler_text_end_position = (int)((double)(p_ruler_endvalue - p_ruler_startvalue) * p_pixels_per_unit);
    double vertical_end_value = (double)p_ruler_endvalue / (double)p_multiplier;
    drawTextOnRuler(painter, chartArea, vertical_ruler_text_end_position, vertical_end_value, p_precision, true, curveUpSideDown);

    for(i = (p_ruler_startvalue / p_divisor) * p_divisor; i <= p_ruler_endvalue; i += p_divisor)
    {
      if(i < p_ruler_startvalue)
      {
        continue;
      }

      int vertical_position = (int)((double)(i - p_ruler_startvalue) * p_pixels_per_unit);
      double vertical_value = (double)i / (double)p_multiplier;

      if ((vertical_position > vertical_ruler_text_start_position + 12) && (vertical_position < vertical_ruler_text_end_position - 12))
      {
        drawTextOnRuler(painter, chartArea, vertical_position, vertical_value, p_precision, true, curveUpSideDown);
      }
    }
  }

/////////////////////////////////// draw the labels ///////////////////////////////////////////

  painter->setPen(TextColor);
  painter->setFont(QFont("Arial", 8));
  QFontMetrics fontMetrics = QFontMetrics(painter->font());

  QString v_ruler_text = v_ruler_name + " [" + v_ruler_unit + "]";
  QRect text_rect = fontMetrics.boundingRect(v_ruler_text);
  painter->rotate(-90);
  painter->drawText(-(chartArea.bottom() - ((chartArea.height()/2) - (text_rect.width() / 2))), chartArea.left() - 70, text_rect.width(), text_rect.height(), Qt::TextSingleLine, v_ruler_text);
  painter->rotate(90);

  painter->drawText(curve_w / 2 - 150, 20, 300, 16, Qt::AlignCenter | Qt::TextSingleLine, header_label);

  QString h_ruler_text = h_ruler_name + " [" + h_ruler_unit + "]";
  text_rect = fontMetrics.boundingRect(h_ruler_text);
  painter->drawText(chartArea.left() + (chartArea.width()/2) - (text_rect.width() / 2), chartArea.bottom() + 40, text_rect.width(), text_rect.height(), Qt::TextSingleLine, h_ruler_text);


/////////////////////////////////// translate coordinates, draw and fill a rectangle ///////////////////////////////////////////

  painter->translate(chartArea.topLeft());

  curve_w = chartArea.width();
  curve_h = chartArea.height();

  painter->fillRect(0, 0, curve_w, curve_h, BackgroundColor);

  painter->setClipping(true);
  painter->setClipRegion(QRegion(0, 0, curve_w, curve_h), Qt::ReplaceClip);

/////////////////////////////////// draw the colorbars /////////////////////////////////////////

  int t;

  if(spectrum_color != NULL)
  {
    if(spectrum_color->auto_adjust)
    {
      spectrum_color->max_colorbar_value = 0.0;
    }
    sum_colorbar_value = 0.0;

    for(i=0; i < spectrum_color->items; i++)
    {
      sum_colorbar_value += spectrum_color->value[i];

      if(spectrum_color->auto_adjust)
      {
        if(spectrum_color->value[i] > spectrum_color->max_colorbar_value)
        {
          spectrum_color->max_colorbar_value = spectrum_color->value[i];
        }
      }
    }

    if(spectrum_color->auto_adjust)
    {
      spectrum_color->max_colorbar_value *= 1.05;
    }

    pixelsPerUnit = (double)curve_w / (h_ruler_max_value - h_ruler_min_value);
    double barWidth = 0;
    int barTextYPosIndex = 0;

    if((spectrum_color->freq[0] > h_ruler_min_value) && (spectrum_color->items > 1))
    {
      t = (spectrum_color->max_colorbar_value - spectrum_color->value[0]) * ((double)curve_h / spectrum_color->max_colorbar_value);
      if(t < 0)  t = 0;

      if(t <= curve_h)
      {
        QColor color = (Qt::GlobalColor)spectrum_color->color[0];
        color.setAlpha(127);

        barWidth = (spectrum_color->freq[0] - h_ruler_min_value) * pixelsPerUnit;

        painter->fillRect(0,
                          t,
                          barWidth,
                          curve_h - t,
                          color);
      }

      strcpy(str, spectrum_color->label[0]);
      if(spectrum_color->method == 0)
      {
        sprintf(str + strlen(str), " (%.1f%%)", (spectrum_color->value[i] * 100.0) / sum_colorbar_value);
      }

      painter->setPen((Qt::GlobalColor)spectrum_color->color[0]);
      painter->drawText(10, 20, str);

      if (barWidth < 80) {
          barTextYPosIndex++;
      }
    }

    for(i=1; i < spectrum_color->items; i++)
    {
      if(spectrum_color->freq[i] > h_ruler_min_value)
      {
        t = (spectrum_color->max_colorbar_value - spectrum_color->value[i]) * ((double)curve_h / spectrum_color->max_colorbar_value);
        if(t < 0)  t = 0;

        if(t <= curve_h)
        {
          QColor color = (Qt::GlobalColor)spectrum_color->color[i];
          color.setAlpha(127);

          barWidth = (spectrum_color->freq[i] - spectrum_color->freq[i-1]) * pixelsPerUnit;

          painter->fillRect((spectrum_color->freq[i-1] - h_ruler_min_value) * pixelsPerUnit,
                            t,
                            barWidth,
                            curve_h - t,
                            color);
        }

        strcpy(str, spectrum_color->label[i]);
        if(spectrum_color->method == 0)
        {
          sprintf(str + strlen(str), " (%.1f%%)", (spectrum_color->value[i] * 100.0) / sum_colorbar_value);
        }
        painter->setPen((Qt::GlobalColor)spectrum_color->color[i]);
        painter->drawText((spectrum_color->freq[i-1] - h_ruler_min_value) * pixelsPerUnit + 10, 20 + (barTextYPosIndex * 20), str);

        if (barWidth < 80) {
            barTextYPosIndex++;
        } else {
            barTextYPosIndex = 0;
        }
      }
    }

    if(spectrum_color->method == 0)
    {
      char sum_colorbar_value_str[64];
      thousandsep(sum_colorbar_value, sum_colorbar_value_str, sizeof sum_colorbar_value_str, 2);

      sprintf(str, "Total Power: %s", sum_colorbar_value_str);
      painter->setPen(Qt::white);
      painter->drawText(curve_w - 200, 20, str);
    }
  }


/////////////////////////////////// draw the rasters ///////////////////////////////////////////

  painter->setPen(RasterColor);
  painter->drawRect (0, 0, curve_w - 1, curve_h - 1);

  // horizontal rasters (draw rasters between p_ruler_startvalue and p_ruler_endvalue with p_divisor steps)
  calculateRulerParameters(
                chartArea.width(), h_ruler_min_value, h_ruler_max_value,
                &p_multiplier, &p_ruler_startvalue, &p_ruler_endvalue, &p_range, &p_pixels_per_unit, &p_divisor, &p_precision
                );

  int isSecondary = 0;
  for(i = (p_ruler_startvalue / p_divisor) * p_divisor; i < p_ruler_endvalue; i += p_divisor/2)
  {
    isSecondary++;

    if(i < p_ruler_startvalue)
    {
      continue;
    }

    if (isSecondary%2)
    {
        painter->setPen(RasterColor);
    } else {
        painter->setPen(SecondaryRasterColor);
    }

    raster_position = (double)(i - p_ruler_startvalue) * p_pixels_per_unit;
    painter->drawLine(raster_position, 0, raster_position, curve_h);
  }

  // vertical rasters
  calculateRulerParameters(
                chartArea.height(), v_ruler_min_value, v_ruler_max_value,
                &p_multiplier, &p_ruler_startvalue, &p_ruler_endvalue, &p_range, &p_pixels_per_unit, &p_divisor, &p_precision
                );

  isSecondary = 0;
  for(i = (p_ruler_startvalue / p_divisor) * p_divisor; i < p_ruler_endvalue; i += p_divisor/2)
  {
    isSecondary++;

    if(i < p_ruler_startvalue)
    {
      continue;
    }

    raster_position = (double)(i - p_ruler_startvalue) * p_pixels_per_unit;

    if (isSecondary%2)
    {
        painter->setPen(RasterColor);
    } else {
        painter->setPen(SecondaryRasterColor);
    }

    if(curveUpSideDown == false)
    {
      painter->drawLine(0, curve_h - raster_position, curve_w, curve_h - raster_position);
    }
    else
    {
      painter->drawLine(0, raster_position, curve_w, raster_position);
    }
  }

/////////////////////////////////// draw the curve ///////////////////////////////////////////

  for (int j=0; j < signals_.count(); j++)
  {
    drawSignalCurve(painter, curve_w, curve_h, signals_[j]);
  }

/////////////////////////////////// draw the line ///////////////////////////////////////////

  if(line1Enabled == true)
  {
    painter->setPen(signals_[0]->GetPen());

    painter->drawLine(line1_start_x * h_step, (line1_start_y + offset) * v_sens, line1_end_x * h_step, (line1_end_y + offset) * v_sens);
  }

/////////////////////////////////// draw the markers ///////////////////////////////////////////

  if(Marker1Enabled == true)
  {
    painter->setPen(Marker1Pen);

    painter->drawLine(curve_w * marker_1_position, 0, curve_w * marker_1_position, curve_h);
  }

  if(Marker2Enabled == true)
  {
    painter->setPen(Marker2Pen);

    painter->drawLine(curve_w * marker_2_position, 0, curve_w * marker_2_position, curve_h);
  }

/////////////////////////////////// draw the cursor ///////////////////////////////////////////

  if(crosshair_1_active)
  {
    painter->setPen(signals_[0]->GetPen());

    // draw the little triangle
    QPainterPath path;
    path.moveTo(crosshair_1_x_position, crosshair_1_y_value);
    path.lineTo(crosshair_1_x_position - 4, crosshair_1_y_value - 10);
    path.lineTo(crosshair_1_x_position + 5, crosshair_1_y_value - 10);
    path.lineTo(crosshair_1_x_position, crosshair_1_y_value);
    painter->fillPath(path, signals_[0]->GetPen().brush());

    // constrant the information to be displayed
    painter->setFont(QFont("Arial", 10));
    QString value_at_crosshair = QString::number(crosshair_1_value, 'g', 5) + " " + v_ruler_unit;
    QString range_at_crosshair = "@" + QString::number(crosshair_1_value_2, 'g', 4) + "-" + QString::number(crosshair_1_value_2 + (1 / signals_[0]->GetHorizontalDensity()), 'g', 4) + " " + h_ruler_unit;

    fontMetrics = QFontMetrics(painter->font());
    QRect crosshairRect_value = fontMetrics.boundingRect(value_at_crosshair);
    QRect crosshairRect_range = fontMetrics.boundingRect(range_at_crosshair);
    QRect crosshairRect = QRect(0, 0,
                                ((crosshairRect_value.width() > crosshairRect_range.width()) ? crosshairRect_value.width() : crosshairRect_range.width()) + 10,
                                ((crosshairRect_value.height() > crosshairRect_range.height()) ? crosshairRect_value.height() : crosshairRect_range.height()) + 25);

    crosshairRect.translate(crosshair_1_x_position - (crosshairRect.width()/2), crosshair_1_y_value - crosshairRect.height() - 10);
    if (crosshairRect.left() < 0)
    {
        crosshairRect.translate(-crosshairRect.x(), 0);
    }
    if (crosshairRect.right() > chartArea.width())
    {
        crosshairRect.translate(chartArea.width()-crosshairRect.right(), 0);
    }

    // draw the rectangle around it and then the information itself
    painter->setPen(signals_[0]->GetPen());
    painter->fillRect(crosshairRect, BackgroundColor);
    painter->drawRect(crosshairRect);

    // add the margin
    crosshairRect.translate(5,5);
    painter->drawText(crosshairRect, value_at_crosshair);

    // move to the next line
    crosshairRect.translate(0,16);
    painter->drawText(crosshairRect, range_at_crosshair);
  }

/////////////////////////////////// draw the buttons ///////////////////////////////////////////

  painter->setPen(Qt::black);
  painter->setFont(QFont("Arial", 8));
  if(printEnabled == true)
  {
    painter->fillRect(curve_w - 43, 3, 40, 18, Qt::gray);
    painter->drawText(curve_w - 38, 16, "print");
  }
  if(cursorEnabled == true)
  {
    painter->fillRect(curve_w - 43, 23, 40, 18, Qt::gray);
    painter->drawText(curve_w - 38, 36, "cursor");
  }
  if(dashBoardEnabled == true)
  {
    painter->fillRect(curve_w - 43, 43, 40, 18, Qt::gray);
    painter->drawText(curve_w - 38, 56, "ctls");
  }
}

void SignalCurve::calculateRulerParameters(int length, double start_value, double end_value, int *multiplier, int *normalized_start_value, int *normalized_end_value, int *range, double *pixels_per_unit, int *divisor, int *precision)
{
    *multiplier = 1;

    while((end_value * *multiplier) < 10000.0)
    {
      *multiplier *= 10;

      if(*multiplier > 10000000)
      {
        throw ("multiplier overflow in calculateRulerParameters");
        break;
      }
    }

    *normalized_start_value = start_value * *multiplier;

    *normalized_end_value = end_value * *multiplier;

    *range = *normalized_end_value - *normalized_start_value;

    *pixels_per_unit = (double)length / (double)*range;

    *divisor = 1;

    while((*range / *divisor) > 10)
    {
      *divisor *= 2;

      if((*range / *divisor) <= 10)
      {
        break;
      }

      *divisor /= 2;

      *divisor *= 5;

      if((*range / *divisor) <= 10)
      {
        break;
      }

      *divisor *= 2;
    }

    *precision = 0;
    if((end_value < 10.0) && (end_value > -10.0) && (start_value < 10.0) && (start_value > -10.0))
    {
      *precision = 1;
      if((end_value < 1.0) && (end_value > -1.0) && (start_value < 1.0) && (start_value > -1.0))
      {
        *precision = 2;
        if((end_value < 0.1) && (end_value > -0.1) && (start_value < 0.1) && (start_value > -0.1))
        {
          *precision = 3;
          if((end_value < 0.01) && (end_value > -0.01) && (start_value < 0.01) && (start_value > -0.01))
          {
            *precision = 4;
          }
        }
      }
    }
}

void SignalCurve::drawTextOnRuler(QPainter *painter, QRect area, int position, double value, int precision, bool is_vertical, bool is_upsidedown)
{
    QString q_str;

    if (precision > 0)
    {
        q_str.setNum(value, 'f', precision);
    } else {
        char value_str[64];
        thousandsep(value, value_str, sizeof value_str, 0);
        q_str.append(value_str);
    }

    if (is_vertical)
    {
        if(is_upsidedown == false)
        {
          int real_vertical_position = area.bottom() - position;

          painter->drawText(area.left() - 57, real_vertical_position - 8, 40, 16, Qt::AlignRight | Qt::AlignVCenter | Qt::TextSingleLine, q_str);

          painter->drawLine(area.left() - 5, real_vertical_position, area.left() - 5 - 10, real_vertical_position);
        }
        else
        {
          int real_vertical_position = area.top() + position;

          painter->drawText(area.left() - 57, real_vertical_position - 8, 40, 16, Qt::AlignRight | Qt::AlignVCenter | Qt::TextSingleLine, q_str);

          painter->drawLine(area.left() - 5, real_vertical_position, area.left() - 5 - 10, real_vertical_position);
        }
    } else {
        int real_horizontal_position = area.left() + position;

        painter->drawText(real_horizontal_position - 30, area.bottom() + 18, 60, 16, Qt::AlignCenter | Qt::TextSingleLine, q_str);

        painter->drawLine(real_horizontal_position, area.bottom() + 5, real_horizontal_position, area.bottom() + 5 + 10);
    }
}


void SignalCurve::drawSignalCurve(QPainter *painter, int curve_w, int curve_h, Signal *signal)
{
    int i;

    double v_sens=0.0,
           offset=0.0,
           h_step=0.0; /* horizontal step alias the width of one bar */;


      if(v_ruler_max_value <= v_ruler_min_value)  return;

      if(signal_display_length < 2)  return;

      if (signals_.count() == 0)  return;

      if(curveUpSideDown == true)
      {
        offset = (-(v_ruler_min_value));

        v_sens = curve_h / (v_ruler_max_value - v_ruler_min_value);
      }
      else
      {
        offset = (-(v_ruler_max_value));

        v_sens = (-(curve_h / (v_ruler_max_value - v_ruler_min_value)));
      }

      h_step = (double)curve_w / (double)signal_display_length; /// width of one bar = width of the area where we draw / number of bars

      QPen linePen = signal->GetPen();
      QBrush fillBrush = QBrush(QColor(linePen.color().red(), linePen.color().green(), linePen.color().blue(), linePen.color().alpha() / 8), Qt::SolidPattern);

      QPolygon curvePolygon = QPolygon();

      for(i = 0; i < signal_display_length; i++)
      {
          double value = signal->GetValues()[i + (int)signal_display_start];
          double vertical_position = (value + offset) * v_sens;

          if (value != signal->SIGNAL_NA_VALUE)
          {
              curvePolygon.append(QPoint((i * h_step) + (h_step/2), vertical_position));
          }
          else
          {
              drawSignalCurveSegment(painter, curvePolygon, curve_h, linePen, fillBrush);
              curvePolygon.clear();
          }

          if(crosshair_1_active)
          {
              if(i==((int)(((double)crosshair_1_x_position) / h_step)))
              {
                  crosshair_1_y_value = vertical_position;
                  crosshair_1_value = (value == signal->SIGNAL_NA_VALUE ? 0.0 : value);
                  double v_value = (h_ruler_max_value - h_ruler_min_value) / signal_display_length;
                  crosshair_1_value_2 = (i * v_value) + h_ruler_min_value;
              }
          }
      }

      drawSignalCurveSegment(painter, curvePolygon, curve_h, linePen, fillBrush);
}

void SignalCurve::drawSignalCurveSegment(QPainter *painter, QPolygon curvePolygon, int curve_h, QPen linePen, QBrush fillBrush)
{
    if (curvePolygon.count() == 0) return;

    // TODO: convert the polygon to a bezier curve

    // draw the curve
    painter->setPen(linePen);
    painter->drawPolyline(curvePolygon);

    // fill below
    curvePolygon.append(QPoint(curvePolygon.at(curvePolygon.count()-1).x(), curve_h));
    curvePolygon.append(QPoint(curvePolygon.at(0).x(), curve_h));

    QPainterPath path;
    path.addPolygon(curvePolygon);
    painter->fillPath(path, fillBrush);
}


void SignalCurve::drawLine(int start_x, double start_y, int end_x, double end_y, QColor lineColor)
{
  line1Color = lineColor;

  line1_start_x = start_x;
  if(line1_start_x < 0)
  {
    line1_start_x = 0;
  }
  if(line1_start_x >= signal_display_length)
  {
    line1_start_x = signal_display_length - 1;
  }

  line1_start_y = start_y;

  line1_end_x = end_x;
  if(line1_end_x < 0)
  {
    line1_end_x = 0;
  }
  if(line1_end_x >= signal_display_length)
  {
    line1_end_x = signal_display_length - 1;
  }

  line1_end_y = end_y;

  line1Enabled = true;

  update_pending = true;
}


void SignalCurve::setLineEnabled(bool stat)
{
  line1Enabled = stat;

  update_pending = true;
}


void SignalCurve::drawCurve(double *sample_buffer, int start_index, int buffer_size, double h_max_value, double h_min_value)
{
    QVector<double> values = QVector<double>();
    for (int i=start_index; i<buffer_size; i++)
    {
        values.append(sample_buffer[i]);
    }

    if (signals_.count() == 0)
    {
        // this is a new signal
        Signal *newSignal = new Signal("ADC", "ADC-FFT", "Frontal lobe", values, "Frequency", "Hz", buffer_size / 256.0, "Intensity", "???", 1.0, SignalType::FFT);

        drawCurve(newSignal, 0.0, chartArea.width() / 20.0, h_min_value, h_max_value);
        resetRulers();
    } else {
        // this is an old signal, so we don't need to calibrate the histogram
        signals_[0]->SetValues(values);

        update_pending = true;
    }
}

void SignalCurve::drawCurve(Signal *signal, double h_min_value, double h_max_value, double v_min_value, double v_max_value)
{
  this->signals_.clear();
  this->signals_.append(signal);

  h_ruler_name = signal->GetHorizontalName();
  h_ruler_unit = signal->GetHorizontalUnit();

  v_ruler_name = signal->GetVerticalName();
  v_ruler_unit = signal->GetVerticalUnit();

  h_ruler_min_value = h_min_value;
  h_ruler_max_value = h_max_value;

  v_ruler_min_value = v_min_value;
  v_ruler_max_value = v_max_value;

  resetRulers();

  signal_display_start = 0;
  signal_display_length = (h_ruler_max_value - h_ruler_min_value) * this->signals_[0]->GetHorizontalDensity();

  update_pending = true;
}

void SignalCurve::addSignal(Signal *signal)
{
    bool signalFound = false;
    for (int j=0; j < signals_.count(); j++)
    {
        if (signals_[j]->GetId() == signal->GetId())
        {
            signalFound = true;
        }
    }

    if (!signalFound)
    {
        this->signals_.append(signal);

        QObject::connect(this->signals_[this->signals_.count()-1], SIGNAL(valuesChanged(QVector<double>)), this, SLOT(signalValueChanged(QVector<double>)));

        h_ruler_name = signal->GetHorizontalName();
        h_ruler_unit = signal->GetHorizontalUnit();

        v_ruler_name = signal->GetVerticalName();
        v_ruler_unit = signal->GetVerticalUnit();

        signal_display_start = 0;
        signal_display_length = (h_ruler_max_value - h_ruler_min_value) * signal->GetHorizontalDensity();

        resetRulers();

        update_pending = true;
    }
}

void SignalCurve::removeSignal(QString signalId)
{
    for (int j=0; j < signals_.count(); j++)
    {
        if ((signalId == "") || (signals_[j]->GetId() == signalId))
        {
            signals_.removeAt(j);
            j--;
        }
    }
}

void SignalCurve::signalValueChanged(QVector<double>)
{
    if ((v_ruler_min_value == DBL_MAX) || (v_ruler_max_value == DBL_MIN))
    {
        // rulers were not initialized yer
        resetRulers();
    }

    update_pending = true;
}

void SignalCurve::setFillSurfaceEnabled(bool enabled)
{
  if(enabled == true)
  {
    fillsurface = 1;
  }
  else
  {
    fillsurface = 0;
  }

  update_pending = true;
}


void SignalCurve::setMarker1Position(double mrk_pos)
{
  marker_1_position = mrk_pos;

  if(marker_1_position > 1.01)  marker_1_position = 1.01;

  if(marker_1_position < 0.0001)  marker_1_position = 0.0001;

  update_pending = true;
}


void SignalCurve::setMarker1Enabled(bool on)
{
  Marker1Enabled = on;

  update_pending = true;
}


void SignalCurve::setMarker1MovableEnabled(bool on)
{
  Marker1MovableEnabled = on;

  update_pending = true;
}


void SignalCurve::setMarker1Color(QColor color)
{
  Marker1Pen.setColor(color);

  update_pending = true;
}


double SignalCurve::getMarker1Position(void)
{
  return(marker_1_position);
}


void SignalCurve::setMarker2Position(double mrk_pos)
{
  marker_2_position = mrk_pos;

  if(marker_2_position > 1.01)  marker_2_position = 1.01;

  if(marker_2_position < 0.0001)  marker_2_position = 0.0001;

  update_pending = true;
}


void SignalCurve::setMarker2Enabled(bool on)
{
  Marker2Enabled = on;

  update_pending = true;
}


void SignalCurve::setMarker2MovableEnabled(bool on)
{
  Marker2MovableEnabled = on;

  update_pending = true;
}


void SignalCurve::setMarker2Color(QColor color)
{
  Marker2Pen.setColor(color);

  update_pending = true;
}


double SignalCurve::getMarker2Position(void)
{
  return(marker_2_position);
}


void SignalCurve::setH_RulerValues(double start, double end)
{
  h_ruler_min_value = start;
  h_ruler_max_value = end;

  update_pending = true;
}


void SignalCurve::setSignalColor(QColor newColor)
{
  QPen pen = signals_[0]->GetPen();
  pen.setColor(newColor);
  signals_[0]->SetPen(pen);

  update_pending = true;
}


void SignalCurve::setCrosshairColor(QColor newColor)
{
  crosshair_1_color = newColor;

  update_pending = true;
}


void SignalCurve::setTraceWidth(int tr_width)
{
    QPen pen = signals_[0]->GetPen();
    pen.setWidth(tr_width);
    signals_[0]->SetPen(pen);

    update_pending = true;
}


void SignalCurve::setBackgroundColor(QColor newColor)
{
  BackgroundColor = newColor;

  update_pending = true;
}


void SignalCurve::setRasterColor(QColor newColor)
{
  RasterColor = newColor;

  update_pending = true;
}


void SignalCurve::setBorderColor(QColor newColor)
{
  BorderColor = newColor;

  update_pending = true;
}


void SignalCurve::setTextColor(QColor newColor)
{
  TextColor = newColor;

  update_pending = true;
}


void SignalCurve::setHorizontalRulerText(const char *name, const char *unit)
{
    if (name != NULL)
    {
        h_ruler_name = name;
    }

    if (unit != NULL)
    {
        h_ruler_unit = unit;
    }

    update_pending = true;
}


void SignalCurve::setVerticalRulerText(const char *name, const char *unit)
{
    if (name != NULL)
    {
        v_ruler_name = name;
    }

    if (unit != NULL)
    {
        v_ruler_unit = unit;
    }

    update_pending = true;
}


void SignalCurve::setHeaderText(const char *str)
{
  header_label = str;

  update_pending = true;
}


void SignalCurve::setCursorEnabled(bool value)
{
  cursorEnabled = value;
}


bool SignalCurve::isCursorEnabled(void)
{
  return(cursorEnabled);
}


bool SignalCurve::isCursorActive(void)
{
  return(crosshair_1_active);
}


void SignalCurve::setPrintEnabled(bool value)
{
  printEnabled = value;
}


void SignalCurve::setDashBoardEnabled(bool value)
{
  dashBoardEnabled = value;
}


void SignalCurve::enableSpectrumColors(struct spectrum_markersblock *spectr_col)
{
  spectrum_color = spectr_col;

  update_pending = true;
}


void SignalCurve::disableSpectrumColors()
{
  spectrum_color = NULL;

  update_pending = true;
}


void SignalCurve::setV_rulerEnabled(bool value)
{
  if(value == true)
  {
    isVerticalRulerVisible = 1;
  }
  else
  {
    isVerticalRulerVisible = 0;
  }
}


void SignalCurve::setUpsidedownEnabled(bool value)
{
  curveUpSideDown = value;
}


int SignalCurve::getCursorPosition(void)
{
  return(crosshair_1_value_2);
}


  /* size is size of destination, returns length of directory */
  /* last character of destination is not a slash! */
int SignalCurve::get_directory_from_path(char *dest, const char *src, int ssize)
{
  int i, len;

  if(ssize<1)
  {
    return(-1);
  }

  if(ssize<2)
  {
    dest[0] = 0;

    return(0);
  }

  len = strlen(src);

  if(len < 1)
  {
    dest[0] = 0;

    return(0);
  }

  for(i=len-1; i>=0; i--)
  {
    if((src[i]=='/') || (src[i]=='\\'))
    {
      break;
    }
  }

  strncpy(dest, src, ssize);

  if(i < ssize)
  {
    dest[i] = 0;
  }
  else
  {
    dest[ssize-1] = 0;
  }

  return(strlen(dest));
}




