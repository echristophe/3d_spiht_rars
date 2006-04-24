// #include "spiht_code_c.h"
// #include "ezw_code_c.h"
// #include "desc_ezw.h"
// #include <stdlib.h>
// #include <stdio.h>
// #include <unistd.h> 
#include "main.h"

//temporaire, remplacer par les variables globales...

// #define NSMAX_CONST 256
// #define NLMAX_CONST 256
// #define NBMAX_CONST 224
// #define NSMIN_CONST 8
// #define NLMIN_CONST 8
// #define NBMIN_CONST 7
// #define MAXQUANT_CONST 20

int ezw_code_signed_c(long int *image, struct stream_struct streamstruct, long int *outputsize, int  maxquantvalue){

// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};
// int maxquant=MAXQUANT_CONST;
int maxquant=(int) maxquantvalue;
int minquant=0;
long int npix=0;
long int i;
// long int value_pix=0;
unsigned char bit=255;
int r=0;
int x,y,l;

long int threshold=0;
int thres_ind=0;

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

// long int nref=0;
long int npos=0;
long int nneg=0;
long int nzeroisol=0;
long int nzerotree=0;

char * image_signed = (char *) malloc (imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*NBITS);

map_zt=(unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));
map_sig= (unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));


npix=imageprop.nsmax * imageprop.nlmax * imageprop.nbmax;

for (i=0;i<npix;i++){
	map_zt[i]=0;
// 	map_sig[i]=0;

}


for (i=0;i< npix;i++){
	bin_value(image[i], &(image_signed[NBITS*i]));
}

printf("#zero before signed rep: %ld \n", count_zero(image_signed));

for (i=0;i< npix;i++){
	min_representation(&(image_signed[NBITS*i]));
}



printf("#zero after signed rep: %ld \n", count_zero(image_signed));

for (i=0;i< npix;i++){
	change_rep(&(image_signed[NBITS*i]));
}

printf("#zero after modif (should be the same): %ld \n", count_zero(image_signed));

// printf("maxquant +1 \n");
// maxquant++;

//codage EZW
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){
	
	threshold= 1 << (long int)thres_ind;
	printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
// 	nref=0;
// 	npos=0;
// 	nneg=0;
// 	nzeroisol=0;
// 	nzerotree=0;	
	
	//refinement pass
// 	for (i=0;i< npix;i++){
// 		if (map_sig[i] == 1){
// 			value_pix=image[i];
// 			bit = get_bit(value_pix, thres_ind);
// 			add_to_stream(stream, count, (int) bit, streamlast);
// 			nref++;
// 		};
// 	};
	
	//significance pass
	
	for (i=0;i< npix;i++){
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));
		if ( (map_zt[i] == 0) && (map_sig[i] ==0) ){
// 		if  (map_zt[i] == 0){
		//This point is NOT part of a zerotree already and is NOT processed during refinement
// 			if (image[i] >= threshold){//POS
			if (image_signed[NBITS*i+thres_ind] == 1){//POS
				map_sig[i] = 1;
				bit = 0;
				add_to_stream(stream, count, (int) bit, streamlast);
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				npos++;
// 				printf("Adding pixel:%d %d %d\n",x,y,l);
			};
// 			if (image[i] <= -threshold){// NEG
			if (image_signed[NBITS*i+thres_ind] == -1){//NEG
				map_sig[i] = 1;
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				bit = 0;
				add_to_stream(stream, count, (int) bit, streamlast);
				nneg++;
			};
// 			if ((image[i] < threshold) &&  (image[i] > -threshold)){ // IZ ou ZT
			if (image_signed[NBITS*i+thres_ind] == 0){// IZ ou ZT
				list_desc=NULL;//should be the case if freed properly before
				r=spat_spec_desc_ezw_signed((struct pixel_struct) {x,y,l}, list_desc, 0, image_signed, thres_ind, map_sig);
				//une modification est-elle necessaire pour tenir compte des elements appartenat deja aux ZT ? NOPE
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
					bit = 1;
					add_to_stream(stream, count, (int) bit, streamlast);
					bit = 1;
					add_to_stream(stream, count, (int) bit, streamlast);
					//do not forget to update map_zt
					list_desc=list_init();
					r=spat_spec_desc_ezw_signed((struct pixel_struct) {x,y,l}, list_desc, 1, image_signed, thres_ind, map_sig);//sans early ending...
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
			};
// 		}else {
// 			if (map_zt[i] = 1) printf("Skip for map %ld\n",i); 
		};
	};
	
	//on remet la map_zt a zero...
	// et dans le cas du signed AUSSI la map_sig
	for (i=0; i<npix; i++){
		map_zt[i]=0;
		map_sig[i]=0;
	};
outputsize[thres_ind]=(*streamlast)*8 + (*count);

printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("Size in bit: %ld \n", *streamlast*8+*count);

// printf("nref: %ld \n",nref);
printf("POS: %ld \n",npos);
printf("NEG: %ld \n",nneg);
printf("IZ:  %ld \n",nzeroisol);
printf("ZTR: %ld \n",nzerotree);
printf("-------------------------\n");

};

free(image_signed);

return 0;
};


int ezw_decode_signed_c(long int *image, struct stream_struct streamstruct, long int *outputsize, int maxquantvalue)
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

long int nref=0;
long int npos=0;
long int nneg=0;
long int nzeroisol=0;
long int nzerotree=0;	
	

long int threshold=0;
int thres_ind=0;
int flagsig=0;
// long int lastprocessed=0;

unsigned char *map_zt = NULL;
unsigned char *map_sig = NULL;

struct list_struct * list_desc=NULL;
struct list_el * current_el=NULL;

char * image_signed = (char *) malloc (imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*NBITS);

unsigned char *stream = streamstruct.stream;
unsigned char * count = streamstruct.count;
long int *streamlast = streamstruct.streamlast;
// unsigned char * count = (unsigned char *) malloc(sizeof(unsigned char));
// long int *streamlast = (long int *) malloc(sizeof(long int));

// *count=0;
// *streamlast=0;

map_zt=(unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));
map_sig= (unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));

for (i=0;i<npix;i++){
	map_zt[i]=0;
	map_sig[i]=0;//pourquoi etait-ce supprime ?

}

npix=imageprop.nsmax * imageprop.nlmax * imageprop.nbmax;

// printf("maxquant +1 \n");
// maxquant++;

//decodage EZW
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){
	
	threshold= 1 << (long int)thres_ind;
	printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
	
	nref=0;
	npos=0;
	nneg=0;
	nzeroisol=0;
	nzerotree=0;	
	
	//refinement pass
// 	flagsig=1;
// 	for (i=0;i< npix;i++){
// 		if (map_sig[i] == 1){
// 			if ((*streamlast)*8+ (*count) <= *outputsize){
// 				bit = read_from_stream(stream, count, streamlast);
// 			} else break; //verifier qu'on sort de toutes les boucles for...
// 			if (bit == 1){
// 			if (image[i]>0) {
// 				image[i] += threshold;
// 			} else {
// 				image[i] -= threshold;
// 			};
// 			};
// 		};
// 	};
	
	if ((*streamlast)*8+ (*count) > *outputsize) break;

	//significance pass
	flagsig=0;
	for (i=0;i< npix;i++){
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));
		if ((map_zt[i] == 1) || (map_sig[i] ==1)){//TODO une seule structure ???
// 		if (map_zt[i] == 1) {
		//This point is part of a zerotree already or is processed during refinement
		} else {
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit= read_from_stream(stream, count, streamlast);
			} else break;
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit2= read_from_stream(stream, count, streamlast);
			} else break;
			if ((bit == 0) && (bit2 == 1)){//POS
				map_sig[i] = 1;
// 				image[i] += threshold;
				image_signed[NBITS*i+thres_ind] = 1;
				npos++;
			};
			if ((bit == 1) && (bit2 == 0)){//NEG
				map_sig[i] = 1;
// 				image[i] -= threshold;
				image_signed[NBITS*i+thres_ind] = -1;
				nneg++;
			};
			if ((bit == 0) && (bit2 == 0)){//IZ
				//nothing to do
				nzeroisol++;
			};
			if ((bit == 1) && (bit2 == 1)){//ZT
				list_desc=list_init();
				r=spat_spec_desc_ezw_signed((struct pixel_struct) {x,y,l}, list_desc, 1, image_signed, thres_ind, map_sig);//sans early ending...
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
		};
	};

	//on remet la map_zt a zero...
	for (i=0; i<npix; i++){
		map_zt[i]=0;
		map_sig[i]=0;
	};
	if ((*streamlast)*8+ (*count) > *outputsize) break;


printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("Size in bit: %ld \n", *streamlast*8+*count);

printf("POS: %ld \n",npos);
printf("NEG: %ld \n",nneg);
printf("IZ:  %ld \n",nzeroisol);
printf("ZTR: %ld \n",nzerotree);
printf("-------------------------\n");
};

for (i=0;i< npix;i++){
// 	bin_value(image[i], &(image_signed[NBITS*i]));
	image[i] = value_signed(&(image_signed[NBITS*i]));
}

//correction finale eventuelle
// if ((*streamlast)*8+ (*count) > *outputsize){//on est sorti car le train de bit etait trop court
// 	if (flagsig == 1){ //sortie durant la phase d'update des coeff sig
// 		lastprocessed=i;
// 		for (i=0;i< lastprocessed;i++){
// 		   if (map_sig[i] == 1){
// 			if (image[i]>0) {
// 				image[i] += threshold/2;
// 			} else {
// 				image[i] -= threshold/2;
// 			};
// 		   };
// 		}
// 		for (i=lastprocessed;i< npix;i++){
// 		   if (map_sig[i] == 1){
// 			if (image[i]>0) {
// 				image[i] += threshold;
// 			} else {
// 				image[i] -= threshold;
// 			};
// 		   };
// 		}		
// 	} else {//sortie durant la sorting pass (plus facile)
// 		for (i=0;i< npix;i++){
// 		   if (map_sig[i] == 1){
// 			if (image[i]>0) {
// 				image[i] += threshold/2;
// 			} else {
// 				image[i] -= threshold/2;
// 			};
// 		   };
// 		};
// 	};
// 
// };

free(image_signed);

return 0;
};

