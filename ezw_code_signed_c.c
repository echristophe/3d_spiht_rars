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

//temporaire, remplacer par les variables globales...

// #define NSMAX_CONST 256
// #define NLMAX_CONST 256
// #define NBMAX_CONST 224
// #define NSMIN_CONST 8
// #define NLMIN_CONST 8
// #define NBMIN_CONST 7
// #define MAXQUANT_CONST 20

#ifdef EZW_ARITH
#include "libQccPack.h"
// #define SYMBOL_STREAM_LENGTH 100000000
// #define SYMBOL_STREAM_LENGTH 100
#endif


int ezw_code_signed_c(long int *image, struct stream_struct streamstruct, long int *outputsize, int  maxquantvalue){

// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};
// int maxquant=MAXQUANT_CONST;
int maxquant=(int) maxquantvalue;
int minquant=0;
long int npix=0;
long int i,m;
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
long int nz=0;

FILE *data_file;//TODO useless after...
int status=0;

char *image_signed =NULL;
// char * image_signed2;

#ifdef EZW_ARITH
long int symb_stream_length = 100000000;
int NUM_SYMBOLS = 4;
long int symb_counter = 0;
// int symbol_stream[symb_stream_length];
int * symbol_stream = (int *) malloc(symb_stream_length*sizeof(int));
unsigned char * streambyte;
FILE *output_file; int status;
QccBitBuffer output_buffer;
QccENTArithmeticModel *model = NULL;
int * argc1=1;
char * argv1[1];
argv1[0] = (char *) malloc(256*sizeof(char)); 
strcpy(argv1[0],"tmp");
#endif

#ifdef OUTPUTSIGNED
unsigned char * imsignbyte;
#endif

image_signed = (char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*NBITS*sizeof(char));
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


#ifdef SIGNED011
for (i=0;i< npix;i++){
	change_rep(&(image_signed[NBITS*i]));
}
printf("#zero after modif: %ld \n", count_zero(image_signed));
#endif

#ifdef OUTPUTSIGNED
    output_file = fopen("/home/christop/Boulot/images/output_stream/imsigned.dat","w");
    imsignbyte = (unsigned char *) malloc (npix*NBITS*sizeof(unsigned char));
    for (i=0; i<npix*NBITS; i++){ //maybe a +1 to put
         imsignbyte[i]= (unsigned char) image_signed[i];
    }
    status = fwrite(imsignbyte, 1, npix*NBITS, output_file);
    status = fclose(output_file);
    free(imsignbyte);
#endif

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
// 	for (i=0;i< npix;i++){//parcours lineaire
// 		x=i % (imageprop.nsmax);
// 		y=(i/imageprop.nsmax) % (imageprop.nlmax);
// 		l=(i/(imageprop.nsmax*imageprop.nlmax));

	for (m=0; m<imageprop.nsmax+imageprop.nlmax+imageprop.nbmax; m++){//parcours zigzag
	  for (x=0; x <= min(m,imageprop.nsmax-1); x++){
	    for (y=0; y<= min(m-x,imageprop.nlmax-1); y++){
	      l=m-x-y;
              if (l<= imageprop.nbmax-1){
			i =  x + (y+l*imageprop.nlmax)*imageprop.nsmax;

		if ( (map_zt[i] == 0) && (map_sig[i] ==0) ){
// 		if  (map_zt[i] == 0){
		//This point is NOT part of a zerotree already and is NOT processed during refinement
// 			if (image[i] >= threshold){//POS
			if (image_signed[NBITS*i+thres_ind] == 1){//POS
				map_sig[i] = 1;
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				npos++;
// 				printf("Adding pixel:%d %d %d\n",x,y,l);
				#ifdef EZW_ARITH
				symbol_stream[symb_counter]=1;
				symb_counter++;
				#endif
			};
// 			if (image[i] <= -threshold){// NEG
			if (image_signed[NBITS*i+thres_ind] == -1){//NEG
				map_sig[i] = 1;
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				bit = 0;
				add_to_stream(stream, count, (int) bit, streamlast);
				nneg++;
				#ifdef EZW_ARITH
				symbol_stream[symb_counter]=2;
				symb_counter++;
				#endif
			};
// 			if ((image[i] < threshold) &&  (image[i] > -threshold)){ // IZ ou ZT
			if (image_signed[NBITS*i+thres_ind] == 0){// IZ ou ZT
#ifdef EZWUSEZ
			   if ( ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) && (l >= imageprop.nbmax / 2) ){//HF, on peut reunir ZTR et IZ : TODO faire au décodage
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
					nz++;
			   } else {
#else
			   {
#endif
				list_desc=NULL;//should be the case if freed properly before
				r=spat_spec_desc_ezw_signed((struct pixel_struct) {x,y,l}, list_desc, 0, image_signed, thres_ind, map_sig);
				//une modification est-elle necessaire pour tenir compte des elements appartenat deja aux ZT ? NOPE
				if ((r ==-1)||(r == 0)){// zero isole (early ending ou pas de desc
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
					nzeroisol++;	
					#ifdef EZW_ARITH
					symbol_stream[symb_counter]=0;
					symb_counter++;
					#endif
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
					r=spat_spec_desc_ezw_signed((struct pixel_struct) {x,y,l}, list_desc, 1, image_signed, thres_ind, map_sig);//sans early ending...
					map_zt[i]=1;
					current_el=first_el(list_desc);
					while (current_el != NULL){
						map_zt[trans_pixel(current_el->pixel)]=1;
						current_el=next_el(list_desc);
					};
					nzerotree++;
					#ifdef EZW_ARITH
					symbol_stream[symb_counter]=3;
					symb_counter++;
					#endif
				};
				list_free(list_desc);
				//ne pas oublier de liberer la liste
			   }
			};
// 		}else {
// 			if (map_zt[i] = 1) printf("Skip for map %ld\n",i); 
			#ifdef EZW_ARITH
			if (symb_counter > symb_stream_length) fprintf(stderr,"Symbol stream overflow\n");
			#endif
		};
	}}}//for zigzag
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
printf("Z: %ld \n",nz);
printf("-------------------------\n");

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


free(image_signed);

#ifdef EZW_ARITH
    QccInit(argc1, argv1);
    QccBitBufferInitialize(&output_buffer);

    output_buffer.type = QCCBITBUFFER_OUTPUT;
    strcpy(output_buffer.filename,"tmp");

    if (QccBitBufferStart(&output_buffer))
    {
    QccErrorAddMessage("%s: Error calling QccBitBufferStart()",
    argv1[0]);
    QccErrorExit();
    }

    if ((model = QccENTArithmeticEncodeStart(&NUM_SYMBOLS,
    1,
    NULL,
    QCCENT_ANYNUMBITS)) == NULL)
    {
    QccErrorAddMessage("%s: Error calling QccENTArithmeticEncodeStart()",
    argv1[0]);
    QccErrorExit();
    }

    printf("We have %ld symbols to encode\n",symb_counter);

    if (QccENTArithmeticEncode(symbol_stream,
//     SYMBOL_STREAM_LENGTH,
    symb_counter,
    model,
    &output_buffer))
    {
    QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
    argv1[0]);
    QccErrorExit();
    }

    if (QccENTArithmeticEncodeEnd(model,
    0,
    &output_buffer))
    {
    QccErrorAddMessage("%s: Error calling QccENTArithmeticEncodeEnd()");
    QccErrorExit();
    }

    if (QccBitBufferEnd(&output_buffer))
    {
    QccErrorAddMessage("%s: Error calling QccBitBufferEnd()",
    argv1[0]);
    QccErrorExit();
    }

    QccENTArithmeticFreeModel(model);

//     QccExit;
    //output symbol_stream in a byte file
    output_file = fopen("/home/christop/Boulot/images/output_stream/symbols.dat","w");
    streambyte = (unsigned char *) malloc (symb_counter*sizeof(unsigned char));
    for (i=0; i<symb_counter; i++){ //maybe a +1 to put
         streambyte[i]= (unsigned char) symbol_stream[i];
    }
    status = fwrite(streambyte, 1, symb_counter, output_file);
    status = fclose(output_file);
    free(streambyte);

    free(symbol_stream);
#endif

return 0;
};


int ezw_decode_signed_c(long int *image, struct stream_struct streamstruct, long int *outputsize, int maxquantvalue)
{


// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};
// int maxquant=MAXQUANT_CONST;
int maxquant=(int) maxquantvalue;
int minquant=0;
long int npix=0;
long int i,m;
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
long int nz=0;
	

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
	nz=0;
	
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
// 	for (i=0;i< npix;i++){
// 		x=i % (imageprop.nsmax);
// 		y=(i/imageprop.nsmax) % (imageprop.nlmax);
// 		l=(i/(imageprop.nsmax*imageprop.nlmax));

	for (m=0; m<imageprop.nsmax+imageprop.nlmax+imageprop.nbmax; m++){//parcours zigzag
	  for (x=0; x <= min(m,imageprop.nsmax-1); x++){
	    for (y=0; y<= min(m-x,imageprop.nlmax-1); y++){
	      l=m-x-y;
              if (l<= imageprop.nbmax-1){
			i =  x + (y+l*imageprop.nlmax)*imageprop.nsmax;

		if ((map_zt[i] == 1) || (map_sig[i] ==1)){//TODO une seule structure ???
// 		if (map_zt[i] == 1) {
		//This point is part of a zerotree already or is processed during refinement
		} else {
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit= read_from_stream(stream, count, streamlast);
			} else break;
// 			if ((*streamlast)*8+ (*count) <= *outputsize){
// 				bit2= read_from_stream(stream, count, streamlast);
// 			} else break;
// 			if ((bit == 1) && (bit2 == 1)){//POS
// 				map_sig[i] = 1;
// // 				image[i] += threshold;
// 				image_signed[NBITS*i+thres_ind] = 1;
// 				npos++;
// 			};
// 			if ((bit == 1) && (bit2 == 0)){//NEG
// 				map_sig[i] = 1;
// // 				image[i] -= threshold;
// 				image_signed[NBITS*i+thres_ind] = -1;
// 				nneg++;
// 			};
			if (bit == 1) {
			 if ((*streamlast)*8+ (*count) <= *outputsize){
				bit2= read_from_stream(stream, count, streamlast);
			 } else break;
			  if (bit2 == 1){//POS
				map_sig[i] = 1;
// 				image[i] += threshold;
				image_signed[NBITS*i+thres_ind] = 1;
				npos++;
			  };
			  if (bit2 == 0){//NEG
				map_sig[i] = 1;
// 				image[i] -= threshold;
				image_signed[NBITS*i+thres_ind] = -1;
				nneg++;
			  };
			} else {
#ifdef EZWUSEZ
		          if ( ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) && (l >= imageprop.nbmax / 2) ){//HF, on peut reunir ZTR et IZ : TODO faire au décodage
				nz++;//rien a faire
			  } else {
#else
			{
#endif
// 			if ((bit == 0) && (bit2 == 0)){//IZ
// 				//nothing to do
// 				nzeroisol++;
// 			};
// 			if ((bit == 0) && (bit2 == 1)){//ZT
// 				list_desc=list_init();
// 				r=spat_spec_desc_ezw_signed((struct pixel_struct) {x,y,l}, list_desc, 1, image_signed, thres_ind, map_sig);//sans early ending...
// 				map_zt[i]=1;
// 				current_el=first_el(list_desc);
// 				while (current_el != NULL){
// 					map_zt[trans_pixel(current_el->pixel)]=1;
// 					current_el=next_el(list_desc);
// 				};
// 				list_free(list_desc);
// 				nzerotree++;
// 			};
// 				//ne pas oublier de liberer la liste

				if ((*streamlast)*8+ (*count) <= *outputsize){
					bit2= read_from_stream(stream, count, streamlast);
				} else break;

				if (bit2 == 0){//IZ
					//nothing to do
					nzeroisol++;
				};
				if (bit2 == 1){//ZT
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
			   }

			}
		};
	}}}//zigzag
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
printf("Z: %ld \n",nz);
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

