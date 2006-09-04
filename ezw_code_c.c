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


#include "main.h"

#ifdef EZW_ARITH
#include "libQccPack.h"
#endif

#ifdef EZW_ARITH

#define CONT_NUM 3
#define CONT_SIGN_GEN 0
#define CONT_SIGN_HF 1
#define CONT_REFINE 2

//4 symbols in total for results in paper
#define NUM_SYMBOLS 4
#define SYMB_POS 0
#define SYMB_NEG 1
#define SYMB_IZ 2
#define SYMB_ZTR 3
#define SYMB_Z 2

#define SYMB_REF_1 1
#define SYMB_REF_0 0
#endif

int ezw_code_c(long int *image, stream_struct streamstruct, long int *outputsize, int maxquantvalue){

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

#ifdef COMPUTE_STAT
long int symb_stream_length = 100000000;
int * symbol_stream = (int *) malloc(symb_stream_length*sizeof(int));
#endif
#ifdef EZW_ARITH
int symbol = -1;
// long int symb_stream_length = 100000000;
/*#ifdef EZWUSEZ
int NUM_SYMBOLS = 5*/;//avec arith, on peut numeroter pareil le Z
int num_context=CONT_NUM;
int num_symbols[CONT_NUM]; 
// #else
// int NUM_SYMBOLS = 4;
// int num_context=CONT_NUM;
// int num_symbols[CONT_NUM]; 
// #endif
long int symb_counter = 0;
// int symbol_stream[symb_stream_length];

unsigned char * streambyte;
FILE *output_file; //int status;
QccBitBuffer output_buffer;
QccENTArithmeticModel *model = NULL;
int * argc1=1;
char * argv1[1];
argv1[0] = (char *) malloc(256*sizeof(char)); 
strcpy(argv1[0],"tmp");
struct stat filestat;
// #ifdef EZW_ARITH_RESET_MODEL
// #ifdef EZWUSEZ
// double * probaref = (double *) calloc(5, sizeof(double));
// double * probasig = (double *) calloc(5, sizeof(double));
// #else
// double * probaref = (double *) calloc(4, sizeof(double));
// double * probasig = (double *) calloc(4, sizeof(double));
// #endif
// #endif
#endif

list_struct * list_desc=NULL;
list_el * current_el=NULL;

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

#ifdef EZW_ARITH
    QccInit(argc1, argv1);
    QccBitBufferInitialize(&output_buffer);

// 	for (i = 0; i < num_context; i++)
// 	num_symbols[i] = NUM_SYMBOLS; 
    num_symbols[CONT_REFINE]=2;
    num_symbols[CONT_SIGN_HF]=3;
    num_symbols[CONT_SIGN_GEN]=4;

    output_buffer.type = QCCBITBUFFER_OUTPUT;
    strcpy(output_buffer.filename,"tmp");

    if (QccBitBufferStart(&output_buffer))
    {
    QccErrorAddMessage("%s: Error calling QccBitBufferStart()",
    argv1[0]);
    QccErrorExit();
    }

    if ((model = QccENTArithmeticEncodeStart(num_symbols,
    num_context,
    NULL,
    QCCENT_ANYNUMBITS)) == NULL)
    {
    QccErrorAddMessage("%s: Error calling QccENTArithmeticEncodeStart()",
    argv1[0]);
    QccErrorExit();
    }
#endif

#ifdef LATEX
m = ((imageprop.nsmax*imageprop.nlmax*imageprop.nbmax) - npos - nneg) / (nzeroisol + nzerotree);
printf("Bit plane & Significant & IZ & ZTR & Average\\\\ \n");
printf("\\hline \n");
#endif

// #ifdef EZW_ARITH_RESET_MODEL
// for (i=0;i<NUM_SYMBOLS;i++){//to correspond to the original EZW with model reset for significant and ref pass
// 	probasig[i]=1.0/NUM_SYMBOLS;
// // 	probaref[i]=1.0/NUM_SYMBOLS;
// }
// probaref[0]=1/2.0;
// probaref[1]=1/2.0;
// for (i=2;i<NUM_SYMBOLS;i++){
// 	probaref[i]=0.0;
// }
// 
// #endif

//codage EZW
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){
	
	threshold= 1 << (long int)thres_ind;
#ifdef DEBUG
	printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
#endif
// 	nref=0;
// 	npos=0;
// 	nneg=0;
// 	nzeroisol=0;
// 	nzerotree=0;	
// 	nz = 0;
	


#ifndef EZWNOREF
#ifndef EZWREFAFTER
#ifdef EZW_ARITH

	#ifdef EZW_ARITH_RESET_MODEL

// 	QccENTArithmeticSetModelAdaption(model, QCCENT_NONADAPTIVE);
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
// 		if (QccENTArithmeticSetModelProbabilities(model, probaref, i))
// 		{
// 		QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelProbabilities()",
// 		argv1[0]);
// 		QccErrorExit();
// 		}
	}
// 	QccENTArithmeticSetModelAdaption(model, QCCENT_ADAPTIVE);
	#endif

	if (QccENTArithmeticSetModelContext(model, CONT_REFINE)) //contexte 0 pour refinement
	{
	QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
	argv1[0]);
	QccErrorExit();
	}
#endif
	//refinement pass
	for (i=0;i< npix;i++){
		if (map_sig[i] == 1){
			value_pix=image[i];
			bit = get_bit(value_pix, thres_ind);
#ifdef EZW_ARITH
			if (bit == 0){
				symbol = SYMB_REF_0;
			} else {
				symbol = SYMB_REF_1;
			}
			if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
			argv1[0]);
			QccErrorExit();
			}
// 			printf("%d",symbol);
#else
			add_to_stream(stream, count, (int) bit, streamlast);
#endif
			nref++;
		};
	};
#endif
#endif
	
	//significance pass
	
	#ifdef EZW_ARITH_RESET_MODEL
// 	QccENTArithmeticSetModelAdaption(model, QCCENT_NONADAPTIVE);
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
// 		if (QccENTArithmeticSetModelProbabilities(model, probasig, i))
// 		{
// 		QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelProbabilities()",
// 		argv1[0]);
// 		QccErrorExit();
// 		}
	}
// 	QccENTArithmeticSetModelAdaption(model, QCCENT_ADAPTIVE);
	#endif
	for (i=0;i< npix;i++){
// 	for (x=0;x<imageprop.nsmax;x++){
// 	for (y=0;y<imageprop.nlmax;y++){
// 	for (l=0;l<imageprop.nbmax;l++){

// 		i = x + (y + l*imageprop.nlmax)*imageprop.nsmax;

// 	for (m=0; m<imageprop.nsmax+imageprop.nlmax+imageprop.nbmax; m++){//parcours zigzag
// 	  for (x=0; x <= min(m,imageprop.nsmax-1); x++){
// 	    for (y=0; y<= min(m-x,imageprop.nlmax-1); y++){
// 	      l=m-x-y;
//               if (l<= imageprop.nbmax-1){
// 			i =  x + (y+l*imageprop.nlmax)*imageprop.nsmax;

		if ( (map_zt[i] == 0) && (map_sig[i] ==0) ){
		//This point is NOT part of a zerotree already and is NOT processed during refinement
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));
/*Selection du contexte*/
#ifdef EZW_ARITH
#ifdef EZWUSEZ
	#ifdef NEWTREE 
	//spat-tree
				if ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) {
	#else
		#ifdef NEWTREE2 
				if ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) {
		#else
	//3D-tree
				if (((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) 
					&& (l >= imageprop.nbmax / 2) ){
		#endif
	#endif
				
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_HF)) //contexte 2 pour POS,NEG,Z
					{
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
					argv1[0]);
					QccErrorExit();
					}
				} else {
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN)) //contexte 1 pour POS, NEG, IZ, ZTR
					{
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
					argv1[0]);
					QccErrorExit();
					}
				}
				
#else
			if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN)) //contexte 1 pour POS, NEG, IZ, ZTR
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
			argv1[0]);
			QccErrorExit();
			}
#endif
#endif
/*Fin de selection du contexte*/
			if (image[i] >= threshold){//POS
				map_sig[i] = 1;
#ifdef EZWNOREF
				image[i] -= threshold;
#endif
#ifdef EZW_ARITH
				symbol = SYMB_POS;

				if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
				{
				QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
				argv1[0]);
				QccErrorExit();
				}
// 				printf("%d",symbol);
#else
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
// 				printf("POS ");
#endif
				npos++;

			};
			if (image[i] <= -threshold){// NEG
				map_sig[i] = 1;
#ifdef EZWNOREF
				image[i] += threshold;
#endif
#ifdef EZW_ARITH
				symbol = SYMB_NEG;
				if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
				{
				QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
				argv1[0]);
				QccErrorExit();
				}
// 				printf("%d",symbol);
#else
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				bit = 0;
				add_to_stream(stream, count, (int) bit, streamlast);
// 				printf("NEG ");
#endif
				nneg++;
			};
			if (map_sig[i] == 0){
// 			if ((image[i] < threshold) &&  (image[i] > -threshold)){ // IZ ou ZT
#ifdef EZWUSEZ
	#ifdef NEWTREE 
	//spat-tree
				if ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) {
	#else
		#ifdef NEWTREE2 
		//spat-tree
				if ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) {
		#else
	//3D-tree
				if (((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) 
					&& (l >= imageprop.nbmax / 2) ){
		#endif
	#endif
#ifdef EZW_ARITH
				symbol = SYMB_Z;
				if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
				{
				QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
				argv1[0]);
				QccErrorExit();
				}
// 				printf("%d",symbol);
#else
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
// 					printf("Z   ");
#endif
					nz++;	

				} else {
#else
				{
#endif
				list_desc=NULL;//should be the case if freed properly before
				r=spat_spec_desc_ezw((pixel_struct) {x,y,l}, list_desc, 0, image,thres_ind, map_sig);
// 				r=spec_desc_ezw((pixel_struct) {x,y,l}, list_desc, 0, image,thres_ind, map_sig);
				//une modification est-elle necessaire pour tenir compte des elements appartenat deja aux ZT ?
				if ((r ==-1)||(r == 0)){// zero isole (early ending ou pas de desc
#ifdef EZW_ARITH
				symbol = SYMB_IZ;
				if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
				{
				QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
				argv1[0]);
				QccErrorExit();
				}
// 				printf("%d",symbol);
#else
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
// 					printf("IZ  ");
#endif
					nzeroisol++;			
				} else {// we have a zerotree
// 					printf("ZTR for %d, %d, %d\n",x,y,l);
// 					if ( (x == 1) && (y == 0) && (l == 7) ){
// 						printf("time for a break !\n");
// 					};
#ifdef EZW_ARITH
				symbol = SYMB_ZTR;
				if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
				{
				QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
				argv1[0]);
				QccErrorExit();
				}
// 				printf("%d",symbol);
#else
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
					bit = 1;
					add_to_stream(stream, count, (int) bit, streamlast);
// 					printf("ZTR ");
#endif
					//do not forget to update map_zt
					list_desc=list_init();
					r=spat_spec_desc_ezw((pixel_struct) {x,y,l}, list_desc, 1, image, thres_ind, map_sig);//sans early ending...
// 					r=spec_desc_ezw((pixel_struct) {x,y,l}, list_desc, 1, image, thres_ind, map_sig);//sans early ending...
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
// 	}}}
// 	}
// 	}
// 	}
	
	//on remet la map_zt a zero...
	for (i=0; i<npix; i++){
		map_zt[i]=0;
#ifdef EZWNOREF
		map_sig[i]=0;
#endif
	};


#ifdef EZWREFAFTER
	//refinement pass
#ifdef EZW_ARITH
	#ifdef EZW_ARITH_RESET_MODEL
// 	QccENTArithmeticSetModelAdaption(model, QCCENT_NONADAPTIVE);
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
// 		if (QccENTArithmeticSetModelProbabilities(model, probaref, i))
// 		{
// 		QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelProbabilities()",
// 		argv1[0]);
// 		QccErrorExit();
// 		}
	}
// 	QccENTArithmeticSetModelAdaption(model, QCCENT_ADAPTIVE);
	#endif
	if (QccENTArithmeticSetModelContext(model, CONT_REFINE)) //contexte 0 pour refinement
	{
	QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
	argv1[0]);
	QccErrorExit();
	}
#endif
	for (i=0;i< npix;i++){
		if ((map_sig[i] == 1) && (abs(image[i]) >= 2*threshold)) {
			value_pix=image[i];
			bit = get_bit(value_pix, thres_ind);
#ifdef EZW_ARITH
			if (bit == 0){
				symbol = SYMB_REF_0;
			} else {
				symbol = SYMB_REF_1;
			}
			if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
			argv1[0]);
			QccErrorExit();
			}
// 			printf("%d",symbol);
#else
			add_to_stream(stream, count, (int) bit, streamlast);
#endif
			nref++;
		};
	};
#endif

#ifdef EZW_ARITH
outputsize[thres_ind]=output_buffer.bit_cnt; //TODO check if correct
#else
outputsize[thres_ind]=(*streamlast)*8 + (*count);
#endif

#ifdef DEBUG
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("Size in bit: %ld \n", *streamlast*8+*count);
#ifdef EZW_ARITH
printf("Size in bit: %ld \n", output_buffer.bit_cnt);
#endif
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

//added june 2006 for partial coding
#ifdef EZW_ARITH 
outputsize[thres_ind]=output_buffer.bit_cnt;
#else
outputsize[0]=(*streamlast)*8 + (*count);
#endif

#ifdef EZW_ARITH
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

   //copie pour assurer la compatibilité


    if (stat("tmp", &filestat) == 0){
        // The size of the file in bytes is in
        // results.st_size
   } else {
	fprintf(stderr, "Error on tmp file...\n");
   }

   data_file = fopen("tmp", "r");
   status = fread(&(streamstruct.stream[*streamstruct.streamlast]), 1, filestat.st_size, data_file);
   status= fclose(data_file);
   *streamstruct.streamlast += filestat.st_size;

//    *streamstruct.count = 0;
//    printf("Size in byte: %ld \n", *streamstruct.streamlast);
   outputsize[0]=(*streamlast)*8 + (*count);
#endif

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
int ezw_decode_c(long int *image, stream_struct streamstruct, long int *outputsize, int maxquantvalue)
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

#ifdef EZW_ARITH
int symbol = -1;
// long int symb_stream_length = 100000000;// ???
// #ifdef EZWUSEZ
// int NUM_SYMBOLS = 5;
// int num_context=CONT_NUM;
// int num_symbols[CONT_NUM]; 
// #else
// int NUM_SYMBOLS = 4;
int num_context=CONT_NUM;
int num_symbols[CONT_NUM]; 
// #endif
long int symb_counter = 0;
// int symbol_stream[symb_stream_length];
// int * symbol_stream = (int *) malloc(symb_stream_length*sizeof(int));
unsigned char * streambyte;
FILE *input_file; //int status;
QccBitBuffer input_buffer;
QccENTArithmeticModel *model = NULL;
int * argc1=1;
char * argv1[1];
argv1[0] = (char *) malloc(256*sizeof(char)); 
strcpy(argv1[0],"spihtcode");
struct stat filestat;
// #ifdef EZW_ARITH_RESET_MODEL
// #ifdef EZWUSEZ
// double probaref[5];
// double probasig[5];
// #else
// double probaref[4];
// double probasig[4];
// #endif
// #endif
#endif

list_struct * list_desc=NULL;
list_el * current_el=NULL;

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

for (i=0;i<npix;i++){
	map_zt[i]=0;
	map_sig[i]=0;

}

#ifdef EZW_ARITH
   //copy pour compatibilité
   data_file = fopen("tmp", "w");
   status = fwrite(&streamstruct.stream[(*streamstruct.headerlength)/8], 1, *outputsize/8 - (*streamstruct.headerlength)/8, data_file);//WARNING depends on header size, not valid if mean sub (TODO add header size property)
//    *streamstruct.streamlast += filestat.st_size;
   status=fclose(data_file);

    QccInit(argc1, argv1);
    QccBitBufferInitialize(&input_buffer);

//     for (i = 0; i < num_context; i++)
// 	num_symbols[i] = NUM_SYMBOLS; 
    num_symbols[CONT_REFINE]=2;
    num_symbols[CONT_SIGN_HF]=3;
    num_symbols[CONT_SIGN_GEN]=4;

    input_buffer.type = QCCBITBUFFER_INPUT;
    strcpy(input_buffer.filename,"tmp");

    if (QccBitBufferStart(&input_buffer))
    {
    QccErrorAddMessage("%s: Error calling QccBitBufferStart()",
    argv1[0]);
    QccErrorExit();
    }

    if ((model = QccENTArithmeticDecodeStart(&input_buffer, 
    num_symbols,
    num_context,
    NULL,
    QCCENT_ANYNUMBITS)) == NULL)
    {
    QccErrorAddMessage("%s: Error calling QccENTArithmeticDecodeStart()",
    argv1[0]);
    QccErrorExit();
    }
#endif

npix=imageprop.nsmax * imageprop.nlmax * imageprop.nbmax;

// #ifdef EZW_ARITH_RESET_MODEL
// for (i=0;i<NUM_SYMBOLS;i++){//to correspond to the original EZW with model reset for significant and ref pass
// 	probasig[i]=1.0/NUM_SYMBOLS;
// // 	probaref[i]=1.0/NUM_SYMBOLS;//tmp
// }
// probaref[0]=1/2.0;
// probaref[1]=1/2.0;
// for (i=2;i<NUM_SYMBOLS;i++){
// 	probaref[i]=0.0;
// }
// #endif

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
	
#ifndef EZWNOREF
#ifndef EZWREFAFTER
	//refinement pass
#ifdef EZW_ARITH
	#ifdef EZW_ARITH_RESET_MODEL
// 	QccENTArithmeticSetModelAdaption(model, QCCENT_NONADAPTIVE);
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
// 		if (QccENTArithmeticSetModelProbabilities(model, probaref, i))
// 		{
// 		QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelProbabilities()",
// 		argv1[0]);
// 		QccErrorExit();
// 		}
	}
// 	QccENTArithmeticSetModelAdaption(model, QCCENT_ADAPTIVE);
	#endif
	if (QccENTArithmeticSetModelContext(model, CONT_REFINE)) //contexte 0 pour refinement
	{
	QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
	argv1[0]);
	QccErrorExit();
	}
#endif
	flagsig=1;
	for (i=0;i< npix;i++){
		if (map_sig[i] == 1){
#ifdef EZW_ARITH
			if (input_buffer.bit_cnt < *outputsize-(*streamstruct.headerlength)){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
// 			printf("%d",symbol);
			bit = (unsigned char) symbol;
#else
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit = read_from_stream(stream, count, streamlast);
			} else break; //verifier qu'on sort de toutes les boucles for...
#endif
			if (bit == 1){
				if (image[i]>0) {
					image[i] += threshold/2;
				} else {
					image[i] -= threshold/2;
				};
			} else {
				if (image[i]>0) {
					image[i] -= round(threshold/2.0+0.1);
				} else {
					image[i] += round(threshold/2.0+0.1);
				};
			};
			nref++;
		};
	};
#ifdef EZW_ARITH	
	if (input_buffer.bit_cnt >= *outputsize-(*streamstruct.headerlength)) break;
#else
	if ((*streamlast)*8+ (*count) > *outputsize) break;
#endif

#endif
#endif

	//significance pass
	#ifdef EZW_ARITH_RESET_MODEL
// 	QccENTArithmeticSetModelAdaption(model, QCCENT_NONADAPTIVE);
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
// 		if (QccENTArithmeticSetModelProbabilities(model, probasig, i))
// 		{
// 		QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelProbabilities()",
// 		argv1[0]);
// 		QccErrorExit();
// 		}
	}
// 	QccENTArithmeticSetModelAdaption(model, QCCENT_ADAPTIVE);
	#endif
	flagsig=0;
	for (i=0;i< npix;i++){
// 		if ((map_zt[i] == 1) || (map_sig[i] ==1)){//TODO une seule structure ???
// 		//This point is part of a zerotree already or is processed during refinement
// 		} else {
		if ((map_zt[i] == 0) && (map_sig[i] ==0)){ //Modif 2006-07-24 to correspond with encoding
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));

// 	for (m=0; m<imageprop.nsmax+imageprop.nlmax+imageprop.nbmax; m++){//parcours zigzag
// 	  for (x=0; x <= min(m,imageprop.nsmax-1); x++){
// 	    for (y=0; y<= min(m-x,imageprop.nlmax-1); y++){
// 	      l=m-x-y;
//               if (l<= imageprop.nbmax-1){
// 			i =  x + (y+l*imageprop.nlmax)*imageprop.nsmax;
// 		if ((map_zt[i] == 0) && (map_sig[i] ==0)){ 
/*Selection du contexte*/
#ifdef EZW_ARITH
#ifdef EZWUSEZ
	#ifdef NEWTREE 
	//spat-tree
				if ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) {
	#else
		#ifdef NEWTREE2 
				if ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) {
		#else
	//3D-tree
				if (((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) 
					&& (l >= imageprop.nbmax / 2) ){
		#endif
	#endif
				
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_HF)) //contexte 2 pour POS,NEG,Z
					{
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
					argv1[0]);
					QccErrorExit();
					}
				} else {
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN)) //contexte 1 pour POS, NEG, IZ, ZTR
					{
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
					argv1[0]);
					QccErrorExit();
					}
				}
				

#else
		if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN)) //contexte 1 pour POS, NEG, IZ, ZTR
		{
		QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
		argv1[0]);
		QccErrorExit();
		}
#endif
#endif
/*Fin de selection du contexte*/
#ifdef EZW_ARITH
			if (input_buffer.bit_cnt < *outputsize-(*streamstruct.headerlength)){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
// 			printf("%d",symbol);
			symbol;	
			if (symbol == SYMB_POS){bit = 1; bit2=1;}//POS
			if (symbol == SYMB_NEG){bit = 1; bit2=0;}//NEG
			if (symbol == SYMB_IZ){bit = 0; bit2=0;}//IZ
			if (symbol == SYMB_ZTR){bit = 0; bit2=1;}//ZTR
			if (symbol == SYMB_Z){bit = 0; bit2=0;}//Z
#else
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit= read_from_stream(stream, count, streamlast);
			} else break;
#endif
			if (bit == 1) {
#ifndef EZW_ARITH
				if ((*streamlast)*8+ (*count) <= *outputsize){
					bit2= read_from_stream(stream, count, streamlast);
				} else break;
#endif
				if (bit2 == 1){//POS
					map_sig[i] = 1;
#ifdef EZWNOREF
					image[i] += threshold;
#else
					image[i] += threshold + threshold/2;
#endif
// 					printf("POS ");
					npos++;
				} else {	//NEG
					map_sig[i] = 1;
#ifdef EZWNOREF
					image[i] -= threshold;
#else
					image[i] -= threshold + threshold/2;
#endif
// 					printf("NEG ");
					nneg++;
				}
			} else { 
#ifdef EZWUSEZ
	#ifdef NEWTREE 
	//spat-tree
				if ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) {
	#else
		#ifdef NEWTREE2 
		//spat-tree
				if ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) {
		#else
	//3D-tree
				if (((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) 
					&& (l >= imageprop.nbmax / 2) ){
		#endif
	#endif
// 				printf("Z   ");
				nz++;//nothing else to do
				} else {
#else
			{
#endif
#ifndef EZW_ARITH
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit2= read_from_stream(stream, count, streamlast);
			} else break;
#endif
			if (bit2 == 0){//IZ
				//nothing to do
// 				printf("IZ  ");
				nzeroisol++;
			} else {//ZT
				list_desc=list_init();
				r=spat_spec_desc_ezw((pixel_struct) {x,y,l}, list_desc, 1, image, thres_ind, map_sig);//sans early ending...
// 				r=spec_desc_ezw((pixel_struct) {x,y,l}, list_desc, 1, image, thres_ind, map_sig);//sans early ending...
				map_zt[i]=1;
				current_el=first_el(list_desc);
				while (current_el != NULL){
					map_zt[trans_pixel(current_el->pixel)]=1;
					current_el=next_el(list_desc);
				};
				list_free(list_desc);
// 				printf("ZTR ");
				nzerotree++;
			};
				//ne pas oublier de liberer la liste

			}
			}
		};
	};
// 	}}}	

	//on remet la map_zt a zero...
	for (i=0; i<npix; i++){
		map_zt[i]=0;
#ifdef EZWNOREF
		map_sig[i]=0;
#endif
	};

#ifdef EZW_ARITH	
	if (input_buffer.bit_cnt >= *outputsize-(*streamstruct.headerlength)) break;
#else
	if ((*streamlast)*8+ (*count) > *outputsize) break;
#endif

#ifdef EZWREFAFTER
	//refinement pass
	flagsig=1;
#ifdef EZW_ARITH
	#ifdef EZW_ARITH_RESET_MODEL
// 	QccENTArithmeticSetModelAdaption(model, QCCENT_NONADAPTIVE);
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
// 		if (QccENTArithmeticSetModelProbabilities(model, probaref, i))
// 		{
// 		QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelProbabilities()",
// 		argv1[0]);
// 		QccErrorExit();
// 		}
	}
// 	QccENTArithmeticSetModelAdaption(model, QCCENT_ADAPTIVE);
	#endif
	if (input_buffer.bit_cnt < *outputsize-(*streamstruct.headerlength)){
	if (QccENTArithmeticSetModelContext(model, CONT_REFINE)) //contexte 0 pour refinement
	{
	QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
	argv1[0]);
	QccErrorExit();
	}
	} else break;
#endif
	for (i=0;i< npix;i++){
		if ((map_sig[i] == 1) &&  (abs(image[i]) >= 2*threshold)) {
#ifdef EZW_ARITH
			if (input_buffer.bit_cnt < *outputsize-(*streamstruct.headerlength)){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
// 			printf("%d",symbol);
			bit = (unsigned char) symbol;
#else
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit = read_from_stream(stream, count, streamlast);
			} else break; //verifier qu'on sort de toutes les boucles for...
#endif
			if (bit == 1){
				if (image[i]>0) {
					image[i] += threshold/2;
				} else {
					image[i] -= threshold/2;
				};
			} else {
				if (image[i]>0) {
					image[i] -= round(threshold/2.0+0.1);;
				} else {
					image[i] += round(threshold/2.0+0.1);;
				};
			};
			nref++;
		};
	};
	
#ifdef EZW_ARITH	
	if (input_buffer.bit_cnt >= *outputsize-(*streamstruct.headerlength)) break;
#else
	if ((*streamlast)*8+ (*count) > *outputsize) break;
#endif
#endif

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

#ifdef EZW_ARITH
    if (QccBitBufferEnd(&input_buffer))
    {
    QccErrorAddMessage("%s: Error calling QccBitBufferEnd()",
    argv1[0]);
    QccErrorExit();
    }

    QccENTArithmeticFreeModel(model);
#endif

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
// #ifdef EZWREFAFTER
// 			if (image[i]>0) {
// 				image[i] += threshold;
// 			} else {
// 				image[i] -= threshold;
// 			};
// #else
// 			if (image[i]>0) {
// 				image[i] += threshold/2;
// 			} else {
// 				image[i] -= threshold/2;
// 			};
// #endif
// 		   };
// 		};
// 	};
// 
// };

return 0;
};

