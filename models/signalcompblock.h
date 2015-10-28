#ifndef SIGNALCOMPBLOCK_H
#define SIGNALCOMPBLOCK_H

#include "global.h"

struct signalcompblock{
        int filenum;
        struct edfhdrblock *edfhdr;
        int num_of_signals;
        int viewbufsize;
        int viewbufoffset;
        long long records_in_viewbuf;
        long long samples_in_viewbuf;
        long long samples_on_screen;
        long long sample_start;
        long long sample_stop;
        int timeoffset;
        int sample_timeoffset;
        int pixels_shift;
        double sample_timeoffset_part;
        double sample_pixel_ratio;
        int edfsignal[MAXSIGNALS];
        int factor[MAXSIGNALS];
        int polarity;
        double sensitivity[MAXSIGNALS];
        int oldvalue;
        long long oldsmplnr;
        long long file_duration;
        char signallabel[512];
        char alias[17];
        int signallabellen;
        char signallabel_bu[512];
        int signallabellen_bu;
        int hascursor1;
        int hascursor2;
        int hasoffsettracking;
        int hasgaintracking;
        int hasruler;
        double screen_offset;
        double voltpercm;
        char physdimension[9];
        char physdimension_bu[9];
        int color;
        int filter_cnt;
        int samples_in_prefilterbuf;
        long long prefilter_starttime;
        int prefilter_reset_sample;
        double filterpreset_a[MAXFILTERS];
        double filterpreset_b[MAXFILTERS];
        struct filter_settings *filter[MAXFILTERS];
        int max_dig_value;
        int min_dig_value;
        int fidfilter_cnt;
        int fidfilter_type[MAXFILTERS];
        double fidfilter_freq[MAXFILTERS];
        double fidfilter_freq2[MAXFILTERS];
        double fidfilter_ripple[MAXFILTERS];
        int fidfilter_order[MAXFILTERS];
        int fidfilter_model[MAXFILTERS];
        FidFilter *fidfilter[MAXFILTERS];
        FidRun *fid_run[MAXFILTERS];
        FidFunc *fidfuncp[MAXFILTERS];
        void *fidbuf[MAXFILTERS];
        void *fidbuf2[MAXFILTERS];
        int fidfilter_setup[MAXFILTERS];
        int stat_cnt;
        int stat_zero_crossing_cnt;
        double stat_sum;
        double stat_sum_sqr;
        double stat_sum_rectified;
        int ravg_filter_cnt;
        int ravg_filter_type[MAXFILTERS];
        int ravg_filter_size[MAXFILTERS];
        int ravg_filter_setup[MAXFILTERS];
        struct ravg_filter_settings *ravg_filter[MAXFILTERS];
        struct ecg_filter_settings *ecg_filter;
        double spike_filter_velocity;
        int spike_filter_holdoff;
        struct spike_filter_settings *spike_filter;
        struct zratio_filter_settings *zratio_filter;
        double zratio_crossoverfreq;
        int spectr_dialog[MAXSPECTRUMDIALOGS];
        int avg_dialog[MAXAVERAGECURVEDIALOGS];
        int zscoredialog[MAXZSCOREDIALOGS];
      };

#endif // SIGNALCOMPBLOCK_H

