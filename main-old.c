/*
 *
 *  Hyperspectral compression program
 *
 * Name:		main.c	
 * Author:		Emmanuel Christophe	
 * Contact:		e.christophe at melaneum.com
 * Description:		Utility functions for hyperspectral image compression
 * Version:		v1.0 - 2006-02	
 * 
 */


//TODO
// Si on veux une gestion pour des stream de longueur > 4Gb, passer les rate, currentpos et autres en long long int (64bits)

#include "main.h"

// #include <stdlib.h>
// #include <stdio.h>
// #include <unistd.h> 
//unistd.h for using sleep function

// #ifdef S64
// #define NSMAX_CONST 64
// #define NLMAX_CONST 64
// #define NBMAX_CONST 56
// #define NSMIN_CONST 8
// #define NLMIN_CONST 8
// #define NBMIN_CONST 7
// #define MAXQUANT_CONST 16
// #endif

// #define NSMAX_CONST 8
// #define NLMAX_CONST 8
// #define NBMAX_CONST 1
// #define NSMIN_CONST 2
// #define NLMIN_CONST 2
// #define NBMIN_CONST 1
// #define MAXQUANT_CONST 5

// #ifdef S256
// #define NSMAX_CONST 256
// #define NLMAX_CONST 256
// #define NBMAX_CONST 224
// #define NSMIN_CONST 8
// #define NLMIN_CONST 8
// #define NBMIN_CONST 7
// #define MAXQUANT_CONST 19
// #endif
// #define NSMIN_CONST 32
// #define NLMIN_CONST 32
// #define NBMIN_CONST 28
// #define MAXQUANT_CONST 16
// #define MAXQUANT_CONST 0
//Be careful and check if MAXQUANT_CONST is enough

// int main(void) /*Uniquement dans un but de test...*/
// {
// int i;
// long int image_data[100];
// long int *image = &image_data[0];
// char stream_data[100];
// char *stream = &stream_data[0];
// long int *outputsize;
// struct list_struct *LSP;
// struct list_el *current_el;
// struct list_el *list_el[10];
// struct pixel_struct pixel;
// 
// outputsize = (long int *) malloc( sizeof( long int ));
// LSP = list_init();
// 
// for (i=0;i<10;i++){
//         pixel.x = i;
// 	pixel.y = i;
// 	pixel.l = i;
// 	list_el[i] = el_init(pixel);
// };	
// 
// /*printf("current_el->pixel %ld\n",current_el->pixel);*/
// 
// for (i=0;i<10;i++){
//    insert_el(LSP,list_el[i]);
// };   
// 
// /*
// *outputsize = 0;
// spiht_code_c(image,stream, outputsize);*/
// 
// current_el = LSP->first;
// for (i=0;i<10;i++){
//    printf("current_el->pixel : %d \n",current_el->pixel.x);
//    current_el=current_el->next;
// };
// 
// LSP->previous = LSP->first->next->next;
// LSP->current = LSP->first->next->next->next;
// remove_current_el(LSP);
// 
// current_el = LSP->first;
// i=0;
// while (current_el != NULL){
//    printf("current_el->pixel : %d \n",current_el->pixel.x);
//    current_el=current_el->next;
//    i++;
// };
// 
// return 0;
// };


int main(void) {

struct pixel_struct pixel;
struct parents_struct parents;
// struct list_struct * list_desc=NULL;
struct imageprop_struct imageprop;
// int directchildonly=1;
long int *image;
// short int *image;
long int *imageout;
long int *outputsize;
unsigned char * stream;
// long int threshold=0;
long int i_l;
long int npix;
// unsigned char output;
int err=0;

long long int dist=0;

unsigned char *count;
 long int *streamlast;

// int i,r;
// struct list_struct *LSP;
// struct list_el *el;
// struct pixel pix;
// long int value=123456;
//00000000 00000001 11100010 01000000
// unsigned char testbit;
// int thres_ind;

int status;
FILE *data_file;
FILE *output_file;

long int maxerr=0;

#ifdef TIME
clock_t start, end;
double elapsedcomp, elapseddecomp;
#endif

// struct list_el * current_el=NULL;

unsigned char *map = (unsigned char *) calloc(NSMAX_CONST*NLMAX_CONST*NBMAX_CONST,sizeof(unsigned char));
// int * zero_map = (int *) malloc(NSMAX_CONST*NLMAX_CONST*NBMAX_CONST*sizeof(int));

image = (long int *) malloc(NSMAX_CONST*NLMAX_CONST*NBMAX_CONST*sizeof(long int));
// image = (short int *) malloc(NSMAX_CONST*NLMAX_CONST*NBMAX_CONST*sizeof(short int));//MODIF
// imageout = (long int *) malloc(NSMAX_CONST*NLMAX_CONST*NBMAX_CONST*sizeof(long int));
outputsize = (long int *) calloc((MAXQUANT_CONST+1), sizeof(long int));
// *outputsize=0;
stream = (unsigned char *) calloc(NSMAX_CONST*NLMAX_CONST*NBMAX_CONST*2,sizeof(unsigned char));//on prend une marge... a faire en finesse plus tard...
count=(unsigned char *) malloc(sizeof(unsigned char *));
streamlast=(long int *) malloc(sizeof(long int *));
// list_desc = list_init();
imageprop.nsmax=NSMAX_CONST;
imageprop.nlmax=NLMAX_CONST;
imageprop.nbmax=NBMAX_CONST;
// imageprop.nsmin=8;
// imageprop.nlmin=8;
// imageprop.nbmin=7;
imageprop.nsmin=NSMIN_CONST;
imageprop.nlmin=NLMIN_CONST;
imageprop.nbmin=NBMIN_CONST;
int *maxquantvalue=(int*) malloc(sizeof(int));

*maxquantvalue=MAXQUANT_CONST;

//test parents
// pixel.x=1;
// pixel.y=61;
// pixel.l=15;
// parents=find_parents(pixel, imageprop);
// for (i_l=0;i_l<imageprop.nsmax*imageprop.nlmax*imageprop.nbmax;i_l++)
// {
// 	map[i_l]=0;
// }
// is_accessible_from(pixel, imageprop, map);
// printf("Pause ici");

//fin test parents


//test int 64 bits
// printf("long long int: %lld\n", dist);
// dist= 1 << 30;
// printf("long long int: %lld\n", dist);
// dist = dist * (1<<30);
// printf("long long int: %lld\n", dist);

npix=imageprop.nsmax*imageprop.nlmax*imageprop.nbmax;
for (i_l=0;i_l<npix;i_l++){
	image[i_l]=0;
// 	imageout[i_l]=0;
// 	zero_map[i_l]=0;
};
for (i_l=0;i_l<npix*2;i_l++){
	stream[i_l]=0;
};


//test
// LSP=list_init();
// for (i=0;i<10;i++){
// // 	pix={i,i,i};
// 	el=el_init((struct pixel_struct) {i,i,i});
// 	insert_el(LSP,el);
// 	printf("%d at address %ud\n",i,el);
// };
// 
// LSP->current = LSP->first;
// 
// r=print_list(LSP);
// r=check_list(LSP);
// printf("check list %d\n",r);
// el=remove_current_el(LSP);//on enleve le 0  et on va au 1
// printf("0 at address %ud\n",el);
// r=print_list(LSP);
// next_el(LSP);//va au 2
// next_el(LSP);//va au 3abs(value)>>thres_ind
// el=remove_current_el(LSP);//enleve le 3 et va au 4
// printf("3 at address %ud\n",el);
// r=print_list(LSP);
// r=check_list(LSP);
// printf("check list %d\n",r);
// next_el(LSP);//5
// next_el(LSP);
// next_el(LSP);
// next_el(LSP);
// next_el(LSP);//9
// el=remove_current_el(LSP);//enleve le 9 et va au NULL
// printf("9 at address %ud\n",el);
// r=print_list(LSP);
// r=check_list(LSP);
// printf("check list %d\n",r);
// 
// 
// printf("fin test\n");

//fin test

//test2
// for (thres_ind=0;i<20;i++){
// 	testbit=get_bit(value,i);
// 	printf("(+)%d : %uc\n",i,testbit);
// 	testbit=get_bit(-value,i);
// 	printf("(-)%d : %uc\n",i,testbit);	
// };


// printf("debut test\n");
// *count=0;
// *streamlast=0;
// printf("Input\n");
// for (i=0;i<20;i++){
// testbit=get_bit(value,i);
// printf(" %uc \n",testbit);
// add_to_stream(stream,count, testbit, streamlast);
// };
// 
// printf("Output\n");
// *count=0;
// *streamlast=0;
// for (i=0;i<20;i++){
// output=read_from_stream(stream, count, streamlast);
// printf(" %uc \n",output);
// };
// printf("fin test\n");
// printf("\n");


//fin test2


// pixel.x=1;
// pixel.y=1;
// pixel.l=1;
// spat_desc_spiht(pixel, list_desc, imageprop, directchildonly, image, threshold);
// spec_desc_spiht(pixel, list_desc, imageprop, directchildonly, image, threshold);
// spat_spec_desc_spiht(pixel, list_desc, imageprop, directchildonly, image, threshold);

// printf("Print descendant list\n");
// current_el = list_desc->first;
// while (current_el != NULL){
//    printf("current_el->pixel : %d, %d, %d \n",current_el->pixel.x,current_el->pixel.y,current_el->pixel.l);
//    current_el=current_el->next;
// };




// sleep(5);
#ifdef S64
data_file = fopen("moff-64-dwt","r");
// #endif
// data_file = fopen("shapirodata","r");
// #else
#endif
#ifdef S256
data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett3-ani-lsb.img","r");
// data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett3-ani-lsb-4l.img","r");//WARNING, also modify NSMIN et cie
// data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett3-ani-lsb-haar.img","r");
#endif
#ifdef S2D
data_file = fopen("barb-dya.img","r");
#endif

status = fread(image, 4, npix, data_file);
// data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett3-lsb.img","r");
// status = fread(image, 2, npix, data_file);

status = fclose(data_file);

//test evaluation de distortion  //coherent with IDL for 0,0,0
// dist = eval_dist_grp(0, 0, 0, image, imageprop, 20);

printf("image[0]: %ld\n",image[0]);

#ifdef TIME
start = clock();
#endif

#ifndef NOENC
#ifdef EZW
// ezw_code_c(image, stream, outputsize, maxquantvalue);
ezw_code_signed_c(image, stream, outputsize, maxquantvalue);
#else
// spiht_code_c(image, stream, outputsize, maxquantvalue);
spiht_code_ra2(image, stream, outputsize, maxquantvalue);
#endif
// waveletDWT(image, imageout);
// spiht_code_c(image, stream, outputsize, maxquantvalue);

#ifdef TIME
end = clock();
elapsedcomp = ((double) (end - start)) / CLOCKS_PER_SEC;
printf("Compression time: %f \n", elapsedcomp);
#endif

// free(image);

printf("Outputsize: %ld \n", *outputsize);
#endif


//output of coded stream
#ifndef NOENC
output_file = fopen("/home/christop/Boulot/images/output_stream/moffett3-ani-spiht.dat","w");
// status = fwrite(stream, 1, *outputsize, output_file);//chgt 14-02-06 a verifier sur le spiht standard outputsize en bit ou byte ???
status = fwrite(stream, 1, (*outputsize+7)/8, output_file);
status = fclose(output_file);
#endif

//put stream to 0
for (i_l=0;i_l<npix*2;i_l++){
	stream[i_l]=0;
};

#ifdef NOENC
// *outputsize = 24529191;
*outputsize = 89525721;
#endif
//Read from file
output_file = fopen("/home/christop/Boulot/images/output_stream/moffett3-ani-spiht.dat","r");
// *outputsize=*outputsize-2; //on enleve 2 octets pour voir...
// *outputsize = 1069354-1;
// *outputsize =33347743-1;
status = fread(stream, 1, (*outputsize+7)/8, output_file);//same as for writing...
status = fclose(output_file);



imageout = (long int *) calloc(NSMAX_CONST*NLMAX_CONST*NBMAX_CONST, sizeof(long int));

for (i_l=0;i_l<npix;i_l++){
// 	image[i_l]=0;
	imageout[i_l]=0;
// 	zero_map[i_l]=0;
};

printf("Decodage...\n");

#ifdef TIME
start = clock();
#endif


//decode
#ifdef EZW
// ezw_decode_c(imageout, stream, outputsize, maxquantvalue);
ezw_decode_signed_c(imageout, stream, outputsize, maxquantvalue);
#else
// spiht_decode_c(imageout, stream, outputsize, maxquantvalue);
spiht_decode_ra2(imageout, stream, outputsize, maxquantvalue);
#endif
printf("Fin decodage\n");

#ifdef TIME
end = clock();
elapseddecomp = ((double) (end - start)) / CLOCKS_PER_SEC;
printf("Compression time: %f \n", elapsedcomp);
printf("Decompression time: %f \n", elapseddecomp);
#endif

// sleep(10);

//save output
output_file = fopen("/home/christop/Boulot/images/output_stream/moffett3-ani-spiht-out.img","w");
// status = fwrite(stream, 1, *outputsize, output_file);//chgt 14-02-06 a verifier sur le spiht standard outputsize en bit ou byte ???
status = fwrite(imageout, 4, npix, output_file);
status = fclose(output_file);


err=0;
for (i_l=0;i_l<npix;i_l++){
	if (abs(image[i_l]-imageout[i_l]) > maxerr){
		maxerr=abs(image[i_l]-imageout[i_l]);
		err=1;
	};
};
if (err) {
fprintf(stderr, "ERREUR MAX %ld\n",maxerr);
} else {
fprintf(stderr, "Decoding OK\n");
}
return 0;
};

