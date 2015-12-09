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


#ifndef EDFBROWSER_GLOBAL_H
#define EDFBROWSER_GLOBAL_H

#include <stdio.h>

#if defined(__APPLE__) || defined(__MACH__) || defined(__APPLE_CC__)

#define fopeno fopen

#else

#define fseeko fseeko64
#define ftello ftello64
#define fopeno fopen64

#endif

#define PROGRAM_NAME "EDFbrowser"
#define PROGRAM_VERSION "1.56"
#define MINIMUM_QT_VERSION 0x040701
#define MAXFILES 32
#define MAXSIGNALS 512
#define MAXFILTERS 16
#define TIME_DIMENSION (10000000LL)
#define MAX_ANNOTATION_LEN 512
#define VIEWTIME_SYNCED_OFFSET 0
#define VIEWTIME_SYNCED_ABSOLUT 1
#define VIEWTIME_UNSYNCED 2
#define VIEWTIME_USER_DEF_SYNCED 3
#define MAX_PATH_LENGTH 1024
#define MAX_RECENTFILES 32
#define MAX_ACTIVE_ANNOT_MARKERS 64
#define MAXSPECTRUMDIALOGS 32
#define MAXSPECTRUMDOCKS 8
#define MAXPREDEFINEDMONTAGES 12
#define MAXAVERAGECURVEDIALOGS 32
#define MAXZSCOREDIALOGS 32

#define ANNOT_ID_NK_TRIGGER   0
#define ANNOT_ID_BS_TRIGGER   1

#define VIDEO_STATUS_STOPPED      0
#define VIDEO_STATUS_STARTUP_1    1
#define VIDEO_STATUS_STARTUP_2    2
#define VIDEO_STATUS_STARTUP_3    3
#define VIDEO_STATUS_STARTUP_4    4
#define VIDEO_STATUS_STARTUP_5    5
#define VIDEO_STATUS_PLAYING     16
#define VIDEO_STATUS_PAUSED      17
#define VIDEO_STATUS_ENDED       18

<<<<<<< HEAD
#include <stdio.h>

#include "multitargetsupport.h"
=======
>>>>>>> refs/remotes/Teuniz/master
#include "filter.h"
#include "third_party/fidlib/fidlib.h"
#include "ravg_filter.h"
#include "spike_filter.h"
#include "ecg_filter.h"
#include "z_ratio_filter.h"



struct edfparamblock{
        char   label[17];
        char   transducer[81];
        char   physdimension[9];
        double phys_min;
        double phys_max;
        int    dig_min;
        int    dig_max;
        char   prefilter[81];
        int    smp_per_record;
        char   reserved[33];
        double offset;
        int    buf_offset;
        double bitvalue;
        int    annotation;
      };

struct edfhdrblock{
        FILE      *file_hdl;
        int       file_num;
        char      version[32];
        char      filename[MAX_PATH_LENGTH];
        char      patient[81];
        char      recording[81];
        char      plus_patientcode[81];
        char      plus_gender[16];
        char      plus_birthdate[16];
        char      plus_patient_name[81];
        char      plus_patient_additional[81];
        char      plus_startdate[16];
        char      plus_admincode[81];
        char      plus_technician[81];
        char      plus_equipment[81];
        char      plus_recording_additional[81];
        long long l_starttime;
        long long utc_starttime;
        char      reserved[45];
        int       hdrsize;
        int       edfsignals;
        long long datarecords;
        int       recordsize;
        int       annot_ch[256];
        int       nr_annot_chns;
        int       edf;
        int       edfplus;
        int       bdf;
        int       bdfplus;
        int       discontinuous;
        int       genuine_nk;
        int       nk_triggers_read;
        int       genuine_biosemi;
        double    data_record_duration;
        long long long_data_record_duration;
        long long viewtime;
        long long starttime_offset;
        long long prefiltertime;
        int       annots_not_read;
        int       recording_len_sec;
        struct edfparamblock *edfparam;
      };

struct zoomhistoryblock{
        int pntr;
        int history_size_tail;
        int history_size_front;
        long long viewtime[64][MAXFILES];
        long long pagetime[64];
        double voltpercm[64][MAXSIGNALS];
        double sensitivity[64][MAXSIGNALS][MAXSIGNALS];
        double screen_offset[64][MAXSIGNALS];
       };

struct annotationblock{
        int file_num;
        long long onset;
        char duration[16];
        char annotation[MAX_ANNOTATION_LEN + 1];
        struct annotationblock *former_annotation;
        struct annotationblock *next_annotation;
        int modified;
        int x_pos;
        int selected;
        int jump;
        int hided;
        int hided_in_list;
        unsigned int ident;
       };

struct active_markersblock{
        int file_num;
        struct annotationblock *list[MAX_ACTIVE_ANNOT_MARKERS];
        int count;
        int selected;
       };


struct graphicLineStruct{
        int x1;
        int y1;
        int x2;
        int y2;
        };

struct graphicBufStruct{
        struct graphicLineStruct graphicLine[MAXSIGNALS];
        };

struct crossHairStruct{
        int color;
        int file_num;
        int active;
        int moving;
        int position;
        int x_position;
        int y_position;
        int y_value;
        double value;
        long long time;
        long long time_relative;
       };

struct video_player_struct{
        int status;
        int cntdwn_timer;
        int poll_timer;
        long long utc_starttime;
        int starttime_diff;
        int stop_det_counter;
        int fpos;
       };

#endif



