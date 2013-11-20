/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2013 Teunis van Beelen
*
* teuniz@gmail.com
*
***************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation version 2 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
***************************************************************************
*
* This version of GPL is at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*
***************************************************************************
*/



#include "unisens2edf.h"



#if defined(__APPLE__) || defined(__MACH__) || defined(__APPLE_CC__)

#define fopeno fopen

#else

#define fseeko fseeko64
#define ftello ftello64
#define fopeno fopen64

#endif





UI_UNISENS2EDFwindow::UI_UNISENS2EDFwindow(char *recent_dir, char *save_dir)
{
  recent_opendir = recent_dir;
  recent_savedir = save_dir;

  myobjectDialog = new QDialog;

  myobjectDialog->setMinimumSize(QSize(300, 75));
  myobjectDialog->setMaximumSize(QSize(300, 75));
  myobjectDialog->setWindowTitle("Unisense to EDF+ converter");
  myobjectDialog->setModal(TRUE);
  myobjectDialog->setAttribute(Qt::WA_DeleteOnClose, TRUE);

  pushButton1 = new QPushButton(myobjectDialog);
  pushButton1->setGeometry(20, 30, 100, 25);
  pushButton1->setText("Select File");

  pushButton2 = new QPushButton(myobjectDialog);
  pushButton2->setGeometry(180, 30, 100, 25);
  pushButton2->setText("Close");

  QObject::connect(pushButton1, SIGNAL(clicked()), this,           SLOT(SelectFileButton()));
  QObject::connect(pushButton2, SIGNAL(clicked()), myobjectDialog, SLOT(close()));

  myobjectDialog->exec();
}


void UI_UNISENS2EDFwindow::SelectFileButton()
{
  int i, j, k, n,
      tmp,
      chns,
      hdl,
      err,
      *buf2,
      bufsize,
      blocks_written,
      progress_steps;

  short tmp3;

  char path[MAX_PATH_LENGTH],
       binfilepath[MAX_PATH_LENGTH],
       outputfilename[MAX_PATH_LENGTH],
       scratchpad[2048],
       str[256],
       *buf1,
       tmp2;

  struct xml_handle *xml_hdl;


  strcpy(path, QFileDialog::getOpenFileName(0, "Select inputfile", QString::fromLocal8Bit(recent_opendir), "XML files (*.xml *.XML)").toLocal8Bit().data());

  if(!strcmp(path, ""))
  {
    return;
  }

  get_directory_from_path(recent_opendir, path, MAX_PATH_LENGTH);

  xml_hdl = xml_get_handle(path);

  if(xml_hdl == NULL)
  {
    sprintf(scratchpad, "Can not open file:\n%s", path);
    QMessageBox messagewindow(QMessageBox::Critical, "Error", scratchpad);
    messagewindow.exec();
    return;
  }

  char_encoding = xml_hdl->encoding;

  if(char_encoding == 0)  // attribute encoding not present
  {
    char_encoding = 2;  // fallback to UTF-8 because it is the default for XML
  }
  else if(char_encoding > 2)  // unknown encoding  FIX ME!!
  {
    char_encoding = 1;  // fallback to ISO-8859-1 (Latin1)
  }

  if(strcmp(xml_hdl->elementname, "unisens"))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find root element \"unisens\".");
    messagewindow.exec();
    xml_close(xml_hdl);
    return;
  }

  if(xml_get_attribute_of_element(xml_hdl, "timestampStart", str, 255))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"timestampStart\".");
    messagewindow.exec();
    return;
  }

  if((strlen(str) < 19) || (strlen(str) > 23) || (str[4] != '-') || (str[7] != '-') || (str[10] != 'T') || (str[13] != ':') || (str[16] != ':'))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"timestampStart\".");
    messagewindow.exec();
    return;
  }

  err = 0;

  for(i=0; i<19; i++)
  {
    if((i == 4) || (i == 7) || (i == 10) || (i == 13) || (i == 16))  continue;

    if(!(isdigit(str[i])))  err = 1;
  }

  if(strlen(str) == 23)
  {
    if(str[19] != '.')  err = 1;

    for(i=20; i<23; i++)
    {
      if(!(isdigit(str[i])))  err = 1;
    }

    starttime_fraction = atoi(str + 20);

    if((starttime_fraction < 0) || (starttime_fraction > 999))  err = 1;
  }
  else
  {
    starttime_fraction = 0;
  }

  if(err)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Invalid format in attribute \"timestampStart\".");
    messagewindow.exec();
    return;
  }

  strncpy(str_timestampStart, str, 19);

  str_timestampStart[19] = 0;

  if(xml_get_attribute_of_element(xml_hdl, "measurementId", str, 255))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"measurementId\".");
    messagewindow.exec();
    return;
  }

  if(strlen(str) < 1)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Attribute \"measurementId\" has no data.");
    messagewindow.exec();
    return;
  }

  if(char_encoding == 1)  // Latin-1
  {
    strncpy(str_measurementId, QString::fromLatin1(str).toLocal8Bit().data(), 128);
  }
  else if(char_encoding == 2)
    {
      strncpy(str_measurementId, QString::fromUtf8(str).toLocal8Bit().data(), 128);
    }
    else
    {
//      strncpy(str_measurementId, str, 128);
      strncpy(str_measurementId, QString::fromUtf8(str).toLocal8Bit().data(), 128);  // default for XML is UTF-8
    }

  str_measurementId[127] = 0;

  xml_goto_root(xml_hdl);

////////////////////////// read parameters from XML-file //////////////////////////////////////////

  total_edf_signals = 0;

  bdf = 0;

  for(file_cnt=0; file_cnt<MAXFILES; file_cnt++)
  {
    if(xml_goto_nth_element_inside(xml_hdl, "signalEntry", file_cnt))
    {
      break;
    }

    if(get_signalparameters_from_BIN_attributes(xml_hdl, file_cnt))
    {
      xml_close(xml_hdl);
      for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
      return;
    }

    if(xml_goto_nth_element_inside(xml_hdl, "binFileFormat", 0))
    {
      QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find element \"binFileFormat\".");
      messagewindow.exec();
      xml_close(xml_hdl);
      for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
      return;
    }

    if(xml_get_attribute_of_element(xml_hdl, "endianess", str, 255))
    {
      QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"endianess\".");
      messagewindow.exec();
      xml_close(xml_hdl);
      for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
      return;
    }

    if(strlen(str) < 1)
    {
      QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"endianess\".");
      messagewindow.exec();
      xml_close(xml_hdl);
      for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
      return;
    }

    if(strcmp(str, "LITTLE") && strcmp(str, "BIG"))
    {
      QMessageBox messagewindow(QMessageBox::Critical, "Error", "Attribute \"endianess\" has an unknown value.");
      messagewindow.exec();
      xml_close(xml_hdl);
      for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
      return;
    }

    if(!(strcmp(str, "LITTLE")))
    {
      big_endian[file_cnt] = 0;
    }
    else
    {
      big_endian[file_cnt] = 1;
    }

    xml_go_up(xml_hdl);

    edf_signal_start[file_cnt] = total_edf_signals;

    for(chns=0; chns<MAXSIGNALS; chns++)
    {
      if(xml_goto_nth_element_inside(xml_hdl, "channel", chns))
      {
        break;
      }

      if(xml_get_attribute_of_element(xml_hdl, "name", str, 255))
      {
        QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"name\" in element \"channel\".");
        messagewindow.exec();
        xml_close(xml_hdl);
        for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
        return;
      }

      if(char_encoding == 1)  // Latin-1
      {
        strncpy(signallabel[total_edf_signals], str, 16);
      }
      else if(char_encoding == 2)
        {
          strncpy(signallabel[total_edf_signals], QString::fromUtf8(str).toLatin1().data(), 16);
        }
        else
        {
//          strncpy(signallabel[total_edf_signals], str, 16);
          strncpy(signallabel[total_edf_signals], QString::fromUtf8(str).toLatin1().data(), 16);  // default for XML is UTF-8
        }

      signallabel[total_edf_signals][16] = 0;

      total_edf_signals++;

      xml_go_up(xml_hdl);
    }

    edf_signal_stop[file_cnt] = total_edf_signals;

    edf_signals[file_cnt] = chns;

    if(chns < 1)
    {
      QMessageBox messagewindow(QMessageBox::Critical, "Error", "No channels in element \"signalEntry\".");
      messagewindow.exec();
      xml_close(xml_hdl);
      for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
      return;
    }

    if(chns >= MAXSIGNALS)
    {
      QMessageBox messagewindow(QMessageBox::Critical, "Error", "Too many channels in element \"signalEntry\".");
      messagewindow.exec();
      xml_close(xml_hdl);
      for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
      return;
    }

    get_directory_from_path(binfilepath, path, MAX_PATH_LENGTH);

    strcat(binfilepath, "/");
    strcat(binfilepath, binfilename[file_cnt]);

    binfile[file_cnt] = fopeno(binfilepath, "rb");
    if(binfile[file_cnt] == NULL)
    {
      sprintf(scratchpad, "Can not open binary file:\n%s", binfilepath);
      QMessageBox messagewindow(QMessageBox::Critical, "Error", scratchpad);
      messagewindow.exec();
      xml_close(xml_hdl);
      for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
      return;
    }

    xml_go_up(xml_hdl);
  }

  if(file_cnt < 1)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find element \"signalEntry\".");
    messagewindow.exec();
    xml_close(xml_hdl);
    return;
  }

  if(total_edf_signals > (MAXSIGNALS - 1))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Too many signals (limit is 511).");
    messagewindow.exec();
    for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
    xml_close(xml_hdl);
    return;
  }

  sf_divider = 10;

  for(i=0; i<file_cnt; i++)
  {
    if(sf[i] % 10)
    {
      sf_divider = 1;

      break;
    }
  }

  if(sf_divider == 1)
  {
    sf_divider = 8;

    for(i=0; i<file_cnt; i++)
    {
      if(sf[i] % 8)
      {
        sf_divider = 1;

        break;
      }
    }
  }

  if(sf_divider == 1)
  {
    sf_divider = 5;

    for(i=0; i<file_cnt; i++)
    {
      if(sf[i] % 5)
      {
        sf_divider = 1;

        break;
      }
    }
  }

  if(sf_divider == 1)
  {
    sf_divider = 4;

    for(i=0; i<file_cnt; i++)
    {
      if(sf[i] % 4)
      {
        sf_divider = 1;

        break;
      }
    }
  }

  if(sf_divider == 1)
  {
    sf_divider = 2;

    for(i=0; i<file_cnt; i++)
    {
      if(sf[i] % 2)
      {
        sf_divider = 1;

        break;
      }
    }
  }

  max_datablocks = 0;

  for(k=0; k<file_cnt; k++)
  {
    sf[k] /= sf_divider;

    fseeko(binfile[k], 0LL, SEEK_END);
    datablocks[k] = ftello(binfile[k]) / (edf_signals[k] * samplesize[k] * sf[k]);
    fseeko(binfile[k], 0LL, SEEK_SET);

    if(datablocks[k] > max_datablocks)
    {
      max_datablocks = datablocks[k];
    }
  }

////////////////////////////////////// Check for and count the triggers ////////////////////////////////////////////////////////////

  total_annotations = 0;

  xml_goto_root(xml_hdl);

  trig_file_cnt = 0;

  for(k=0; k<MAXFILES; k++)
  {
    if(xml_goto_nth_element_inside(xml_hdl, "eventEntry", k))
    {
      break;
    }

    if(get_signalparameters_from_EVT_attributes(xml_hdl, k))
    {
      xml_close(xml_hdl);
      for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
      return;
    }

    if(!xml_goto_nth_element_inside(xml_hdl, "csvFileFormat", 0))
    {
      if(xml_get_attribute_of_element(xml_hdl, "separator", str, 255))
      {
        QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"separator\".");
        messagewindow.exec();
        xml_close(xml_hdl);
        for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
        return;
      }

      if(strlen(str) != 1)
      {
        QMessageBox messagewindow(QMessageBox::Critical, "Error", "Value for \"separator\" must be one character.");
        messagewindow.exec();
        xml_close(xml_hdl);
        for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
        return;
      }

      csv_sep[k] = str[0];

      if(xml_get_attribute_of_element(xml_hdl, "decimalSeparator", str, 255))
      {
        csv_dec_sep[k] = '.';  // if attribute decimalSeparator is not present, fall back to a dot
      }
      else
      {
        if(strlen(str) != 1)
        {
          QMessageBox messagewindow(QMessageBox::Critical, "Error", "Value for \"decimalSeparator\" must be one character.");
          messagewindow.exec();
          xml_close(xml_hdl);
          for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
          return;
        }

        csv_dec_sep[k] = str[0];

        if(csv_sep[k] == csv_dec_sep[k])
        {
          QMessageBox messagewindow(QMessageBox::Critical, "Error", "Attribute \"decimalSeparator\" and \"separator\" have equal values.");
          messagewindow.exec();
          xml_close(xml_hdl);
          for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
          return;
        }

        xml_go_up(xml_hdl);
      }

      trig_file_cnt++;
    }

    xml_go_up(xml_hdl);
  }

  if(count_events_from_csv_files(trig_file_cnt, path, &total_annotations))
  {
    xml_close(xml_hdl);
    for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
    return;
  }

////////////////////// Initialize EDF-file ////////////////////////////////////////////////////////

  outputfilename[0] = 0;
  if(recent_savedir[0]!=0)
  {
    strcpy(outputfilename, recent_savedir);
    strcat(outputfilename, "/");
  }
  strcat(outputfilename, str_measurementId);
  remove_extension_from_filename(outputfilename);
  if(bdf == 1)
  {
    strcat(outputfilename, ".bdf");

    strcpy(outputfilename, QFileDialog::getSaveFileName(0, "Select outputfile", QString::fromLocal8Bit(outputfilename), "BDF files (*.bdf *.BDF)").toLocal8Bit().data());

    if(!strcmp(outputfilename, ""))
    {
      for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
      xml_close(xml_hdl);
      return;
    }

    get_directory_from_path(recent_savedir, outputfilename, MAX_PATH_LENGTH);

    hdl = edfopen_file_writeonly(outputfilename, EDFLIB_FILETYPE_BDFPLUS, total_edf_signals);
  }
  else
  {
    strcat(outputfilename, ".edf");

    strcpy(outputfilename, QFileDialog::getSaveFileName(0, "Select outputfile", QString::fromLocal8Bit(outputfilename), "EDF files (*.edf *.EDF)").toLocal8Bit().data());

    if(!strcmp(outputfilename, ""))
    {
      for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
      xml_close(xml_hdl);
      return;
    }

    get_directory_from_path(recent_savedir, outputfilename, MAX_PATH_LENGTH);

    hdl = edfopen_file_writeonly(outputfilename, EDFLIB_FILETYPE_EDFPLUS, total_edf_signals);
  }

  if(hdl<0)
  {
    snprintf(scratchpad, 2048, "Can not open file %s for writing.", outputfilename);
    QMessageBox messagewindow(QMessageBox::Critical, "Error", scratchpad);
    messagewindow.exec();
    for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
    xml_close(xml_hdl);
    return;
  }

  err = 0;

  for(k=0; k<file_cnt; k++)
  {
    for(i=0; i<edf_signals[k]; i++)
    {
      j = edf_signal_start[k] + i;

      if(edf_set_samplefrequency(hdl, j, sf[k]))  err = 1;

      if(edf_set_digital_maximum(hdl, j, digmax[k]))  err = 1;

      if(edf_set_digital_minimum(hdl, j, digmin[k]))  err = 1;

      if(edf_set_physical_maximum(hdl, j, physmax[k]))  err = 1;

      if(edf_set_physical_minimum(hdl, j, physmin[k]))  err = 1;

      if(edf_set_physical_dimension(hdl, j, physdim[k]))  err = 1;

      if(edf_set_label(hdl, j, signallabel[i]))  err = 1;
    }
  }

  for(i=0; i<total_edf_signals; i++)
  {
    if(edf_set_label(hdl, i, signallabel[i]))  err = 1;
  }

  if(edf_set_startdatetime(hdl, atoi(str_timestampStart), atoi(str_timestampStart + 5),
                          atoi(str_timestampStart + 8), atoi(str_timestampStart + 11),
                          atoi(str_timestampStart + 14), atoi(str_timestampStart + 17)))  err = 1;

  if(edf_set_datarecord_duration(hdl, 100000 / sf_divider))  err = 1;

  if(total_annotations > max_datablocks)
  {
    tmp = (total_annotations / max_datablocks) + 1;

    if(tmp > 64)  tmp = 64;

    if(edf_set_number_of_annotation_signals(hdl, tmp))  err = 1;
  }

  if(err)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "An error occurred during initializing the EDF-header.");
    messagewindow.exec();
    for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
    xml_close(xml_hdl);
    edfclose_file(hdl);
    return;
  }

////////////////////// Prepare data-conversion ////////////////////////////////////////////////////

  bufsize = 0;

  for(k=0; k<file_cnt; k++)   // bufsize is expressed in bytes
  {
    buf1_offset[k] = bufsize;

    buf1_freadsize[k] = (edf_signals[k] * samplesize[k] * sf[k]);

    bufsize += buf1_freadsize[k];
  }

  buf1 = (char *)malloc(bufsize);
  if(buf1 == NULL)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Malloc error (buf1)");
    messagewindow.exec();
    for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
    xml_close(xml_hdl);
    edfclose_file(hdl);
    return;
  }

  j = 0;

  for(k=0; k<file_cnt; k++)  // calculate the total number of samples in one datarecord
  {
    buf2_offset[k] = j;

    j += (edf_signals[k] * sf[k]);
  }

  buf2 = (int *)malloc(j * sizeof(int));
  if(buf2 == NULL)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Malloc error (buf2)");
    messagewindow.exec();
    for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
    xml_close(xml_hdl);
    edfclose_file(hdl);
    free(buf1);
    return;
  }

////////////////////// Start data-conversion //////////////////////////////////////////////////////

  QProgressDialog progress("Converting binary file(s)...", "Cancel", 0, max_datablocks, myobjectDialog);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(200);

  progress_steps = max_datablocks / 100;
  if(progress_steps < 1)
  {
    progress_steps = 1;
  }

  int sf_t, signals_t, *buf2_t;

  char *buf1_t;

  for(blocks_written=0; blocks_written<max_datablocks; blocks_written++)
  {
    if(!(blocks_written % progress_steps))
    {
      progress.setValue(blocks_written);

      qApp->processEvents();

      if(progress.wasCanceled() == TRUE)
      {
        break;
      }
    }

    for(k=0; k<file_cnt; k++)
    {
      sf_t = sf[k];

      signals_t = edf_signals[k];

      buf1_t = buf1 + buf1_offset[k];

      buf2_t = buf2 + buf2_offset[k];

      if(blocks_written < datablocks[k])
      {
        n = fread(buf1_t, buf1_freadsize[k], 1, binfile[k]);
        if(n != 1)
        {
          progress.reset();
          QMessageBox messagewindow(QMessageBox::Critical, "Error", "A read error occurred during the conversion.");
          messagewindow.exec();
          for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
          xml_close(xml_hdl);
          edfclose_file(hdl);
          free(buf1);
          free(buf2);
          return;
        }

        if((samplesize[k] == 2) && (big_endian[k] == 0) && (straightbinary[k] == 0))  // int16
        {
          for(i=0; i<sf_t; i++)
          {
            for(j=0; j<signals_t; j++)
            {
              buf2_t[(j * sf_t) + i] = *(((short *)buf1_t) + (i * signals_t) + j);
            }
          }
        }
        else if((samplesize[k] == 2) && (big_endian[k] == 0) && (straightbinary[k] == 1))  // uint16
          {
            for(i=0; i<sf_t; i++)
            {
              for(j=0; j<signals_t; j++)
              {
                tmp3 = *(((short *)buf1_t) + (i * signals_t) + j) + 32768;

                buf2_t[(j * sf_t) + i] = tmp3;
              }
            }
          }
          else if((samplesize[k] == 1) && (straightbinary[k] == 0))  // int8
            {
              for(i=0; i<sf_t; i++)
              {
                for(j=0; j<signals_t; j++)
                {
                  buf2_t[(j * sf_t) + i] = *(((signed char *)buf1_t) + (i * signals_t) + j);
                }
              }
            }
            else if((samplesize[k] == 1) && (straightbinary[k] == 1))  // uint8
              {
                for(i=0; i<sf_t; i++)
                {
                  for(j=0; j<signals_t; j++)
                  {
                    tmp2 = *(((signed char *)buf1_t) + (i * signals_t) + j) + 128;

                    buf2_t[(j * sf_t) + i] = tmp2;
                  }
                }
              }
              else if((samplesize[k] == 2) && (big_endian[k] == 1) && (straightbinary[k] == 0))  // int16  big endian
                {
                  for(i=0; i<sf_t; i++)
                  {
                    for(j=0; j<signals_t; j++)
                    {
                      tmp3 = *(((short *)buf1_t) + (i * signals_t) + j);

                      tmp3 = (((unsigned short)tmp3 & 0xFF00) >> 8) | (((unsigned short)tmp3 & 0x00FF) << 8);

                      buf2_t[(j * sf_t) + i] = tmp3;
                    }
                  }
                }
                else if((samplesize[k] == 2) && (big_endian[k] == 1) && (straightbinary[k] == 1))  // uint16  big endian
                  {
                    for(i=0; i<sf_t; i++)
                    {
                      for(j=0; j<signals_t; j++)
                      {
                        tmp3 = *(((short *)buf1_t) + (i * signals_t) + j);

                        tmp3 = (((unsigned short)tmp3 & 0xFF00) >> 8) | (((unsigned short)tmp3 & 0x00FF) << 8);

                        tmp3 += 32768;

                        buf2_t[(j * sf_t) + i] = tmp3;
                      }
                    }
                  }
                  else if((samplesize[k] == 4) && (big_endian[k] == 0) && (straightbinary[k] == 0))  // int32
                    {
                      for(i=0; i<sf_t; i++)
                      {
                        for(j=0; j<signals_t; j++)
                        {
                          buf2_t[(j * sf_t) + i] = *(((int *)buf1_t) + (i * signals_t) + j);
                        }
                      }
                    }
                    else if((samplesize[k] == 4) && (big_endian[k] == 0) && (straightbinary[k] == 1))  // uint32
                      {
                        for(i=0; i<sf_t; i++)
                        {
                          for(j=0; j<signals_t; j++)
                          {
                            tmp = *(((int *)buf1_t) + (i * signals_t) + j);

                            tmp += 0x80000000;

                            buf2_t[(j * sf_t) + i] = tmp;
                          }
                        }
                      }
                      else if((samplesize[k] == 4) && (big_endian[k] == 1) && (straightbinary[k] == 0))  // int32  big endian
                        {
                          for(i=0; i<sf_t; i++)
                          {
                            for(j=0; j<signals_t; j++)
                            {
                                tmp = *(((int *)buf1_t) + (i * signals_t) + j);

                                tmp = (((unsigned int)tmp & 0xFF000000) >> 24) | (((unsigned int)tmp & 0x00FF0000) >> 8) | (((unsigned int)tmp & 0x0000FF00) << 8) | (((unsigned int)tmp & 0x000000FF) << 24);

                                buf2_t[(j * sf_t) + i] = tmp;
                            }
                          }
                        }
                        else if((samplesize[k] == 4) && (big_endian[k] == 1) && (straightbinary[k] == 1))  // int32  big endian
                          {
                            for(i=0; i<sf_t; i++)
                            {
                              for(j=0; j<signals_t; j++)
                              {
                                tmp = *(((int *)buf1_t) + (i * signals_t) + j);

                                tmp += 0x80000000;

                                buf2_t[(j * sf_t) + i] = tmp;
                              }
                            }
                          }
      }
      else if(blocks_written == datablocks[k])
        {
          for(i=0; i<sf_t; i++)
          {
            for(j=0; j<signals_t; j++)
            {
              buf2_t[(j * sf_t) + i] = 0;
            }
          }
        }
    }

    if(edf_blockwrite_digital_samples(hdl, buf2))
    {
      progress.reset();
      QMessageBox messagewindow(QMessageBox::Critical, "Error", "A write error occurred.");
      messagewindow.exec();
      for(i=0; i<file_cnt; i++)  fclose(binfile[i]);
      xml_close(xml_hdl);
      edfclose_file(hdl);
      free(buf1);
      free(buf2);
      return;
    }
  }

  progress.reset();

  free(buf1);
  free(buf2);
  for(i=0; i<file_cnt; i++)  fclose(binfile[i]);

////////////////////////////////////// Write triggers ////////////////////////////////////////////////////////////

  if(get_events_from_csv_files(trig_file_cnt, hdl, path))
  {
    edfclose_file(hdl);

    xml_close(xml_hdl);

    return;
  }


  xml_close(xml_hdl);

  edfclose_file(hdl);
}


int UI_UNISENS2EDFwindow::get_events_from_csv_files(int max_files, int edf_hdl, const char *path)
{
  int i, k,
      len,
      len2,
      progress_steps,
      triggers_written=0;

  char scratchpad[2048],
       csvfilepath[MAX_PATH_LENGTH],
       linebuf[1024],
       *ptr,
       sep,
       annotation[64];

  long long onset;

  FILE *csvfile;


  QProgressDialog progress("Converting triggers ...", "Cancel", 0, total_annotations, myobjectDialog);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(200);

  progress_steps = total_annotations / 100;
  if(progress_steps < 1)
  {
    progress_steps = 1;
  }

  for(k=0; k<max_files; k++)
  {
    get_directory_from_path(csvfilepath, path, MAX_PATH_LENGTH);

    strcat(csvfilepath, "/");
    strcat(csvfilepath, evtfilename[k]);

    csvfile = fopeno(csvfilepath, "rb");
    if(csvfile == NULL)
    {
      sprintf(scratchpad, "Can not open csv file:\n%s", evtfilename[k]);
      QMessageBox messagewindow(QMessageBox::Critical, "Error", scratchpad);
      messagewindow.exec();
      return(1);
    }

    sep = csv_sep[k];

    while(1)
    {
      if(!(triggers_written % progress_steps))
      {
        progress.setValue(triggers_written);

        qApp->processEvents();

        if(progress.wasCanceled() == TRUE)
        {
          fclose(csvfile);

          return(1);
        }
      }

      ptr = fgets(linebuf, 1024, csvfile);

      if(ptr == NULL)  break;

      len = strlen(linebuf);

      if(len < 3)  continue;

      for(i=0; i<len; i++)
      {
        if((*ptr == '\n') || (*ptr == '\r') || (*ptr == 0))
        {
          break;
        }

        if(*ptr == sep)
        {
          if((i < (len - 2)) && (i > 0))
          {
            *ptr = 0;

            onset = atoi(linebuf);

            strncpy(annotation, ++ptr, 46);

            annotation[45] = 0;

            len2 = strlen(annotation);

            if(annotation[len2 - 1] == '\n')
            {
              annotation[--len2] = 0;
            }

            if(annotation[len2 - 1] == '\r')
            {
              annotation[--len2] = 0;
            }

            if(annotation[len2 - 1] == sep)
            {
              annotation[--len2] = 0;
            }

            remove_leading_spaces(annotation);

            remove_trailing_spaces(annotation);

            onset *= 10000LL;

            onset /= evt_sf[k];

            if(char_encoding == 1)
            {
              if(edfwrite_annotation_latin1(edf_hdl, onset, -1LL, annotation))
              {
                progress.reset();
                QMessageBox messagewindow(QMessageBox::Critical, "Error", "An error occurred during writing the events to the EDF-file.");
                messagewindow.exec();
                return(1);
              }
            }

            if(char_encoding == 2)
            {
              if(edfwrite_annotation_utf8(edf_hdl, onset, -1LL, annotation))
              {
                progress.reset();
                QMessageBox messagewindow(QMessageBox::Critical, "Error", "An error occurred during writing the events to the EDF-file.");
                messagewindow.exec();
                return(1);
              }
            }

            triggers_written++;

            break;
          }
          else
          {
            break;
          }
        }

        ptr++;
      }
    }

    fclose(csvfile);
  }

  progress.reset();

  return(0);
}


int UI_UNISENS2EDFwindow::count_events_from_csv_files(int max_files, const char *path, int *result)
{
  int evt_cnt=0, k;

  char scratchpad[2048],
       csvfilepath[MAX_PATH_LENGTH],
       linebuf[1024],
       *ptr;

  FILE *csvfile;


  *result = 0;

  for(k=0; k<max_files; k++)
  {
    get_directory_from_path(csvfilepath, path, MAX_PATH_LENGTH);

    strcat(csvfilepath, "/");
    strcat(csvfilepath, evtfilename[k]);

    csvfile = fopeno(csvfilepath, "rb");
    if(csvfile == NULL)
    {
      sprintf(scratchpad, "Can not open csv file:\n%s", evtfilename[k]);
      QMessageBox messagewindow(QMessageBox::Critical, "Error", scratchpad);
      messagewindow.exec();
      return(1);
    }

    while(1)
    {
      ptr = fgets(linebuf, 1024, csvfile);

      if(ptr == NULL)  break;

      evt_cnt++;
    }

    fclose(csvfile);
  }

  *result = evt_cnt;

  return(0);
}


int UI_UNISENS2EDFwindow::get_signalparameters_from_BIN_attributes(struct xml_handle *xml_hdl, int file_nr)
{
  char str[256],
       scratchpad[2048];


  if(xml_get_attribute_of_element(xml_hdl, "id", str, 255))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"id\".");
    messagewindow.exec();
    return(1);
  }

  if(strlen(str) < 1)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"id\".");
    messagewindow.exec();
    return(1);
  }

  strncpy(binfilename[file_nr], str, MAX_PATH_LENGTH - 1);

  if(xml_get_attribute_of_element(xml_hdl, "adcResolution", str, 255))
  {
    adcres[file_nr] = 0;
  }
  else if(strlen(str) < 1)
    {
      adcres[file_nr] = 0;
    }
    else adcres[file_nr] = atoi(str);

  if(xml_get_attribute_of_element(xml_hdl, "unit", str, 255))
  {
    strcpy(physdim[file_nr], "X");
  }
  else if(strlen(str) < 1)
    {
      strcpy(physdim[file_nr], "X");
    }
    else if(char_encoding == 1)  // Latin-1
      {
        strncpy(physdim[file_nr], str, 8);
      }
      else if(char_encoding == 2)
        {
          strncpy(physdim[file_nr], QString::fromUtf8(str).toLatin1().data(), 8);
        }
        else
        {
//          strncpy(physdim[file_nr], str, 8);
          strncpy(physdim[file_nr], QString::fromUtf8(str).toLatin1().data(), 8);  // default for XML is UTF-8
        }

  physdim[file_nr][8] = 0;

  if(xml_get_attribute_of_element(xml_hdl, "sampleRate", str, 255))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"sampleRate\".");
    messagewindow.exec();
    return(1);
  }

  if(strlen(str) < 1)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"sampleRate\".");
    messagewindow.exec();
    return(1);
  }

  sf[file_nr] = atoi(str);

  if((sf[file_nr] < 1) || (sf[file_nr] > 1000000))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Attribute \"sampleRate\" is out of range.");
    messagewindow.exec();
    return(1);
  }

  if(!xml_get_attribute_of_element(xml_hdl, "baseline", str, 255))
  {
    if(strlen(str) < 1)
    {
      QMessageBox messagewindow(QMessageBox::Critical, "Error", "Attribute \"baseline\" has no value.");
      messagewindow.exec();
      return(1);
    }

    baseline[file_nr] = atoi(str);
  }
  else
  {
    baseline[file_nr] = 0;
  }

  if(!xml_get_attribute_of_element(xml_hdl, "adcZero", str, 255))
  {
    if(strlen(str) < 1)
    {
      QMessageBox messagewindow(QMessageBox::Critical, "Error", "Attribute \"adcZero\" has no value.");
      messagewindow.exec();
      return(1);
    }

    adczero[file_nr] = atoi(str);
  }
  else
  {
    adczero[file_nr] = 0;
  }

  if(xml_get_attribute_of_element(xml_hdl, "lsbValue", str, 255))
  {
    physmax[file_nr] = 1.0;
  }
  else if(strlen(str) < 1)
    {
      physmax[file_nr] = 1.0;
    }
    else physmax[file_nr] = atof(str);

  if((physmax[file_nr] < -1000000.0) || (physmax[file_nr] > 1000000.0))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Attribute \"lsbValue\" is out of range.");
    messagewindow.exec();
    return(1);
  }

  physmin[file_nr] = physmax[file_nr];

  if(xml_get_attribute_of_element(xml_hdl, "dataType", str, 255))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"dataType\".");
    messagewindow.exec();
    return(1);
  }

  if(!strcmp(str, "uint8"))
  {
    straightbinary[file_nr] = 1;
    samplesize[file_nr] = 1;
    physmax[file_nr] *= (127 - baseline[file_nr]);
    physmin[file_nr] *= (-128 - baseline[file_nr]);
    digmax[file_nr] = 127;
    digmin[file_nr] = -128;
  }
  else if(!strcmp(str, "int8"))
    {
      straightbinary[file_nr] = 0;
      samplesize[file_nr] = 1;
      physmax[file_nr] *= (127 - baseline[file_nr]);
      physmin[file_nr] *= (-128 - baseline[file_nr]);
      digmax[file_nr] = 127;
      digmin[file_nr] = -128;
    }
    else if(!strcmp(str, "uint16"))
      {
        straightbinary[file_nr] = 1;
        samplesize[file_nr] = 2;
        physmax[file_nr] *= (32767 - baseline[file_nr]);
        physmin[file_nr] *= (-32768 - baseline[file_nr]);
        digmax[file_nr] = 32767;
        digmin[file_nr] = -32768;
      }
      else if(!strcmp(str, "int16"))
        {
          straightbinary[file_nr] = 0;
          samplesize[file_nr] = 2;
          physmax[file_nr] *= (32767 - baseline[file_nr]);
          physmin[file_nr] *= (-32768 - baseline[file_nr]);
          digmax[file_nr] = 32767;
          digmin[file_nr] = -32768;
        }
        else if(!strcmp(str, "uint32"))
          {
            bdf = 1;
            straightbinary[file_nr] = 1;
            samplesize[file_nr] = 4;
            physmax[file_nr] *= (8388607 - baseline[file_nr]);
            physmin[file_nr] *= (-8388608 - baseline[file_nr]);
            digmax[file_nr] = 8388607;
            digmin[file_nr] = -8388608;
          }
          else if(!strcmp(str, "int32"))
            {
              bdf = 1;
              straightbinary[file_nr] = 0;
              samplesize[file_nr] = 4;
              physmax[file_nr] *= (8388607 - baseline[file_nr]);
              physmin[file_nr] *= (-8388608 - baseline[file_nr]);
              digmax[file_nr] = 8388607;
              digmin[file_nr] = -8388608;
            }
            else
            {
              snprintf(scratchpad, 2047, "Unsupported binary datatype: %s", str);
              QMessageBox messagewindow(QMessageBox::Critical, "Error", scratchpad);
              messagewindow.exec();
              return(1);
            }

  return(0);
}


int UI_UNISENS2EDFwindow::get_signalparameters_from_EVT_attributes(struct xml_handle *xml_hdl, int file_nr)
{
  char str[256];

  if(xml_get_attribute_of_element(xml_hdl, "id", str, 255))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"id\".");
    messagewindow.exec();
    return(1);
  }

  if(strlen(str) < 1)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"id\".");
    messagewindow.exec();
    return(1);
  }

  strncpy(evtfilename[file_nr], str, MAX_PATH_LENGTH - 1);

  if(xml_get_attribute_of_element(xml_hdl, "sampleRate", str, 255))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"sampleRate\".");
    messagewindow.exec();
    return(1);
  }

  if(strlen(str) < 1)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not find attribute \"sampleRate\".");
    messagewindow.exec();
    return(1);
  }

  evt_sf[file_nr] = atoi(str);

  if((evt_sf[file_nr] < 1) || (evt_sf[file_nr] > 1000000))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Attribute \"sampleRate\" is out of range.");
    messagewindow.exec();
    return(1);
  }

  return(0);
}














