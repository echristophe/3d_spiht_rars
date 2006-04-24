
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

#include "main.h"



/* 
  WARNING this function only output the 1 and supposed that stream has been properly
  initialized at 0 (with a calloc for example).
*/
void add_to_stream(unsigned char *stream, unsigned char *count, int input, long int *streamlast)
{
// #ifdef SIZE
	nbitswritten++;
// #endif
	stream[(*streamlast)] += (input << (*count)); //no effect if input is 0, be aware do not forget to initialize strem to 0!
	(*count)++;
	if ((*count) == 8){
		(*count) = 0;
		(*streamlast)++;
	}

	
};

// ;count indicate the number of BIT from the begining of the stream
// ;if over streamlength, consider every bit as 0
unsigned char read_from_stream(unsigned char * stream, unsigned char * count, long int *streamlast){
	unsigned char output=0;
// 	streamlength=n_elements(stream)*8
// 	for i=0, output_length-1 do begin
// 		if (count GE streamlength) then begin ;buffer overflow...
// 		output[i]=0
// 		endif else begin ;normal situation
// 		streampos=floor(count / 8)
// 		bitpos= count mod 8
// 		output[i]=get_bit(stream[streampos],2L^(bitpos))
// 		endelse
// 		count++
// 	endfor
// #ifdef SIZE
	nbitsread++;
// #endif
	output=get_bit((long int) stream[*streamlast],(int) *count);
	(*count)++;
	if ((*count) == 8){
		*count = 0;
		(*streamlast)++;
	};
	return output;
};

struct list_el * el_init(struct pixel_struct pixel){
    struct list_el * ptr;
    ptr = (struct list_el *) malloc( sizeof(struct list_el));
    ptr->next = NULL;
    ptr->pixel = pixel;
    ptr->type = 0;
    ptr->thres = 0;
    return ptr;
};

struct list_struct * list_init(void){
    struct list_struct * ptr;
    ptr = (struct list_struct *) malloc( sizeof(struct list_struct));
    ptr->first = NULL;
    ptr->last = NULL;
    ptr->current = NULL;
    ptr->previous = NULL;	
    return ptr;
};

void list_free(struct list_struct * list){
	struct list_el * previous=NULL;
	struct list_el * current_el=NULL;
	if (list != NULL){
	previous=list->first;
	current_el=list->first;
	while (current_el != NULL){
		previous=current_el;
		current_el=current_el->next;
		free(previous);
	};
	free(list);
	};
};

void list_flush(struct list_struct * list){
	struct list_el * previous=NULL;
	struct list_el * current_el=NULL;
	if (list != NULL){
	previous=list->first;
	current_el=list->first;
	while (current_el != NULL){
		previous=current_el;
		current_el=current_el->next;
		free(previous);
	};
	list->first=NULL;
	list->last=NULL;
	list->current=NULL;
	list->previous=NULL;
	};
};

struct list_el * first_el(struct list_struct * list){
	list->previous=NULL;
	list->current=list->first;
	return list->current;
};


// struct list_el * next_el(struct list_struct * list){
// 	list->previous=list->current;
// 	list->current=list->previous->next;
// 	return list->current;
// };

struct list_el * next_el(struct list_struct * list){
//modif 21-02-06
	if (list->current != NULL){
		list->previous=list->current;
		list->current=list->previous->next;
		return list->current;
	} else {
		list->previous = NULL;
		list->current = list->first;
		return list->current;
	}
};

/*Insert el at the end of list*/
void insert_el(struct list_struct * list, struct list_el * el)
{
	if (list->last == NULL){/* The list is still empty*/ 
		list->last = el;
		list->first = el;
	}
	else {
		list->last->next = el;
		list->last = el;
		list->last->next=NULL;/*Et oui, si on se met à la fin, il ne faut plus rien derrière...*/
	}
};


void insert_el_inplace(struct list_struct * list, struct list_el * el)
{
// 	if (list->current == NULL){ /* The list is still empty*/
// 		list->last = el;
// 		list->first = el;
// 		printf("WARNING WARNING: empty list situation");
// 	}
// 	else {
// 		if (list->current == list->last){//on ajoute a la fin
// 			list->last = el;
// 		};
// 		el->next = list->current->next;
// 		list->current->next = el;
// 	}
	if (list->first == NULL){ /* The list is still empty*/
		list->last = el;
		list->first = el;
		list->last->next = NULL;//
// 		printf("WARNING WARNING: empty list situation\n");
	} else {
		if (list->current == NULL){//we want to put this element at the first place
			el->next = list->first;
			list->first = el;
		} else {
			if (list->current == list->last){//on ajoute a la fin
				list->last = el;
				list->last->next = NULL;//mod 22-2-06
			};
			el->next = list->current->next;
			list->current->next = el;
		}
	}
};

/*Insert el after 'place' and change 'place' to this element*/
void insert_el_after(struct list_struct * list, struct list_el * el, struct list_el ** place)
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
};

struct list_el * remove_current_el(struct list_struct * list)
{
	struct list_el * ptr;
	ptr=list->current;
	if (list->current == list->first){/*if current is the first*/
		if (list->current->next == NULL){/*list is now empty*/
			list->first = NULL;
			list->last = NULL;
			list->current = NULL;
			list->previous = NULL;
		}
		else {
			list->first = list->current->next; 
			list->current = list->first;//on passe au suivant
			list->previous = NULL;
		}
	}
	else {
		if (list->current == list->last){/*if current is the last*/
			if (list->previous == NULL){/*This should NEVER happen*/
				list->first = NULL;
				list->last = NULL;
				list->current = NULL;
				list->previous = NULL;
				printf("This should NEVER happen: list inconsistent\n");
			}
			else {
				list->last = list->previous;  /* Attention */
				list->current=NULL;//on passe au suivant qui n'existe pas
				list->last->next=NULL;
// 				list->previous=NULL;//modif 22/02/06
			};
		}
		else {
			list->previous->next = list->current->next;
			list->current = list->previous->next;//on passe directement au suivant
// 			list->previous=NULL;//modif 22/02/06
		};
	};
	return ptr; /*Pour eventuellement liberer la memoire si ce n'est pas un deplacement*/
};

int check_list(struct list_struct * list){
	struct list_el * el_current=NULL;
	struct list_el * el_previous=NULL;
	struct list_el * el;
	el_current=list->current;
	el_previous=list->previous;
	el = first_el(list);
	while (el != NULL){
		el=next_el(list);
// 		el=el->next;
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

int count_list(struct list_struct * list){
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

int print_list(struct list_struct * list)
{
	struct list_el * el_current=NULL;
	struct list_el * el_previous=NULL;
	struct list_el * el=NULL;
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


// long int trans_pixel(struct pixel_struct pixel)
// {
//    return pixel.x + (pixel.y + pixel.l*imageprop.nlmax)*imageprop.nsmax;
// };

//WARNING (ne marchera pas pour thres_ind =32)
unsigned char get_bit(long int value,int thres_ind){
// 	unsigned char bitvalue;
// 	bitvalue= byte(
// (floor(abs(value)/threshold)*threshold-floor(abs(value)/(threshold*2))*threshold*2) /threshold
// )
// 	bitvalue= (((abs(value)>>thres_ind)<<thres_ind) - ((abs(value)>>(thres_ind+1))<<(thres_ind+1)
// 	)) >>thres_ind;

// 	return bitvalue;


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

//value_abs_pix MUST be positive
//declaree en inline
// int isLSC2(long int value_abs_pix, int threshold){
// 	return ((value_abs_pix >= threshold) && (value_abs_pix < 2*threshold));
// };

//WARNING 
//Plus valable dans le cas où une ssbde de longueur impaire est decomposee...
//Seulement la derniere ssbde peut etre de longueur impaire.(pour spect)
struct parents_struct find_parents(struct pixel_struct pixel){
	struct parents_struct parents;
	parents.spat.x=-1;//spatial parent
	parents.spat.y=-1;
	parents.spat.l=-1;
	parents.spec.x=-1;//spectral parent
	parents.spec.y=-1;
	parents.spec.l=-1;

// 	if ((pixel.x < imageprop.nsmin) && (pixel.y < imageprop.nlmin)){//no spatial parents
// 
// 	} else if 
	if ((pixel.x >= 2*imageprop.nsmin) || (pixel.y >= 2*imageprop.nlmin)){//cas classique
// 		parents.spat.x=floor(pixel.x/2);
// 		parents.spat.y=floor(pixel.y/2);
		parents.spat.x=pixel.x/2;
		parents.spat.y=pixel.y/2;
		parents.spat.l=pixel.l;	
	} else {
		if ((pixel.x >= imageprop.nsmin) && (pixel.y < imageprop.nlmin)){//2
// 			parents.spat.x=floor((pixel.x-imageprop.nsmin+1)/2)*2+1;
// 			parents.spat.y=floor(pixel.y/2)*2;
			parents.spat.x=((pixel.x-imageprop.nsmin)/2)*2+1;
			parents.spat.y=(pixel.y/2)*2;
			parents.spat.l=pixel.l;
		}
		if ((pixel.x < imageprop.nsmin) && (pixel.y >= imageprop.nlmin)){//3
// 			parents.spat.x=floor(pixel.x/2)*2
// 			parents.spat.y=floor((pixel.y-imageprop.nlmin+1)/2)*2+1;
			parents.spat.x=(pixel.x/2)*2;
			parents.spat.y=((pixel.y-imageprop.nlmin)/2)*2+1;
			parents.spat.l=pixel.l;
		}
		if ((pixel.x >= imageprop.nsmin) && (pixel.y >= imageprop.nlmin)){//4
// 			parents.spat.x=floor((pixel.x-imageprop.nsmin+1)/2)*2+1;
// 			parents.spat.y=floor((pixel.y-imageprop.nlmin+1)/2)*2+1;
			parents.spat.x=((pixel.x-imageprop.nsmin)/2)*2+1;
			parents.spat.y=((pixel.y-imageprop.nlmin)/2)*2+1;
			parents.spat.l=pixel.l;
		}
	}


	if (pixel.l >= 2* imageprop.nbmin){
		parents.spec.x=pixel.x;
		parents.spec.y=pixel.y;
// 		parents.spec.l=floor(pixel.l/2);
		parents.spec.l=(pixel.l/2);		
	} else { 
		if (pixel.l >= imageprop.nbmin){
			parents.spec.x=pixel.x;
			parents.spec.y=pixel.y;
// 			parents.spec.l=floor((pixel.l-imageprop.nbmin+1)/2)*2+1;
			parents.spec.l=((pixel.l-imageprop.nbmin)/2)*2+1;
			if (parents.spec.l == imageprop.nbmin) parents.spec.l = imageprop.nbmin-1;//a ajouter au spat si on veut des longueurs impaires dans la LL
		}	
	}
	
	return parents;
};

int is_accessible_from(struct pixel_struct pixel,unsigned char * map){
	struct parents_struct parents;
	int out=0;
	parents = find_parents(pixel);
// 	printf("parents.spat %d %d %d\n",parents.spat.x,parents.spat.y,parents.spat.l);
// 	printf("parents.spec %d %d %d\n",parents.spec.x,parents.spec.y,parents.spec.l);
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
	struct pixel_struct pixel;
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



//valide par rapport a IDL sur le groupe issu de 0,0,0
// sur moffett3 sans correction moyenne
//total: 1959089642955
// long long int eval_dist_grp(int iloc, int jloc,int kloc, long int *image, int thres_ind){
// 	long long int dist=0;
// 	int i,j,k;
// 	int ii, ji, ki;
// 	int ie, je, ke;
// 	struct pixel_struct pixel;
// 	ki=2*kloc;
// 	ji=2*jloc;
// 	ii=2*iloc;
// 	ke= (2*kloc+2 < imageprop.nbmin ? 2*kloc+2 : imageprop.nbmin);
// 	je= (2*jloc+2 < imageprop.nlmin ? 2*jloc+2 : imageprop.nlmin);
// 	ie= (2*iloc+2 < imageprop.nsmin ? 2*iloc+2 : imageprop.nsmin);
// 	for (i=ii; i<ie; i++){
// 	for (j=ji; j<je; j++){
// 	for (k=ki; k<ke; k++){
// 		pixel.x=i;
// 		pixel.y=j;
// 		pixel.l=k;
// 		spat_spec_desc_spiht_cumul(pixel, image, thres_ind, &dist);
// 	}
// 	}
// 	}
// 	return dist;
// }

//update distortion value
int update_dist(struct pixel_struct pixel, int thres_ind, long long int * dist,  long int *image){
	long long int value=0;
	value = (long long int) abs(image[trans_pixel(pixel)]);
	*dist -= (1<<thres_ind)
		*(2*(value - ((value>>(thres_ind+1))<<(thres_ind+1)))-(1<<thres_ind));
	return 0;
}

//Is it time to add the cutting point to the rate-distortion list ?
int add_to_rddata(struct rddata_struct *rddata, long long int rate, long long int dist){
	if ((*rddata).reval[(*rddata).ptcourant] > rate) {
		return 0;
	} else {
		(*rddata).r[(*rddata).ptcourant] = rate;
		(*rddata).d[(*rddata).ptcourant] = dist;
		(*rddata).ptcourant++;
	}
	return 1;
}

//Compute cost J for a given lambda
//and put ptcourant to the min
//TODO cost_j value useful for debuging but useless waste of memory
//to remove later...
int compute_cost(struct rddata_struct *rddata, float lambda){
	int i=0;
	float min=0;
	if (lambda != 0) {
	   for (i=0; i< NUMRD; i++){
		(*rddata).cost_j[i] =  (*rddata).d[i] + lambda*(*rddata).r[i];
		if ((*rddata).cost_j[i] != 0){//otherwise, it means not really computed
			if (min == 0){ //first point to add there
				min = (*rddata).cost_j[i];
				(*rddata).ptcourant = i;
			} else {
				if ((*rddata).cost_j[i] < min){ // reread carefully in case of equality to choose the best TODO
					min=(*rddata).cost_j[i];
					(*rddata).ptcourant = i;
				}
			}
		}
	   }	
	} else {//lambda = 0 we want lossless here
	   min = (*rddata).d[NUMRD-1];//should be the final answer anyway unless particular situation (all even numbers for eg).
	  (*rddata).ptcourant = NUMRD-1;
	   for (i=0; i< NUMRD; i++){
		if ((*rddata).r[i] !=0 ){
			if (((*rddata).d[i]  <= min) && ((*rddata).d[i] < (*rddata).d[(*rddata).ptcourant])){//TODO to reread carefully, we want the min here
				min=(*rddata).d[i];
				(*rddata).ptcourant = i;
			}
		}
	   }
	}
	if ((*rddata).ptcourant == NUMRD) fprintf(stderr, "Compute_cost to check...\n");//test 
	return (*rddata).ptcourant;//this is the minimum position
}

//fin the lambda corresponding to a given rate
float compute_lambda(struct datablock_struct *datablock, long int rate, int nblock){
	float lambda_inf = 0.0;
	float lambda_sup = 500000.0;
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
	while ((lambda_sup-lambda_inf) > 1.0) {//TODO verifier ce qu'il se passe quand on commence en dehors du domaine... ou quand le taux est trop grand...
		current_rate=0;
		lambda = (lambda_sup + lambda_inf) /2.;
		for (i=0;i<nblock;i++){
#ifdef RES_RATE
// 			for (j=0;j<NRES;j++){
// 			posmin[j]=compute_cost(&(datablock[i].rddata[j]),lambda);
// 			if ((j>NRESSPEC) && (posmin[j] > posmin[j-NRESSPEC])) posmin[j]=posmin[j-NRESSPEC];
//  			if (((j % NRESSPEC)!=0) && (posmin[j] > posmin[j-1])) posmin[j]=posmin[j-1];
// 			current_rate += datablock[i].rddata[j].r[posmin[j]];//HUM, check priorities...
// 			}
			for (j=imageprop.nres-1;j>=0;j--){
			posmin[j]=compute_cost(&(datablock[i].rddata[j]),lambda);
				if ((j<imageprop.nres-imageprop.nresspec) && (posmin[j] < posmin[j+imageprop.nresspec])) posmin[j]=posmin[j+imageprop.nresspec];
 			if (((j % imageprop.nresspec)!=imageprop.nresspec-1) && (posmin[j] < posmin[j+1])) 
				posmin[j]=posmin[j+1];
			current_rate += datablock[i].rddata[j].r[posmin[j]];//HUM, check priorities...
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
int datablock_init(struct datablock_struct *datablock){
	(*datablock).stream = (unsigned char *) calloc(sizeblockstream,sizeof(unsigned char *));
	if ((*datablock).stream == NULL) {fprintf(stderr, "******** Allocation problem in datablock_init (strem)\n");};
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

int datablock_free(struct datablock_struct *datablock){
	free((*datablock).stream);
	free((*datablock).streamlast);
	free((*datablock).count);
	free((*datablock).partsize);
	return 0;
}

//outputs in stream functions

//output the next part size (there is definitly more efficient way to do that...) TODO
int add_to_stream_number(unsigned long int number, unsigned char * stream, unsigned char *count, long int *streamlast, int numbits){
	int i=0;
// 	printf("Adding %lu at %ld : %d\n",number, *streamlast, *count);
//	if (number > (1<<(numbits-2))) { //taking a 2 bits marging again... 
	if (number > (1<<(32-2))) {
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
// 	printf("Reading at %ld : %d\n", *streamlast, *count);
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

int interleavingblocks(struct datablock_struct *datablock, int nblock, unsigned char * stream, unsigned char * count, long int * streamlast, float lambda, int *flagfirst){
	int i=0;
#ifdef RES_RATE
// 	int posmin[NRES];
	int posmin;
#else
	int posmin;
#endif
	long int insize;
	long long int rate;
	int j;
	long int sum=0;
// 	int flagfirst=0;
// 	if ((*streamlast == 0) && (*count ==0)) flagfirst=1;
// 	if ((*streamlast == 8) && (*count ==0)) flagfirst=1;//WARNING, depending on header size...
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
				} else {
					posmin=compute_cost(&(datablock[i].rddata[j]),0.0);//would be faster just to take the last point
				}
				sum = datablock[i].rddata[j].r[posmin];//voltige
				add_to_stream_number(sum, stream, count, streamlast, NUMBITSPARTSIZE);
#else
				/*if (j>3)*/ sum=0;
				sum += (datablock[i].partsize)[j];
				/*if (j>=3) */add_to_stream_number(sum, stream, count, streamlast, NUMBITSPARTSIZE);
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
// 			#ifdef DEBUG
// 			printf("Cutting point for %d (res %d): %lld (adding %lld to stream WRONG)\n", i,j, rate, rate - (datablock[i].currentpos));
// 			printf("Block %d begin at position %ld\n",i, (*streamlast)*8+ (*count));
// 			#endif
// 			add_to_stream_number(rate - (datablock[i].currentpos), stream,count,streamlast);
			//copy the corresponding number of bits in stream
			copy_to_stream((datablock[i].currentpos), rate+(datablock[i].currentpos), datablock[i].stream,  stream, count, streamlast, insize);
// 			(datablock[i].currentpos) += rate;
			//jump to the end
			datablock[i].currentpos += (datablock[i].partsize)[j];

			
		}
		#ifdef DEBUG
		printf("Cutting point for %d (res %d): %lld (adding %lld to stream WRONG)\n", i,j, rate, rate - (datablock[i].currentpos));
		printf("Block %d begin at position %ld\n",i, (*streamlast)*8+ (*count));
		#endif
#else
		posmin=compute_cost(&(datablock[i].rddata),lambda);//this is not needed if compute_lambda was called before
// 		posmin=datablock[i].rddata.ptcourant; //however, this is not the case for 0 (lossless). TODO OPTI
		rate=datablock[i].rddata.r[posmin];
		insize = (*(datablock[i].streamlast))*8+(*(datablock[i].count));
		if (insize > sizeblockstream*8) {fprintf(stderr, "******** Block size overflow: %ld \n", insize);}
		//output the size of the next part in stream
		#ifdef DEBUG
		printf("Cutting point for %d : %lld (adding %lld to stream)\n", i, rate, rate - (datablock[i].currentpos));
		printf("Block %d begin at position %ld\n",i, (*streamlast)*8+ (*count));
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
int desinterleavingblocks(struct datablock_struct *datablock, int nblock, struct stream_struct streamstruct, long int  insize, int nlayer){
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
// #ifdef RES_RATE
// 				(datablock[i].partsize)[j]=read_from_stream_number(stream, &count, &streamlast);
// #else

// 				if (j< 3) (datablock[i].partsize)[j] = -1;
				/*if (j>=3) */(datablock[i].partsize)[j]=read_from_stream_number(stream, &count, &streamlast, NUMBITSPARTSIZE);

// #endif
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


// void print_lastm(struct list_el * lastm[NRES][MAXQUANT_CONST+1]){
// int i,j;
// for (j=MAXQUANT_CONST; j>=0; j--){
// for (i=0; i<imageprop.nres; i++){
// 	if (lastm[i][j] == NULL){
// 		printf(" NULL   ");
// 	} else {
// 		printf("(%d,%d,%d) ",lastm[i][j]->pixel.x,lastm[i][j]->pixel.y,lastm[i][j]->pixel.l);
// 	}
// }
// printf("\n");
// }
// 
// }


long int file_size(FILE *f){
long int currentpos;
long int end;

currentpos = ftell (f);
fseek (f, 0, SEEK_END);
end = ftell (f);
fseek (f, currentpos, SEEK_SET);

return end;
}

int init_coder_param(struct coder_param_struct * coder_param, int nblock){
	(*coder_param).nblock = nblock;
	(*coder_param).maxres = (char *) calloc(nblock,sizeof(char *));
#ifdef RES_SCAL
	(*coder_param).maxresspat = (char *) calloc(nblock,sizeof(char *));
	(*coder_param).maxresspec = (char *) calloc(nblock,sizeof(char *));
#endif
	(*coder_param).maxquant = (char *) calloc(nblock,sizeof(char *));
	(*coder_param).minquant = (char *) calloc(nblock,sizeof(char *));
	(*coder_param).nlayer = 100;
	return 0;
}

int free_coder_param(struct coder_param_struct * coder_param){
	(*coder_param).nblock = 0;
	free((*coder_param).maxres);
#ifdef RES_SCAL
	free((*coder_param).maxresspat);
	free((*coder_param).maxresspec);
#endif
	free((*coder_param).maxquant);
	free((*coder_param).minquant);
	free(coder_param);
	return 0;
}

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
	  mean[i] = meantmp / npixband;//Doesnot need to be precise ok for the rounding
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
int check_end(long int *image, struct list_struct ** list, struct coder_param_struct coder_param, int blockind){
int res;
struct list_el * current_el=NULL;
struct list_el * current_sav=NULL;
struct list_el * previous_sav=NULL;	
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

int waveletscaling(long int * image, int spatdec, int specdec, int dir ){

int i,j,k,l;
int currentns,currentnl,currentnb;
int iindex, jindex, kindex;
double multfact,multfactfull;
double * mask;

mask = (double *) calloc(imageprop.nsmax * imageprop.nlmax,sizeof(double));
//fabrication du mask
	for (i=0;i<imageprop.nsmax;i++){
	for (j=0;j<imageprop.nlmax;j++){
		multfact=1.0;
		currentns=imageprop.nsmin;
		iindex=spatdec-1;
		while (i >= currentns) {
			currentns *= 2;
			iindex--;
		}
		currentnl=imageprop.nlmin;
		jindex=spatdec-1;
		while (j >= currentnl) {
			currentnl *= 2;
			jindex--;
		}
		
		if (iindex > jindex+1) 	iindex = jindex+1;
		if (jindex > iindex+1)  jindex = iindex+1;	

		if ((iindex == spatdec-1) && (jindex == spatdec-1)){
			multfact = (1 << spatdec);
		} else {
			for (l=0; l<iindex+jindex;l++){
				multfact *= M_SQRT2;
			}
			if (iindex <0 ) multfact /= M_SQRT2;
			if (jindex <0 ) multfact /= M_SQRT2;
			if ((iindex == jindex-1) || (iindex == jindex+1)){
				multfact *= M_SQRT2;
			}
		}
		mask[i+imageprop.nsmax*j]=multfact;

	}
	}

	for (k=0;k<imageprop.nbmax;k++){
		multfact=1.0;
		currentnb=imageprop.nbmin;
		kindex=specdec-1;
		while (k >= currentnb) { //WARNING: won't be working for any size
			currentnb *= 2;
			kindex--;
		}
		for (l=0; l<kindex;l++){
			multfact *= M_SQRT2;
		}
		if (kindex <0 ) multfact /= M_SQRT2;


			for (i=0;i<imageprop.nsmax;i++){
			for (j=0;j<imageprop.nlmax;j++){
				multfactfull = multfact * mask[i+imageprop.nsmax*j];
// 				if (multfactfull < 1.0) multfactfull = 1.0; //warning, this is not correct.. (perf drop but lossless
// 				multfactfull *= 4;
				if (dir ==0) {
				image[i+imageprop.nsmax*(j+imageprop.nlmax*k)] =
					 round(image[i+imageprop.nsmax*(j+imageprop.nlmax*k)]
						*multfactfull);
				} else {
				image[i+imageprop.nsmax*(j+imageprop.nlmax*k)] =
					 round(image[i+imageprop.nsmax*(j+imageprop.nlmax*k)] 
						/ multfactfull);
				}
			}
			}


	}
	return 0;
}

