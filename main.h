/*
 *
 *  Hyperspectral compression program
 *
 * Name:		main.h	
 * Author:		Emmanuel Christophe	
 * Contact:		e.christophe at melaneum.com
 * Description:		Utility functions header for hyperspectral image compression
 * Version:		v1.0 - 2006-02	
 * 
 */


#ifndef HEADER
#define HEADER

#include <stdlib.h>
#include <stdio.h>
// #include <unistd.h> 
#include "math.h"

#include "signdigit.h"

#ifdef S64
#define NSMAX_CONST 64
#define NLMAX_CONST 64
#define NBMAX_CONST 56
#define NSMIN_CONST 8
#define NLMIN_CONST 8
#define NBMIN_CONST 7
#define MAXQUANT_CONST 16

#else

// #ifdef S256
#define NSMAX_CONST 256
#define NLMAX_CONST 256
#define NBMAX_CONST 224
#define NSMIN_CONST 8
#define NLMIN_CONST 8
#define NBMIN_CONST 7
#define MAXQUANT_CONST 19

// #define NSMIN_CONST 16
// #define NLMIN_CONST 16
// #define NBMIN_CONST 14
// #define MAXQUANT_CONST 18

//for signed representation
// #define MAXQUANT_CONST 20 
#endif

#define NBITS 32
// #define MIN(X,Y) ((X) < (Y) ? : (X) : (Y))

struct pixel_struct{
     short int x;
     short int y;
     short int l;
};

struct parents_struct{
	struct pixel_struct spat;
	struct pixel_struct spec;
};

struct list_el{
     struct list_el *next;
     struct pixel_struct pixel;
     unsigned char type; /*0 type A and 1 type B*/
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
};


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

int check_map(unsigned char * map, struct imageprop_struct imageprop);

long int trans_pixel(struct pixel_struct pixel, struct imageprop_struct imageprop);

long int count_map(unsigned char * map, long int size);

unsigned char get_bit(long int value,int thres_ind);

void add_to_stream(unsigned char *stream, unsigned char *count, int input, long int *streamlast);

unsigned char read_from_stream(unsigned char * stream, unsigned char * count, long int *streamlast);

int isLSC(long int value_pix, int thres_ind);

struct parents_struct find_parents(struct pixel_struct pixel, struct imageprop_struct imageprop);

int is_accessible_from(struct pixel_struct pixel, struct imageprop_struct imageprop, unsigned char * map);

#ifdef DEBUG
int check_accessibility_of_all(struct imageprop_struct imageprop, unsigned char * map_LSC, unsigned char * map_LIC, unsigned char * map_LIS);
#endif


int spat_spec_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spec_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spat_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spat_spec_desc_ezw(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spec_desc_ezw(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spat_desc_ezw(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spiht_code_c(long int *image, unsigned char *stream,long int * outputsize, int *maxquantvalue);

int spiht_decode_c(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue);

int ezw_code_c(long int *image, unsigned char *stream,long int * outputsize, int *maxquantvalue);

int ezw_decode_c(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue);

//signed digit representation
int check_zero(char * a, int t);
int add_bin(char * a, char * b, char * c);
int min_representation(char * a);
long int value_signed(char * a);
int bin_value(long int value, char * a);

int spat_spec_desc_ezw_signed(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, char *image_signed, int thres_ind);
int spec_desc_ezw_signed(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, char *image_signed, int thres_ind);
int spat_desc_ezw_signed(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, char *image_signed, int thres_ind);

int ezw_code_signed_c(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue);
int ezw_decode_signed_c(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue);


//random access
int spiht_code_ra(long int *image, unsigned char *stream,long int * outputsize, int *maxquantvalue);

int spiht_decode_ra(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue);


//rate allocation
#define NUMRD 100
struct rddata_struct{
	long long int r[NUMRD];
	long long int d[NUMRD];
	long long int reval[NUMRD];
	int ptcourant;
	float cost_j[NUMRD];
};

long long int eval_dist_grp(int iloc, int jloc,int kloc, long int *image, struct imageprop_struct imageprop, int thres_ind);

int update_dist(struct pixel_struct pixel, int thres_ind, long long int * dist, long int *image, struct imageprop_struct imageprop);

int add_to_rddata(struct rddata_struct *rddata, long long int rate, long long int dist);

int compute_cost(struct rddata_struct *rddata, float lambda);

int spat_spec_desc_spiht_cumul(struct pixel_struct pixel, struct imageprop_struct imageprop, long int *image, int thres_ind, long long int * dist);
int spec_desc_spiht_cumul(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, long long int * dist);
int spat_desc_spiht_cumul(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, long long int * dist);

struct datablock_struct{
	unsigned char * stream;
	long int * streamlast;
	unsigned char * count;
	struct rddata_struct rddata;
	long int currentpos;
};

//Warning, valable pour au pire une compression nulle sur 16 bit avec des arbres a 5 niveaux
// dans un second temps, verifier les depassements.
// size in bytes...
#define SIZEBLOCKSTREAM 524288

int datablock_init(struct datablock_struct *datablock);

int datablock_free(struct datablock_struct *datablock);

//codage des longueurs sur 24 bits (une bonne marge)
#define NUMBITSPARTSIZE 24
int add_to_stream_number(unsigned long int number, unsigned char * stream, unsigned char *count, long int *streamlast);
unsigned long int read_from_stream_number(unsigned char * stream, unsigned char *count, long int *streamlast);

int copy_to_stream(long int currentpos, long long int rate, unsigned char * streamin, unsigned char * streamout, unsigned char * count, long int * streamlast, long int insize);

int interleavingblocks(struct datablock_struct *datablock, int nblock, unsigned char * stream, unsigned char * count, long int * streamlast, float lambda);

int desinterleavingblocks(struct datablock_struct *datablock, int nblock, unsigned char * stream, long int  insize);

#endif

