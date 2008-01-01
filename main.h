/*
 *
 *  Hyperspectral compression program
 *
 * Name:		main.h	
 * Author:		Emmanuel Christophe	
 * Contact:		e.christophe at melaneum.com
 * Description:		Utility functions header for hyperspectral image compression
 * Version:		v1.1 - 2006-10	
 * 
 */


#ifndef HEADER
#define HEADER

#include <stdlib.h>
#include <stdio.h>
// #include <unistd.h> 
#include <math.h>

#ifdef TIME
#include <time.h>
#endif

#ifdef EZWREF
#define REFINEMENT
#endif

#ifdef EZWREFAFTER
#define REFINEMENT
#endif

#include "libQccPack.h"
#include "libQccPackVQ.h"
#include "libQccPackENT.h"

#include "signdigit.h"


#define NBITS 32

#define ENCODE	0
#define DECODE  1

long int nbitsread;
long int nbitsreadheader;
long int nbitswritten;
long int nbitswrittenheader;

#ifdef CHECKEND
long int * imageoriglobal;
#endif

#define ROUND(a) (((a)-floor(a))<0.5 ? floor(a):(floor(a)+1))

#define WEIGHTMULTVALUE 1000
// #define WEIGHTMULTVALUE 1

#ifdef TEMPWEIGHTCHECKING
long int * imageweight;
long int * imageweightcount;
#endif

//Warning, valable pour au pire une compression nulle sur 16 bit avec des arbres a 5 niveaux 
// et un point de depart de 2x2x2
// dans un second temps, verifier les depassements.
// size in bytes...
// #define SIZEBLOCKSTREAM 524288
long int sizeblockstream;

// 
typedef struct{
     short int x;
     short int y;
     short int l;
//      long int trans_pixel;
} pixel_struct;

typedef struct{
	pixel_struct spat;
	pixel_struct spec;
} parents_struct;

// typedef struct list_el;

typedef struct list_el_struct{
     struct list_el_struct *next;
     pixel_struct pixel;
     unsigned char type; /*0 type A and 1 type B*/
     unsigned char thres;//used later as an information concerning the threshold where it has been added
} list_el;


typedef struct{
      list_el *current;
      list_el *previous;
      list_el *first;
      list_el *last;
} list_struct;


typedef struct{
   int nsmax;
   int nlmax;
   int nbmax;
   int nsmin;
   int nlmin;
   int nbmin;
   int nres;
   int nresspec;
   int nresspat;
   int maxquant;
} imageprop_struct;

typedef struct{
	unsigned char * stream;
	long int * streamlast;
	unsigned char * count;
	int * headerlength;
} stream_struct;

typedef struct{
	int nblock;
	char * maxres;
// #ifdef RES_SCAL
	char * maxresspat;
	char * maxresspec;
// #endif
	char * maxquant;
	char * minquant;
	char * roi_filename;
        unsigned char flag_roi;
	char * filename;
	char * output_filename;
	int type;
	int nlayer;
	float rate;
	unsigned char * flag_meansub;
	unsigned char * flag_arith;
	unsigned char * flag_spiht;
        unsigned char * flag_wavelet_int;
} coder_param_struct;

// typedef struct{
// 	char * filename;
// 	char * output_filename;
// 	int type;
// 	int nlayer;
// 	float rate;
// 	unsigned char flag_meansub;
// 	unsigned char flag_arith;
// } coder_option_struct;


//declaration as global
//  imageprop_struct imageprop;
imageprop_struct imageprop;

int print_imageprop();
int print_coderparam(coder_param_struct coder_param);

static inline long int min(long int a, long int b){
	return (a<b)?a:b;
}

 list_el * el_init(pixel_struct pixel);
 list_struct * list_init(void);
void list_free( list_struct * list);
void list_flush( list_struct * list);

 list_el * first_el( list_struct * list);
 list_el * next_el( list_struct * list);
void insert_el( list_struct * list,  list_el * el);
void insert_el_inplace( list_struct * list,  list_el * el);
void insert_el_after(list_struct * list, list_el * el, list_el ** place);
 list_el * remove_current_el( list_struct * list);
int check_list(list_struct * list);
int count_list(list_struct * list);
int print_list(list_struct * list);

int check_map(unsigned char * map);

static inline long int trans_pixel(pixel_struct pixel){
   return pixel.x + (pixel.y + pixel.l*imageprop.nlmax)*imageprop.nsmax;
};

long int count_map(unsigned char * map, long int size);

unsigned char get_bit(long int value,int thres_ind);

int add_to_stream(unsigned char *stream, unsigned char *count, int input, long int *streamlast);

unsigned char read_from_stream(unsigned char * stream, unsigned char * count, long int *streamlast);

int isLSC(long int value_pix, int thres_ind);

static inline int isLSC2(long int value_abs_pix, int threshold){
	return ((value_abs_pix >= threshold) && (value_abs_pix < 2*threshold));
};

parents_struct find_parents(pixel_struct pixel);

int is_accessible_from(pixel_struct pixel,unsigned char * map);

#ifdef DEBUG
int check_accessibility_of_all(unsigned char * map_LSC, unsigned char * map_LIC, unsigned char * map_LIS);
#endif


int spat_spec_desc_spiht(pixel_struct pixel, list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

// int spec_desc_spiht(pixel_struct pixel, list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spec_desc_spiht(pixel_struct pixel, list_struct * list_desc, int directchildonly, long int *image, long int threshold, unsigned char *map_LSC);

// int spat_desc_spiht(pixel_struct pixel, list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spat_desc_spiht(pixel_struct pixel, list_struct * list_desc, int directchildonly, long int *image, long int threshold, unsigned char *map_LSC);

int spat_spec_desc_ezw(pixel_struct pixel, list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spec_desc_ezw(pixel_struct pixel, list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spat_desc_ezw(pixel_struct pixel, list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

// int spiht_code_c(long int *image, unsigned char *stream,long int * outputsize, int *maxquantvalue);
// int spiht_decode_c(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue);

int spiht_code_c(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param);
int spiht_decode_c(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param);

int ezw_code_c(long int *image, stream_struct streamstruct,long int * outputsize, coder_param_struct coder_param);

int ezw_decode_c(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param);

//signed digit representation
int check_zero(char * a, int t);
int add_bin(char * a, char * b, char * c);
int min_representation(char * a);
long int value_signed(char * a);
int bin_value(long int value, char * a);

int spat_spec_desc_ezw_signed(pixel_struct pixel, list_struct * list_desc, int directchildonly, char *image_signed, int thres_ind, unsigned char *map_LSC);
int spec_desc_ezw_signed(pixel_struct pixel, list_struct * list_desc, int directchildonly, char *image_signed, int thres_ind, unsigned char *map_LSC);
int spat_desc_ezw_signed(pixel_struct pixel, list_struct * list_desc, int directchildonly, char *image_signed, int thres_ind, unsigned char *map_LSC);

int ezw_code_signed_c(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param);
int ezw_decode_signed_c(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param);

int change_rep(char * a);

//random access
int spiht_code_ra(long int *image, unsigned char *stream,long int * outputsize, int *maxquantvalue);

int spiht_decode_ra(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue);

//random access2
// int spiht_code_ra2(long int *image, unsigned char *stream,long int * outputsize, int *maxquantvalue);
// 
// int spiht_decode_ra2(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue);



int spiht_code_ra2(long int *image, unsigned char *stream,long int * outputsize, coder_param_struct coder_param);
int spiht_decode_ra2(long int *image, unsigned char *stream, long int *outputsize, coder_param_struct coder_param);

int spiht_code_ra3(long int *image, unsigned char *stream,long int * outputsize, coder_param_struct coder_param);
int spiht_decode_ra3(long int *image, unsigned char *stream, long int *outputsize, coder_param_struct coder_param);

int spiht_code_ra5(long int *image, stream_struct streamstruct,long int * outputsize, coder_param_struct coder_param);
int spiht_decode_ra5(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param);

//rate allocation
#define NUMRD 10000
typedef struct{
	long long int r[NUMRD];
	long long int d[NUMRD];
	long long int reval[NUMRD];
	int ptcourant;
	float cost_j[NUMRD];
} rddata_struct;

// long long int eval_dist_grp(int iloc, int jloc,int kloc, long int *image, int thres_ind);

int update_dist(pixel_struct pixel, int thres_ind, long long int * dist, long int *image);
int update_dist_first(pixel_struct pixel, int thres_ind, long long int * dist,  long int *image);
int update_dist1(pixel_struct pixel, int thres_ind, long long int * dist,  long int *image);
int update_dist0(pixel_struct pixel, int thres_ind, long long int * dist,  long int *image);
int compute_weightingFactor(pixel_struct pixel);

int add_to_rddata(rddata_struct *rddata, long long int rate, long long int dist);

int compute_cost(rddata_struct *rddata, float lambda);

int spat_spec_desc_spiht_cumul(pixel_struct pixel, long int *image, int thres_ind, long long int * dist);
int spec_desc_spiht_cumul(pixel_struct pixel, list_struct * list_desc, int directchildonly, long int *image, int thres_ind, long long int * dist);
int spat_desc_spiht_cumul(pixel_struct pixel, list_struct * list_desc, int directchildonly, long int *image, int thres_ind, long long int * dist);


typedef struct{
	unsigned char * stream;
	long int * streamlast;
	unsigned char * count;
	long int * partsize;
#ifdef RES_RATE
	rddata_struct rddata[NRES];
#else
	rddata_struct rddata;
#endif
	long int currentpos;

} datablock_struct;





int datablock_init(datablock_struct *datablock);

int datablock_free(datablock_struct *datablock);

//codage des longueurs sur 24 bits (une bonne marge)
#define NUMBITSPARTSIZE 24
int add_to_stream_number(unsigned long int number, unsigned char * stream, unsigned char *count, long int *streamlast, int numbits);
unsigned long int read_from_stream_number(unsigned char * stream, unsigned char *count, long int *streamlast, int numbits);

int copy_to_stream(long int currentpos, long long int rate, unsigned char * streamin, unsigned char * streamout, unsigned char * count, long int * streamlast, long int insize);

float compute_lambda(datablock_struct *datablock, long int rate, int nblock);

int interleavingblocks(datablock_struct *datablock, int nblock, unsigned char * stream, unsigned char * count, long int * streamlast, float lambda, int * flagfirst);

int desinterleavingblocks(datablock_struct *datablock, int nblock, stream_struct streamstruct, long int  insize, int nlayer);

int sizeChecking();
void usage(char *str1);


int preinit_coder_param(coder_param_struct * coder_param);
int init_coder_param(coder_param_struct * coder_param, int nblock);
int free_coder_param(coder_param_struct * coder_param);

int encode(coder_param_struct coder_param);
int decode(coder_param_struct coder_param);

long int file_size(FILE *f);

long int find_max(long int * image, long int npix);

int compute_mean(long int * image, long int *mean);
int add_mean(long int * image, long int *mean);
int substract_mean(long int * image, long int *mean);

int waveletDWT(long int * imagein, long int * imageout, int specdec, int spatdec, coder_param_struct coder_param);
int waveletIDWT(long int * imagein, long int * imageout, int specdec, int spatdec, coder_param_struct coder_param);



long int count_zero(char * image);
int output_rd(datablock_struct *datablock, int nblock);

long int * read_hyper(char * filename, long int npix, int type);
int write_hyper(char * filename, long int * image, long int npix, int type);


int compare_hyper_long(long int * imagedeg, long int * imageref);
int compare_hyper_short(short int * imagedeg, short int * imageref);

int write_magic(stream_struct streamstruct, coder_param_struct coder_param);
int read_magic(stream_struct streamstruct, coder_param_struct coder_param);
int write_header(stream_struct streamstruct);
int read_header(stream_struct streamstruct);
int write_header_mean(stream_struct streamstruct,long int * mean);
int read_header_mean(stream_struct streamstruct,long int * mean);

int read_roi_spec(coder_param_struct coder_param);

pixel_struct init_pixel();

int QccENTArithmeticResetModel(QccENTArithmeticModel *model,
                             int context);
#endif

