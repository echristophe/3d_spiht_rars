
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


#include "libQccPack.h"
#include "libQccPackVQ.h"
#include "libQccPackENT.h"

#include "main.h"



int print_imageprop(){
  printf("------------------------------------------------------------------\n");
  printf("Image size: %d x %d x %d\n",imageprop.nsmax, imageprop.nlmax, imageprop.nbmax);
  printf("Decomposition depth: %d subbands spectral, %d subbands spatial\n", imageprop.nresspec, imageprop.nresspat);
  printf("LLL subband size: %d x %d x %d\n",imageprop.nsmin, imageprop.nlmin, imageprop.nbmin);
  printf("Max bitplane: %d (all values are below %ld)\n",imageprop.maxquant,((long int) 1)<<(imageprop.maxquant));

  printf("------------------------------------------------------------------\n");
  return 0;
}

int print_coderparam(coder_param_struct coder_param){

  printf("Roi coding ?: %d\n", coder_param.flag_roi);
  printf("------------------------------------------------------------------\n");
  return 0;
}

/* 
  WARNING this function only output the 1 and supposed that stream has been properly
  initialized at 0 (with a calloc for example).
*/
int add_to_stream(unsigned char *stream, unsigned char *count, int input, long int *streamlast)
{
  nbitswritten++;
  stream[(*streamlast)] += (input << (*count)); //no effect if input is 0, be aware do not forget to initialize strem to 0!
  (*count)++;
  if ((*count) == 8){
    (*count) = 0;
    (*streamlast)++;
  }
  return 0;
}


unsigned char read_from_stream(unsigned char * stream, unsigned char * count, long int *streamlast){
  unsigned char output=0;
  nbitsread++;
  output=get_bit((long int) stream[*streamlast],(int) *count);
  (*count)++;
  if ((*count) == 8){
    *count = 0;
    (*streamlast)++;
  }
  return output;
}



list_el * el_init(pixel_struct pixel){
  list_el * ptr;
  ptr = (list_el *) malloc( sizeof(list_el));
  ptr->next = NULL;
  ptr->pixel = pixel;
  ptr->type = 0;
  ptr->thres = 0;
  return ptr;
}

list_struct * list_init(void){
  list_struct * ptr;
  ptr = (list_struct *) malloc( sizeof(list_struct));
  ptr->first = NULL;
  ptr->last = NULL;
  ptr->current = NULL;
  ptr->previous = NULL;	
  return ptr;
}

void list_free(list_struct * list){
  list_el * previous=NULL;
  list_el * current_el=NULL;
  if (list != NULL){
    previous=list->first;
    current_el=list->first;
    while (current_el != NULL){
      previous=current_el;
      current_el=current_el->next;
      free(previous);
    }
    free(list);
  }
}

void list_flush(list_struct * list){
  list_el * previous=NULL;
  list_el * current_el=NULL;
  if (list != NULL){
    previous=list->first;
    current_el=list->first;
    while (current_el != NULL){
      previous=current_el;
      current_el=current_el->next;
      free(previous);
    }
    list->first=NULL;
    list->last=NULL;
    list->current=NULL;
    list->previous=NULL;
  }
}

list_el * first_el(list_struct * list){
  list->previous=NULL;
  list->current=list->first;
  return list->current;
}


list_el * next_el(list_struct * list){
  if (list->current != NULL){
    list->previous=list->current;
    list->current=list->previous->next;
    return list->current;
  } else {
    list->previous = NULL;
    list->current = list->first;
    return list->current;
  }
}

/*Insert el at the end of list*/
void insert_el(list_struct * list, list_el * el)
{
  if (list->last == NULL){/* The list is still empty*/ 
    list->last = el;
    list->first = el;
  }
  else {
    list->last->next = el;
    list->last = el;
    list->last->next=NULL;
  }
}


void insert_el_inplace(list_struct * list, list_el * el)
{
  if (list->first == NULL){ /* The list is still empty*/
    list->last = el;
    list->first = el;
    list->last->next = NULL;
  } else {
    if (list->current == NULL){//we want to put this element at the first place
      el->next = list->first;
      list->first = el;
    } else {
      if (list->current == list->last){
        list->last = el;
        list->last->next = NULL;
      }
      el->next = list->current->next;
      list->current->next = el;
    }
  }
}

/*Insert el after 'place' and change 'place' to this element*/
void insert_el_after(list_struct * list, list_el * el, list_el ** place)
{
  if ((*place) != el){
    if ((*place) != NULL){
      el->next = (*place)->next;
      if ((*place) == list->last) list->last = el;
      (*place)->next = el;
    } else {
      printf("This should not happen\n");
      list->last = el; // this should not happen
    }
  }
  (*place) = el;
}

list_el * remove_current_el(list_struct * list)
{
  list_el * ptr;
  ptr=list->current;
  if (list->current == list->first){/*if current is the first*/
    if (list->current->next == NULL){/*list is now empty*/
      list->first = NULL;
      list->last = NULL;
      list->current = NULL;
      list->previous = NULL;
    } else {
      list->first = list->current->next; 
      list->current = list->first;//on passe au suivant
      list->previous = NULL;
    }
  } else {
    if (list->current == list->last){/*if current is the last*/
      if (list->previous == NULL){/*This should NEVER happen*/
        list->first = NULL;
        list->last = NULL;
        list->current = NULL;
        list->previous = NULL;
        printf("This should NEVER happen: list inconsistent\n");
      } else {
        list->last = list->previous;
        list->current=NULL;
        list->last->next=NULL;
      }
    } else {
      list->previous->next = list->current->next;
      list->current = list->previous->next;
    }
  }
  return ptr; 
}

int check_list(list_struct * list){
  list_el * el_current=NULL;
  list_el * el_previous=NULL;
  list_el * el;
  el_current=list->current;
  el_previous=list->previous;
  el = first_el(list);
  while (el != NULL){
    el=next_el(list);
  };
  if ((list->previous) == (list->last)){
    list->current=el_current;
    list->previous=el_previous;
    return 0;
  } else {
    list->current=el_current;
    list->previous=el_previous;
    return 1;
  }
};

int count_list(list_struct * list){
  int count=0;
  printf("WARNING: destructif effect on list");
  list->current=list->first;
  list->previous=NULL;
  while (list->current !=NULL){
    count++;
    next_el(list);
  };
  return count;	
};

int check_map(unsigned char * map){
  int i,j,k;
  for (k=0;k<imageprop.nbmax;k++){
    for (j=0;j<imageprop.nlmax;j++){
      for (i=0;i<imageprop.nsmax;i++){	
        if (map[i + j*imageprop.nsmax + k*imageprop.nsmax*imageprop.nlmax] > 1) {
          fprintf(stderr, "Map erronee: %d %d %d\n",i,j,k);
          return 1;
        };
      };
    };
  };
  return 0;
};

int print_list(list_struct * list)
{
  list_el * el_current=NULL;
  list_el * el_previous=NULL;
  list_el * el=NULL;
  int output;
  el_current=list->current;
  el_previous=list->previous;
  el = first_el(list);
  while (el != NULL){
    printf("%d %d %d (t=%d) (type=%d)\n",el->pixel.x,el->pixel.y,el->pixel.l,el->thres,el->type);
    el=next_el(list);
  };
  
  if ((list->previous) == (list->last)) {
    list->current=el_current;
    list->previous=el_previous;
    output= 0;
  } else {
    list->current=el_current;
    list->previous=el_previous;
    output= 1;
  };
  return output;
};


long int count_map(unsigned char * map, long int size){
  long int i=0;
  long int count=0;
  for (i=0; i<size; i++){
    if (map[i] == 1){count++;};
  };
  return count;
};


unsigned char get_bit(long int value,int thres_ind){
    return ( (abs(value) &( ((long int) 1)<<thres_ind)) != 0);
}


int isLSC(long int value_pix, int thres_ind){
  long int threshold= 1 << (long int)thres_ind;
  if ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold)) {
    return 1;
  } else {
    return 0;
  }
};


parents_struct find_parents(pixel_struct pixel){
  parents_struct parents;
  parents.spat.x=-1;//spatial parent
  parents.spat.y=-1;
  parents.spat.l=-1;
  parents.spec.x=-1;//spectral parent
  parents.spec.y=-1;
  parents.spec.l=-1;

  if ((pixel.x >= 2*imageprop.nsmin) || (pixel.y >= 2*imageprop.nlmin)){//cas classique
    parents.spat.x=pixel.x/2;
    parents.spat.y=pixel.y/2;
    parents.spat.l=pixel.l;	
  } else {
    if ((pixel.x >= imageprop.nsmin) && (pixel.y < imageprop.nlmin)){//2
      parents.spat.x=((pixel.x-imageprop.nsmin)/2)*2+1;
      parents.spat.y=(pixel.y/2)*2;
      parents.spat.l=pixel.l;
    }
    if ((pixel.x < imageprop.nsmin) && (pixel.y >= imageprop.nlmin)){//3
      parents.spat.x=(pixel.x/2)*2;
      parents.spat.y=((pixel.y-imageprop.nlmin)/2)*2+1;
      parents.spat.l=pixel.l;
    }
    if ((pixel.x >= imageprop.nsmin) && (pixel.y >= imageprop.nlmin)){//4
      parents.spat.x=((pixel.x-imageprop.nsmin)/2)*2+1;
      parents.spat.y=((pixel.y-imageprop.nlmin)/2)*2+1;
      parents.spat.l=pixel.l;
    }
  }


  if (pixel.l >= 2* imageprop.nbmin){
      parents.spec.x=pixel.x;
      parents.spec.y=pixel.y;
      parents.spec.l=(pixel.l/2);		
  } else { 
    if (pixel.l >= imageprop.nbmin){
      parents.spec.x=pixel.x;
      parents.spec.y=pixel.y;
      parents.spec.l=((pixel.l-imageprop.nbmin)/2)*2+1;
      if (parents.spec.l == imageprop.nbmin) parents.spec.l = imageprop.nbmin-1;//a ajouter au spat si on veut des longueurs impaires dans la LL
    }	
  }
  
  return parents;
};

int is_accessible_from(pixel_struct pixel,unsigned char * map){
  parents_struct parents;
  int out=0;
  parents = find_parents(pixel);
  if (parents.spat.x != -1){
    if (map[trans_pixel(parents.spat)] == 1){
      return 1;
    } else {
      out=is_accessible_from(parents.spat,map);
    }
  }
  if (out == 1) return 1;
  if (parents.spec.x != -1){
    if (map[trans_pixel(parents.spec)] == 1){
      return 1;
    } else {
      out=is_accessible_from(parents.spec,map);
    }
  }
  return out;
}

#ifdef DEBUG
int check_accessibility_of_all(unsigned char * map_LSC, unsigned char * map_LIC, unsigned char * map_LIS){
  int i,j,k, out;
  pixel_struct pixel;
  out=0;
  printf("Verification d'accessibilite\n");
  for (i=0; i<imageprop.nsmax; i++){
    printf("%d ",i);
    for (j=0; j<imageprop.nlmax; j++){
      for (k=0; k<imageprop.nbmax; k++){
        pixel.x=i;
        pixel.y=j;
        pixel.l=k;
        out=0;
        if ((map_LSC[trans_pixel(pixel)] == 0) &&
          (map_LIC[trans_pixel(pixel)] == 0)){
          out=is_accessible_from(pixel,map_LIS);
        } else {
          out=1;
        }
        if (out == 0) {
          printf("BEEEEEP !!\n");
          return 0;
        };
      }
    }
  }
  printf("\n");
  return out;
}
#endif


//update distortion value
int update_dist(pixel_struct pixel, int thres_ind, long long int * dist,  long int *image){
  long long int value=0;
  value = (long long int) abs(image[trans_pixel(pixel)]);
  *dist -= (1<<thres_ind)
          *(2*(value - ((value>>(thres_ind+1))<<(thres_ind+1)))-(1<<thres_ind));
  return 0;
}

int update_dist_first(pixel_struct pixel, int thres_ind, long long int * dist,  long int *image){
  long long int value=0;
  int weightingFactor;
  weightingFactor = compute_weightingFactor(pixel);
  value = (long long int) abs(image[trans_pixel(pixel)]);
//   *dist -= -weightingFactor*((1<<(thres_ind-1))+(1<<(thres_ind)))
//           *(2*value -(1<<(thres_ind-1)) - (1<<(thres_ind)));
  if (thres_ind != 0 ){
    *dist -= weightingFactor*((1<<(thres_ind-1))+(1<<(thres_ind)))
          *(2*value -( (1<<(thres_ind-1)) + (1<<(thres_ind)) ) );
  } else {
    *dist -= 1;
  }
  return 0;
}

int update_dist1(pixel_struct pixel, int thres_ind, long long int * dist,  long int *image){
  long long int value=0;
  int weightingFactor;
  weightingFactor = compute_weightingFactor(pixel);
  value = (long long int) abs(image[trans_pixel(pixel)]);
  if (thres_ind != 0 ){
    *dist -= weightingFactor*(1<<(thres_ind-1))
          *(2*(value - ((value>>(thres_ind+1))<<(thres_ind+1))-(1<<(thres_ind)) )-(1<<(thres_ind-1)));
  } else {
    //nothing should happen here
  }
  return 0;
}

int update_dist0(pixel_struct pixel, int thres_ind, long long int * dist,  long int *image){
  long long int value=0;
  int weightingFactor;
  weightingFactor = compute_weightingFactor(pixel);
  value = (long long int) abs(image[trans_pixel(pixel)]);
  if (thres_ind != 0 ){
  *dist -= -weightingFactor*(1<<(thres_ind-1))
          *(2*(value - ((value>>(thres_ind+1))<<(thres_ind+1))-(1<<(thres_ind)) )+(1<<(thres_ind-1)));
  }else {
//     *dist -= weightingFactor*(1<<thres_ind)
//           *(2*(value - ((value>>(thres_ind+1))<<(thres_ind+1))-(1<<(thres_ind)) )-(1<<thres_ind));
     *dist -= 1;
  }
  return 0;
}

int compute_weightingFactor(pixel_struct pixel){
  double factorLowPass = 0.982954; //valid only for the 9/7 wavelet
  double factorHighPass = 1.040435;
//   double factorLowPass = 1/1.040435; //valid only for the 9/7 wavelet
//   double factorHighPass = 1/0.982954;
  int spec_pos = 0;
  int spat_pos = 0;
  int currentValuex=pixel.x;
  int currentValuey=pixel.y;
  int currentValuel=pixel.l;
  double spectralFactor = 1.0;
  double spatialFactor = 1.0;
  int finalFactor = 0;
  if (currentValuex < imageprop.nsmin) currentValuex=imageprop.nsmin-1;
  if (currentValuey < imageprop.nlmin) currentValuey=imageprop.nlmin-1;
  if (currentValuel < imageprop.nbmin) currentValuel=imageprop.nbmin-1;
  while (currentValuel < imageprop.nbmax/2){//forcement, ca marche pas avec 0....
    spec_pos++;
    currentValuel = currentValuel << 1;
    spectralFactor *= factorLowPass;
  }
  if (spec_pos != imageprop.nresspec-1){spectralFactor *= factorHighPass;}
  
  
  while ((currentValuex < imageprop.nsmax/2) && (currentValuey < imageprop.nlmax/2)){
    spatialFactor *= factorLowPass * factorLowPass;
    currentValuex = currentValuex << 1;
    currentValuey = currentValuey << 1;
    spat_pos++;
  }
  if (spat_pos != imageprop.nresspat -1){
     spatialFactor *= factorHighPass;
     if ((currentValuex >= imageprop.nsmax/2) && (currentValuey >= imageprop.nlmax/2)){
       spatialFactor *= factorHighPass;
     } else {
       spatialFactor *= factorLowPass;
     }
  }
  finalFactor = round(WEIGHTMULTVALUE*spatialFactor*spectralFactor);
//   finalFactor = round(WEIGHTMULTVALUE/(spatialFactor*spectralFactor));
#ifdef TEMPWEIGHTCHECKING
  imageweight[trans_pixel(pixel)] += finalFactor;
  imageweightcount[trans_pixel(pixel)] += 1;
#endif
  return finalFactor;
//   return WEIGHTMULTVALUE; //WARNING
}

//Is it time to add the cutting point to the rate-distortion list ?
int add_to_rddata(rddata_struct *rddata, long long int rate, long long int dist){
  //WARNING to invert below
#ifdef OLDRATE0 
  if ((*rddata).reval[(*rddata).ptcourant] > rate) {
    return 0;
  } else {
    (*rddata).r[(*rddata).ptcourant] = rate;
    (*rddata).d[(*rddata).ptcourant] = dist;
    (*rddata).ptcourant++;
  }
  return 1;
#else
    if ((rate > (*rddata).reval[((*rddata).ptcourant)+1]) 
         && ((*rddata).ptcourant < NUMRD-1)) {
      //update to new point position
      (*rddata).ptcourant++;
    }
    if (( (*rddata).d[(*rddata).ptcourant] == 0 ) 
            || ( dist < (*rddata).d[(*rddata).ptcourant])) {
      //now we know we are at the right position, take the lower dist point
      (*rddata).r[(*rddata).ptcourant] = rate;
      (*rddata).d[(*rddata).ptcourant] = dist;
    }
    return 0;
#endif
}

//Compute cost J for a given lambda
//and put ptcourant to the min
//TODO cost_j value useful for debuging but useless waste of memory
//to remove later...
int compute_cost(rddata_struct *rddata, float lambda){
  int i=0;
  float min=0;
  if (lambda != 0) {
    for (i=0; i< NUMRD; i++){
      (*rddata).cost_j[i] =  (*rddata).d[i] + lambda*(*rddata).r[i];
      if ((*rddata).cost_j[i] != 0){//otherwise, it means not really computed
        if (min == 0){ //first point to add there
          min = (*rddata).cost_j[i];
          (*rddata).ptcourant = i;
        } 
        else {
          if ((*rddata).cost_j[i] < min){ // reread carefully in case of equality to choose the best TODO
            min=(*rddata).cost_j[i];
            (*rddata).ptcourant = i;
          }
        }
      }
    }	
  } 
  else {//lambda = 0 we want lossless here
    min = (*rddata).d[NUMRD-1];//should be the final answer anyway unless particular situation (all even numbers for eg).
    (*rddata).ptcourant = NUMRD-1;
    if (0){
    for (i=0; i< NUMRD; i++){
      if ((*rddata).r[i] !=0 ){
        if (((*rddata).d[i]  <= min) && ((*rddata).d[i] < (*rddata).d[(*rddata).ptcourant])){//TODO to reread carefully, we want the min here
                min=(*rddata).d[i];
                (*rddata).ptcourant = i;
        }
      }
    }
    }
  }
  if ((*rddata).ptcourant == NUMRD) fprintf(stderr, "Compute_cost to check...\n");//test 
  return (*rddata).ptcourant;//this is the minimum position
}

//fin the lambda corresponding to a given rate
float compute_lambda(datablock_struct *datablock, long int rate, int nblock){
  float lambda_inf = 0.0;
  float lambda_sup = 500000.0*WEIGHTMULTVALUE;
  float lambda = 0.0;
  long int current_rate=0;
#ifdef RES_RATE
// 	int posmin[NRES];
  int * posmin = (int *) calloc(imageprop.nres, sizeof(int));
#else
  int posmin;
#endif
  int i;
#ifdef RES_RATE
  int j;
#endif	
  while ((lambda_sup-lambda_inf) > 1.0*WEIGHTMULTVALUE) {
    current_rate=0;
    lambda = (lambda_sup + lambda_inf) /2.;
    for (i=0;i<nblock;i++){
      
#ifdef RES_RATE
      for (j=imageprop.nres-1;j>=0;j--){
      posmin[j]=compute_cost(&(datablock[i].rddata[j]),lambda);
              if ((j<imageprop.nres-imageprop.nresspec) && (posmin[j] < posmin[j+imageprop.nresspec])) posmin[j]=posmin[j+imageprop.nresspec];
      if (((j % imageprop.nresspec)!=imageprop.nresspec-1) && (posmin[j] < posmin[j+1])) 
              posmin[j]=posmin[j+1];
      current_rate += datablock[i].rddata[j].r[posmin[j]];
      }
#else
      posmin=compute_cost(&(datablock[i].rddata),lambda);
      current_rate += datablock[i].rddata.r[posmin];
#endif
    }
    if (current_rate > rate) {
            lambda_inf=lambda;
    } else {
            lambda_sup=lambda;
    }
  }
  printf("Going for lambda=%f giving rate %ld\n",lambda,current_rate);
  return lambda;
}

//functions to take care of data_block
int datablock_init(datablock_struct *datablock){
  (*datablock).stream = (unsigned char *) calloc(sizeblockstream,sizeof(unsigned char *));
  if ((*datablock).stream == NULL) {
      fprintf(stderr, "******** Allocation problem in datablock_init (stream)\n");
  };
  (*datablock).streamlast = (long int *) malloc(sizeof(long int));
  if ((*datablock).streamlast == NULL) {fprintf(stderr, "******** Allocation problem in datablock_init (streamlast)\n");};
  (*datablock).count = (unsigned char *) malloc(sizeof(unsigned char));
  if ((*datablock).count == NULL) {fprintf(stderr, "******** Allocation problem in datablock_init (count)\n");};
  #ifdef RES_SCAL
  (*datablock).partsize = (long int *) calloc(imageprop.nres,sizeof(long int));
  #else
  (*datablock).partsize = (long int *) calloc(imageprop.maxquant+1,sizeof(long int));	
  #endif
  if ((*datablock).partsize == NULL) {fprintf(stderr, "******** Allocation problem in datablock_init (partsize)\n");};
  *((*datablock).streamlast) = 0;
  *((*datablock).count)=0;
  (*datablock).currentpos=0;
  return 0;
}

int datablock_free(datablock_struct *datablock){
  free((*datablock).stream);
  free((*datablock).streamlast);
  free((*datablock).count);
  free((*datablock).partsize);
  return 0;
}

//output the next part size (TODO there is definitly more efficient way to do that...)
int add_to_stream_number(unsigned long int number, unsigned char * stream, unsigned char *count, long int *streamlast, int numbits){
  int i=0;
  if (number > (1<<(32-2))) {//taking a 2 bits marging again... 
    printf("********** WARNING Possible error in size encoding *************\n");
    return 1;
  }
  if (number >= (1<<numbits)) {
    fprintf(stderr,"********** WARNING Header size too short for encoding *************\n");
  }
  for (i=0;i<numbits;i++){
    add_to_stream(stream, count, (int) get_bit(number,i), streamlast);
  }
#ifdef SIZE
  nbitswrittenheader += numbits;
#endif
  return 0;
}

unsigned long int read_from_stream_number(unsigned char * stream, unsigned char *count, long int *streamlast, int numbits){
  unsigned long int n=0;
  unsigned char bit=0;
  int i;
  for (i=0; i<numbits;i++){
          bit=read_from_stream(stream, count, streamlast);
          if (bit ==1){
                  n += (1<<i);//ca devrait etre dans le meme ordre qu'a l'ecriture
          }
  }
#ifdef SIZE
  nbitsreadheader += numbits;
#endif
  return n;
}

//copy data from currentpos to rate from datablock[].stream do stream
//possibilite de passer a une gestion en octet... plus rapide pour les copies
//permettrai de garder la taille en octet en memoire (long int -> int  gain 16 bits)
//ne pas oublier en outside d'updater currentpos !!!
int copy_to_stream(long int currentpos, long long int rate, unsigned char * streamin, unsigned char * streamout, unsigned char * count, long int * streamlast, long int insize){
  long int i=0;
  unsigned char bit=0;
  unsigned char countin;
  long int streamlastin;
  countin = currentpos % 8;
  streamlastin = (currentpos) / 8;
  for (i=0; i < (rate-currentpos); i++){
    if (streamlastin*8+countin > insize) {
      fprintf(stderr, "******** Reading overflow (could be normal if stream truncated, to check\n");
      return 1;
    };
    bit = read_from_stream(streamin, &countin, &streamlastin);
    add_to_stream(streamout, count, bit, streamlast);
  }
  return 0;
}


//interleaving block streams for lambda

int interleavingblocks(datablock_struct *datablock, int nblock, unsigned char * stream, unsigned char * count, long int * streamlast, float lambda, int *flagfirst){
  int i=0;
#ifdef RES_RATE
  int posmin;
#else
  int posmin;
#endif
  long int insize;
  long long int rate;
  int j;
  long int sum=0;

  for (i=0;i<nblock;i++){
    //ajout des headers de block
    if (*flagfirst){
      sum = 0;
      #ifdef RES_SCAL
      for (j=0;j<imageprop.nres;j++){
#ifdef RES_RATE
//WARNING using RES_RATE option is probably valid only when ONE layer is used
          if (lambda != 0.0) {
            posmin=datablock[i].rddata[j].ptcourant;
          } 
          else {
            posmin=compute_cost(&(datablock[i].rddata[j]),0.0);//would be faster just to take the last point
          }
          sum = datablock[i].rddata[j].r[posmin];//voltige
          add_to_stream_number(sum, stream, count, streamlast, NUMBITSPARTSIZE);
#else
          sum=0;
          sum += (datablock[i].partsize)[j];
          add_to_stream_number(sum, stream, count, streamlast, NUMBITSPARTSIZE);
#endif
        }
        #else
        for (j=imageprop.maxquant;j>=0;j--){
          if (j<10) sum=0;
          sum += (datablock[i].partsize)[j];
          if (j<=10) add_to_stream_number(sum, stream, count, streamlast, NUMBITSPARTSIZE);
        }	
        #endif
        }
        //fin ajout header de block
#ifdef RES_RATE
        for (j=0;j<imageprop.nres;j++){
          posmin=datablock[i].rddata[j].ptcourant;
          if (posmin > imageprop.nres) {
            fprintf(stderr, "******** Pb with posmin: %d \n", posmin);
          }
          rate=datablock[i].rddata[j].r[posmin];//be careful, here the rate is for the resolution only (not the total from 0)
          insize = (*(datablock[i].streamlast))*8+(*(datablock[i].count));
          if (insize > sizeblockstream*8) {fprintf(stderr, "******** Block size overflow: %ld \n", insize);}
          //output the size of the next part in stream

          //copy the corresponding number of bits in stream
          copy_to_stream((datablock[i].currentpos), rate+(datablock[i].currentpos), datablock[i].stream,  stream, count, streamlast, insize);
          //jump to the end
          datablock[i].currentpos += (datablock[i].partsize)[j];
        }
        #ifdef DEBUG2
        printf("Cutting point for %d (res %d): %lld (adding %lld to stream WRONG)\n", i,j, rate, rate - (datablock[i].currentpos));
        printf("Block %d begin at position %ld\n",i, (*streamlast)*8+ (*count));
        #endif
#else
        posmin=compute_cost(&(datablock[i].rddata),lambda);//this is not needed if compute_lambda was called before
        rate=datablock[i].rddata.r[posmin];
        insize = (*(datablock[i].streamlast))*8+(*(datablock[i].count));
        if (insize > sizeblockstream*8) {fprintf(stderr, "******** Block size overflow: %ld \n", insize);}
        //output the size of the next part in stream
        #ifdef DEBUG2
        printf("Cutting point for %d : %lld (adding %lld to stream)\n", i, rate, rate - (datablock[i].currentpos));
        printf("Block %d begin at position %ld\n",i, (*streamlast)*8+ (*count));
        printf("  which is the point %d\n",datablock[i].rddata.ptcourant);
        #endif
        add_to_stream_number(rate - (datablock[i].currentpos), stream,count,streamlast, NUMBITSPARTSIZE);
        //copy the corresponding number of bits in stream
        copy_to_stream((datablock[i].currentpos), rate, datablock[i].stream,  stream, count, streamlast, insize);
        (datablock[i].currentpos)=rate;
        

#endif
  }
  *flagfirst = 0;
  return 0;
}

//desinterleaving block and storing in datablock_struct
int desinterleavingblocks(datablock_struct *datablock, int nblock, stream_struct streamstruct, long int  insize, int nlayer){
  int i=0;
  int j;
  int err=0;
  int layer=0;
  long int pos;
  unsigned long int nbits=0;
  unsigned char * stream;
  unsigned char count=0;
  long int streamlast=0;
  int flagfirst=1;

  stream = streamstruct.stream;
  count = *(streamstruct.count);
  streamlast = *(streamstruct.streamlast);

  while ( ((streamlast*8+ count) <= insize) && (layer<nlayer) ){
    //ajout des headers de block
    if (flagfirst){
      #ifdef RES_SCAL
      for (j=0;j<imageprop.nres;j++){
        (datablock[i].partsize)[j]=read_from_stream_number(stream, &count, &streamlast, NUMBITSPARTSIZE);
      }
      #else
      for (j=imageprop.maxquant;j>=0;j--){
        if (j>10) (datablock[i].partsize)[j] = -1;
        if (j<=10) (datablock[i].partsize)[j] = read_from_stream_number(stream, &count, &streamlast, NUMBITSPARTSIZE);
      }	
      #endif
    }
    //fin lecture header de block
#ifdef RES_RATE
    for (j=0;j<imageprop.nres;j++){
      nbits = (datablock[i].partsize)[j];
      pos = streamlast*8+count;
      if (pos < 0) {fprintf(stderr, "******** pos neg in desinterleavingblocks\n");};
      err=copy_to_stream(pos, pos + nbits, stream, datablock[i].stream, datablock[i].count, datablock[i].streamlast, insize);
      if (err) {return 1;}
      streamlast =  (pos + nbits)/8;
      count = (pos + nbits)%8;
    }
    i++;
    if (i == nblock) {i=0;flagfirst=0;layer++;};
#else
    nbits = read_from_stream_number(stream,&count,&streamlast,NUMBITSPARTSIZE);
    pos = streamlast*8+count;
    if (pos < 0) {fprintf(stderr, "******** pos neg in desinterleavingblocks\n");};
    #ifdef DEBUG
    printf("Cutting point for %d : %ld (adding %lu to stream)\n", i, ((*(datablock[i].stream))*8+(*(datablock[i].count))+nbits), nbits);
    #endif
    err=copy_to_stream(pos, pos + nbits, stream, datablock[i].stream, datablock[i].count, datablock[i].streamlast, insize);//ce serait plus simple avec un objet stream...
    if (err) {return 1;}
    streamlast =  (pos + nbits)/8;
    count = (pos + nbits)%8;
    i++;
    if (i == nblock) {i=0;flagfirst=0;layer++;};
#endif
  }
  return 0;
}



long int file_size(FILE *f){
  long int currentpos;
  long int end;
  
  currentpos = ftell (f);
  fseek (f, 0, SEEK_END);
  end = ftell (f);
  fseek (f, currentpos, SEEK_SET);
  
  return end;
}

int init_coder_param(coder_param_struct * coder_param, int nblock){
  (*coder_param).nblock = nblock;
  (*coder_param).maxres = (char *) calloc(nblock,sizeof(char *));
// #ifdef RES_SCAL
  (*coder_param).maxresspat = (char *) calloc(nblock,sizeof(char *));
  (*coder_param).maxresspec = (char *) calloc(nblock,sizeof(char *));
// #endif
  (*coder_param).maxquant = (char *) calloc(nblock,sizeof(char *));
  (*coder_param).minquant = (char *) calloc(nblock,sizeof(char *));
// 	(*coder_param).nlayer = 100;
  return 0;
}

int free_coder_param(coder_param_struct * coder_param){
  (*coder_param).nblock = 0;
  free((*coder_param).maxres);
// #ifdef RES_SCAL
  free((*coder_param).maxresspat);
  free((*coder_param).maxresspec);
// #endif
  free((*coder_param).maxquant);
  free((*coder_param).minquant);

  free((*coder_param).filename);
  free((*coder_param).output_filename);
  free((*coder_param).roi_filename);

  free((*coder_param).flag_meansub);
  free((*coder_param).flag_arith);
  free((*coder_param).flag_spiht);
  free((*coder_param).flag_wavelet_int);

  free(coder_param);
  return 0;
}

int preinit_coder_param(coder_param_struct * coder_param){
  (*coder_param).filename = (char *) calloc(256,sizeof(char));
  (*coder_param).output_filename = (char *) calloc(256,sizeof(char));
  (*coder_param).roi_filename = (char *) calloc(256,sizeof(char));
  (*coder_param).type = 2;
  (*coder_param).nlayer = 100;
  (*coder_param).rate = 0;
  (*coder_param).flag_roi = 0;
  (*coder_param).flag_meansub=(unsigned char *) calloc(1, sizeof(unsigned char));
  (*coder_param).flag_arith= (unsigned char *) calloc(1, sizeof(unsigned char));
  (*coder_param).flag_spiht= (unsigned char *) calloc(1, sizeof(unsigned char));
  (*coder_param).flag_wavelet_int= (unsigned char *) calloc(1, sizeof(unsigned char));
  return 0;
}

// int prefree_coder_param(coder_param_struct * coder_param){
// 	free((*coder_param).filename);
// 	free((*coder_param).output_filename);
// 	free(coder_param);
// 	return 0;
// }

long int find_max(long int * image, long int npix){
  long int i_l=0;
  long int max =0;
  for (i_l=0;i_l<npix;i_l++){
    if (abs(image[i_l]) > max) max = abs(image[i_l]);
  }
  return max;
}

int compute_mean(long int * image, long int *mean){
  long int i_l=0;
  int i=0;
  int npixband;
  long long int meantmp=0;
  npixband=imageprop.nsmax*imageprop.nlmax;
  for (i=0;i<imageprop.nbmax;i++){
    meantmp=0;
    for (i_l=0;i_l<npixband;i_l++){
      meantmp += image[i_l+i*npixband];
    }
    mean[i] = meantmp / npixband;//Does not need to be precise ok for the rounding
  }
  return 0;
}

//substract one mean per band
int substract_mean(long int * image, long int *mean){
  long int i_l=0;
  int i=0;
  int npixband;
  npixband=imageprop.nsmax*imageprop.nlmax;
  for (i=0;i<imageprop.nbmax;i++){
    for (i_l=0;i_l<npixband;i_l++){
      image[i_l+i*npixband] -=  mean[i];
    }
  }
  return 0;
}

int add_mean(long int * image, long int *mean){
  long int i_l=0;
  int i=0;
  int npixband;
  npixband=imageprop.nsmax*imageprop.nlmax;
  for (i=0;i<imageprop.nbmax;i++){
    for (i_l=0;i_l<npixband;i_l++){
          image[i_l+i*npixband] +=  mean[i];
    }
  }
  return 0;
}

#ifdef CHECKEND
int check_end(long int *image, list_struct ** list, coder_param_struct coder_param, int blockind){
  int res;
  list_el * current_el=NULL;
  list_el * current_sav=NULL;
  list_el * previous_sav=NULL;	
  long int err, maxerr;
  
  for (res=0; res<(coder_param.maxres[blockind]); res++){	
    current_sav = list[res]->current;
    previous_sav = list[res]->previous;
    current_el=first_el(list[res]);
    err=0; maxerr=0;
    while (current_el != NULL){
      err=abs(image[trans_pixel(current_el->pixel)]-imageoriglobal[trans_pixel(current_el->pixel)]);
      if (err > maxerr) maxerr=err;
      current_el=next_el(list[res]);
    }
    list[res]->current = current_sav;
    list[res]->previous = previous_sav;
    printf("Max err for res %d blk %d: %ld\n", res, blockind, maxerr);
  }
  return 0;
}
#endif


// int waveletscaling(long int * image, int spatdec, int specdec, int dir ){
// 
//   int i,j,k,l;
//   int currentns,currentnl,currentnb;
//   int iindex, jindex, kindex;
//   double multfact,multfactfull;
//   double * mask;
//   
//   mask = (double *) calloc(imageprop.nsmax * imageprop.nlmax,sizeof(double));
// //fabrication du mask
//   for (i=0;i<imageprop.nsmax;i++){
//     for (j=0;j<imageprop.nlmax;j++){
//       multfact=1.0;
//       currentns=imageprop.nsmin;
//       iindex=spatdec-1;
//       while (i >= currentns) {
//         currentns *= 2;
//         iindex--;
//       }
//       currentnl=imageprop.nlmin;
//       jindex=spatdec-1;
//       while (j >= currentnl) {
//         currentnl *= 2;
//         jindex--;
//       }
//       
//       if (iindex > jindex+1) iindex = jindex+1;
//       if (jindex > iindex+1) jindex = iindex+1;	
// 
//       if ((iindex == spatdec-1) && (jindex == spatdec-1)){
//         multfact = (1 << spatdec);
//       } 
//       else {
//         for (l=0; l<iindex+jindex;l++){
//           multfact *= M_SQRT2;
//         }
//         if (iindex <0 ) multfact /= M_SQRT2;
//         if (jindex <0 ) multfact /= M_SQRT2;
//         if ((iindex == jindex-1) || (iindex == jindex+1)){
//           multfact *= M_SQRT2;
//         }
//       }
//       mask[i+imageprop.nsmax*j]=multfact;
//     }
//   }
// 
//   for (k=0;k<imageprop.nbmax;k++){
//     multfact=1.0;
//     currentnb=imageprop.nbmin;
//     kindex=specdec-1;
//     while (k >= currentnb) { //WARNING: won't be working for any size
//       currentnb *= 2;
//       kindex--;
//     }
//     for (l=0; l<kindex;l++){
//       multfact *= M_SQRT2;
//     }
//     if (kindex <0 ) multfact /= M_SQRT2;
//       for (i=0;i<imageprop.nsmax;i++){
//         for (j=0;j<imageprop.nlmax;j++){
//           multfactfull = multfact * mask[i+imageprop.nsmax*j];
//           if (dir ==0) {
//             image[i+imageprop.nsmax*(j+imageprop.nlmax*k)] = 
//                 round(image[i+imageprop.nsmax*(j+imageprop.nlmax*k)]
//                                          *multfactfull);
//           } 
//           else {
//           image[i+imageprop.nsmax*(j+imageprop.nlmax*k)] =
//                 round(image[i+imageprop.nsmax*(j+imageprop.nlmax*k)] 
//                                        / multfactfull);
//           }
//         }
//       }
// 
//   }
//   return 0;
// }


int output_rd(datablock_struct *datablock, int nblock){
  int i;
  int status;
  FILE *r_file;
  FILE *d_file;
  
  r_file = fopen("output_r.dat", "w");
  d_file = fopen("output_d.dat", "w");
  for (i=0;i<nblock;i++){
    status = fwrite(datablock[i].rddata.r, 8, NUMRD, r_file);
    status = fwrite(datablock[i].rddata.d, 8, NUMRD, d_file);
  }
  status = fclose(r_file);
  status = fclose(d_file);
  
  return 0;
}



long int * read_hyper(char * filename, long int npix, int type){
  FILE *data_file;
  long int * image;
  short int * image_short;
  unsigned char * image_byte;
  long int i_l;
  int status=0;

  image = (long int *) calloc(npix,sizeof(long int));
  data_file = fopen(filename, "r");
  if (data_file == NULL) {
    fprintf(stderr, "Error opening file...\n");
    return NULL;
  }

  if (type ==1){
    image_byte = (unsigned char *) calloc(npix,sizeof(short int));
    status = fread(image_byte, 1, npix, data_file);
    for (i_l=0;i_l<npix;i_l++){
      image[i_l] = (long int) image_byte[i_l];
    }
    free(image_byte);
  }
  if (type ==2){
    image_short = (short int *) calloc(npix,sizeof(short int));
    status = fread(image_short, 2, npix, data_file);
    for (i_l=0;i_l<npix;i_l++){
      image[i_l] = (long int) image_short[i_l];
    }
    free(image_short);
  }
  if (type ==4){
      status = fread(image, 4, npix, data_file);
  }
  status = fclose(data_file);
  return image;
}


int write_hyper(char * filename, long int * image, long int npix, int type){
  FILE *data_file;
  int status=0;
  short int * image_short;
  unsigned char * image_byte;
  long int i_l=0;

  data_file = fopen(filename, "w");
  if (data_file == NULL) {
    fprintf(stderr, "Error opening file...\n");
    return 1;
  }

  if (type == 1){
    image_byte = (unsigned char *) calloc(npix,sizeof(unsigned char));
    for (i_l=0;i_l<npix;i_l++){
      image_byte[i_l] = (unsigned char) image[i_l];
    }
    status = fwrite(image_byte, type, npix, data_file);
    free(image_byte);
  }
  if (type == 2){
    image_short = (short int *) calloc(npix,sizeof(short int));
    for (i_l=0;i_l<npix;i_l++){
      image_short[i_l] = (short int) image[i_l];
    }
    status = fwrite(image_short, type, npix, data_file);
    free(image_short);		
  }
  if (type == 4){
    status = fwrite(image, type, npix, data_file);
  }
  if (status != npix) {
    fprintf(stderr, "Error writing output file...\n");
    return 1;
  }
  status = fclose(data_file);
  return status;

}

int compare_hyper_long(long int * imagedeg, long int * imageref){
  long int maxerr=0;
  long int maxerrnz=0;
  int err=0;
  long int i_l=0;
  long int npix=imageprop.nsmax*imageprop.nlmax*imageprop.nbmax;

  for (i_l=0;i_l<npix;i_l++){
    if (abs(imagedeg[i_l]-imageref[i_l]) > maxerr){
      maxerr=abs(imagedeg[i_l]-imageref[i_l]);
      err=1;
    };
    if (imagedeg[i_l] != 0){
      if (abs(imagedeg[i_l]-imageref[i_l]) > maxerrnz){
        maxerrnz=abs(imagedeg[i_l]-imageref[i_l]);
        err=1;
      };
    };
  };
  
  if (err) {
  fprintf(stderr, "ERREUR MAX %ld\n",maxerr);
  fprintf(stderr, "ERREUR MAX (NZ) %ld\n",maxerrnz);
  } else {
  fprintf(stderr, "Decoding OK (maxerr= %ld)\n",maxerr);
  }
  return err;
}

int compare_hyper_short(short int * imagedeg, short int * imageref){
  long int maxerr=0;
  long int maxerrnz=0;
  int err=0;
  long int i_l=0;
  long int npix=imageprop.nsmax*imageprop.nlmax*imageprop.nbmax;

  for (i_l=0;i_l<npix;i_l++){
    if (abs(imagedeg[i_l]-imageref[i_l]) > maxerr){
      maxerr=abs(imagedeg[i_l]-imageref[i_l]);
      err=1;
    };
    if (imagedeg[i_l] != 0){
      if (abs(imagedeg[i_l]-imageref[i_l]) > maxerrnz){
        maxerrnz=abs(imagedeg[i_l]-imageref[i_l]);
        err=1;
      };
    };
  };
  
  if (err) {
    fprintf(stderr, "ERREUR MAX %ld\n",maxerr);
    fprintf(stderr, "ERREUR MAX (NZ) %ld\n",maxerrnz);
  } else {
    fprintf(stderr, "Decoding OK (maxerr= %ld)\n",maxerr);
  }
  return err;
}

//magic number to identify the caracteristics of the stream
// bit 0: 0->ezw, 1->spith
// bit 1: 0->no mean sub, 1->mean sub
// bit 2: 0->no arith, 1-> arith
int write_magic(stream_struct streamstruct, coder_param_struct coder_param){
  unsigned char * stream = streamstruct.stream;
  long int * streamlast = streamstruct.streamlast;
  unsigned char * count = streamstruct.count;

  add_to_stream_number(*(coder_param.flag_spiht), stream, count, streamlast, 1);
  add_to_stream_number(*(coder_param.flag_meansub), stream, count, streamlast, 1);
  add_to_stream_number(*(coder_param.flag_arith), stream, count, streamlast, 1);
  add_to_stream_number(*(coder_param.flag_wavelet_int), stream, count, streamlast, 1);
  add_to_stream_number(1, stream, count, streamlast, 1);
  add_to_stream_number(1, stream, count, streamlast, 1);
  add_to_stream_number(1, stream, count, streamlast, 1);
  add_to_stream_number(1, stream, count, streamlast, 1);
  return 0;
}

int read_magic(stream_struct streamstruct, coder_param_struct coder_param){
  unsigned char * stream = streamstruct.stream;
  long int * streamlast = streamstruct.streamlast;
  unsigned char * count = streamstruct.count;
  char tmp;

  *(coder_param.flag_spiht)= read_from_stream_number(stream, count, streamlast, 1);
  *(coder_param.flag_meansub)= read_from_stream_number(stream, count, streamlast, 1);
  *(coder_param.flag_arith)= read_from_stream_number(stream, count, streamlast, 1);
  *(coder_param.flag_wavelet_int)= read_from_stream_number(stream, count, streamlast, 1);
  tmp= read_from_stream_number(stream, count, streamlast, 1);
  tmp= read_from_stream_number(stream, count, streamlast, 1);
  tmp= read_from_stream_number(stream, count, streamlast, 1);
  tmp= read_from_stream_number(stream, count, streamlast, 1);
#ifdef DEBUG2
  fprintf(stderr, "*(coder_param.flag_spiht): %i\n",*(coder_param.flag_spiht));
  fprintf(stderr, "*(coder_param.flag_meansub): %i\n",*(coder_param.flag_meansub));
  fprintf(stderr, "*(coder_param.flag_arith): %i\n",*(coder_param.flag_arith));
  fprintf(stderr, "*(coder_param.flag_wavelet_int): %i\n",*(coder_param.flag_wavelet_int));
#endif
  return 0;
}


int write_header(stream_struct streamstruct){
  
  unsigned char * stream = streamstruct.stream;
  long int * streamlast = streamstruct.streamlast;
  unsigned char * count = streamstruct.count;

  add_to_stream_number(imageprop.nsmax, stream, count, streamlast, 16);
  add_to_stream_number(imageprop.nlmax, stream, count, streamlast, 16);
  add_to_stream_number(imageprop.nbmax, stream, count, streamlast, 16);
  add_to_stream_number(imageprop.maxquant, stream, count, streamlast, 8);
  add_to_stream_number(imageprop.nresspec, stream, count, streamlast, 4);
  add_to_stream_number(imageprop.nresspat, stream, count, streamlast, 4);

  *streamstruct.headerlength=8* (* streamlast)+(*count);//it will also count the magic num
  return 0;
}

int read_header(stream_struct streamstruct){

  unsigned char * stream = streamstruct.stream;
  long int * streamlast = streamstruct.streamlast;
  unsigned char * count = streamstruct.count;
  
  imageprop.nsmax = read_from_stream_number(stream, count, streamlast, 16);
  imageprop.nlmax = read_from_stream_number(stream, count, streamlast, 16);
  imageprop.nbmax = read_from_stream_number(stream, count, streamlast, 16);
  imageprop.maxquant = read_from_stream_number(stream, count, streamlast, 8);
  imageprop.nresspec = read_from_stream_number(stream, count, streamlast, 4);
  imageprop.nresspat = read_from_stream_number(stream, count, streamlast, 4);
  *streamstruct.headerlength=8* (*streamlast)+(*count);
  return 0;
	
}

int write_header_mean(stream_struct streamstruct,long int * mean){

  unsigned char * stream = streamstruct.stream;
  long int * streamlast = streamstruct.streamlast;
  unsigned char * count = streamstruct.count;
  long int i_l=0;

  printf("Be aware: using mean substraction\n");
  for (i_l=0;i_l<imageprop.nbmax;i_l++){
  add_to_stream_number(mean[i_l], stream, count, streamlast, 16);
  }

  *streamstruct.headerlength=8* (*streamlast)+(*count);
  return 0;
}


int read_header_mean(stream_struct streamstruct,long int * mean){

  unsigned char * stream = streamstruct.stream;
  long int * streamlast = streamstruct.streamlast;
  unsigned char * count = streamstruct.count;
  long int i_l=0;
  printf("Be aware: using mean substraction\n");
  for (i_l=0;i_l<imageprop.nbmax;i_l++){
          mean[i_l] = read_from_stream_number(stream, count, streamlast, 16);
  }
  *streamstruct.headerlength=8* (*streamlast)+(*count);
  return 0;
}

pixel_struct init_pixel(){
  pixel_struct new_pixel;
  new_pixel.x=-1;
  new_pixel.y=-1;
  new_pixel.l=-1;
  return new_pixel;
}

int read_roi_spec(coder_param_struct coder_param){
  FILE *file_roi;
  int current_value=0;
  int status=0;
  int i=0;
  int specres_value = 0;
  int spatres_value = 0;
  int minquant_value = 0;
  int iloc, jloc, kloc;
  int blockind=0;
  int niloc, njloc, nkloc;
  int maxLineSize = 256;
  char * line = calloc(maxLineSize, sizeof(char));
  niloc=(imageprop.nsmin+1)/2;
  njloc=(imageprop.nlmin+1)/2;
  nkloc=(imageprop.nbmin+1)/2;
  file_roi = fopen(coder_param.roi_filename, "r");
  if (file_roi == NULL) {
    fprintf(stderr, "Error opening ROI file...\n");
    return NULL;
  }
  // Initialize with the default values (for all blocks
  // some will be modified after
  status = fgets(line, maxLineSize, file_roi);
  status = sscanf(line, "%i",&current_value );
  if (current_value == 0) {
    specres_value = imageprop.nresspec;
  } else {
    specres_value = current_value;
  }
  status = fgets(line, maxLineSize, file_roi);
  status = sscanf(line, "%i",&current_value );
  if (current_value == 0) {
    spatres_value = imageprop.nresspat;
  } else {
    spatres_value = current_value;
  }
  status = fgets(line, maxLineSize, file_roi);
  status = sscanf(line, "%i",&current_value );
  minquant_value = current_value;

  for (i=0; i<coder_param.nblock; i++){
    coder_param.maxres[i]=imageprop.nres;
// #ifdef RES_SCAL
    coder_param.maxresspec[i]=specres_value;
    coder_param.maxresspat[i]=spatres_value;
// #endif
    coder_param.maxquant[i]=imageprop.maxquant;
    coder_param.minquant[i]=minquant_value;
  }
  
  fprintf(stderr, "Check ROI default: %i %i %i\n",specres_value, spatres_value, minquant_value);
    
  while (1){
    status = fgets(line, maxLineSize, file_roi);
    if (status == 0) break;
    status = sscanf(line , "%i %i %i %i %i %i", 
                &iloc, &jloc, &kloc, &specres_value, &spatres_value, &minquant_value);
    blockind = iloc + jloc*niloc + kloc * niloc *njloc;
    coder_param.maxresspec[blockind]=specres_value;
    coder_param.maxresspat[blockind]=spatres_value;
    coder_param.minquant[blockind]=minquant_value;
    fprintf(stderr, "Check ROI %i : %i %i %i\n",blockind,specres_value, spatres_value, minquant_value);
  }
  
  return 0;
}


//This function should be in the later version (>0.53 of QccPack)
int QccENTArithmeticResetModel(QccENTArithmeticModel *model,
                             int context)
{
  int symbol;

    for (symbol = 0; symbol <= model->num_symbols[context]; symbol++) {
      model->frequencies[context][symbol] = 1;
      model->cumulative_frequencies[context][symbol] =
      model->num_symbols[context] - symbol;
    }
    model->frequencies[context][0] = 0;

  return(0);

}


