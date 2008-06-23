/*
 *
 *  Hyperspectral compression program
 *
 * Name:		main.c	
 * Author:		Emmanuel Christophe	
 * Contact:		e.christophe at melaneum.com
 * Description:		Utility functions for hyperspectral image compression
 * Version:		v1.4 - 2008-01	
 * 
 */


#include "main.h"


#ifdef EZW_ARITH
#include "libQccPack.h"
// #define SYMBOL_STREAM_LENGTH 100000000
// #define SYMBOL_STREAM_LENGTH 100
#endif

// #ifdef EZW_ARITH
   #ifdef REFINEMENT
	#ifdef EZW_ARITH_CONTEXT
		#define CONT_REFINE_0 0
		#define CONT_REFINE_1 1
		#define CONT_REFINE_2 2
		#define CONT_REFINE_3 3
		#define CONT_SIGN_GEN_0 4
		#define CONT_SIGN_GEN_1 5
		#define CONT_SIGN_HF_0 6
		#define CONT_SIGN_HF_1 7
		
		#define CONT_NUM 8
	#else
		#define CONT_REFINE 0
		#define CONT_SIGN_GEN 1
		#define CONT_SIGN_HF 2
		
		#define CONT_NUM 3
	#endif
   #else
	#ifdef EZW_ARITH_CONTEXT
		#define CONT_SIGN_GEN_0 0
		#define CONT_SIGN_GEN_1 1
		#define CONT_SIGN_HF_0 2
		#define CONT_SIGN_HF_1 3
	
		#ifndef SIGNED011
// 			#define CONT_SIGN_GEN_FOLLOW1 4 //not necessary for the jump all
// 			#define CONT_NUM 5
			#define CONT_NUM 4 
		#else
			#define CONT_NUM 4
		#endif
	#else
//attention, defini sans verifications: update-> semble marcher...
		#define CONT_SIGN_GEN 0
		#define CONT_SIGN_HF 1	
		#define CONT_NUM 2
	#endif
   #endif

	#define SYMB_POS 0
	#define SYMB_NEG 1
	#define SYMB_IZ 2
	#define SYMB_ZTR 3
	#define SYMB_Z 2

#ifdef REFINEMENT
	#define SYMB_REF_POS 0
	#define SYMB_REF_NEG 1
	#define SYMB_REF_0 2
#endif
	#define NUM_SYMBOLS 4

// #endif


int printCompileOptions(){
	int val=0;
	printf("Compilation options:\n");
#ifdef EZW 
	printf("EZW\n");
#endif
#ifdef DEBUG 
	printf("DEBUG\n");
#endif
#ifdef NEWTREE
	printf("NEWTREE : AT tree\n");
#else
	#ifdef NEWTREE2
		printf("NEWTREE2 : AOT tree\n");
	#else
		printf("-------- : OT tree\n");
	#endif
#endif

#ifdef SIGNED 
	printf("SIGNED\n");
#endif
#ifdef SIGNED011
	printf("SIGNED011\n");
#endif
#ifdef EZWUSEZ 
	printf("EZWUSEZ\n");
#endif
#ifdef EZWREF
	printf("EZWREF\n");
#endif
#ifdef EZWREFAFTER
	printf("EZWREFAFTER\n");
#endif
#ifdef REFINEMENT
	printf("REFINEMENT\n");
#endif
#ifdef EZW_ARITH 
	printf("EZW_ARITH\n");
#endif
#ifdef EZW_ARITH_CONTEXT 
	printf("EZW_ARITH_CONTEXT \n");
#endif
#ifdef EZW_ARITH_RESET_MODEL
	printf("EZW_ARITH_RESET_MODEL\n");
#endif
	printf("--------------------\n");

#ifdef CONT_NUM
	val=CONT_NUM;
	printf("CONT_NUM: %d\n",val);
#endif
#ifdef NUM_SYMBOLS
	val=NUM_SYMBOLS;
	printf("NUM_SYMBOLS: %d\n",val);
#endif

return 0;
}



int ezw_code_signed_c(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param){

int maxquant=(int) (*coder_param.maxquant);
int minquant=(int) (*coder_param.minquant);
long int npix=0;
long int i;
// long int m;
unsigned char bit=255;
// unsigned char bit2=255;
int r=0;
int x,y,l;


long int threshold=0;
int thres_ind=0;

unsigned char *map_zt = NULL;
unsigned char *map_sig = NULL;
#ifdef EZWREFAFTER
unsigned char *map_sig_last = NULL;
#endif

list_struct * list_desc=NULL;
list_el * current_el=NULL;

// #ifndef EZW_ARITH
unsigned char *stream = streamstruct.stream;
unsigned char * count = streamstruct.count;
long int *streamlast = streamstruct.streamlast;
// #endif

#ifdef DEBUG
#ifdef REFINEMENT
long int nref=0;
#endif
long int npos=0;
long int nneg=0;
long int nzeroisol=0;
long int nzerotree=0;
long int nz=0;

#ifdef REFINEMENT
long int nref_jumped=0;
long int nref_1=0;
long int nref_2=0;
#endif
#endif

FILE *data_file;//TODO useless after...

int status=0;

char *image_signed =NULL;
#ifdef OUTPUTSIGNED
FILE *output_file;
#endif

#ifdef COMPUTE_STAT
long int symb_stream_length = 100000000;
int * symbol_stream = (int *) malloc(symb_stream_length*sizeof(int));
long int symb_counter = 0;
FILE *output_file;
unsigned char * streambyte;
#endif

#ifdef TIME
clock_t start, end;
double elapsedcomp;
#endif

#ifdef OUTPUTSIGNED
unsigned char * imsignbyte;
#endif

// #ifdef EZW_ARITH
int symbol = -1;
int num_context=CONT_NUM;
int num_symbols[CONT_NUM]; 


// FILE *output_file; //int status;
QccBitBuffer output_buffer;
QccENTArithmeticModel *model = NULL;
int argc1=1;
char * argv1[1];
argv1[0] = (char *) malloc(256*sizeof(char)); 
strcpy(argv1[0],"spihtcode");
struct stat filestat;

// #endif



image_signed = (char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*NBITS*sizeof(char));
map_zt=(unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));
map_sig= (unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));
#ifdef EZWREFAFTER
map_sig_last= (unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));
#endif

#ifdef EZW_ARITH_CONTEXT
char context_x=0;
char context_y=0;
char context_l=0;
char context_bt=0;
#endif

#ifndef REFINEMENT
#ifndef SIGNED011
int flag_jump_coding=0;
#endif
#endif

#ifdef DEBUG

printCompileOptions();
#endif


npix=imageprop.nsmax * imageprop.nlmax * imageprop.nbmax;

for (i=0;i<npix;i++){
	map_zt[i]=0;
	map_sig[i]=0;
#ifdef EZWREFAFTER
	map_sig_last[i]=0;
#endif
}

#ifdef SIGNED011
fprintf(stderr, "Attention, non compatible avec le flag_jump_coding (-> update: should be OK)\n");
#endif

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
    QccInit(argc1, argv1);
    QccBitBufferInitialize(&output_buffer);

	for (i = 0; i < num_context; i++)
	num_symbols[i] = NUM_SYMBOLS; 
#ifndef REFINEMENT
#ifdef EZW_ARITH_CONTEXT
	num_symbols[CONT_SIGN_GEN_0] = 4;
	num_symbols[CONT_SIGN_GEN_1] = 4;
	num_symbols[CONT_SIGN_HF_0] = 3;
	num_symbols[CONT_SIGN_HF_1] = 3;
#endif
#endif


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
// #endif
}

#ifdef TIME
start = clock();
#endif

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

#ifdef TIME
end = clock();
elapsedcomp = ((double) (end - start)) / CLOCKS_PER_SEC;
printf("Conversion time: %f \n", elapsedcomp);
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

// check maxbitplane (could be one more)
for (i=0;i< npix;i++){
	if (image_signed[NBITS*i+maxquant+1] !=0){
		printf("WARNING: ONE MORE BITPLANE REQUIRED!!! (oups !)\n");
		maxquant++;
		break;
	}
}
if (i == npix){
printf("WARNING: NO EXTRA BITPLANE NEEDED !!! (and none added anyway...) \n");
}

//codage EZW
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){
	
	threshold= 1 << (long int)thres_ind;
	printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);


//refinement pass
#ifndef EZWREFAFTER
#ifdef EZWREF

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){

	#ifdef EZW_ARITH_RESET_MODEL
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
	#endif

#ifndef EZW_ARITH_CONTEXT
	if (QccENTArithmeticSetModelContext(model, CONT_REFINE)) //contexte 0 pour refinement
	{
	QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
	argv1[0]);
	QccErrorExit();
	}
#endif
// #endif
}

// if non adjacent and preceedings non 0, no output (0 for sure)
// 1 1 for 1
// 1 0 for -1
// 0 for 0
	for (i=0;i< npix;i++){//parcours lineaire
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));

		if (map_sig[i] == 1){
#ifndef SIGNED011
if (image_signed[NBITS*i+thres_ind+1] != 0){
//nothing
nref_jumped++;
} else {
#else 
{
#endif
#ifdef EZW_ARITH_CONTEXT
	//contexte
	if ((x>0) && (map_sig[x-1 + (y+l*imageprop.nlmax)*imageprop.nsmax]==1)) {
		context_x = (image_signed[NBITS*(x-1 + (y+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind] != 0);
	} else {
		context_x = 0;
	}
	if ((y>0) && (map_sig[x + (y-1+l*imageprop.nlmax)*imageprop.nsmax]==1)) {
		context_y = (image_signed[NBITS*(x + (y-1+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind] != 0);
	} else {
		context_y = 0;
	}
	if ((l>0) && (map_sig[x + (y+(l-1)*imageprop.nlmax)*imageprop.nsmax]==1)) {
		context_l = (image_signed[NBITS*(x + (y+(l-1)*imageprop.nlmax)*imageprop.nsmax)+thres_ind] != 0);
	} else {
		context_l = 0;
	}
	if ((context_x == 0) && (context_y == 0) && (context_l == 0) ){
		if (QccENTArithmeticSetModelContext(model, CONT_REFINE_0)){ //contexte 0 pour refinement
		QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
		QccErrorExit();
		}
	} else {
		context_bt = image_signed[NBITS*(x + (y+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind+1];
		if (context_bt == -1) {
			if (QccENTArithmeticSetModelContext(model, CONT_REFINE_1)){ //contexte 0 pour refinement
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
			QccErrorExit();
			}
		}
		if (context_bt == 0) {
			if (QccENTArithmeticSetModelContext(model, CONT_REFINE_2)){ //contexte 0 pour refinement
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
			QccErrorExit();
			}
		}
		if (context_bt == 1) {
			if (QccENTArithmeticSetModelContext(model, CONT_REFINE_3)){ //contexte 0 pour refinement
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
			QccErrorExit();
			}
		}
	}
#endif
		   if (image_signed[NBITS*i+thres_ind] == 1){
			bit = 1;
			bit2 = 1;
			nref_2++;
		   }
		    if (image_signed[NBITS*i+thres_ind] == -1){
			bit = 1;
			bit2 = 0;
			nref_2++;
		   }
		    if (image_signed[NBITS*i+thres_ind] == 0){
			bit = 0;
			nref_1++;
		   }

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
			symbol = -1;//to make sure we are not going to encode rubish
			if ((bit == 1) && (bit2 == 1)) symbol = SYMB_REF_POS;
			if ((bit == 1) && (bit2 == 0)) symbol = SYMB_REF_NEG;
			if (bit == 0) symbol = SYMB_REF_0;

			if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer)){
				QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()", argv1[0]);
				QccErrorExit();
			}
			#ifdef DEBUG3  
			printf("%d",symbol);  
			#endif
// #else
} else {
			add_to_stream(stream, count, (int) bit, streamlast);
			if (bit != 0) add_to_stream(stream, count, (int) bit2, streamlast);
// #endif
}

		   nref++;
}
		};
	};
#endif
#endif
	
	//significance pass
if (*(coder_param.flag_arith) == 1){
	#ifdef EZW_ARITH_RESET_MODEL
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
	#endif
}

	for (i=0;i< npix;i++){//parcours lineaire
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));


		if ( (map_zt[i] == 0) && (map_sig[i] ==0) ){
#ifndef REFINEMENT
#ifndef SIGNED011
			flag_jump_coding=0;
#endif
#endif
		//This point is NOT part of a zerotree already and is NOT processed during refinement
/*Selection du contexte*/
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
			symbol = -1;//to make sure we are not going to encode rubish
#ifdef EZW_ARITH_CONTEXT
			if (x>0) {
				context_x = map_sig[(x-1 + (y+l*imageprop.nlmax)*imageprop.nsmax)];
			} else {
				context_x = 0;
			}
			if (y>0) {
				context_y = map_sig[(x + (y-1+l*imageprop.nlmax)*imageprop.nsmax)];
			} else {
				context_y = 0;
			}
			if (l>0) {
				context_l = map_sig[(x + (y+(l-1)*imageprop.nlmax)*imageprop.nsmax)];
			} else {
				context_l = 0;
			}
#ifndef REFINEMENT
			context_bt = image_signed[NBITS*(x + (y+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind+1];
#endif

#endif

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
#ifdef EZW_ARITH_CONTEXT
#ifndef REFINEMENT
#ifndef SIGNED011
			if (context_bt == 0){
#endif
#endif
				if ((context_x == 0) && (context_y == 0) && (context_l == 0) ){
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_HF_0)){
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
					QccErrorExit();
					}
				} else {
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_HF_1)){
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
					QccErrorExit();
					}
				}
#ifndef REFINEMENT
#ifndef SIGNED011
			} else {
				flag_jump_coding=1;
			}
// #else
// }
#endif
#endif

#else
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_HF)) //contexte 2 pour POS,NEG,Z
					{
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
					argv1[0]);
					QccErrorExit();
					}
#endif
				} else {
#ifdef EZW_ARITH_CONTEXT
#ifndef REFINEMENT
#ifndef SIGNED011
			if (context_bt == 0){
#endif
#endif
				if ((context_x == 0) && (context_y == 0) && (context_l == 0) ){
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN_0)){
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
					QccErrorExit();
					}
				} else {
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN_1)){
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
					QccErrorExit();
					}
				}
#ifndef REFINEMENT
#ifndef SIGNED011
			} else {
				flag_jump_coding=1;
			}
// #else
// }
#endif
#endif
#else
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN)) //contexte 1 pour POS, NEG, IZ, ZTR
					{
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
					argv1[0]);
					QccErrorExit();
					}
#endif
				}

				
#else
			if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN)) //contexte 1 pour POS, NEG, IZ, ZTR
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
			argv1[0]);
			QccErrorExit();
			}
#endif
// #endif
}
/*Fin de selection du contexte*/


// 			if (image[i] >= threshold){//POS
#ifndef REFINEMENT
#ifndef SIGNED011
if (flag_jump_coding ==0){
#else
{
#endif
#else
{
#endif
			if (image_signed[NBITS*i+thres_ind] == 1){//POS
				map_sig[i] = 1;
#ifdef EZWREFAFTER
				map_sig_last[i] = 1;
#endif
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
				symbol = SYMB_POS;

				if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
				{
				QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
				argv1[0]);
				QccErrorExit();
				}
				#ifdef DEBUG3  
				printf("%d",symbol);  
				#endif
// #else
} else {

				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
// #endif
}
				#ifdef DEBUG
				npos++;
				#endif
// 				printf("Adding pixel:%d %d %d\n",x,y,l);
				#ifdef COMPUTE_STAT
				symbol_stream[symb_counter]=1;
				symb_counter++;
				#endif
			};
// 			if (image[i] <= -threshold){// NEG
			if (image_signed[NBITS*i+thres_ind] == -1){//NEG
				map_sig[i] = 1;
#ifdef EZWREFAFTER
				map_sig_last[i] = 1;
#endif
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
				symbol = SYMB_NEG;
				if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
				{
				QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
				argv1[0]);
				QccErrorExit();
				}
				#ifdef DEBUG3  
				printf("%d",symbol);  
				#endif
// #else
} else {
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				bit = 0;
				add_to_stream(stream, count, (int) bit, streamlast);
// #endif
}
				#ifdef DEBUG
				nneg++;
				#endif
				#ifdef COMPUTE_STAT
				symbol_stream[symb_counter]=2;
				symb_counter++;
				#endif
			};
// 			if ((image[i] < threshold) &&  (image[i] > -threshold)){ // IZ ou ZT
			if (image_signed[NBITS*i+thres_ind] == 0){// IZ ou ZT
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
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
				symbol = SYMB_Z;
				if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
				{
				QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
				argv1[0]);
				QccErrorExit();
				}
				#ifdef DEBUG3  
				printf("%d",symbol);  
				#endif
// #else
} else {
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
// #endif
}
					#ifdef DEBUG
					nz++;
					#endif
			   } else {
#else
			   {
#endif
				list_desc=NULL;//should be the case if freed properly before
				r=spat_spec_desc_ezw_signed((pixel_struct) {x,y,l}, list_desc, 0, image_signed, thres_ind, map_sig);
				//une modification est-elle necessaire pour tenir compte des elements appartenat deja aux ZT ? NOPE
				if ((r ==-1)||(r == 0)){// zero isole (early ending ou pas de desc
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
				symbol = SYMB_IZ;
				if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
				{
				QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
				argv1[0]);
				QccErrorExit();
				}
				#ifdef DEBUG3  
				printf("%d",symbol);  
				#endif
// #else
} else {
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
// #endif
}
					#ifdef DEBUG
					nzeroisol++;
					#endif	
					#ifdef COMPUTE_STAT
					symbol_stream[symb_counter]=0;
					symb_counter++;
					#endif
				} else {// we have a zerotree
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
				symbol = SYMB_ZTR;
				if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
				{
				QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
				argv1[0]);
				QccErrorExit();
				}
				#ifdef DEBUG3  
				printf("%d",symbol);  
				#endif
// #else
} else {
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
					bit = 1;
					add_to_stream(stream, count, (int) bit, streamlast);
// #endif
}
					//do not forget to update map_zt
					list_desc=list_init();
					r=spat_spec_desc_ezw_signed((pixel_struct) {x,y,l}, list_desc, 1, image_signed, thres_ind, map_sig);//sans early ending...
					map_zt[i]=1;
					current_el=first_el(list_desc);
					while (current_el != NULL){
						map_zt[trans_pixel(current_el->pixel)]=1;
						current_el=next_el(list_desc);
					};
					#ifdef DEBUG
					nzerotree++;
					#endif
					#ifdef COMPUTE_STAT
					symbol_stream[symb_counter]=3;
					symb_counter++;
					#endif
				};
				list_free(list_desc);
				//ne pas oublier de liberer la liste
			   }
			};
			#ifdef COMPUTE_STAT
			if (symb_counter > symb_stream_length) fprintf(stderr,"Symbol stream overflow\n");
			#endif
		}//le flag_jump_coding
		};
	};
	

//refinement pass
#ifdef EZWREFAFTER
#ifdef EZWREF

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
	#ifdef EZW_ARITH_RESET_MODEL
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
	#endif

#ifndef EZW_ARITH_CONTEXT
	if (QccENTArithmeticSetModelContext(model, CONT_REFINE)) //contexte 0 pour refinement
	{
	QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
	argv1[0]);
	QccErrorExit();
	}
#endif
// #endif
}

// if non adjacent and preceedings non 0, no output (0 for sure)
// 1 1 for 1
// 1 0 for -1
// 0 for 0
	for (i=0;i< npix;i++){//parcours lineaire
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));
		if ((map_sig[i] == 1) && (map_sig_last[i] == 0)){
#ifndef SIGNED011
if (image_signed[NBITS*i+thres_ind+1] != 0){
//nothing
nref_jumped++;
} else {
#else 
{
#endif
#ifdef EZW_ARITH_CONTEXT
	//contexte
	if (x>0) {
		context_x = (image_signed[NBITS*(x-1 + (y+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind] != 0);
	} else {
		context_x = 0;
	}
	if (y>0) {
		context_y = (image_signed[NBITS*(x + (y-1+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind] != 0);
	} else {
		context_y = 0;
	}
	if (l>0) {
		context_l = (image_signed[NBITS*(x + (y+(l-1)*imageprop.nlmax)*imageprop.nsmax)+thres_ind] != 0);
	} else {
		context_l = 0;
	}
	if ((context_x == 0) && (context_y == 0) && (context_l == 0) ){
		if (QccENTArithmeticSetModelContext(model, CONT_REFINE_0)){ //contexte 0 pour refinement
		QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
		QccErrorExit();
		}
	} else {
		context_bt = image_signed[NBITS*(x + (y+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind+1];
		if (context_bt == -1) {
			if (QccENTArithmeticSetModelContext(model, CONT_REFINE_1)){ //contexte 0 pour refinement
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
			QccErrorExit();
			}
		}
		if (context_bt == 0) {
			if (QccENTArithmeticSetModelContext(model, CONT_REFINE_2)){ //contexte 0 pour refinement
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
			QccErrorExit();
			}
		}
		if (context_bt == 1) {
			if (QccENTArithmeticSetModelContext(model, CONT_REFINE_3)){ //contexte 0 pour refinement
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
			QccErrorExit();
			}
		}
	}
#endif
		   if (image_signed[NBITS*i+thres_ind] == 1){
			bit = 1;
			bit2 = 1;
			nref_2++;
		   }
		    if (image_signed[NBITS*i+thres_ind] == -1){
			bit = 1;
			bit2 = 0;
			nref_2++;
		   }
		    if (image_signed[NBITS*i+thres_ind] == 0){
			bit = 0;
			nref_1++;
		   }

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
			symbol = -1;//to make sure we are not going to encode rubish
			if ((bit == 1) && (bit2 == 1)) symbol = SYMB_REF_POS;
			if ((bit == 1) && (bit2 == 0)) symbol = SYMB_REF_NEG;
			if (bit == 0) symbol = SYMB_REF_0;

			if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer)){
				QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()", argv1[0]);
				QccErrorExit();
			}
			#ifdef DEBUG3  
			printf("%d",symbol);  
			#endif
// #else
} else {
			add_to_stream(stream, count, (int) bit, streamlast);
			if (bit != 0) add_to_stream(stream, count, (int) bit2, streamlast);
				
// #endif
}

		   nref++;
}
		};
	};
#endif
#endif


	//on remet la map_zt a zero...
	// et dans le cas du signed AUSSI la map_sig
	for (i=0; i<npix; i++){
		map_zt[i]=0;
#ifndef EZWREF
		map_sig[i]=0;//on ne remet pas a zero si on a l'etape de refinement
#endif
#ifdef EZWREFAFTER
		map_sig_last[i]=0;//on remet systematiquement a zero
#endif
	};

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
outputsize[thres_ind]=output_buffer.bit_cnt; //TODO check if correct
// #else
} else {
outputsize[thres_ind]=(*streamlast)*8 + (*count);
// #endif
}

#ifdef DEBUG
// #ifndef EZW_ARITH
if (*(coder_param.flag_arith) == 0){
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("Size in bit: %ld \n", *streamlast*8+*count);
// #endif
}
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
printf("Size in bit: %d \n", output_buffer.bit_cnt);
// #endif
}
#ifdef EZWREF
printf("nref: %ld \n",nref);
printf("nref_jumped: %ld \n",nref_jumped);
printf("nref_1: %ld \n",nref_1);
printf("nref_2: %ld \n",nref_2);
#endif
printf("POS: %ld \n",npos);
printf("NEG: %ld \n",nneg);
printf("IZ:  %ld \n",nzeroisol);
printf("ZTR: %ld \n",nzerotree);
printf("Z: %ld \n",nz);
printf("-------------------------\n");
#endif

};

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){ 
outputsize[thres_ind]=output_buffer.bit_cnt;
// #else
} else {
outputsize[0]=(*streamlast)*8 + (*count);
// #endif
}

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
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

   //copie pour assurer la compatibilite

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

   outputsize[0]=(*streamstruct.streamlast)*8 + (*streamstruct.count);
// #endif
}

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

#ifdef COMPUTE_STAT
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


int ezw_decode_signed_c(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param)
{


int maxquant=(int) (*coder_param.maxquant);
int minquant=(int) (*coder_param.minquant);
long int npix=0;
long int i,m;
// long int value_pix=0;
unsigned char bit=255;
unsigned char bit2=255;
int r=0;
int x,y,l;

#ifdef DEBUG
#ifdef REFINEMENT
long int nref=0;
#endif
long int npos=0;
long int nneg=0;
long int nzeroisol=0;
long int nzerotree=0;	
long int nz=0;
	
#ifdef REFINEMENT
long int nref_jumped=0;
long int nref_1=0;
long int nref_2=0;
#endif
#endif

long int threshold=0;
int thres_ind=0;

unsigned char *map_zt = NULL;
unsigned char *map_sig = NULL;
#ifdef EZWREFAFTER
unsigned char *map_sig_last = NULL;
#endif

// #ifdef EZW_ARITH
int symbol = -1;
int num_context= CONT_NUM;
int num_symbols[CONT_NUM]; 
// long int symb_counter = 0;
// unsigned char * streambyte;
// FILE *input_file; //int status;
QccBitBuffer input_buffer;
QccENTArithmeticModel *model = NULL;
int * argc1=NULL;
char * argv1[1];
argv1[0] = (char *) malloc(256*sizeof(char)); 
strcpy(argv1[0],"spihtcode");
// struct stat filestat;
FILE *data_file;
int status=0;
// #endif

list_struct * list_desc=NULL;
list_el * current_el=NULL;

char * image_signed = (char *) malloc (imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*NBITS);

// #ifndef EZW_ARITH
unsigned char *stream = streamstruct.stream;
unsigned char * count = streamstruct.count;
long int *streamlast = streamstruct.streamlast;
// #endif

map_zt=(unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));
map_sig= (unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));
#ifdef EZWREFAFTER
map_sig_last= (unsigned char *) malloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax*sizeof(unsigned char));
#endif

#ifdef EZW_ARITH_CONTEXT
char context_x=0;
char context_y=0;
char context_l=0;
char context_bt=0;
#endif

#ifndef REFINEMENT
#ifndef SIGNED011
int flag_jump_coding=0;
#endif
#endif

#ifdef DEBUG
printCompileOptions();
#endif

for (i=0;i<npix;i++){
	map_zt[i]=0;
	map_sig[i]=0;//pourquoi etait-ce supprime ?
#ifdef EZWREFAFTER
	map_sig_last[i]=0;
#endif
}

for (m=0; m<imageprop.nsmax*imageprop.nlmax*imageprop.nbmax*NBITS; m++){
	image_signed[m]=0;
}

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
   //copy pour compatibilite   
   data_file = fopen("tmp", "w");
   status = fwrite(&streamstruct.stream[(*streamstruct.headerlength)/8], 1, *outputsize/8 - (*streamstruct.headerlength)/8, data_file);
   status=fclose(data_file);

    QccInit(*argc1, argv1);
    QccBitBufferInitialize(&input_buffer);

    for (i = 0; i < num_context; i++)
	num_symbols[i] = NUM_SYMBOLS; 
#ifndef REFINEMENT
#ifdef EZW_ARITH_CONTEXT
	num_symbols[CONT_SIGN_GEN_0] = 4;
	num_symbols[CONT_SIGN_GEN_1] = 4;
	num_symbols[CONT_SIGN_HF_0] = 3;
	num_symbols[CONT_SIGN_HF_1] = 3;
#endif
#endif

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
// #endif
}

npix=imageprop.nsmax * imageprop.nlmax * imageprop.nbmax;


printf("WARNING: not adding bitplane... check if correct\n");

//decodage EZW
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){
	
	threshold= 1 << (long int)thres_ind;
	printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
	
	

	//refinement pass
#ifndef EZWREFAFTER
#ifdef EZWREF
// if non adjacent and preceedings non 0, no output (0 for sure)
// 1 1 for 1
// 1 0 for -1
// 0 for 0

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){

	#ifdef EZW_ARITH_RESET_MODEL
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
	#endif

#ifndef EZW_ARITH_CONTEXT
	if (QccENTArithmeticSetModelContext(model, CONT_REFINE)) //contexte 0 pour refinement
	{
	QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
	argv1[0]);
	QccErrorExit();
	}
#endif
// #endif
}
	for (i=0;i< npix;i++){//parcours lineaire
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));
		if  (map_sig[i] == 1){
#ifndef SIGNED011
if (image_signed[NBITS*i+thres_ind+1] != 0){
   image_signed[NBITS*i+thres_ind] = 0;
   nref_jumped++;
} else {
#else 
{
#endif

#ifdef EZW_ARITH_CONTEXT
	//contexte
	if ((x>0) && (map_sig[x-1 + (y+l*imageprop.nlmax)*imageprop.nsmax]==1)){
		context_x = (image_signed[NBITS*(x-1 + (y+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind] != 0);
	} else {
		context_x = 0;
	}
	if ((y>0) && (map_sig[x + (y-1+l*imageprop.nlmax)*imageprop.nsmax]==1)) {
		context_y = (image_signed[NBITS*(x + (y-1+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind] != 0);
	} else {
		context_y = 0;
	}
	if ((l>0) && (map_sig[x + (y+(l-1)*imageprop.nlmax)*imageprop.nsmax]==1)) {
		context_l = (image_signed[NBITS*(x + (y+(l-1)*imageprop.nlmax)*imageprop.nsmax)+thres_ind] != 0);
	} else {
		context_l = 0;
	}
	if ((context_x == 0) && (context_y == 0) && (context_l == 0) ){
		if (QccENTArithmeticSetModelContext(model, CONT_REFINE_0)){ //contexte 0 pour refinement
		QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
		QccErrorExit();
		}
	} else {
		context_bt = image_signed[NBITS*(x + (y+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind+1];
		if (context_bt == -1) {
			if (QccENTArithmeticSetModelContext(model, CONT_REFINE_1)){ //contexte 0 pour refinement
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
			QccErrorExit();
			}
		}
		if (context_bt == 0) {
			if (QccENTArithmeticSetModelContext(model, CONT_REFINE_2)){ //contexte 0 pour refinement
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
			QccErrorExit();
			}
		}
		if (context_bt == 1) {
			if (QccENTArithmeticSetModelContext(model, CONT_REFINE_3)){ //contexte 0 pour refinement
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
			QccErrorExit();
			}
		}
	}
#endif

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
			if (input_buffer.bit_cnt < *outputsize-(*streamstruct.headerlength)){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
			#ifdef DEBUG3  
			printf("%d",symbol);  
			#endif
			if (symbol == SYMB_REF_0){
				image_signed[NBITS*i+thres_ind] = 0;
				nref_1;
		   	}
			if (symbol == SYMB_REF_POS){
				image_signed[NBITS*i+thres_ind] = 1;
				nref_2;
			}
			if (symbol == SYMB_REF_NEG){
				image_signed[NBITS*i+thres_ind] = -1;
				nref_2;
			}

// #else
} else {

		   if ((*streamlast)*8+ (*count) <= *outputsize){
			bit= read_from_stream(stream, count, streamlast);
		   } else break;
		   if (bit == 0){
			image_signed[NBITS*i+thres_ind] = 0;
		   } else {
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit= read_from_stream(stream, count, streamlast);
		   	} else break;
			if (bit == 1){
				image_signed[NBITS*i+thres_ind] = 1;
			} else {
				image_signed[NBITS*i+thres_ind] = -1;
			}
		   }
// #endif
}
		   nref++;
}
		};
	};
#endif
#endif


// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
	if (input_buffer.bit_cnt >= *outputsize-(*streamstruct.headerlength)) break;
// #else
} else {
	if ((*streamlast)*8+ (*count) > *outputsize) break;
// #endif
}

	//significance pass
if (*(coder_param.flag_arith) == 1){
	#ifdef EZW_ARITH_RESET_MODEL
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
	#endif
}


	for (i=0;i< npix;i++){//parcours lineaire
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));


		if ((map_zt[i] == 0) && (map_sig[i] ==0)){
#ifndef REFINEMENT
#ifndef SIGNED011
			flag_jump_coding=0;
#endif
#endif
/*Selection du contexte*/
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){

#ifdef EZW_ARITH_CONTEXT
			if (x>0) {
				context_x = map_sig[(x-1 + (y+l*imageprop.nlmax)*imageprop.nsmax)];
			} else {
				context_x = 0;
			}
			if (y>0) {
				context_y = map_sig[(x + (y-1+l*imageprop.nlmax)*imageprop.nsmax)];
			} else {
				context_y = 0;
			}
			if (l>0) {
				context_l = map_sig[(x + (y+(l-1)*imageprop.nlmax)*imageprop.nsmax)];
			} else {
				context_l = 0;
			}
// #endif //deplacement 02-08-2006
#ifndef REFINEMENT
			context_bt = image_signed[NBITS*(x + (y+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind+1];
#endif
#endif
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

#ifdef EZW_ARITH_CONTEXT
#ifndef REFINEMENT
#ifndef SIGNED011
			if (context_bt == 0){
#endif
#endif
				if ((context_x == 0) && (context_y == 0) && (context_l == 0) ){
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_HF_0)){
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
					QccErrorExit();
					}
				} else {
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_HF_1)){
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
					QccErrorExit();
					}
				}
#ifndef REFINEMENT
#ifndef SIGNED011
			} else {
				flag_jump_coding=1;
			}
// #else
// }
#endif
#endif
#else				
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_HF)) //contexte 2 pour POS,NEG,Z
					{
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
					argv1[0]);
					QccErrorExit();
					}
#endif
				} else {
#ifdef EZW_ARITH_CONTEXT
#ifndef REFINEMENT
#ifndef SIGNED011
			if (context_bt == 0){
#endif
#endif
				if ((context_x == 0) && (context_y == 0) && (context_l == 0) ){
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN_0)){
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
					QccErrorExit();
					}
				} else {
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN_1)){
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
					QccErrorExit();
					}
				}
#ifndef REFINEMENT
#ifndef SIGNED011
			} else {
				flag_jump_coding=1;
			}
// #else
// }
#endif
#endif
#else
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN)) //contexte 1 pour POS, NEG, IZ, ZTR
					{
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
					argv1[0]);
					QccErrorExit();
					}
#endif
				}
				

#else
		if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN)) //contexte 1 pour POS, NEG, IZ, ZTR
		{
		QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
		argv1[0]);
		QccErrorExit();
		}
#endif
// #endif
}
/*Fin de selection du contexte*/
// #ifdef EZW_ARITH
// if (*(coder_param.flag_arith) == 1){ //on le met plus bas
	#ifndef REFINEMENT
		#ifndef SIGNED011
		if (flag_jump_coding ==0){
		#else
		{
		#endif
	#else
	{
	#endif
		if (*(coder_param.flag_arith) == 1){
			if (input_buffer.bit_cnt < *outputsize-(*streamstruct.headerlength)){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
			#ifdef DEBUG3  
			printf("%d",symbol);  
			#endif
			bit=255;
			bit2=255;	
			if (symbol == SYMB_POS){bit = 1; bit2=1;}//POS
			if (symbol == SYMB_NEG){bit = 1; bit2=0;}//NEG
			if (symbol == SYMB_IZ){bit = 0; bit2=0;}//IZ
			if (symbol == SYMB_ZTR){bit = 0; bit2=1;}//ZTR
			if (symbol == SYMB_Z){bit = 0; bit2=0;}//Z
// 		}//
// #else
		} else {
// 			{
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit= read_from_stream(stream, count, streamlast);
			} else break;
		// #endif
		}
			if (bit == 1) {
// #ifndef EZW_ARITH
if (*(coder_param.flag_arith) == 0){
			 if ((*streamlast)*8+ (*count) <= *outputsize){
				bit2= read_from_stream(stream, count, streamlast);
			 } else break;
// #endif
}
			  if (bit2 == 1){//POS
				map_sig[i] = 1;
#ifdef EZWREFAFTER
				map_sig_last[i] = 1;
#endif
				image_signed[NBITS*i+thres_ind] = 1;
				#ifdef DEBUG
				npos++;
				#endif
			  };
			  if (bit2 == 0){//NEG
				map_sig[i] = 1;
#ifdef EZWREFAFTER
				map_sig_last[i] = 1;
#endif
				image_signed[NBITS*i+thres_ind] = -1;
				#ifdef DEBUG
				nneg++;
				#endif
			  };
			} else {
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

				#ifdef DEBUG
				nz++;//rien a faire
				#endif
			  } else {
#else
			{
#endif

// #ifndef EZW_ARITH
if (*(coder_param.flag_arith) == 0){
				if ((*streamlast)*8+ (*count) <= *outputsize){
					bit2= read_from_stream(stream, count, streamlast);
				} else break;
// #endif
}

				if (bit2 == 0){//IZ
					//nothing to do
					#ifdef DEBUG
					nzeroisol++;
					#endif
				};
				if (bit2 == 1){//ZT
					list_desc=list_init();
					r=spat_spec_desc_ezw_signed((pixel_struct) {x,y,l}, list_desc, 1, image_signed, thres_ind, map_sig);//sans early ending...
					map_zt[i]=1;
					current_el=first_el(list_desc);
					while (current_el != NULL){
						map_zt[trans_pixel(current_el->pixel)]=1;
						current_el=next_el(list_desc);
					};
					list_free(list_desc);
					#ifdef DEBUG
					nzerotree++;
					#endif
				};
			   }

			}
		}//le flag_jump_coding
		};
	};

#ifdef EZWREFAFTER
#ifdef EZWREF
// if non adjacent and preceedings non 0, no output (0 for sure)
// 1 1 for 1
// 1 0 for -1
// 0 for 0

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
	#ifdef EZW_ARITH_RESET_MODEL
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
	#endif

#ifndef EZW_ARITH_CONTEXT
	if (QccENTArithmeticSetModelContext(model, CONT_REFINE)) //contexte 0 pour refinement
	{
	QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
	argv1[0]);
	QccErrorExit();
	}
#endif
// #endif
}
	for (i=0;i< npix;i++){//parcours lineaire
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));
		if ((map_sig[i] == 1) && (map_sig_last[i] == 0)){
#ifndef SIGNED011
if (image_signed[NBITS*i+thres_ind+1] != 0){
   image_signed[NBITS*i+thres_ind] = 0;
   nref_jumped++;
} else {
#else 
{
#endif

#ifdef EZW_ARITH_CONTEXT
	//contexte
	if (x>0) {
		context_x = (image_signed[NBITS*(x-1 + (y+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind] != 0);
	} else {
		context_x = 0;
	}
	if (y>0) {
		context_y = (image_signed[NBITS*(x + (y-1+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind] != 0);
	} else {
		context_y = 0;
	}
	if (l>0) {
		context_l = (image_signed[NBITS*(x + (y+(l-1)*imageprop.nlmax)*imageprop.nsmax)+thres_ind] != 0);
	} else {
		context_l = 0;
	}
	if ((context_x == 0) && (context_y == 0) && (context_l == 0) ){
		if (QccENTArithmeticSetModelContext(model, CONT_REFINE_0)){ //contexte 0 pour refinement
		QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
		QccErrorExit();
		}
	} else {
		context_bt = image_signed[NBITS*(x + (y+l*imageprop.nlmax)*imageprop.nsmax)+thres_ind+1];
		if (context_bt == -1) {
			if (QccENTArithmeticSetModelContext(model, CONT_REFINE_1)){ 
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
			QccErrorExit();
			}
		}
		if (context_bt == 0) {
			if (QccENTArithmeticSetModelContext(model, CONT_REFINE_2)){
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
			QccErrorExit();
			}
		}
		if (context_bt == 1) {
			if (QccENTArithmeticSetModelContext(model, CONT_REFINE_3)){
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",argv1[0]);
			QccErrorExit();
			}
		}
	}
#endif

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
			if (input_buffer.bit_cnt < *outputsize-(*streamstruct.headerlength)){
				if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
				{
				QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
				argv1[0]);
				QccErrorExit();
				}
			} else break;
			#ifdef DEBUG3  
			printf("%d",symbol);  
			#endif
			if (symbol == SYMB_REF_0){
				image_signed[NBITS*i+thres_ind] = 0;
				nref_1;
		   	}
			if (symbol == SYMB_REF_POS){
				image_signed[NBITS*i+thres_ind] = 1;
				nref_2;
			}
			if (symbol == SYMB_REF_NEG){
				image_signed[NBITS*i+thres_ind] = -1;
				nref_2;
			}

// #else
} else {

		   if ((*streamlast)*8+ (*count) <= *outputsize){
			bit= read_from_stream(stream, count, streamlast);
		   } else break;
		   if (bit == 0){
			image_signed[NBITS*i+thres_ind] = 0;
		   } else {
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit= read_from_stream(stream, count, streamlast);
		   	} else break;
			if (bit == 1){
				image_signed[NBITS*i+thres_ind] = 1;
			} else {
				image_signed[NBITS*i+thres_ind] = -1;
			}
		   }
// #endif
}
		   nref++;
}
		};
	};
#endif
#endif




	//on remet la map_zt a zero...
	for (i=0; i<npix; i++){
		map_zt[i]=0;
#ifndef EZWREF
		map_sig[i]=0;
#endif
#ifdef EZWREFAFTER
		map_sig_last[i]=0;//on remet systematiquement a zero
#endif
	};

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
	if (input_buffer.bit_cnt >= *outputsize-(*streamstruct.headerlength)) break;
// #else
} else {
	if ((*streamlast)*8+ (*count) > *outputsize) break;
// #endif
}

// #ifndef EZW_ARITH
if (*(coder_param.flag_arith) == 0){
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("Size in bit: %ld \n", *streamlast*8+*count);
// #endif
}
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
printf("Size in bit: %d \n", input_buffer.bit_cnt);
// #endif
}
#ifdef EZWREF
printf("nref: %ld \n",nref);
printf("nref_jumped: %ld \n",nref_jumped);
printf("nref_1: %ld \n",nref_1);
printf("nref_2: %ld \n",nref_2);
#endif
#ifdef DEBUG
printf("POS: %ld \n",npos);
printf("NEG: %ld \n",nneg);
printf("IZ:  %ld \n",nzeroisol);
printf("ZTR: %ld \n",nzerotree);
printf("Z: %ld \n",nz);
#endif
printf("-------------------------\n");

};

// #ifdef EZW_ARITH

if (*(coder_param.flag_arith) == 1){
    if (QccBitBufferEnd(&input_buffer))
    {
    QccErrorAddMessage("%s: Error calling QccBitBufferEnd()",
    argv1[0]);
    QccErrorExit();
    }

    QccENTArithmeticFreeModel(model);
// #endif
}

for (i=0;i< npix;i++){
	image[i] = value_signed(&(image_signed[NBITS*i]));
}


free(image_signed);

return 0;
};



