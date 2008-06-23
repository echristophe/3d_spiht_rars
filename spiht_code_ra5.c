/*
 *
 *  Hyperspectral compression program
 *
 * Name:		spiht_code_ra5.c	
 * Author:		Emmanuel Christophe	
 * Contact:		e.christophe at melaneum.com
 * Description:		Random access based coding and decoding for hyperspectral image
 * Version:		v1.4 - 2008-01	
 * 
 */


#include "main.h"

// int test=0;

/*********************************
***          CODER             ***
**********************************/


int spiht_code_ra5(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param){

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
int nbmin=imageprop.nbmin;

list_struct ** LSC = (list_struct **) calloc(imageprop.nres, sizeof(list_struct *));
list_struct ** LIC = (list_struct **) calloc(imageprop.nres, sizeof(list_struct *));
list_struct ** LIS = (list_struct **) calloc(imageprop.nres, sizeof(list_struct *));
pixel_struct pixel;

list_el * el=NULL;
list_el * current_el=NULL;
list_el * tmp_el=NULL;
list_el * current_child=NULL;
list_el * lastLIScurrentthres=NULL;

list_struct * list_desc=NULL;
list_struct * list_grand_desc=NULL;

long int value_pix=0;
unsigned char bit=255;
unsigned char bitsig=255;
int r=0;
int ngrandchild=0;

unsigned char * count;
long int *streamlast;
unsigned char * stream;
long int threshold=0;
int thres_ind=0;
int i,j,k;
int iloc,jloc,kloc;
int niloc, njloc, nkloc;
int ii,ji,ki;
int ie,je,ke;
int blockind;
int nblock;

// long long int ratebefore=0;
		
long int npix=nsmax*nbmax*nlmax;

int res=0;
int islastloop=0;
int * flagfirst = (int *) malloc (sizeof(int));


//debit-distortion
long long int dist=0;
// int weightingFactor=100;
float alpha, beta, rate_i, rate_e;
datablock_struct * datablock;
float lambda=0.0;

int maxres=imageprop.nres;
#ifdef RES_SCAL
int maxresspec=imageprop.nresspec;
int maxresspat=imageprop.nresspat;
int resspat=0;
int resspec=0;
#endif	

// #ifdef NOLISTB
// int is_accessible=0;
// parents_struct parents;
// #endif


// #ifdef EZW_ARITH
#define CONT_NUM 1
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
FILE *data_file;//TODO useless after...
int status=0;
// #endif

#ifdef DEBUG
long int nLICloop;
long int nLSCloop;
long int nLISloop;
long int nLISloopA;
long int nLISloopB;
#endif


unsigned char *map_LSC = NULL;
#ifndef NEWTREE
unsigned char *map_LIC = NULL;
unsigned char *map_LIS = NULL;

map_LSC=(unsigned char *) calloc(nsmax*nbmax*nlmax,sizeof(unsigned char));
map_LIC= (unsigned char *) calloc(nsmax*nbmax*nlmax,sizeof(unsigned char));
map_LIS= (unsigned char *) calloc(nsmax*nbmax*nlmax,sizeof(unsigned char));
#endif

stream = streamstruct.stream;
count = streamstruct.count;
streamlast = streamstruct.streamlast;


for (i=0; i<maxres; i++){
LSC[i] = list_init();
LIC[i] = list_init();
LIS[i] = list_init();
}


//This is just to get NUMRD point with exp spacing between rate_i and rate_e
/*rate_i = 200.*WEIGHTMULTVALUE;
rate_e = 3000000.*WEIGHTMULTVALUE*/;
rate_i = 200.;
rate_e = 3000000.;
alpha = exp(1./NUMRD * log(rate_e/rate_i));
beta = NUMRD * log(rate_i) / log(rate_e/rate_i);

for (i=0;i<imageprop.maxquant-1;i++){
   outputsize[i]=0;
}


niloc=(nsmin+1)/2;
njloc=(nlmin+1)/2;
nkloc=(nbmin+1)/2;
nblock=niloc* njloc* nkloc;

datablock=NULL;
datablock=(datablock_struct *) malloc(nblock* sizeof(datablock_struct));







//**********************************************
//Parcours des differentes localisations
//**********************************************

// block initialization
for (kloc=0;kloc<nkloc;kloc++){
   for (jloc=0;jloc<njloc;jloc++){
      for (iloc=0;iloc<niloc;iloc++){
	blockind = iloc + jloc*niloc + kloc * niloc *njloc;
	datablock_init(&(datablock[blockind]));
#ifdef RES_RATE
	for (j=0;j<imageprop.nres;j++){
	(datablock[blockind].rddata[j]).ptcourant=0;
	for (i=0;i<NUMRD;i++){
		(((datablock[blockind]).rddata[j]).reval)[i]= 0; //we want to keep one cutting point per threshold i from 0 to maxquant, the rest won't be used...
		(((datablock[blockind]).rddata[j]).r)[i]=0; //faster to use separate loops for memory access ?
		(((datablock[blockind]).rddata[j]).d)[i]=0;
	}
	}
#else
	datablock[blockind].rddata.ptcourant=0;
	for (i=0;i<NUMRD;i++){
		((datablock[blockind]).rddata.reval)[i]= (long long int) pow(alpha, beta+i);
		((datablock[blockind]).rddata.r)[i]=0; //faster to use separate loops for memory access ?
		((datablock[blockind]).rddata.d)[i]=0;
	}	
#endif
      }
   }
}


for (kloc=0;kloc<nkloc;kloc++){
   for (jloc=0;jloc<njloc;jloc++){
      for (iloc=0;iloc<niloc;iloc++){

// test=0;
// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
    QccInit(argc1, argv1);
    QccBitBufferInitialize(&output_buffer);

    num_symbols[0]=2;
//     num_symbols[CONT_REFINE]=2;
//     num_symbols[CONT_SIGN_HF]=3;
//     num_symbols[CONT_SIGN_GEN]=4;

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

blockind = iloc + jloc*niloc + kloc * niloc *njloc;

//SPIHT 1)
// printf("Initialization...\n");
ki=2*kloc;
ji=2*jloc;
ii=2*iloc;
ke= (2*kloc+2 < nbmin ? 2*kloc+2 : nbmin);
je= (2*jloc+2 < nlmin ? 2*jloc+2 : nlmin);
ie= (2*iloc+2 < nsmin ? 2*iloc+2 : nsmin);


#ifdef DEBUG2
printf("Coding grp: %d %d %d\n",iloc, jloc, kloc);
printf("  maxquant: %d minquant: %d maxres: %d\n",
    coder_param.maxquant[blockind], coder_param.minquant[blockind], coder_param.maxres[blockind]);
#else
fprintf(stderr,".");
#endif

for (k=ki;k<ke;k++){
   for (j=ji;j<je;j++){
      for (i=ii;i<ie;i++){
	pixel.x=i;
	pixel.y=j;
	pixel.l=k;
	el=el_init(pixel);
	el->thres=(coder_param.maxquant[blockind])+1;
	insert_el(LIC[0],el);
#ifndef NEWTREE
	(map_LIC[trans_pixel(pixel)])++;
#endif
	if ((i % 2) || (j % 2) || (k % 2) || (((nbmin % 2) == 1) && (k == nbmin-1) && (k!=0))){ 
		el=el_init(pixel);
		el->thres=(coder_param.maxquant[blockind])+1;
		insert_el(LIS[0],el);
#ifndef NEWTREE
		(map_LIS[trans_pixel(pixel)])++;
#endif
	 };
      };
   };
};
dist=0;//added 08-11-2006 HUM

//SPIHT 2)
#ifdef RES_SCAL
for (res=0; res<(coder_param.maxres[blockind]); res++){	
nbitswritten=0;
#ifdef SPATFIRST
resspat=res % maxresspat;
resspec=res / maxresspat;
#else
resspat=res / maxresspec;
resspec=res % maxresspec;
#endif


if( (resspat < coder_param.maxresspat[blockind]) &&  (resspec < coder_param.maxresspec[blockind])){
#ifdef RES_RATE
dist=0;
ratebefore= *(datablock[blockind].streamlast)*8+*(datablock[blockind].count);
#endif

 if (*(coder_param.flag_arith) == 1){
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
}

for (thres_ind=(coder_param.maxquant[blockind]); thres_ind >= (coder_param.minquant[blockind]); thres_ind--){	
#endif

#ifndef RES_SCAL
for (thres_ind=(coder_param.maxquant[blockind]); thres_ind >= (coder_param.minquant[blockind]); thres_ind--){	
nbitswritten=0;

 if (*(coder_param.flag_arith) == 1){
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
}

for (res=0; res<(coder_param.maxres[blockind]); res++){	
#endif


threshold= 1 << (long int)thres_ind;

#ifdef DEBUG
printf("Processing for thres_ind %d (threshold: %ld) at resolution %d...\n",thres_ind, threshold, res);
#endif

#ifdef DEBUG
nLSCloop=0;
nLICloop=0;
nLISloop=0;
nLISloopA=0;
nLISloopB=0;
#endif


// if (blockind == 52 && thres_ind == 14 ){
//   printf("Stop here\n");
// } 


/*******************************
LIC
********************************/
current_el=first_el(LSC[res]); //right positionning for adding from LIC
while ((current_el != NULL) && (current_el->thres > thres_ind)){
   	current_el=next_el(LSC[res]);
}
if ((current_el != NULL) && (current_el->thres <= thres_ind)) {
LSC[res]->current = LSC[res]->previous;
}
if (current_el == NULL){
LSC[res]->current = LSC[res]->last;
}


//SPIHT 2.1)
current_el=first_el(LIC[res]);
// printf("LIC processing \n");

while ((current_el != NULL) && (current_el->thres > thres_ind)){
#ifdef DEBUG
   nLICloop++;
#endif
   value_pix=image[trans_pixel(current_el->pixel)];
   bit = get_bit(value_pix, thres_ind);	
   if (*(coder_param.flag_arith) == 1){
        symbol =  bit;
        if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
        {
        QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
        argv1[0]);
        QccErrorExit();
        }
//         test++;
   } else {
    add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));//SPIHT 2.1.1)
   }
  if (bit == 1) { //SPIHT 2.1.2)
     bitsig = (value_pix > 0);
   if (*(coder_param.flag_arith) == 1){
         symbol =  bitsig;
        if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
        {
        QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
        argv1[0]);
        QccErrorExit();
        }
//         test++;
   } else {
     add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bitsig, (datablock[blockind].streamlast));
   }
if (coder_param.rate != 0.0){
#ifdef OLDRATE
     update_dist(current_el->pixel, thres_ind, &dist, image);
#else
     update_dist_first(current_el->pixel, thres_ind, &dist, image);
//       update_dist1(current_el->pixel, thres_ind, &dist, image);
#endif
}
// #ifdef RES_RATE
// #else
   if (*(coder_param.flag_arith) == 1){
     add_to_rddata(&datablock[blockind].rddata, output_buffer.bit_cnt, dist);
   } else {
     add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
   }
// #endif

#ifndef NEWTREE
     (map_LIC[trans_pixel(current_el->pixel)])--;
     (map_LSC[trans_pixel(current_el->pixel)])++;
#endif
     remove_current_el(LIC[res]);
     insert_el_inplace(LSC[res], current_el);
	if (LSC[res]->current == NULL){
		LSC[res]->current=LSC[res]->first;
	}else {
		next_el(LSC[res]);
	}
	current_el->thres=thres_ind;
     current_el = LIC[res]->current;
   } else {
      current_el=next_el(LIC[res]);
   };
}

/*******************************
LSC before LIS
********************************/
#ifdef LSCBEFORE
current_el=first_el(LSC[res]);

while ((current_el != NULL) && (current_el->thres > thres_ind)){
#ifdef DEBUG
	nLSCloop++;
#endif
	value_pix=image[trans_pixel(current_el->pixel)];
	bit = get_bit(value_pix, thres_ind);
      if (*(coder_param.flag_arith) == 1){
          symbol =  bit;
          if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
          {
          QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
          argv1[0]);
          QccErrorExit();
          }
//               test++;
      } else {
	add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));
      }
	if (bit ==1) {
if (coder_param.rate != 0.0){
#ifdef OLDRATE
		update_dist(current_el->pixel, thres_ind, &dist, image);
#else
                update_dist1(current_el->pixel, thres_ind, &dist, image);
#endif
}
// #ifdef RES_RATE
// #else
   if (*(coder_param.flag_arith) == 1){
     add_to_rddata(&datablock[blockind].rddata, output_buffer.bit_cnt, dist);
   } else {
     add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);          
   }
// #endif
	} else {
if (coder_param.rate != 0.0){
#ifndef OLDRATE
                update_dist0(current_el->pixel, thres_ind, &dist, image);
#endif
}
// #ifdef RES_RATE
// #else
   if (*(coder_param.flag_arith) == 1){
     add_to_rddata(&datablock[blockind].rddata, output_buffer.bit_cnt, dist);
   } else {
		add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
   }
// #endif
        }
	current_el=next_el(LSC[res]);
};

#ifdef RES_RATE
   if (*(coder_param.flag_arith) == 1){
     add_to_rddata(&datablock[blockind].rddata, output_buffer.bit_cnt, dist);
   } else {
add_to_rddata(&((datablock[blockind].rddata)[res]), *(datablock[blockind].streamlast)*8+*(datablock[blockind].count)-ratebefore, dist);//we want to keep one point per threshold.
   }
#endif

#endif

// 
// ;for each entry in the LIS
/*******************************
LIS
********************************/
current_el=first_el(LIS[res]); //right positionning for adding from LIS
while ((current_el != NULL) && (current_el->thres >= thres_ind)){
	current_el=next_el(LIS[res]);
}
if ((current_el != NULL) && (current_el->thres < thres_ind)) {
	LIS[res]->current = LIS[res]->previous;
}
if (current_el == NULL){
	LIS[res]->current = LIS[res]->last;
}
lastLIScurrentthres=LIS[res]->current;

current_el=first_el(LIS[res]);
islastloop=0;
// printf("LIS processing \n");

while ((current_el != NULL) && (current_el->thres >= thres_ind)){ //SPIHT 2.2)
#ifdef DEBUG
nLISloop++;
#endif
// #ifndef NOLISTB
    if (current_el->type == 0) { //SPIHT 2.2.1);if the set is of type A
// #endif
#ifdef DEBUG
        nLISloopA++;
#endif

	r=spat_spec_desc_spiht(current_el->pixel, list_desc, 0, image, thres_ind, map_LSC);
	
	bit = (r == -1);//at least one descendant is significant
      if (*(coder_param.flag_arith) == 1){
          symbol =  bit;
          if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
          {
          QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
          argv1[0]);
          QccErrorExit();
          }
//               test++;
      } else {
        add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast)); //SPIHT 2.2.1.1
      }
        if (bit == 1) { //SPIHT 2.2.1.2
	   list_desc=list_init();
	   r=spat_spec_desc_spiht(current_el->pixel, list_desc, 1, image, thres_ind, map_LSC);
	   current_child = first_el(list_desc);
 	   ngrandchild = 0;
	   list_grand_desc=list_init();
	   while (current_child !=NULL){ //SPIHT 2.2.1.2.1
#ifndef NEWTREE
	      if ((map_LSC[trans_pixel(current_child->pixel)] == 0) && (map_LIC[trans_pixel(current_child->pixel)] == 0)){ 
#endif
        	value_pix=image[trans_pixel(current_child->pixel)]; 
		bit = get_bit(value_pix, thres_ind);
	
      if (*(coder_param.flag_arith) == 1){
          symbol =  bit;
          if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
          {
          QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
          argv1[0]);
          QccErrorExit();
          }
//               test++;
      } else {
		add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast)); //SPIHT 2.2.1.2.1.1
      }
		if (bit == 0){ //SPIHT 2.2.1.2.1.3
#ifndef NEWTREE
 			(map_LIC[trans_pixel(current_child->pixel)])++;
#endif
			el=el_init(current_child->pixel);
			el->thres = thres_ind;
#ifdef RES_SCAL
			if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
#ifdef SPATFIRST
				insert_el(LIC[res+1],el);
#else
				insert_el(LIC[res+maxresspec],el);
#endif
			} else {//this is a spectral children
#ifdef SPATFIRST
				insert_el(LIC[res+maxresspat],el);
#else
				insert_el(LIC[res+1],el);
#endif			
			}
#else
			insert_el(LIC[res+1],el);
#endif
		} else { //SPIHT 2.2.1.2.1.2
			bitsig = (value_pix > 0);
      if (*(coder_param.flag_arith) == 1){
          symbol =  bitsig;
          if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
          {
          QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
          argv1[0]);
          QccErrorExit();
          }
//               test++;
      } else {
			add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bitsig, (datablock[blockind].streamlast));
      }
if (coder_param.rate != 0.0){
#ifdef OLDRATE
			update_dist(current_child->pixel, thres_ind, &dist, image);
#else
                        update_dist_first(current_child->pixel, thres_ind, &dist, image);
//                         update_dist1(current_child->pixel, thres_ind, &dist, image);
#endif
}
// #ifdef RES_RATE
// #else
   if (*(coder_param.flag_arith) == 1){
     add_to_rddata(&datablock[blockind].rddata, output_buffer.bit_cnt, dist);
   } else {
			add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
   }
// #endif
#ifndef NEWTREE
			(map_LSC[trans_pixel(current_child->pixel)])++;
#endif
			el=el_init(current_child->pixel);
			el->thres = thres_ind;
#ifdef RES_SCAL
			if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
				#ifdef SPATFIRST
				insert_el(LSC[res+1],el);
				#else
				insert_el(LSC[res+maxresspec],el);
				#endif
			} else {//this is a spectral children
				#ifdef SPATFIRST
				insert_el(LSC[res+maxresspat],el);
				#else
				insert_el(LSC[res+1],el);
				#endif				
			}
#else			
			insert_el(LSC[res+1],el);
#endif			
		};
		r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
		if (r != 0) ngrandchild++;


#ifndef NEWTREE
	      };//end of if on maps
#endif
	      current_child=next_el(list_desc);
	   };

	   list_free(list_grand_desc);
	   list_free(list_desc);
	   if (ngrandchild > 0){//SPIHT 2.2.1.2.2
	      if (current_el != lastLIScurrentthres){
		tmp_el=remove_current_el(LIS[res]); //don't free the memory here
		insert_el_after(LIS[res],current_el,&lastLIScurrentthres);
	      }

              current_el->type=1;
	   } else {
		tmp_el=remove_current_el(LIS[res]);
#ifndef NEWTREE		
		(map_LIS[trans_pixel(tmp_el->pixel)])--;
#endif
		free(tmp_el);
		
	   };
	   tmp_el=NULL;
	}else{//fin du 2.2.1.2
	   current_el=next_el(LIS[res]);
	};
	} else { //SPIHT 2.2.2 ;else the set is of type B
#ifdef DEBUG
	  nLISloopB++;
#endif
		//output Sn(L(i,j))
	  list_desc=list_init();
	  r=spat_spec_desc_spiht(current_el->pixel, list_desc, 1, image, thres_ind, map_LSC);
	  current_child = first_el(list_desc);
	  bit = 0;
	  while ((bit == 0) && (current_child !=NULL)){
#ifndef NEWTREE
		if(map_LIS[trans_pixel(current_child->pixel)] == 0){
#endif
		   r=spat_spec_desc_spiht(current_child->pixel, list_desc, 0, image, thres_ind, map_LSC);
		   if (r == -1) {bit = 1;};
#ifndef NEWTREE
		};
#endif
		current_child=next_el(list_desc);
	 };
      if (*(coder_param.flag_arith) == 1){
          symbol =  bit;
          if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
          {
          QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
          argv1[0]);
          QccErrorExit();
          }
//               test++;
      } else {
	 add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));
      }

	 if (bit == 1){
		current_child = first_el(list_desc);
		while (current_child !=NULL){
#ifndef NEWTREE
			if (map_LIS[trans_pixel(current_child->pixel)] == 0){
#endif
				el=el_init(current_child->pixel);
				el->thres = thres_ind;
						
#ifdef RES_SCAL
				if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
					#ifdef SPATFIRST
					insert_el(LIS[res+1],el);
					#else
					insert_el(LIS[res+maxresspec],el);
					#endif
				} else {//this is a spectral children
					#ifdef SPATFIRST
					insert_el(LIS[res+maxresspat],el);
					#else
					insert_el(LIS[res+1],el);
					#endif				
				}
#else 
				insert_el(LIS[res+1],el);
#endif
#ifndef NEWTREE
				(map_LIS[trans_pixel(el->pixel)])++;
			};
#endif
			current_child=next_el(list_desc);
		};
	  	tmp_el=remove_current_el(LIS[res]);
#ifndef NEWTREE
		(map_LIS[trans_pixel(tmp_el->pixel)])--;
#endif
	  	free(tmp_el);
							
	  } else {
	    current_el=next_el(LIS[res]);
	  };
	  list_free(list_desc); 
    };
current_el=LIS[res]->current;
};

/*******************************
LSC
********************************/
#ifndef LSCBEFORE
current_el=first_el(LSC[res]);

while ((current_el != NULL) && (current_el->thres > thres_ind)){
#ifdef DEBUG
	nLSCloop++;
#endif
	value_pix=image[trans_pixel(current_el->pixel)];
	bit = get_bit(value_pix, thres_ind);
      if (*(coder_param.flag_arith) == 1){
          symbol =  bit;
          if (QccENTArithmeticEncode(&symbol, 1, model, &output_buffer))
          {
          QccErrorAddMessage("%s: Error calling QccENTArithmeticEncode()",
          argv1[0]);
          QccErrorExit();
          }
//               test++;
      } else {
	add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));
      }
	if (bit ==1) {
if (coder_param.rate != 0.0){
#ifdef OLDRATE
		update_dist(current_el->pixel, thres_ind, &dist, image);
#else
//                 printf("old dist: %lld\n", dist);
                update_dist1(current_el->pixel, thres_ind, &dist, image);
//                 printf("new dist: %lld\n", dist);
//                 printf("  (with thresh: %d\n", thres_ind);
#endif
}
// #ifdef RES_RATE
// #else
   if (*(coder_param.flag_arith) == 1){
     add_to_rddata(&datablock[blockind].rddata, output_buffer.bit_cnt, dist);
   } else {
		add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
   }
// #endif
	} else {
if (coder_param.rate != 0.0){
#ifndef OLDRATE
            update_dist0(current_el->pixel, thres_ind, &dist, image);
#endif
}
#ifdef RES_RATE
#else
   if (*(coder_param.flag_arith) == 1){
     add_to_rddata(&datablock[blockind].rddata, output_buffer.bit_cnt, dist);
   } else {
		add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
   }
#endif
        }
	current_el=next_el(LSC[res]);
};

#ifdef RES_RATE
   if (*(coder_param.flag_arith) == 1){
     add_to_rddata(&datablock[blockind].rddata, output_buffer.bit_cnt, dist);
   } else {
add_to_rddata(&((datablock[blockind].rddata)[res]), *(datablock[blockind].streamlast)*8+*(datablock[blockind].count)-ratebefore, dist);//we want to keep one point per threshold.
   }
#endif

#endif

/********* END LSC  ************/




#ifndef RES_SCAL
};//fin resolution
if (*(coder_param.flag_arith) == 1){
nbitswritten = output_buffer.bit_cnt;
} 
(datablock[blockind].partsize)[thres_ind] = nbitswritten;

#endif

if (*(coder_param.flag_arith) == 1){
outputsize[thres_ind] += output_buffer.bit_cnt;
}else{
outputsize[thres_ind] += (*(datablock[blockind].streamlast))*8 + (*(datablock[blockind].count));
}

// printf("test %d\n",test);
// printf("at %ld bits for thres %d\n",output_buffer.bit_cnt,thres_ind);//tmp

};//end threshold



#ifdef RES_SCAL
}//end of if on resspat and resspec
if (*(coder_param.flag_arith) == 1){
nbitswritten = output_buffer.bit_cnt;
} 
(datablock[blockind].partsize)[res] = nbitswritten;

#ifdef RES_RATE

#endif
};//end resolution
#endif




#ifndef RES_RATE
//flush coder
QccENTArithmeticEncodeFlush(model, &output_buffer);//not sure if required...


if (*(coder_param.flag_arith) == 1){
(datablock[blockind].rddata.r)[NUMRD-1]= output_buffer.bit_cnt;//warning, don't take the flush into account...
// printf("Full blocksize  recorded: %d\n",(datablock[blockind].rddata.r)[NUMRD-1]);
} else {
(datablock[blockind].rddata.r)[NUMRD-1]=(*(datablock[blockind].streamlast))*8 + (*(datablock[blockind].count));
}
(datablock[blockind].rddata.d)[NUMRD-1]=dist;
// printf("dist: %lld\n", dist); 
#endif

for (i=0; i<imageprop.nres; i++){
list_flush(LSC[i]);
list_flush(LIC[i]);
list_flush(LIS[i]);
}


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
//    status = fread(&(streamstruct.stream[*streamstruct.streamlast]), 1, filestat.st_size, data_file);
   status = fread(&(datablock[blockind].stream[*(datablock[blockind].streamlast)]), 1, filestat.st_size, data_file);
   status= fclose(data_file);
   *(datablock[blockind].streamlast) = filestat.st_size;//why += ???
//    if ((datablock[blockind].count) != 0){
//        printf(" ****************************************************************\n");
//     }
//    outputsize[0]=(*(datablock[blockind].streamlast))*8 + (*(datablock[blockind].count));
    outputsize[0]=(*(datablock[blockind].streamlast))*8;
// outputsize[0] = filestat.st_size * 8
(datablock[blockind].rddata.r)[NUMRD-1]=filestat.st_size * 8; //test
// #endif
}

#ifdef DEBUG2
printf("Full blocksize  (in bit): %ld\n",*(datablock[blockind].streamlast)*8 + (*(datablock[blockind].count)));
#endif

}// End of 
}// bloc 
}// processing

printf("\n");

*flagfirst = 1; //this command the output of the secondary header for the first layer


//TODO just a check to output RD curves (to be removed)
#ifdef OUTPUTRD
 output_rd(datablock, nblock);
#endif

nbitswritten=0;

	printf("Stream size before: %ld \n",*streamlast);
	printf("count before:       %uc \n",*count);
	printf("Size in bit before: %ld \n", *streamlast*8+*count);

if (coder_param.rate != 0.0){

	lambda=compute_lambda(datablock, coder_param.rate*npix, nblock);
	printf("Interleaving for lambda=%f\n",lambda);
	interleavingblocks(datablock, nblock, stream, count, streamlast, lambda, flagfirst);
	
	#ifdef DEBUG2
	printf("Stream size: %ld \n",*streamlast);
	printf("count:       %uc \n",*count);
	printf("Size in bit: %ld \n", *streamlast*8+*count);
	printf("Rate: %f \n", (float) (*streamlast*8+*count)/((long int) nsmax*nlmax*nbmax));
	#endif
	printf("The rate may not be accurate due to header if there is less than 5 decomposition level\n");
} else {
	printf("Interleaving for lambda=0 (lossless)\n");
	lambda=0.;
	interleavingblocks(datablock, nblock, stream, count, streamlast, lambda, flagfirst);
			
	#ifdef DEBUG2
	printf("Stream size: %ld \n",*streamlast);
	printf("count:       %uc \n",*count);
	printf("Size in bit: %ld \n", *streamlast*8+*count);
	printf("Rate: %f \n", (float) (*streamlast*8+*count)/((long int) nsmax*nlmax*nbmax));
	#endif		
		
}


*outputsize = *streamlast*8+*count;


#ifndef NEWTREE
free(map_LSC);
free(map_LIC);
free(map_LIS);
#endif

//do not forget to free each block separatly first
for (i=0;i<nblock;i++){
	datablock_free(&(datablock[i]));
}

free(datablock);

free(count);
free(streamlast);

return 0;
};



/*********************************
***         DECODER            ***
**********************************/

int spiht_decode_ra5(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param){

int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
int nbmin=imageprop.nbmin;

list_struct ** LSC = (list_struct **) calloc(imageprop.nres, sizeof(list_struct *));
list_struct ** LIC = (list_struct **) calloc(imageprop.nres, sizeof(list_struct *));
list_struct ** LIS = (list_struct **) calloc(imageprop.nres, sizeof(list_struct *));
pixel_struct pixel;
// parents_struct parents;
// int is_accessible=0;


list_el * el=NULL;
list_el * current_el=NULL;
list_el * tmp_el=NULL;
list_el * current_child=NULL;
list_el * lastLIScurrentthres=NULL;



// list_el * lastprocessed=NULL;

list_struct * list_desc=NULL;
list_struct * list_grand_desc=NULL;
unsigned char bit=255;
unsigned char bitsig=255;
int r=0;
int ngrandchild=0;
unsigned char * stream;
unsigned char * count;
long int *streamlast;

long int threshold=0;
int thres_ind=0;
int i,j,k;
int iloc,jloc,kloc;
int niloc, njloc, nkloc;
int ii,ji,ki;
int ie,je,ke;
// long int i_l;
int blockind;
int nblock;

long int endpos=0;

long int err=0;
// long int maxerr=0;

// int minquant=0;

 #ifdef RES_SCAL
int maxresspec=imageprop.nresspec;
int maxresspat=imageprop.nresspat;
#endif
int maxres=imageprop.nres;	
	

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
FILE *data_file;
int status=0;
// #endif

// int lastres=0;
// int flagLSClastres=0;
// int LSCprocessed = 0;

#ifdef CHECKEND
int flagLIS = 0;
#endif

datablock_struct * datablock;

int res=0;
#ifdef RES_SCAL
int resspat=0;
int resspec=0;
#endif

#ifdef DEBUG
long int nLICloop;
long int nLSCloop;
long int nLISloop;
long int nLISloopA;
long int nLISloopB;
#endif

char flagLSC=0;

unsigned char *map_LSC = NULL;
#ifndef NEWTREE
unsigned char *map_LIC = NULL;
unsigned char *map_LIS = NULL;

map_LSC=(unsigned char *) calloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax,sizeof(unsigned char));
map_LIC= (unsigned char *) calloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax,sizeof(unsigned char));
map_LIS= (unsigned char *) calloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax,sizeof(unsigned char));
#endif

stream = streamstruct.stream;
count = streamstruct.count;
streamlast = streamstruct.streamlast;

nbitsread=0;//warning, we put it back to zero later
nbitsreadheader=0;

for (i=0;i<maxres;i++){//warning, have to be correct with maxresspat et maxresspec
LSC[i] = list_init();
LIC[i] = list_init();
LIS[i] = list_init();
}

#ifndef NEWTREE
for (i_l=0;i_l<imageprop.nsmax*imageprop.nlmax*imageprop.nbmax;i_l++)
{
	map_LSC[i_l]=0;
	map_LIC[i_l]=0;
	map_LIS[i_l]=0;
}
#endif


niloc=(nsmin+1)/2;
njloc=(nlmin+1)/2;
nkloc=(nbmin+1)/2;
nblock=niloc* njloc* nkloc;

datablock=(datablock_struct *) malloc(nblock* sizeof(datablock_struct));

for (i=0; i<nblock; i++){
	datablock_init(&(datablock[i]));
}

//Desinterleaving stream
err=desinterleavingblocks(datablock, nblock, streamstruct, *outputsize, coder_param.nlayer);//carefull for the bit counting later...
if (err) {fprintf(stderr, "******** ERROR desinterleavingblocks (truncation ?)\n");};

for (i=0; i<nblock; i++){
	datablock[i].currentpos=(*(datablock[i].streamlast))*8+ *(datablock[i].count);
	*(datablock[i].streamlast)=0;
	*(datablock[i].count)=0;
}

nbitsread=0;

//******************************************
//Parcours des differentes localisations
//******************************************

for (kloc=0;kloc<nkloc;kloc++){
   for (jloc=0;jloc<njloc;jloc++){
      for (iloc=0;iloc<niloc;iloc++){
// test=0;

#ifdef DEBUG2
printf("Decoding grp: %d %d %d\n",iloc, jloc, kloc);
#endif

blockind = iloc + jloc*niloc + kloc * niloc *njloc;
*outputsize = datablock[blockind].currentpos;

// #ifdef EZW_ARITH
if (*(coder_param.flag_arith) == 1){
   data_file = fopen("tmp", "w");
//    status = fwrite(&streamstruct.stream[(*streamstruct.headerlength)/8], 1, *outputsize/8 - (*streamstruct.headerlength)/8, data_file);
   //we want the current block
   status = fwrite(datablock[blockind].stream ,1, (datablock[blockind].currentpos+7)/8,data_file);
   status=fclose(data_file);

    QccInit(argc1, argv1);
    QccBitBufferInitialize(&input_buffer);

    num_symbols[0]=2;
//     num_symbols[CONT_REFINE]=2;
//     num_symbols[CONT_SIGN_HF]=3;
//     num_symbols[CONT_SIGN_GEN]=4;

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
    datablock[blockind].currentpos)) == NULL) //WARNING, the +8 is empirical
//     QCCENT_ANYNUMBITS)) == NULL)
    {
    QccErrorAddMessage("%s: Error calling QccENTArithmeticDecodeStart()",
    argv1[0]);
    QccErrorExit();
    }


// #endif
}



//SPIHT 1)
ki=2*kloc;
ji=2*jloc;
ii=2*iloc;
ke= (2*kloc+2 < nbmin ? 2*kloc+2 : nbmin);
je= (2*jloc+2 < nlmin ? 2*jloc+2 : nlmin);
ie= (2*iloc+2 < nsmin ? 2*iloc+2 : nsmin);

for (k=ki;k<ke;k++){
   for (j=ji;j<je;j++){
      for (i=ii;i<ie;i++){
	pixel.x=i;
	pixel.y=j;
	pixel.l=k;
	el=el_init(pixel);
	el->thres=(coder_param.maxquant[blockind])+1;
	insert_el(LIC[0],el);
#ifndef NEWTREE
	(map_LIC[trans_pixel(pixel)])++;
#endif
	if ((i % 2) || (j % 2) || (k % 2) || (((nbmin % 2) == 1) && (k == nbmin-1) && (k!=0))){ 
		el=el_init(pixel);
		el->thres=(coder_param.maxquant[blockind])+1;
		insert_el(LIS[0],el);
#ifndef NEWTREE
		(map_LIS[trans_pixel(pixel)])++;
#endif
	 };
      };
   };
};

endpos =0;

#ifdef RES_SCAL
for (res=0; res<(coder_param.maxres[blockind]); res++){	
	
#ifdef SPATFIRST
resspat=res % maxresspat;
resspec=res / maxresspat;
#else
resspat=res / maxresspec;
resspec=res % maxresspec;
#endif

 if (*(coder_param.flag_arith) == 1){
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
}

#ifdef DEBUG
printf("Processing for res %d ...\n",res);
#endif	
if( (resspat < coder_param.maxresspat[blockind]) &&  (resspec < coder_param.maxresspec[blockind])){
	
		for (thres_ind=(coder_param.maxquant[blockind]); thres_ind >= (coder_param.minquant[blockind]); thres_ind--){

#endif
//SPIHT 2)


#ifndef RES_SCAL
for (thres_ind=(coder_param.maxquant[blockind]); thres_ind >= (coder_param.minquant[blockind]); thres_ind--){	

 if (*(coder_param.flag_arith) == 1){
	for (i=0;i<CONT_NUM;i++){
		QccENTArithmeticResetModel(model, i);
	}
}

	if (thres_ind>10){
		maxres =imageprop.nres; //WARNING valid ONLY if encoding was full !!!
	}else{
		maxres=(coder_param.maxres[blockind]);
	}
        #ifdef DEBUG
        printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
        #endif
	for (res=0; res<maxres; res++){

#endif


threshold= 1 << (long int)thres_ind;




flagLSC=0;
#ifdef CHECKEND
flagLIS=0;
#endif

#ifdef DEBUG
nLSCloop=0;
nLICloop=0;
nLISloop=0;
nLISloopA=0;
nLISloopB=0;
#endif

/*******************************
LIC
********************************/


// Used for Significant pass
current_el=first_el(LSC[res]); //right positionning for adding from LIC
while ((current_el != NULL) && (current_el->thres > thres_ind)){
   	current_el=next_el(LSC[res]);
}
if ((current_el != NULL) && (current_el->thres <= thres_ind)) {
LSC[res]->current = LSC[res]->previous;
}
if (current_el == NULL){
LSC[res]->current = LSC[res]->last;
}



// ;for each entry in the LIC
//SPIHT 2.1)
current_el=first_el(LIC[res]);
// printf("LIC processing \n");
while ((current_el != NULL) && (current_el->thres > thres_ind) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize)){
// printf(".");
#ifdef DEBUG
   nLICloop++;
#endif

if (*(coder_param.flag_arith) == 1){
			if (input_buffer.bit_cnt < *outputsize){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
//                         test++;
			bit = (unsigned char) symbol;
} else {
   if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
      bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
   } else {
	   current_el=first_el(LSC[res]); //correct positionning for the final correction -> SHOULD BE USELESS NOW...
	   break;
   }
}
  if (bit == 1) { //SPIHT 2.1.2)
if (*(coder_param.flag_arith) == 1){
			if (input_buffer.bit_cnt < *outputsize){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
//                         test++;
			bitsig = (unsigned char) symbol;
} else {
     if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
        bitsig = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
     } else{
	     current_el=first_el(LSC[res]); //correct positionning for the final correction
	     break;
     }
}
     image[trans_pixel(current_el->pixel)] += threshold + threshold/2;

     if (bitsig == 0) {
     	image[trans_pixel(current_el->pixel)] = -image[trans_pixel(current_el->pixel)];
     };
#ifndef NEWTREE
     (map_LIC[trans_pixel(current_el->pixel)])--;
     (map_LSC[trans_pixel(current_el->pixel)])++;
#endif
     remove_current_el(LIC[res]);
     insert_el_inplace(LSC[res], current_el);
	if (LSC[res]->current == NULL){
		LSC[res]->current=LSC[res]->first;
	}else {
		next_el(LSC[res]);
	}

     current_el->thres = thres_ind;
     current_el = LIC[res]->current;
   } else {
      current_el=next_el(LIC[res]);
   };
}

/*******************************
LSC before LIS
********************************/
#ifdef LSCBEFORE
current_el=first_el(LSC[res]);
while ((current_el != NULL) && (current_el->thres > thres_ind) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize)){
#ifdef DEBUG
	nLSCloop++;
#endif
if (*(coder_param.flag_arith) == 1){
			if (input_buffer.bit_cnt < *outputsize){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
//                         test++;
			bit = (unsigned char) symbol;
} else {
	if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
	bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
	flagLSC=1;
	} else break;
}
	if (bit == 1){
		if (image[trans_pixel(current_el->pixel)] > 0){
			image[trans_pixel(current_el->pixel)] += threshold/2;
		} else {
			image[trans_pixel(current_el->pixel)] -= threshold/2;
		};
	} else {
		if (image[trans_pixel(current_el->pixel)] > 0){
			image[trans_pixel(current_el->pixel)] -= ROUND(threshold/2.0+0.1);
		} else {
			image[trans_pixel(current_el->pixel)] += ROUND(threshold/2.0+0.1);
		};
	}
	current_el=next_el(LSC[res]);
};
#endif

/*******************************
LIS
********************************/
#ifdef LSCBEFORE
#ifdef RES_SCAL
if( (thres_ind == coder_param.minquant[blockind]) && 
     ((resspat == coder_param.maxresspat[blockind]-1) && (coder_param.maxresspat[blockind] !=1 )) ){
		#ifdef DEBUG
			printf("Skipping LIS for res %d thres %d (spec: %d, spat: %d)\n", res, thres_ind, resspec, resspat);	
		#endif
 }else {
#endif 
#endif

#ifdef CHECKEND
flagLIS=1;
#endif
	
// ;for each entry in the LIS
current_el=first_el(LIS[res]); //right positionning for adding from LIS
while ((current_el != NULL) && (current_el->thres >= thres_ind)){
	current_el=next_el(LIS[res]);
}
if ((current_el != NULL) && (current_el->thres < thres_ind)) {
	LIS[res]->current = LIS[res]->previous;
}
if (current_el == NULL){
	LIS[res]->current = LIS[res]->last;
}
lastLIScurrentthres=LIS[res]->current;


current_el=first_el(LIS[res]);
// printf("LIS processing \n");
while ((current_el != NULL)&& (current_el->thres >= thres_ind) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize)){ //SPIHT 2.2)
#ifdef DEBUG
nLISloop++;
#endif
    if (current_el->type == 0) { //SPIHT 2.2.1);if the set is of type A
#ifdef DEBUG
        nLISloopA++;
#endif
//          //SPIHT 2.2.1.1
if (*(coder_param.flag_arith) == 1){
			if (input_buffer.bit_cnt < *outputsize){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
//                         test++;
			bit = (unsigned char) symbol;
} else {
        if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
	   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
        } else break;
}
        if (bit == 1) { //SPIHT 2.2.1.2
	   list_desc=list_init();
	   r=spat_spec_desc_spiht(current_el->pixel, list_desc, 1, image, thres_ind, map_LSC);
	   current_child = first_el(list_desc);
 	   ngrandchild = 0;
	   list_grand_desc=list_init();
	   while ((current_child !=NULL)&& ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize)){ //SPIHT 2.2.1.2.1
#ifndef NEWTREE
	      if ((map_LSC[trans_pixel(current_child->pixel)] == 0) && (map_LIC[trans_pixel(current_child->pixel)] == 0)){ 
#endif
if (*(coder_param.flag_arith) == 1){
			if (input_buffer.bit_cnt < *outputsize){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
//                         test++;
			bit = (unsigned char) symbol;
} else {
		if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
		   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
		} else break;
}
		if (bit == 0){ //SPIHT 2.2.1.2.1.3
#ifndef NEWTREE
 			(map_LIC[trans_pixel(current_child->pixel)])++;
#endif
			el=el_init(current_child->pixel);
			el->thres=thres_ind;
#ifdef RES_SCAL
			if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
#ifdef SPATFIRST
				insert_el(LIC[res+1],el);
#else
				insert_el(LIC[res+maxresspec],el);
#endif
			} else {//this is a spectral children
#ifdef SPATFIRST
				insert_el(LIC[res+maxresspat],el);
#else
				insert_el(LIC[res+1],el);
#endif				
			}
#else
			insert_el(LIC[res+1],el);
#endif
		} else { //SPIHT 2.2.1.2.1.2
if (*(coder_param.flag_arith) == 1){
			if (input_buffer.bit_cnt < *outputsize){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
//                         test++;
			bitsig = (unsigned char) symbol;
} else {
			if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
			   bitsig = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
			} else break;
}
			image[trans_pixel(current_child->pixel)] += threshold + threshold/2; 
			if (bitsig == 0){
				image[trans_pixel(current_child->pixel)] = -image[trans_pixel(current_child->pixel)];
			};
#ifndef NEWTREE
			(map_LSC[trans_pixel(current_child->pixel)])++;
#endif
			el=el_init(current_child->pixel);
			el->thres=thres_ind;
#ifdef RES_SCAL
			if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
#ifdef SPATFIRST
				insert_el(LSC[res+1],el);
#else
				insert_el(LSC[res+maxresspec],el);
#endif
			} else {//this is a spectral children
#ifdef SPATFIRST
				insert_el(LSC[res+maxresspat],el);
#else
				insert_el(LSC[res+1],el);
#endif				
			}
#else
			insert_el(LSC[res+1],el);			
#endif
		};
		r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
		if (r != 0) ngrandchild++;

#ifndef NEWTREE
	      };//end of cdt on maps
#endif
	      current_child=next_el(list_desc);
	   };

	   list_free(list_grand_desc);
	   list_free(list_desc);
	   if (ngrandchild > 0){//SPIHT 2.2.1.2.2
	      if (current_el != lastLIScurrentthres){
		tmp_el=remove_current_el(LIS[res]); //do not free memory here
		insert_el_after(LIS[res],current_el,&lastLIScurrentthres); 
	      }

		current_el->type=1;
	   } else {
		tmp_el=remove_current_el(LIS[res]);
#ifndef NEWTREE		
		(map_LIS[trans_pixel(tmp_el->pixel)])--;
#endif
		free(tmp_el); //la par contre, il disparait...
		
	   };
	   tmp_el=NULL;
	}else{//fin du 2.2.1.2
	   current_el=next_el(LIS[res]);
	};
	} else { //SPIHT 2.2.2 ;else the set is of type B
#ifdef DEBUG
	  nLISloopB++;
#endif
		//On doit faire l'output de Sn(L(i,j))
	  bit = 0;
if (*(coder_param.flag_arith) == 1){
			if (input_buffer.bit_cnt < *outputsize){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
//                         test++;
			bit = (unsigned char) symbol;
} else {
	if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
	   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
	} else break;
}
	  if (bit == 1){
		list_desc=list_init();//list_free(list_desc);  //decode only done
	  	r=spat_spec_desc_spiht(current_el->pixel, list_desc, 1, image, thres_ind, map_LSC);//decode only
		current_child = first_el(list_desc);
		while (current_child !=NULL){
#ifndef NEWTREE		
			if (map_LIS[trans_pixel(current_child->pixel)] == 0){
#endif
				el=el_init(current_child->pixel);

				el->thres = thres_ind;
						
		#ifdef RES_SCAL
				if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
					#ifdef SPATFIRST
					insert_el(LIS[res+1],el);
					#else
					insert_el(LIS[res+maxresspec],el);
					#endif
				} else {//this is a spectral children
					#ifdef SPATFIRST
					insert_el(LIS[res+maxresspat],el);
					#else
					insert_el(LIS[res+1],el);
					#endif				
				}
		#else 
				insert_el(LIS[res+1],el);
		#endif
#ifndef NEWTREE
				(map_LIS[trans_pixel(el->pixel)])++;
			};
#endif			
			current_child=next_el(list_desc);
		};
	  	tmp_el=remove_current_el(LIS[res]);
#ifndef NEWTREE
		(map_LIS[trans_pixel(tmp_el->pixel)])--;
#endif
	  	free(tmp_el);
		list_free(list_desc);//decode only
	  } else {
	    current_el=next_el(LIS[res]);
	  };

    };
current_el=LIS[res]->current;   
}; //endwhile
#ifdef LSCBEFORE
#ifdef RES_SCAL
}
#endif
#endif

/*******************************
LSC
********************************/
#ifndef LSCBEFORE
current_el=first_el(LSC[res]);
while ((current_el != NULL) && (current_el->thres > thres_ind) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize)){
#ifdef DEBUG
	nLSCloop++;
#endif
if (*(coder_param.flag_arith) == 1){
			if (input_buffer.bit_cnt < *outputsize){
			if (QccENTArithmeticDecode(&input_buffer, model, &symbol, 1))
			{
			QccErrorAddMessage("%s: Error calling QccENTArithmeticDecode()",
			argv1[0]);
			QccErrorExit();
			}
			} else break;
//                         test++;
			bit = (unsigned char) symbol;
} else {
	if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
	   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
	   flagLSC=1;
	} else break;
}
	if (bit == 1){
		if (image[trans_pixel(current_el->pixel)] > 0){
			image[trans_pixel(current_el->pixel)] += threshold/2;
		} else {
			image[trans_pixel(current_el->pixel)] -= threshold/2;
		};
	} else {
		if (image[trans_pixel(current_el->pixel)] > 0){
			image[trans_pixel(current_el->pixel)] -= ROUND(threshold/2.0+0.1);
		} else {
			image[trans_pixel(current_el->pixel)] += ROUND(threshold/2.0+0.1);
		};
	}
	current_el=next_el(LSC[res]);
};
#endif

if (*(coder_param.flag_arith) == 1){
if (input_buffer.bit_cnt > *outputsize){//ne doit pas servir, on devrait recuperer l'erreur de arithmeticDecode en cas de troncature de la stream
#ifdef DEBUG2
 	printf("Sortie: fin du train de bit (threshold %ld)\n", threshold);
#endif
#ifdef CHECKEND
	 printf("Sortie: fin du train de bit (blk: %d) (threshold %ld, res %d, flagLSC:%d, flagLIS:%d)\n", blockind, threshold, res, flagLSC, flagLIS);
#endif
	break;
}
} else {
if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) >= *outputsize){
#ifdef DEBUG
 	printf("Sortie: fin du train de bit (threshold %ld)\n", threshold);
#endif
#ifdef CHECKEND
	 printf("Sortie: fin du train de bit (blk: %d) (threshold %ld, res %d, flagLSC:%d, flagLIS:%d)\n", blockind, threshold, res, flagLSC, flagLIS);
#endif
	break;
};
}

};//fin threshold


//we are between the internal and the external loop in any case (RES_SCAL or not) time to check if a jump is necessary (case of partial decoding)...
#ifdef DEBUG
printf("Before jump %ld %d\n",*(datablock[blockind].streamlast),*(datablock[blockind].count));
#endif
#ifdef RES_SCAL
}
endpos += (datablock[blockind].partsize)[res];
  if (*(coder_param.flag_arith) == 1){
//   input_buffer.bit_cnt = endpos;
//TODO resynchroniser le decodeur arithmetique en lisant les output...
  } else {
  *(datablock[blockind].streamlast) = endpos /8;
  *(datablock[blockind].count) = endpos % 8;
  }
#else
if (thres_ind <= 10){
endpos += (datablock[blockind].partsize)[thres_ind];
  if (*(coder_param.flag_arith) == 1){
//   input_buffer.bit_cnt = endpos;
//TODO resynchroniser le decodeur arithmetique en lisant les output..
  } else {
  *(datablock[blockind].streamlast) = endpos /8;
  *(datablock[blockind].count) = endpos % 8;
  }
}
#endif
#ifdef DEBUG
printf("After jump %ld %d\n",*(datablock[blockind].streamlast),*(datablock[blockind].count));
#endif

// printf("test %d\n",test);
// printf("at %ld bits for thres %d\n",input_buffer.bit_cnt,thres_ind);//tmp

if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) > *outputsize){//temporaire, il faudrait faire plus fin
#ifdef DEBUG
 	printf("Sortie: fin du train de bit (threshold %ld)\n", threshold);
#endif
	break;
};

#ifndef RES_SCAL



};//fin res
#endif


#ifdef RES_SCAL
};//fin res
#endif

#ifdef DEBUG2
  if (*(coder_param.flag_arith) == 1){
printf("Processed %d bits for current block\n",input_buffer.bit_cnt);
}
#endif

#ifdef CHECKEND
check_end(image, LSC, coder_param, blockind);
if (0) 1+2;
#endif



for (i=0; i<imageprop.nres; i++){
list_flush(LSC[i]);
list_flush(LIC[i]);
list_flush(LIS[i]);
}

}// End of
}// group
}// processing

#ifdef DEBUG2
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
#endif

#ifndef NEWTREE
free(map_LSC);
free(map_LIC);
free(map_LIS);
#endif

free(count);
free(streamlast);
 
return 0;
}



