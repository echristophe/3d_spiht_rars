/*
 *
 *  Hyperspectral compression program
 *
 * Name:		main.c
 * Author:		Emmanuel Christophe	
 * Contact:		e.christophe at melaneum.com
 * Description:		Utility functions for hyperspectral image compression
 * Version:		v1.1 - 2006-10	
 * 
 */


#include "main.h"

#ifdef EZW_ARITH
#include "libQccPack.h"
#endif

// #ifdef EZW_ARITH

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
// #endif

int ezw_code_c(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param){

int maxquant=(int) (*coder_param.maxquant);
int minquant=(int) (*coder_param.minquant);
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

// #ifdef EZW_ARITH
int symbol = -1;

int num_context=CONT_NUM;
int num_symbols[CONT_NUM]; 

// long int symb_counter = 0;

// unsigned char * streambyte;
// FILE *output_file; //int status;
QccBitBuffer output_buffer;
QccENTArithmeticModel *model = NULL;
int argc1=1;
char * argv1[1];
argv1[0] = (char *) malloc(256*sizeof(char)); 
strcpy(argv1[0],"tmp");
struct stat filestat;

// #endif

list_struct * list_desc=NULL;
list_el * current_el=NULL;

unsigned char *stream = streamstruct.stream;
unsigned char * count = streamstruct.count;
long int *streamlast = streamstruct.streamlast;


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

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
    QccInit(argc1, argv1);
    QccBitBufferInitialize(&output_buffer);

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
}
// #endif

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
	


#ifndef EZWNOREF
#ifndef EZWREFAFTER
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
	#ifdef EZW_ARITH_RESET_MODEL

	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
	#endif

	if (QccENTArithmeticSetModelContext(model, CONT_REFINE)) //context for refinement
	{
	QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
	argv1[0]);
	QccErrorExit();
	}
}
// #endif
	//refinement pass
	for (i=0;i< npix;i++){
		if (map_sig[i] == 1){
			value_pix=image[i];
			bit = get_bit(value_pix, thres_ind);

// #ifdef EZW_ARITH
			if (*(coder_param.flag_arith) == 1){
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
// #else
			} else {
			add_to_stream(stream, count, (int) bit, streamlast);
// #endif
			}
			nref++;
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
	for (i=0;i< npix;i++){

		if ( (map_zt[i] == 0) && (map_sig[i] ==0) ){
		//This point is NOT part of a zerotree already and is NOT processed during refinement
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));
/*Selection du contexte*/
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
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
				
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_HF)) //context for POS,NEG,Z
					{
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
					argv1[0]);
					QccErrorExit();
					}
				} else {
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN)) //context for POS, NEG, IZ, ZTR
					{
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
					argv1[0]);
					QccErrorExit();
					}
				}
				
#else
			if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN)) //context for POS, NEG, IZ, ZTR
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
			argv1[0]);
			QccErrorExit();
			}
#endif
// #endif
}
/*Fin de selection du contexte*/
			if (image[i] >= threshold){//POS
				map_sig[i] = 1;
#ifdef EZWNOREF
				image[i] -= threshold;
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
// #else
} else {
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
// #endif
}
				npos++;

			};
			if (image[i] <= -threshold){// NEG
				map_sig[i] = 1;
#ifdef EZWNOREF
				image[i] += threshold;
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
// #else
} else {
				bit = 1;
				add_to_stream(stream, count, (int) bit, streamlast);
				bit = 0;
				add_to_stream(stream, count, (int) bit, streamlast);
// 				printf("NEG ");
// #endif 
}
				nneg++;
			};
			if (map_sig[i] == 0){
#ifdef EZWUSEZ
	#ifdef NEWTREE 
	//spat-tree
				if ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) 
	#else
		#ifdef NEWTREE2 
		//spat-tree
				if ((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) 
		#else
	//3D-tree
				if (((x >= imageprop.nsmax / 2) || (y >= imageprop.nlmax / 2)) 
					&& (l >= imageprop.nbmax / 2) )
		#endif
	#endif
				{
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
				symbol = SYMB_Z;
				if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
				{
				QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
				argv1[0]);
				QccErrorExit();
				}
// #else
} else {
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
// #endif
}
					nz++;	

				} else {
#else
				{
#endif
				list_desc=NULL;//should be the case if freed properly before
				r=spat_spec_desc_ezw((pixel_struct) {x,y,l}, list_desc, 0, image,thres_ind, map_sig);
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
// #else
} else {
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
// #endif
}
					nzeroisol++;			
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
// #else
} else {
					bit = 0;
					add_to_stream(stream, count, (int) bit, streamlast);
					bit = 1;
					add_to_stream(stream, count, (int) bit, streamlast);
// #endif
}
					list_desc=list_init();
					r=spat_spec_desc_ezw((pixel_struct) {x,y,l}, list_desc, 1, image, thres_ind, map_sig);
					map_zt[i]=1;
					current_el=first_el(list_desc);
					while (current_el != NULL){
						map_zt[trans_pixel(current_el->pixel)]=1;
						current_el=next_el(list_desc);
					};
					nzerotree++;
				};
				list_free(list_desc);
				}
			};

		};
	};
	
	//on remet la map_zt a zero...
	for (i=0; i<npix; i++){
		map_zt[i]=0;
#ifdef EZWNOREF
		map_sig[i]=0;
#endif
	};


#ifdef EZWREFAFTER
	//refinement pass
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
	#ifdef EZW_ARITH_RESET_MODEL
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
	#endif
	if (QccENTArithmeticSetModelContext(model, CONT_REFINE)) //context for refinement
	{
	QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
	argv1[0]);
	QccErrorExit();
	}
// #endif
}
	for (i=0;i< npix;i++){
		if ((map_sig[i] == 1) && (abs(image[i]) >= 2*threshold)) {
			value_pix=image[i];
			bit = get_bit(value_pix, thres_ind);
// #ifdef EZW_ARITH
			if (*(coder_param.flag_arith) == 1){
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
// #else
			} else {
				add_to_stream(stream, count, (int) bit, streamlast);
// #endif
			}
			nref++;
		};
	};
#endif

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
outputsize[thres_ind]=output_buffer.bit_cnt; //TODO check if correct
// #else
} else {
outputsize[thres_ind]=(*streamlast)*8 + (*count);
// #endif
}

#ifdef DEBUG
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("Size in bit: %ld \n", *streamlast*8+*count);
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
	printf("Size in bit: %d \n", output_buffer.bit_cnt);
}
// #endif
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
// #ifdef EZW_ARITH 
if (*(coder_param.flag_arith) == 1){
	outputsize[thres_ind]=output_buffer.bit_cnt;
} else {
// #else
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


    if (stat("tmp", &filestat) == 0){
   } else {
	fprintf(stderr, "Error on tmp file...\n");
   }

   data_file = fopen("tmp", "r");
   status = fread(&(streamstruct.stream[*streamstruct.streamlast]), 1, filestat.st_size, data_file);
   status= fclose(data_file);
   *streamstruct.streamlast += filestat.st_size;

   outputsize[0]=(*streamlast)*8 + (*count);
// #endif
}

#ifdef IDLSIMU
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
#endif

return 0;
};


int ezw_decode_c(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param)
{

int maxquant=(int) (*coder_param.maxquant);
int minquant=(int) (*coder_param.minquant);
long int npix=0;
long int i;
unsigned char bit=255;
unsigned char bit2=255;
int r=0;
int x,y,l;

long int threshold=0;
int thres_ind=0;
int flagsig=0;
// long int lastprocessed=0;

unsigned char *map_zt = NULL;
unsigned char *map_sig = NULL;

// #ifdef EZW_ARITH
int symbol = -1;

int num_context=CONT_NUM;
int num_symbols[CONT_NUM];

// long int symb_counter = 0;

// unsigned char * streambyte;
// FILE *input_file;
QccBitBuffer input_buffer;
QccENTArithmeticModel *model = NULL;
int argc1=1;
char * argv1[1];
argv1[0] = (char *) malloc(256*sizeof(char)); 
strcpy(argv1[0],"spihtcode");
// struct stat filestat;
// #endif

list_struct * list_desc=NULL;
list_el * current_el=NULL;

unsigned char *stream = streamstruct.stream;
unsigned char * count = streamstruct.count;
long int *streamlast = streamstruct.streamlast;

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

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
   data_file = fopen("tmp", "w");
   status = fwrite(&streamstruct.stream[(*streamstruct.headerlength)/8], 1, *outputsize/8 - (*streamstruct.headerlength)/8, data_file);
   status=fclose(data_file);

    QccInit(argc1, argv1);
    QccBitBufferInitialize(&input_buffer);

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
// #endif
}

npix=imageprop.nsmax * imageprop.nlmax * imageprop.nbmax;



//decodage EZW
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){
	
	threshold= 1 << (long int)thres_ind;
#ifdef DEBUG
	printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
#endif
	nref=0;
	nzeroisol=0;
	nzerotree=0;	
	nz = 0;	
	
#ifndef EZWNOREF
#ifndef EZWREFAFTER
	//refinement pass
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
	#ifdef EZW_ARITH_RESET_MODEL
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
	#endif
	if (QccENTArithmeticSetModelContext(model, CONT_REFINE)) //context for refinement
	{
	QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
	argv1[0]);
	QccErrorExit();
	}
// #endif
}
	flagsig=1;
	for (i=0;i< npix;i++){
		if (map_sig[i] == 1){
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
			bit = (unsigned char) symbol;
// #else
} else {
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit = read_from_stream(stream, count, streamlast);
			} else break;
// #endif
}
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
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
	if (input_buffer.bit_cnt >= *outputsize-(*streamstruct.headerlength)) break;
// #else
} else {
	if ((*streamlast)*8+ (*count) > *outputsize) break;
// #endif
}

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
	flagsig=0;
	for (i=0;i< npix;i++){
		if ((map_zt[i] == 0) && (map_sig[i] ==0)){
		x=i % (imageprop.nsmax);
		y=(i/imageprop.nsmax) % (imageprop.nlmax);
		l=(i/(imageprop.nsmax*imageprop.nlmax));

/*Selection du contexte*/
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
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
				
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_HF)) //context for POS,NEG,Z
					{
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
					argv1[0]);
					QccErrorExit();
					}
				} else {
					if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN)) //context for POS, NEG, IZ, ZTR
					{
					QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
					argv1[0]);
					QccErrorExit();
					}
				}
				

#else
		if (QccENTArithmeticSetModelContext(model, CONT_SIGN_GEN)) //context for POS, NEG, IZ, ZTR
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
if (*(coder_param.flag_arith) == 1){
			if (input_buffer.bit_cnt < *outputsize-(*streamstruct.headerlength)){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
			if (symbol == SYMB_POS){bit = 1; bit2=1;}//POS
			if (symbol == SYMB_NEG){bit = 1; bit2=0;}//NEG
			if (symbol == SYMB_IZ){bit = 0; bit2=0;}//IZ
			if (symbol == SYMB_ZTR){bit = 0; bit2=1;}//ZTR
			if (symbol == SYMB_Z){bit = 0; bit2=0;}//Z
// #else
} else {
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
#ifdef EZWNOREF
					image[i] += threshold;
#else
					image[i] += threshold + threshold/2;
#endif
					npos++;
				} else {	//NEG
					map_sig[i] = 1;
#ifdef EZWNOREF
					image[i] -= threshold;
#else
					image[i] -= threshold + threshold/2;
#endif
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
				nz++;
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
				nzeroisol++;
			} else {//ZT
				list_desc=list_init();
				r=spat_spec_desc_ezw((pixel_struct) {x,y,l}, list_desc, 1, image, thres_ind, map_sig);
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
	};

	for (i=0; i<npix; i++){
		map_zt[i]=0;
#ifdef EZWNOREF
		map_sig[i]=0;
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

#ifdef EZWREFAFTER
	//refinement pass
	flagsig=1;
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
	#ifdef EZW_ARITH_RESET_MODEL
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
	#endif
	if (input_buffer.bit_cnt < *outputsize-(*streamstruct.headerlength)){
	if (QccENTArithmeticSetModelContext(model, CONT_REFINE)) //context for refinement
	{
	QccErrorAddMessage("%s: Error calling QccENTArithmeticSetModelContext()",
	argv1[0]);
	QccErrorExit();
	}
	} else break;
// #endif
}
	for (i=0;i< npix;i++){
		if ((map_sig[i] == 1) &&  (abs(image[i]) >= 2*threshold)) {
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
			bit = (unsigned char) symbol;
// #else
} else {
			if ((*streamlast)*8+ (*count) <= *outputsize){
				bit = read_from_stream(stream, count, streamlast);
			} else break;
// #endif
}
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
	
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
	if (input_buffer.bit_cnt >= *outputsize-(*streamstruct.headerlength)) break;
// #else
} else {
	if ((*streamlast)*8+ (*count) > *outputsize) break;
// #endif
}
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


return 0;
};

