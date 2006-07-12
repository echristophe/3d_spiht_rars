/*
 *
 *  Hyperspectral compression program
 *
 * Name:		main.h	
 * Author:		Emmanuel Christophe	
 * Contact:		e.christophe at melaneum.com
 * Description:		Utility functions header for hyperspectral image compression
 * Version:		v1.0 - 2006-04	
 * 
 */


#ifndef HEADER
#define HEADER

#include <stdlib.h>
#include <stdio.h>
// #include <unistd.h> 
#include "math.h"

#ifdef TIME
#include <time.h>
#endif

#include "signdigit.h"

// #if !defined(S64) && !defined(S2D) && !defined(S512)
// #define S256
// #endif

// #ifdef S64
// #define NSMAX_CONST 64
// #define NLMAX_CONST 64
// #define NBMAX_CONST 56
// #define NSMIN_CONST 8
// #define NLMIN_CONST 8
// #define NBMIN_CONST 7
// #define MAXQUANT_CONST 16
// #endif
// 
// #ifdef S256
// #define NSMAX_CONST 256
// #define NLMAX_CONST 256
// #define NBMAX_CONST 224
// #define NSMIN_CONST 8
// #define NLMIN_CONST 8
// #define NBMIN_CONST 7
// #define MAXQUANT_CONST 19
// #endif
// 
// #ifdef S2D
// #define NSMAX_CONST 512
// #define NLMAX_CONST 512
// #define NBMAX_CONST 1
// #define NSMIN_CONST 16
// #define NLMIN_CONST 16
// #define NBMIN_CONST 1
// #define MAXQUANT_CONST 12
// #endif
// 
// #ifdef S512
// #define NSMAX_CONST 512
// #define NLMAX_CONST 512
// #define NBMAX_CONST 224
// #define NSMIN_CONST 16
// #define NLMIN_CONST 16
// #define NBMIN_CONST 7
// #define MAXQUANT_CONST 20
// #endif
// 
// 
// #ifdef S64
// #define NRES 16
// #define NRESSPEC 4
// #define NRESSPAT 4
// #else
// #define NRES 36
// #define NRESSPEC 6
// #define NRESSPAT 6
// #endif



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

//Warning, valable pour au pire une compression nulle sur 16 bit avec des arbres a 5 niveaux 
// et un point de depart de 2x2x2
// dans un second temps, verifier les depassements.
// size in bytes...
// #define SIZEBLOCKSTREAM 524288
long int sizeblockstream;

// 
struct pixel_struct{
     short int x;
     short int y;
     short int l;
//      long int trans_pixel;
};

struct parents_struct{
	struct pixel_struct spat;
	struct pixel_struct spec;
};

struct list_el{
     struct list_el *next;
     struct pixel_struct pixel;
     unsigned char type; /*0 type A and 1 type B*/
     unsigned char thres;
	//used later as an information concerning the threshold where it has been added
};


struct list_struct{
     struct list_el *current;
     struct list_el *previous;
     struct list_el *first;
     struct list_el *last;
};


struct imageprop_struct{
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
};

struct stream_struct{
	unsigned char * stream;
	long int * streamlast;
	unsigned char * count;
};

struct coder_param_struct{
	int nblock;
	char * maxres;
#ifdef RES_SCAL
	char * maxresspat;
	char * maxresspec;
#endif
	char * maxquant;
	char * minquant;
	int nlayer;
	float rate;
};

//declaration as global
struct imageprop_struct imageprop;

void print_imageprop();

static inline min(long int a, long int b){
	return (a<b)?a:b;
}

struct list_el * el_init(struct pixel_struct pixel);
struct list_struct * list_init(void);
void list_free(struct list_struct * list);
void list_flush(struct list_struct * list);

struct list_el * first_el(struct list_struct * list);
struct list_el * next_el(struct list_struct * list);
void insert_el(struct list_struct * list, struct list_el * el);
void insert_el_inplace(struct list_struct * list, struct list_el * el);
struct list_el * remove_current_el(struct list_struct * list);
int check_list(struct list_struct * list);
int count_list(struct list_struct * list);
int print_list(struct list_struct * list);

int check_map(unsigned char * map);

static inline long int trans_pixel(struct pixel_struct pixel){
   return pixel.x + (pixel.y + pixel.l*imageprop.nlmax)*imageprop.nsmax;
};

long int count_map(unsigned char * map, long int size);

unsigned char get_bit(long int value,int thres_ind);

void add_to_stream(unsigned char *stream, unsigned char *count, int input, long int *streamlast);

unsigned char read_from_stream(unsigned char * stream, unsigned char * count, long int *streamlast);

int isLSC(long int value_pix, int thres_ind);

static inline int isLSC2(long int value_abs_pix, int threshold){
	return ((value_abs_pix >= threshold) && (value_abs_pix < 2*threshold));
};

struct parents_struct find_parents(struct pixel_struct pixel);

int is_accessible_from(struct pixel_struct pixel,unsigned char * map);

#ifdef DEBUG
int check_accessibility_of_all(unsigned char * map_LSC, unsigned char * map_LIC, unsigned char * map_LIS);
#endif


int spat_spec_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

// int spec_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spec_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, long int threshold, unsigned char *map_LSC);

// int spat_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spat_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, long int threshold, unsigned char *map_LSC);

int spat_spec_desc_ezw(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spec_desc_ezw(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spat_desc_ezw(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

// int spiht_code_c(long int *image, unsigned char *stream,long int * outputsize, int *maxquantvalue);
// int spiht_decode_c(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue);

int spiht_code_c(long int *image, struct stream_struct streamstruct, long int *outputsize, struct coder_param_struct coder_param);
int spiht_decode_c(long int *image, struct stream_struct streamstruct, long int *outputsize, struct coder_param_struct coder_param);

int ezw_code_c(long int *image, struct stream_struct streamstruct,long int * outputsize, int maxquantvalue);

int ezw_decode_c(long int *image, struct stream_struct streamstruct, long int *outputsize, int maxquantvalue);

//signed digit representation
int check_zero(char * a, int t);
int add_bin(char * a, char * b, char * c);
int min_representation(char * a);
long int value_signed(char * a);
int bin_value(long int value, char * a);

int spat_spec_desc_ezw_signed(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, char *image_signed, int thres_ind, unsigned char *map_LSC);
int spec_desc_ezw_signed(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, char *image_signed, int thres_ind, unsigned char *map_LSC);
int spat_desc_ezw_signed(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, char *image_signed, int thres_ind, unsigned char *map_LSC);

int ezw_code_signed_c(long int *image, struct stream_struct streamstruct, long int *outputsize, int maxquantvalue);
int ezw_decode_signed_c(long int *image, struct stream_struct streamstruct, long int *outputsize, int maxquantvalue);

int change_rep(char * a);

//random access
int spiht_code_ra(long int *image, unsigned char *stream,long int * outputsize, int *maxquantvalue);

int spiht_decode_ra(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue);

//random access2
// int spiht_code_ra2(long int *image, unsigned char *stream,long int * outputsize, int *maxquantvalue);
// 
// int spiht_decode_ra2(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue);



int spiht_code_ra2(long int *image, unsigned char *stream,long int * outputsize, struct coder_param_struct coder_param);
int spiht_decode_ra2(long int *image, unsigned char *stream, long int *outputsize, struct coder_param_struct coder_param);

int spiht_code_ra3(long int *image, unsigned char *stream,long int * outputsize, struct coder_param_struct coder_param);
int spiht_decode_ra3(long int *image, unsigned char *stream, long int *outputsize, struct coder_param_struct coder_param);

int spiht_code_ra5(long int *image, struct stream_struct streamstruct,long int * outputsize, struct coder_param_struct coder_param);
int spiht_decode_ra5(long int *image, struct stream_struct streamstruct, long int *outputsize, struct coder_param_struct coder_param);

//rate allocation
#define NUMRD 1000
struct rddata_struct{
	long long int r[NUMRD];
	long long int d[NUMRD];
	long long int reval[NUMRD];
	int ptcourant;
	float cost_j[NUMRD];
};

// long long int eval_dist_grp(int iloc, int jloc,int kloc, long int *image, int thres_ind);

int update_dist(struct pixel_struct pixel, int thres_ind, long long int * dist, long int *image);

int add_to_rddata(struct rddata_struct *rddata, long long int rate, long long int dist);

int compute_cost(struct rddata_struct *rddata, float lambda);

int spat_spec_desc_spiht_cumul(struct pixel_struct pixel, long int *image, int thres_ind, long long int * dist);
int spec_desc_spiht_cumul(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, int thres_ind, long long int * dist);
int spat_desc_spiht_cumul(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, int thres_ind, long long int * dist);


struct datablock_struct{
	unsigned char * stream;
	long int * streamlast;
	unsigned char * count;
	long int * partsize;
#ifdef RES_RATE
	struct rddata_struct rddata[NRES];
#else
	struct rddata_struct rddata;
#endif
	long int currentpos;

};





int datablock_init(struct datablock_struct *datablock);

int datablock_free(struct datablock_struct *datablock);

//codage des longueurs sur 24 bits (une bonne marge)
#define NUMBITSPARTSIZE 24
int add_to_stream_number(unsigned long int number, unsigned char * stream, unsigned char *count, long int *streamlast, int numbits);
unsigned long int read_from_stream_number(unsigned char * stream, unsigned char *count, long int *streamlast, int numbits);

int copy_to_stream(long int currentpos, long long int rate, unsigned char * streamin, unsigned char * streamout, unsigned char * count, long int * streamlast, long int insize);

float compute_lambda(struct datablock_struct *datablock, long int rate, int nblock);

int interleavingblocks(struct datablock_struct *datablock, int nblock, unsigned char * stream, unsigned char * count, long int * streamlast, float lambda, int * flagfirst);

int desinterleavingblocks(struct datablock_struct *datablock, int nblock, struct stream_struct streamstruct, long int  insize, int nlayer);

void usage(char *str1);



int init_coder_param(struct coder_param_struct * coder_param, int nblock);
int free_coder_param(struct coder_param_struct * coder_param);

int encode(char * filename, char * output_filename, int type, float rate);
int decode(char * filename, char * output_filename, int type);

long int file_size(FILE *f);

long int find_max(long int * image, long int npix);

int compute_mean(long int * image, long int *mean);
int substract_mean(long int * image, long int *mean);

int waveletDWT(long int * imagein, long int * imageout, int specdec, int spatdec);
int waveletIDWT(long int * imagein, long int * imageout, int specdec, int spatdec);



long int count_zero(char * image);
int output_rd(struct datablock_struct *datablock, int nblock);

#endif

