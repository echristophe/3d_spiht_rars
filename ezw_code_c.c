/*
 *
 *  Hyperspectral compression program
 *
 * Name:		main.c	
 * Author:		Emmanuel Christophe	
 * Contact:		e.christophe at melaneum.com
 * Description:		Utility functions for hyperspectral image compression
 * Version:		v1.0 - 2006-04	
 * 
 */


// #include "spiht_code_c.h"
// #include "ezw_code_c.h"
// #include "desc_ezw.h"
// #include <stdlib.h>
// #include <stdio.h>
// #include <unistd.h> 
#include "main.h"

// int ezw_code_c(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue){
int ezw_code_c(long int *image, struct stream_struct streamstruct, long int *outputsize, int maxquantvalue){

// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};
// int maxquant=MAXQUANT_CONST;
int maxquant=(int) maxquantvalue;
int minquant=0;
long int npix=0;
long int i;
long int value_pix=0;
unsigned char bit=255;
int r=0;
int x,y,l;

long int threshold=0;
int thres_ind=0;
#ifdef LATEX
float m =0.0;
#endif

unsigned char *map_zt = NULL;
unsigned char *map_sig = NULL;

struct list_struct * list_desc=NULL;
struct list_el * current_el=NULL;

unsigned char *stream = streamstruct.stream;
unsigned char * count = streamstruct.count;
long int *streamlast = streamstruct.streamlast;
// unsigned char * count = (unsigned char *) malloc(sizeof(unsigned char));
// long int *streamlast = (long int *) malloc(sizeof(long int));

// *count=0;
// *streamlast=0;

long int nref=0;
long int npos=0;
long int nneg=0;
long int nzeroisol=0;
long int nzerotree=0;
long int nz=0;

FILE *data_file;//TODO useless after...
int status=0;

map_zt=(unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));
map_sig= (unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));

npix=imageprop.nsmax * imageprop.nlmax * imageprop.nbmax;

for (i=0;i<npix;i++){
	map_zt[i]=0;
	map_sig[i]=0;

}

#ifdef LATEX
m = ((imageprop.nsmax*imageprop.nlmax*imageprop.nbmax) - npos - nneg) / (nzeroisol + nzerotree);
printf("Bit plane & Significant & IZ & ZTR & Average\\\\ \n");
printf("\\hline \n");
#endif

//codage EZW
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){
	
	threshold= 1 << (long int)thres_ind;
#ifdef DEBUG
	printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
#endif
	nref=0;
// 	npos=0;
// 	nneg=0;
	nzeroisol=0;
	nzerotree=0;	
	nz = 0;
	
	//refinement pass
	for (i=0;i< npix;i++){
		if (map_sig[i] == 1){
			value_pix=image[i];
			bit = get_bit(value_pix, thres_ind);
			add_to_stream(stream, count, (int) bit, streamlast);
			nref++;
		};
	};
	
	//significance pass
	
	for (i=0;i< npix;i++){
// 	for (x=0;x<imageprop.nsmax;x++){
// 	for (y=0;y<imageprop.nlmax;y++){
// 	for (l=0;l<imageprop.nbmax;l++){
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));
// 		i = x + (y + l*imageprop.nlmax)*imageprop.nsmax;
		if ( (map_zt[i] == 0) && (map_sig[i] ==0) ){
		//This point is NOT part of a zerotree already and is NOT processed during refinement
			if (image[i] >= threshold){//POS
				map_sig[i] = 1;
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				npos++;
			};
			if (image[i] <= -threshold){// NEG
				map_sig[i] = 1;
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				bit = 0;
				add_to_stream(stream, count, (int) bit, streamlast);
				nneg++;
			};
			if ((image[i] < threshold) &&  (image[i] > -threshold)){ // IZ ou ZT
#ifdef EZWUSEZ
	#ifdef NEWTREE 
	//spat-tree
				if ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) {
	#else
	//3D-tree
				if (((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) 
					&& (l >= imageprop.nbmax / 2) ){
	#endif
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
					nz++;	

				} else {
#else
				{
#endif
				list_desc=NULL;//should be the case if freed properly before
				r=spat_spec_desc_ezw((struct pixel_struct) {x,y,l}, list_desc, 0, image,thres_ind, map_sig);
// 				r=spec_desc_ezw((struct pixel_struct) {x,y,l}, list_desc, 0, image,thres_ind, map_sig);
				//une modification est-elle necessaire pour tenir compte des elements appartenat deja aux ZT ?
				if ((r ==-1)||(r == 0)){// zero isole (early ending ou pas de desc
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
					nzeroisol++;			
				} else {// we have a zerotree
// 					printf("ZTR for %d, %d, %d\n",x,y,l);
// 					if ( (x == 1) && (y == 0) && (l == 7) ){
// 						printf("time for a break !\n");
// 					};
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
					bit = 1;
					add_to_stream(stream, count, (int) bit, streamlast);
					//do not forget to update map_zt
					list_desc=list_init();
					r=spat_spec_desc_ezw((struct pixel_struct) {x,y,l}, list_desc, 1, image, thres_ind, map_sig);//sans early ending...
// 					r=spec_desc_ezw((struct pixel_struct) {x,y,l}, list_desc, 1, image, thres_ind, map_sig);//sans early ending...
					map_zt[i]=1;
					current_el=first_el(list_desc);
					while (current_el != NULL){
						map_zt[trans_pixel(current_el->pixel)]=1;
						current_el=next_el(list_desc);
					};
					nzerotree++;
				};
				list_free(list_desc);
				//ne pas oublier de liberer la liste
				}
			};
// 		}else {
// 			if (map_zt[i] = 1) printf("Skip for map %ld\n",i); 
// #endif

		};
	};
// 	}
// 	}
// 	}
	
	//on remet la map_zt a zero...
	for (i=0; i<npix; i++){
		map_zt[i]=0;
	};
outputsize[thres_ind]=(*streamlast)*8 + (*count);
#ifdef DEBUG
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("Size in bit: %ld \n", *streamlast*8+*count);

printf("nref: %ld \n",nref);
printf("npos: %ld \n",npos);
printf("nneg: %ld \n",nneg);
printf("nzeroisol: %ld \n",nzeroisol);
printf("nzerotree: %ld \n",nzerotree);
printf("nz: %ld \n",nz);
printf("-------------------------\n");
#endif

#ifdef LATEX
m = ((double) (imageprop.nsmax*imageprop.nlmax*imageprop.nbmax) - npos - nneg) / (nzeroisol + nzerotree);
printf("%d & %ld & %ld & %ld & %f \\\\ \n",thres_ind, npos+nneg, nzeroisol, nzerotree, m);
#endif

};

outputsize[0]=(*streamlast)*8 + (*count);//added june 2006 for partial EZW coding

//Just for global IDL serialing
data_file = fopen("/tmp/maxquant.dat","w");
if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
status = fwrite(&(imageprop.maxquant), 2, 1, data_file);
status = fclose(data_file);

data_file = fopen("/tmp/outputsize.dat","w");
if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
status = fwrite(outputsize, 4, imageprop.maxquant, data_file);
status = fclose(data_file);
//fin du global IDL

return 0;
};


// int ezw_decode_c(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue)
int ezw_decode_c(long int *image, struct stream_struct streamstruct, long int *outputsize, int maxquantvalue)
{


// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};
// int maxquant=MAXQUANT_CONST;
int maxquant=(int) maxquantvalue;
int minquant=0;
long int npix=0;
long int i;
// long int value_pix=0;
unsigned char bit=255;
unsigned char bit2=255;
int r=0;
int x,y,l;

long int threshold=0;
int thres_ind=0;
int flagsig=0;
long int lastprocessed=0;

unsigned char *map_zt = NULL;
unsigned char *map_sig = NULL;

struct list_struct * list_desc=NULL;
struct list_el * current_el=NULL;

unsigned char *stream = streamstruct.stream;
unsigned char * count = streamstruct.count;
long int *streamlast = streamstruct.streamlast;
// unsigned char * count = (unsigned char *) malloc(sizeof(unsigned char));
// long int *streamlast = (long int *) malloc(sizeof(long int));

// *count=0;
// *streamlast=0;

long int nref=0;
long int npos=0;
long int nneg=0;
long int nzeroisol=0;
long int nzerotree=0;
long int nz=0;

map_zt=(unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));
map_sig= (unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));

for (i=0;i<npix;i++){
	map_zt[i]=0;
	map_sig[i]=0;

}

npix=imageprop.nsmax * imageprop.nlmax * imageprop.nbmax;

//decodage EZW
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){
	
	threshold= 1 << (long int)thres_ind;
#ifdef DEBUG
	printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
#endif
	nref=0;
// 	npos=0;
// 	nneg=0;
	nzeroisol=0;
	nzerotree=0;	
	nz = 0;	
	
	//refinement pass
	flagsig=1;
	for (i=0;i< npix;i++){
		if (map_sig[i] == 1){
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit = read_from_stream(stream, count, streamlast);
			} else break; //verifier qu'on sort de toutes les boucles for...
			if (bit == 1){
			if (image[i]>0) {
				image[i] += threshold;
			} else {
				image[i] -= threshold;
			};
			};
			nref++;
		};
	};
	
	if ((*streamlast)*8+ (*count) > *outputsize) break;

	//significance pass
	flagsig=0;
	for (i=0;i< npix;i++){
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));
		if ((map_zt[i] == 1) || (map_sig[i] ==1)){//TODO une seule structure ???
		//This point is part of a zerotree already or is processed during refinement
		} else {
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit= read_from_stream(stream, count, streamlast);
			} else break;

			if (bit == 1) {
				if ((*streamlast)*8+ (*count) <= *outputsize){
					bit2= read_from_stream(stream, count, streamlast);
				} else break;
				if (bit2 == 1){//POS
					map_sig[i] = 1;
					image[i] += threshold;
					npos++;
				} else {	//NEG
					map_sig[i] = 1;
					image[i] -= threshold;
					nneg++;
				}
			} else { 
#ifdef EZWUSEZ
	#ifdef NEWTREE 
	//spat-tree
				if ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) {
	#else
	//3D-tree
				if (((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) 
					&& (l >= imageprop.nbmax / 2) ){
	#endif
				nz++;//nothing else to do
			} else {
#else
			{
#endif
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit2= read_from_stream(stream, count, streamlast);
			} else break;
			if (bit2 == 0){//IZ
				//nothing to do
				nzeroisol++;
			} else {//ZT
				list_desc=list_init();
				r=spat_spec_desc_ezw((struct pixel_struct) {x,y,l}, list_desc, 1, image, thres_ind, map_sig);//sans early ending...
// 				r=spec_desc_ezw((struct pixel_struct) {x,y,l}, list_desc, 1, image, thres_ind, map_sig);//sans early ending...
				map_zt[i]=1;
				current_el=first_el(list_desc);
				while (current_el != NULL){
					map_zt[trans_pixel(current_el->pixel)]=1;
					current_el=next_el(list_desc);
				};
				list_free(list_desc);
				nzerotree++;
			};
				//ne pas oublier de liberer la liste

			}
			}
		};
	};

	//on remet la map_zt a zero...
	for (i=0; i<npix; i++){
		map_zt[i]=0;
	};
	if ((*streamlast)*8+ (*count) > *outputsize) break;
#ifdef DEBUG
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("Size in bit: %ld \n", *streamlast*8+*count);

printf("nref: %ld \n",nref);
printf("npos: %ld \n",npos);
printf("nneg: %ld \n",nneg);
printf("nzeroisol: %ld \n",nzeroisol);
printf("nzerotree: %ld \n",nzerotree);
printf("nz: %ld \n",nz);
printf("-------------------------\n");
#endif
};

//correction finale eventuelle
if ((*streamlast)*8+ (*count) > *outputsize){//on est sorti car le train de bit etait trop court
	if (flagsig == 1){ //sortie durant la phase d'update des coeff sig
		lastprocessed=i;
		for (i=0;i< lastprocessed;i++){
		   if (map_sig[i] == 1){
			if (image[i]>0) {
				image[i] += threshold/2;
			} else {
				image[i] -= threshold/2;
			};
		   };
		}
		for (i=lastprocessed;i< npix;i++){
		   if (map_sig[i] == 1){
			if (image[i]>0) {
				image[i] += threshold;
			} else {
				image[i] -= threshold;
			};
		   };
		}		
	} else {//sortie durant la sorting pass (plus facile)
		for (i=0;i< npix;i++){
		   if (map_sig[i] == 1){
			if (image[i]>0) {
				image[i] += threshold/2;
			} else {
				image[i] -= threshold/2;
			};
		   };
		};
	};

};

return 0;
};

