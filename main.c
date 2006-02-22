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
#else
data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett3-ani-lsb.img","r");
// data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett3-ani-lsb-4l.img","r");//WARNING, also modify NSMIN et cie
// data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett3-ani-lsb-haar.img","r");
#endif
status = fread(image, 4, npix, data_file);
// data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett3-lsb.img","r");
// status = fread(image, 2, npix, data_file);

status = fclose(data_file);

//test evaluation de distortion  //coherent with IDL for 0,0,0
// dist = eval_dist_grp(0, 0, 0, image, imageprop, 20);

printf("image[0]: %ld\n",image[0]);
#ifndef NOENC
#ifdef EZW
// ezw_code_c(image, stream, outputsize, maxquantvalue);
ezw_code_signed_c(image, stream, outputsize, maxquantvalue);
#else
// spiht_code_c(image, stream, outputsize, maxquantvalue);
spiht_code_ra(image, stream, outputsize, maxquantvalue);
#endif
// waveletDWT(image, imageout);
// spiht_code_c(image, stream, outputsize, maxquantvalue);

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

//decode
#ifdef EZW
// ezw_decode_c(imageout, stream, outputsize, maxquantvalue);
ezw_decode_signed_c(imageout, stream, outputsize, maxquantvalue);
#else
// spiht_decode_c(imageout, stream, outputsize, maxquantvalue);
spiht_decode_ra(imageout, stream, outputsize, maxquantvalue);
#endif
printf("Fin decodage\n");

// sleep(10);

//save output
output_file = fopen("/home/christop/Boulot/images/output_stream/moffett3-ani-spiht-out.img","w");
// status = fwrite(stream, 1, *outputsize, output_file);//chgt 14-02-06 a verifier sur le spiht standard outputsize en bit ou byte ???
status = fwrite(imageout, 4, npix, output_file);
status = fclose(output_file);


err=0;
for (i_l=0;i_l<npix;i_l++){
	if ((image[i_l]-imageout[i_l]) != 0){
		err=1;
		break;
	};
};
if (err) {
fprintf(stderr, "ERREUR à %ld\n",i_l);
} else {
fprintf(stderr, "Decoding OK\n");
}
return 0;
};


/*
// Attention, changement de l'emploi de count par rapport à IDL !!!
//on considere que la stream est assez longue deja... */

/* 
  WARNING this function only output the 1 and supposed that stream has been properly
  initialized at 0 (with a calloc for example).
*/
void add_to_stream(unsigned char *stream, unsigned char *count, int input, long int *streamlast)
{
	long int pos;
	unsigned char count_local;
	pos = *streamlast;
	count_local = *count;
	stream[pos] += (input << count_local); //no effect if input is 0, be aware...
	(count_local)++;
	if ((count_local) == 8){
		(count_local) = 0;
		pos++;
		*streamlast = pos;
	}
	*count = count_local;
	
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
		printf("WARNING WARNING: empty list situation");
	} else {
		if (list->current == NULL){//we want to put this element at the first place
			el->next = list->first;
			list->first = el;
		} else {
			if (list->current == list->last){//on ajoute a la fin
				list->last = el;
			};
			el->next = list->current->next;
			list->current->next = el;
		}
	}
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
				printf("This should NEVER happen: list inconsistent");
			}
			else {
				list->last = list->previous;  /* Attention */
				list->current=NULL;//on passe au suivant qui n'existe pas
				list->last->next=NULL;
			};
		}
		else {
			list->previous->next = list->current->next;
			list->current = list->previous->next;//on passe directement au suivant
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

int check_map(unsigned char * map, struct imageprop_struct imageprop){
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
		printf("%d %d %d \n",el->pixel.x,el->pixel.y,el->pixel.l);
		el=next_el(list);
	};
	
	if ((list->previous) == (list->last)) {
	  list->current=el_current;
	  list->previous=el_previous;
	  output= 0;
	} else 
	{
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


long int trans_pixel(struct pixel_struct pixel, struct imageprop_struct imageprop)
{
   return pixel.x + pixel.y*imageprop.nsmax + pixel.l*imageprop.nsmax*imageprop.nlmax;
};

//WARNING (ne marchera pas pour thres_ind =32)
unsigned char get_bit(long int value,int thres_ind){
	unsigned char bitvalue;
// 	bitvalue= byte(
// (floor(abs(value)/threshold)*threshold-floor(abs(value)/(threshold*2))*threshold*2) /threshold
// )
	bitvalue= (((abs(value)>>thres_ind)<<thres_ind) - ((abs(value)>>(thres_ind+1))<<(thres_ind+1)
	)) >>thres_ind;
	return bitvalue;
}


int isLSC(long int value_pix, int thres_ind){
	long int threshold= 1 << (long int)thres_ind;
	if ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold)) {
		return 1;
	} else {
		return 0;
	}
};


//WARNING 
//Plus valable dans le cas où une ssbde de longueur impaire est decomposee...
//Seulement la derniere ssbde peut etre de longueur impaire.(pour spect)
struct parents_struct find_parents(struct pixel_struct pixel, struct imageprop_struct imageprop){
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

int is_accessible_from(struct pixel_struct pixel, struct imageprop_struct imageprop, unsigned char * map){
	struct parents_struct parents;
	int out=0;
	parents = find_parents(pixel, imageprop);
// 	printf("parents.spat %d %d %d\n",parents.spat.x,parents.spat.y,parents.spat.l);
// 	printf("parents.spec %d %d %d\n",parents.spec.x,parents.spec.y,parents.spec.l);
	if (parents.spat.x != -1){
		if (map[trans_pixel(parents.spat,imageprop)] == 1){
			return 1;
		} else {
			out=is_accessible_from(parents.spat,imageprop,map);
		}
	}
	if (out == 1) return 1;
	if (parents.spec.x != -1){
		if (map[trans_pixel(parents.spec,imageprop)] == 1){
			return 1;
		} else {
			out=is_accessible_from(parents.spec,imageprop,map);
		}
	}

	return out;

}

#ifdef DEBUG
int check_accessibility_of_all(struct imageprop_struct imageprop, unsigned char * map_LSC, unsigned char * map_LIC, unsigned char * map_LIS){
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
			if ((map_LSC[trans_pixel(pixel, imageprop)] == 0) &&
				(map_LIC[trans_pixel(pixel, imageprop)] == 0)){
				out=is_accessible_from(pixel,imageprop,map_LIS);
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
long long int eval_dist_grp(int iloc, int jloc,int kloc, long int *image, struct imageprop_struct imageprop, int thres_ind){
	long long int dist=0;
	long long int value=0;
	int i,j,k;
	int ii, ji, ki;
	int ie, je, ke;
	struct pixel_struct pixel;
// 	struct list_struct * list_desc =NULL;
// 	struct list_el * el =NULL;
	int tmpcount=0;
	ki=2*kloc;
	ji=2*jloc;
	ii=2*iloc;
	ke= (2*kloc+2 < imageprop.nbmin ? 2*kloc+2 : imageprop.nbmin);
	je= (2*jloc+2 < imageprop.nlmin ? 2*jloc+2 : imageprop.nlmin);
	ie= (2*iloc+2 < imageprop.nsmin ? 2*iloc+2 : imageprop.nsmin);
// 	printf("pt: %d %d %d -> %lld",i ,j, k, dist);
// 	list_desc = list_init();
	for (i=ii; i<ie; i++){
	for (j=ji; j<je; j++){
	for (k=ki; k<ke; k++){
		pixel.x=i;
		pixel.y=j;
		pixel.l=k;
// 		el=el_init(pixel);
// 		insert_el(list_desc, el);
		//recherche de TOUS les descendants
// 		spat_spec_desc_spiht(pixel, list_desc, imageprop, 0, zero_map, 0, zero_map);
// 		spat_spec_desc_spiht_cumul(pixel, list_desc, imageprop, image, thres_ind, &dist);
		spat_spec_desc_spiht_cumul(pixel, imageprop, image, thres_ind, &dist);
// 		spat_spec_desc_spiht_cumul(pixel, NULL, imageprop, image, thres_ind, &dist);

// 		//parcours des descendant et ajout a dist
// 		el = first_el(list_desc);
// 		tmpcount=0;
// 		while (el !=NULL){
// 			value = (long long int) image[trans_pixel(el->pixel, imageprop)];
// 			dist += value * value ;
// 			el=next_el(list_desc);
// 			tmpcount++;
// 		}
	
		//nettoyage
// 		list_flush(list_desc);
// 		printf("pt: %d %d %d -> %lld (%d pt)\n",i ,j, k, dist, tmpcount);
	}
	}
	}
// 	free(el);
// 	list_free(list_desc);
	return dist;
}

//update distortion value
int update_dist(struct pixel_struct pixel, int thres_ind, long long int * dist,  long int *image, struct imageprop_struct imageprop){
	long long int value=0;
	value = (long long int) abs(image[trans_pixel(pixel,imageprop)]);
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
	return (*rddata).ptcourant;//this is the minimum position
}


//functions to take care of data_block
int datablock_init(struct datablock_struct *datablock){
	(*datablock).stream = (unsigned char *) calloc(SIZEBLOCKSTREAM,sizeof(unsigned char *));
	if ((*datablock).stream == NULL) {fprintf(stderr, "******** Allocation problem in datablock_init\n");};
	(*datablock).streamlast = (long int *) malloc(sizeof(long int));
	if ((*datablock).streamlast == NULL) {fprintf(stderr, "******** Allocation problem in datablock_init\n");};
	(*datablock).count = (unsigned char *) malloc(sizeof(unsigned char));
	if ((*datablock).count == NULL) {fprintf(stderr, "******** Allocation problem in datablock_init\n");};
	*((*datablock).streamlast) = 0;
	*((*datablock).count)=0;
	(*datablock).currentpos=0;
	return 0;
}

int datablock_free(struct datablock_struct *datablock){
	free((*datablock).stream);
	free((*datablock).streamlast);
	free((*datablock).count);
	return 0;
}

//outputs in stream functions

//output the next part size (there is definitly more efficient way to do that...) TODO
int add_to_stream_number(unsigned long int number, unsigned char * stream, unsigned char *count, long int *streamlast){
	int i=0;
// 	printf("Adding %lu at %ld : %d\n",number, *streamlast, *count);
	if (number > (1<<(NUMBITSPARTSIZE-2))) { //taking a 2 bits marging again... 
		printf("********** WARNING Possible error in size encoding *************\n");
		return 1;
	}
	for (i=0;i<NUMBITSPARTSIZE;i++){
		add_to_stream(stream, count, (int) get_bit(number,i), streamlast);
	}
	return 0;
}

unsigned long int read_from_stream_number(unsigned char * stream, unsigned char *count, long int *streamlast){
	unsigned long int n=0;
	unsigned char bit=0;
	int i;
// 	printf("Reading at %ld : %d\n", *streamlast, *count);
	for (i=0; i<NUMBITSPARTSIZE;i++){
		bit=read_from_stream(stream, count, streamlast);
		if (bit ==1){
			n += (1<<i);//ca devrait etre dans le meme ordre qu'a l'ecriture
		}
	}
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

int interleavingblocks(struct datablock_struct *datablock, int nblock, unsigned char * stream, unsigned char * count, long int * streamlast, float lambda){
	int i=0;
	int posmin;
	long int insize;
	long long int rate;
	for (i=0;i<nblock;i++){
		posmin=compute_cost(&(datablock[i].rddata),lambda);
		rate=datablock[i].rddata.r[posmin];
		insize = (*(datablock[i].streamlast))*8+(*(datablock[i].count));
		if (insize > SIZEBLOCKSTREAM*8) {fprintf(stderr, "******** Block size overflow: %ld \n", insize);}
		//output the size of the next part in stream
		#ifdef DEBUG
		printf("Cutting point for %d : %lld (adding %lld to stream)\n", i, rate, rate - (datablock[i].currentpos));
		printf("Block %d begin at position %ld\n",i, (*streamlast)*8+ (*count));
		#endif
		add_to_stream_number(rate - (datablock[i].currentpos), stream,count,streamlast);
		//copy the corresponding number of bits in stream
		copy_to_stream((datablock[i].currentpos), rate, datablock[i].stream,  stream, count, streamlast, insize);
		(datablock[i].currentpos)=rate;
	}
	return 0;
}

//desinterleaving block and storing in datablock_struct
int desinterleavingblocks(struct datablock_struct *datablock, int nblock, unsigned char * stream, long int  insize){
	int i=0;
	int err=0;
	long int pos;
	unsigned long int nbits=0;
	unsigned char count=0;
	long int streamlast=0;
	while ((streamlast*8+ count) <= insize){
		nbits = read_from_stream_number(stream,&count,&streamlast);
		pos = streamlast*8+count;
		if (pos < 0) {fprintf(stderr, "******** pos neg in desinterleavingblocks\n");};
// 		printf("Cutting point for %d : %ld (adding %lu to stream)\n", i, ((*(datablock[i].stream))*8+(*(datablock[i].count))+nbits), nbits);
		err=copy_to_stream(pos, pos + nbits, stream, datablock[i].stream, datablock[i].count, datablock[i].streamlast, insize);//ce serait plus simple avec un objet stream...
		if (err) {return 1;}
		streamlast =  (pos + nbits)/8;
		count = (pos + nbits)%8;
		i++;
		if (i == nblock) {i=0;};
	}
	return 0;
}

