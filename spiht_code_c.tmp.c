#include "spiht_code_c.h"
#include "desc.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
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

image = (long int *) malloc(NSMAX_CONST*NLMAX_CONST*NBMAX_CONST*sizeof(long int));
// image = (short int *) malloc(NSMAX_CONST*NLMAX_CONST*NBMAX_CONST*sizeof(short int));//MODIF
imageout = (long int *) malloc(NSMAX_CONST*NLMAX_CONST*NBMAX_CONST*sizeof(long int));
outputsize = (long int *) malloc(MAXQUANT_CONST * sizeof(long int));
*outputsize=0;
stream = (unsigned char *) malloc(NSMAX_CONST*NLMAX_CONST*NBMAX_CONST*2*sizeof(unsigned char));//on prend une marge... a faire en finesse plus tard...
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

*maxquantvalue=19;

npix=imageprop.nsmax*imageprop.nlmax*imageprop.nbmax;
for (i_l=0;i_l<npix;i_l++){
	image[i_l]=0;
	imageout[i_l]=0;
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
#endif
status = fread(image, 4, npix, data_file);
// data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett3-lsb.img","r");
// status = fread(image, 2, npix, data_file);

status = fclose(data_file);

printf("image[0]: %ld\n",image[0]);
// #ifdef EZW
// ezw_code_c(image, stream, outputsize, maxquantvalue);
// #else
//spiht_code_c(image, stream, outputsize, maxquantvalue);
// #endif
// waveletDWT(image, imageout);
spiht_code_c(image, stream, outputsize, maxquantvalue);


printf("Outputsize: %ld \n", *outputsize);



//output of coded stream
output_file = fopen("/home/christop/Boulot/images/output_stream/moffett3-ani-spiht.dat","w");
status = fwrite(stream, 1, *outputsize, output_file);
status = fclose(output_file);

//put stream to 0
for (i_l=0;i_l<npix*2;i_l++){
	stream[i_l]=0;
};

//Read from file
output_file = fopen("/home/christop/Boulot/images/output_stream/moffett3-ani-spiht.dat","r");
// *outputsize=*outputsize-2; //on enleve 2 octets pour voir...
// *outputsize = 1069354-1;
// *outputsize =33347743-1;
status = fread(stream, 1, *outputsize, output_file);
status = fclose(output_file);

//decode
// #ifdef EZW
// ezw_decode_c(imageout, stream, outputsize, maxquantvalue);
// #else
spiht_decode_c(imageout, stream, outputsize, maxquantvalue);
// #endif
printf("Fin decodage\n");

// sleep(10);


for (i_l=0;i_l<npix;i_l++){
	if ((image[i_l]-imageout[i_l]) != 0){
		fprintf(stderr, "ERREUR à %ld\n",i_l);
	};
};

return 0;
};


/*
// Attention, changement de l'emploi de count par rapport à IDL !!!
//on considere que la stream est assez longue deja...
//a mettre en inline après... */
void add_to_stream(unsigned char *stream, unsigned char *count, int input, long int *streamlast)
{
	long int pos;
	unsigned char count_local;
	pos = *streamlast;
	count_local = *count;
	stream[pos] += (input << count_local);
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

struct list_el * first_el(struct list_struct * list){
	list->previous=NULL;
	list->current=list->first;
	return list->current;
};


struct list_el * next_el(struct list_struct * list){
	list->previous=list->current;
	list->current=list->previous->next;
	return list->current;
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
	if (list->current == NULL){ /* The list is still empty*/
		list->last = el;
		list->first = el;
		printf("WARNING WARNING: empty list situation");
	}
	else {
		if (list->current == list->last){//on ajoute a la fin
			list->last = el;
		};
		el->next = list->current->next;
		list->current->next = el;
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

unsigned char get_bit(long int value,int thres_ind){
	unsigned char bitvalue;
// 	bitvalue= byte(
// (floor(abs(value)/threshold)*threshold-floor(abs(value)/(threshold*2))*threshold*2) /threshold
// )
	bitvalue= (((abs(value)>>thres_ind)<<thres_ind) - ((abs(value)>>(thres_ind+1))<<(thres_ind+1)
	)) >>thres_ind;
	return bitvalue;
}


int spiht_code_c(long int *image, unsigned char *stream, long int *outputsize, long int *maxquantvalue)
{

// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, 8, 8, 7};
struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
int nbmin=imageprop.nbmin;

struct list_struct * LSC=NULL;
struct list_struct * LIC=NULL;
struct list_struct * LIS=NULL;
struct pixel_struct pixel;
// int maxquant=imageprop.maxquant;
// int maxquant=MAXQUANT_CONST;
int maxquant= (int) (*maxquantvalue);
int minquant=0;
struct list_el * el=NULL;
struct list_el * current_el=NULL;
struct list_el * tmp_el=NULL;
struct list_el * current_child=NULL;
struct list_el * lastLSC=NULL;


struct list_struct * list_desc=NULL;
struct list_struct * tmp_list=NULL;
struct list_struct * list_grand_desc=NULL;

long int value_pix=0;
unsigned char bit=255;
unsigned char bitsig=255;
int r=0;
int ngrandchild=0;
unsigned char * count = (unsigned char *) malloc(sizeof(unsigned char));
long int *streamlast = (long int *) malloc(sizeof(long int));
// long int bitrate = 0;
long int threshold=0;
int thres_ind=0;
int i,j,k;
long int i_l;
long int nLICloop;
long int nLSCloop;
long int nLISloop;
long int nLISloopA;
long int nLISloopB;

unsigned char *map_LSC = NULL;
unsigned char *map_LIC = NULL;
unsigned char *map_LIS = NULL;

map_LSC=(unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));
map_LIC= (unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));
map_LIS= (unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));

*count=0;
*streamlast=0;
LSC = list_init();
LIC = list_init();
LIS = list_init();

// list_desc=list_init();
// tmp_list=list_init();

for (i_l=0;i_l<nsmax*nlmax*nbmax;i_l++)
{
	map_LSC[i_l]=0;
	map_LIC[i_l]=0;
	map_LIS[i_l]=0;
}

//SPIHT 1)
// printf("Initialization...\n");
for (k=0;k<nbmin;k++){
   for (j=0;j<nlmin;j++){
      for (i=0;i<nsmin;i++){
	pixel.x=i;
	pixel.y=j;
	pixel.l=k;
	el=el_init(pixel);
	insert_el(LIC,el);
	(map_LIC[trans_pixel(pixel, imageprop)])++;
	if ((i % 2) || (j % 2) || (k % 2) || (((nbmin % 2) == 1) && (k == nbmin-1) && (k!=0))){ 
		el=el_init(pixel);
		insert_el(LIS,el);
		(map_LIS[trans_pixel(pixel, imageprop)])++;
	 };
      };
   };
};

// r=check_list(LIC);
// if (r != 0) {
//    fprintf(stderr, "LIC non conforme\n");
//    return 1;
// };
// r=check_list(LIS);
// if (r != 0) {
//    fprintf(stderr, "LIS non conforme\n");
//    return 1;
// };


//SPIHT 2)
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){

threshold= 1 << (long int)thres_ind;

printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
// printf("Sorting pass for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);

nLSCloop=0;
nLICloop=0;
nLISloop=0;
nLISloopA=0;
nLISloopB=0;

// Used for Significant pass
lastLSC = LSC->last;

// ;for each entry in the LIC
//SPIHT 2.1)
current_el=first_el(LIC);
// printf("LIC processing \n");
while (current_el != NULL){
// printf(".");
   nLICloop++;
   value_pix=image[trans_pixel(current_el->pixel,imageprop)];
//    bit = ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold));//TODO possibilite d'enlever un abs
   bit = get_bit(value_pix, thres_ind);	
   add_to_stream(stream, count, (int) bit, streamlast);//SPIHT 2.1.1)
  if (bit == 1) { //SPIHT 2.1.2)
     bitsig = (value_pix > 0);
     add_to_stream(stream, count, (int) bitsig, streamlast);
     (map_LIC[trans_pixel(current_el->pixel, imageprop)])--;
     (map_LSC[trans_pixel(current_el->pixel, imageprop)])++;
     remove_current_el(LIC);
     insert_el(LSC, current_el);
     current_el = LIC->current;//est passe au suivant dans le move
     
   } else {
      current_el=next_el(LIC);
   };
}


// ;for each entry in the LIS
current_el=first_el(LIS);
// printf("LIS processing \n");
while (current_el != NULL){ //SPIHT 2.2)
// printf("Processing in LIS: %d %d %d\n",current_el->pixel.x,current_el->pixel.y,current_el->pixel.l);
nLISloop++;
// if (LIS->current == NULL) printf("Uh Oh 2.2\n");
#ifndef NOLISTB
    if (current_el->type == 0) { //SPIHT 2.2.1);if the set is of type A
#endif
        nLISloopA++;
	r=spat_spec_desc_spiht(current_el->pixel, /*tmp_list*/list_desc, imageprop, 0, image, thres_ind, map_LSC);
// 	list_free(tmp_list);//test
	bit = (r == -1);//il y a au moins un des descendants qui est significatif
        add_to_stream(stream, count, (int) bit, streamlast); //SPIHT 2.2.1.1
        if (bit == 1) { //SPIHT 2.2.1.2
	   list_desc=list_init();//list_free(list_desc);//TODO faire juste un nettoyage de la liste sans la liberer pour eviter un malloc (voir list_free plus bas) done
	   r=spat_spec_desc_spiht(current_el->pixel, list_desc, imageprop, 1, image, thres_ind, map_LSC);//verifier qu'on va jusqu'au bout avec le onlychild a 1
	   current_child = first_el(list_desc);
 	   ngrandchild = 0;
	   list_grand_desc=list_init();
	   while (current_child !=NULL){ //SPIHT 2.2.1.2.1
	      if ((map_LSC[trans_pixel(current_child->pixel, imageprop)] == 0) && (map_LIC[trans_pixel(current_child->pixel, imageprop)] == 0)){ 
        	value_pix=image[trans_pixel(current_child->pixel,imageprop)]; 
// 		bit = ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold));
		bit = get_bit(value_pix, thres_ind);	
		add_to_stream(stream, count, (int) bit, streamlast); //SPIHT 2.2.1.2.1.1
		if (bit == 0){ //SPIHT 2.2.1.2.1.3
 			(map_LIC[trans_pixel(current_child->pixel, imageprop)])++;
			el=el_init(current_child->pixel);
			insert_el(LIC,el);
		} else { //SPIHT 2.2.1.2.1.2
			bitsig = (value_pix > 0);
			add_to_stream(stream, count, (int) bitsig, streamlast);
			(map_LSC[trans_pixel(current_child->pixel, imageprop)])++;
			el=el_init(current_child->pixel);
			insert_el(LSC,el);			
		};
#ifndef NOLISTB
		r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, imageprop, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
		if (r != 0) ngrandchild++;
#else
	 if (map_LIS[trans_pixel(current_child->pixel, imageprop)] == 0){
		el=el_init(current_child->pixel);
		#ifndef INPLACE
		insert_el(LIS,el);
		#else
		insert_el_inplace(LIS,el);
		#endif
		(map_LIS[trans_pixel(el->pixel, imageprop)])++;
	}
#endif
	      };
	      current_child=next_el(list_desc);
	   };
// 	   current_child = first_el(list_desc); //Verification de l'existance de petits-enfants
// 	   ngrandchild = 0;
// 	   list_grand_desc=list_init();//apparemment peu d'influence...
// 	   while ((ngrandchild == 0)&&(current_child !=NULL)){
// 		//if ((map_LSC[trans_pixel(current_child->pixel, imageprop)] == 0) && (map_LIC[trans_pixel(current_child->pixel, imageprop)] == 0)){//attention, ca c'est encore plus fin... (i256)
// 		   r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, imageprop, 1, image, thres_ind, map_LSC);
// 		   if (r != 0) ngrandchild++; //on a au moins un descendant...
// 		//};
// 		current_child=next_el(list_desc);
// 	   };
	   list_free(list_grand_desc);
	   list_free(list_desc);//TODO a modifier pour eviter un malloc
#ifndef NOLISTB
	   if (ngrandchild > 0){//SPIHT 2.2.1.2.2
#ifndef INPLACE
		tmp_el=remove_current_el(LIS); //il ne faut pas liberer la memoire ici
		insert_el(LIS,current_el); //Attention, l'ordre des operations est critique ici
#else
// 		insert_el_inplace(LIS,current_el);
//On ne fait rien dans ce cas, on change juste le type, le meme coefficient va etre traite (le passage au suivant est fait par remove_current_el dans l'autre cas
#endif
		current_el->type=1; // on ne peut pas le rajouter à la fin avant de le supprimer du milieu
	   } else {
#endif
		tmp_el=remove_current_el(LIS);
		(map_LIS[trans_pixel(tmp_el->pixel, imageprop)])--;
		free(tmp_el); //la par contre, il disparait...
		
#ifndef NOLISTB
	   };
#endif
	   tmp_el=NULL;
// 	   if (LIS->current == NULL) printf("Uh Oh 2.2.1.2 (if)\n");
	}else{//fin du 2.2.1.2
	   current_el=next_el(LIS);
// 	   if (LIS->current == NULL) printf("Uh Oh 2.2.1.2 (else)\n");
	};
#ifndef NOLISTB
	} else { //SPIHT 2.2.2 ;else the set is of type B
	  nLISloopB++;
		//On doit faire l'output de Sn(L(i,j))
	  list_desc=list_init();//list_free(list_desc);//done
	  r=spat_spec_desc_spiht(current_el->pixel, list_desc, imageprop, 1, image, thres_ind, map_LSC);
	  current_child = first_el(list_desc);
	  bit = 0;
	  while ((bit == 0) && (current_child !=NULL)){
		if(map_LIS[trans_pixel(current_child->pixel,imageprop)] == 0){//attention, ca c'est fin... (i512)
		   r=spat_spec_desc_spiht(current_child->pixel, /*tmp_list*/list_desc, imageprop, 0, image, thres_ind, map_LSC);
		   if (r == -1) {bit = 1;};
		};
		current_child=next_el(list_desc);
	 };
	 add_to_stream(stream, count, (int) bit, streamlast);

	 if (bit == 1){
		current_child = first_el(list_desc);
		while (current_child !=NULL){
			if (map_LIS[trans_pixel(current_child->pixel,imageprop)] == 0){
				el=el_init(current_child->pixel); //TODO possibilite de recuperer direct sans faire de malloc..
#ifndef INPLACE
				insert_el(LIS,el);
#else
				insert_el_inplace(LIS,el);
#endif
				(map_LIS[trans_pixel(el->pixel, imageprop)])++;
			};
			current_child=next_el(list_desc);
		};
	  	tmp_el=remove_current_el(LIS);
		(map_LIS[trans_pixel(tmp_el->pixel, imageprop)])--;
	  	free(tmp_el);
// 		if (LIS->current == NULL) printf("Uh Oh 2.2.2 (if)\n");
	  } else {
	    current_el=next_el(LIS);
// 	    if (LIS->current == NULL) printf("Uh Oh 2.2.2 (else)\n");
	  };
	  list_free(list_desc); //possibilite de les recuperer direct... (pareil dans 2.2.1.2.1)

    };
#endif
// if (check_map(map_LIS, imageprop)== 1) {printf("erreur");}; //TODO remove after
current_el=LIS->current;
}; //endwhile


current_el=first_el(LSC);
// printf("LSC processing \n");
if (lastLSC != NULL){//Attention à la premiere boucle quand c'est encore vide
while (current_el != lastLSC){
	nLSCloop++;
	value_pix=image[trans_pixel(current_el->pixel,imageprop)];
	bit = get_bit(value_pix, thres_ind);
	add_to_stream(stream, count, (int) bit, streamlast);
	current_el=next_el(LSC);
};
if (current_el != NULL){/*a priori c'est jamais le cas (== lastLSC)*/
	nLSCloop++;
	value_pix=image[trans_pixel(current_el->pixel,imageprop)];
// 	bit = ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold));
	bit = get_bit(value_pix, thres_ind);	
	add_to_stream(stream, count, (int) bit, streamlast);
};
};

printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("Size in bit: %ld \n", *streamlast*8+*count);
printf("nLSCloop: %ld\n",nLSCloop);
printf("nLICloop: %ld\n",nLICloop);
printf("nLISloop: %ld\n",nLISloop);
printf("nLISloopA: %ld\n",nLISloopA);
printf("nLISloopB: %ld\n",nLISloopB);

printf("nLSCmap: %ld\n",count_map(map_LSC,nsmax*nbmax*nlmax));
printf("nLICmap: %ld\n",count_map(map_LIC,nsmax*nbmax*nlmax));
printf("nLISmap: %ld\n",count_map(map_LIS,nsmax*nbmax*nlmax));

outputsize[thres_ind]=(*streamlast)*8 + (*count); //add a +1 ???
printf("-------------------------\n");
};

free(map_LSC);
free(map_LIC);
free(map_LIS);


// return, 0
// outputsize[thres_ind]=*streamlast+1; //add a +1 ???

free(count);
free(streamlast);
list_free(LSC);
// free(LSC);
list_free(LIC);
// free(LIC);
list_free(LIS);
// free(LIS);


return 0;
};


int spiht_decode_c(long int *image, unsigned char *stream, long int *outputsize, long int *maxquantvalue)
{

// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, 8, 8, 7};
struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
int nbmin=imageprop.nbmin;

struct list_struct * LSC=NULL;
struct list_struct * LIC=NULL;
struct list_struct * LIS=NULL;
struct pixel_struct pixel;
// int maxquant=imageprop.maxquant;
// int maxquant=MAXQUANT_CONST;
int maxquant=(int) *maxquantvalue;
int minquant=0;
struct list_el * el=NULL;
struct list_el * current_el=NULL;
struct list_el * tmp_el=NULL;
struct list_el * current_child=NULL;

struct list_el * lastLSC=NULL;
struct list_el * lastprocessed=NULL;

struct list_struct * list_desc=NULL;
struct list_struct * list_grand_desc=NULL;
// long int value_pix=0;
unsigned char bit=255;
unsigned char bitsig=255;
int r=0;
int ngrandchild=0;
unsigned char * count = (unsigned char *) malloc(sizeof(unsigned char));
long int *streamlast = (long int *) malloc(sizeof(long int));
// long int bitrate = 0;
long int threshold=0;
int thres_ind=0;
int i,j,k;
long int i_l;
long int nLICloop;
long int nLSCloop;
long int nLISloop;
long int nLISloopA;
long int nLISloopB;

unsigned char *map_LSC = NULL;
unsigned char *map_LIC = NULL;
unsigned char *map_LIS = NULL;
char flagLSC=0;

map_LSC=(unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));
map_LIC= (unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));
map_LIS= (unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));

*count=0;
*streamlast=0;
LSC = list_init();
LIC = list_init();
LIS = list_init();

// list_desc = list_init();

for (i_l=0;i_l<nsmax*nlmax*nbmax;i_l++)
{
	map_LSC[i_l]=0;
	map_LIC[i_l]=0;
	map_LIS[i_l]=0;
}

//SPIHT 1)
// printf("Initialization...\n");
for (k=0;k<nbmin;k++){
   for (j=0;j<nlmin;j++){
      for (i=0;i<nsmin;i++){
	pixel.x=i;
	pixel.y=j;
	pixel.l=k;
	el=el_init(pixel);
	insert_el(LIC,el);
	(map_LIC[trans_pixel(pixel, imageprop)])++;
	if ((i % 2) || (j % 2) || (k % 2) || (((nbmin % 2) == 1) && (k == nbmin-1) && (k!=0))){ 
		el=el_init(pixel);
		insert_el(LIS,el);
		(map_LIS[trans_pixel(pixel, imageprop)])++;
	 };
      };
   };
};

// r=check_list(LIC);
// if (r != 0) {
//    fprintf(stderr, "LIC non conforme\n");
//    return 1;
// };
// r=check_list(LIS);
// if (r != 0) {
//    fprintf(stderr, "LIS non conforme\n");
//    return 1;
// };


//SPIHT 2)
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){

threshold= 1 << (long int)thres_ind;

printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
// printf("Sorting pass for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);

flagLSC=0;
nLSCloop=0;
nLICloop=0;
nLISloop=0;
nLISloopA=0;
nLISloopB=0;

// Used for Significant pass
lastLSC = LSC->last;

// ;for each entry in the LIC
//SPIHT 2.1)
current_el=first_el(LIC);
// printf("LIC processing \n");
while ((current_el != NULL) && ((*streamlast)*8+ (*count) <= *outputsize)){
// printf(".");
   nLICloop++;
//    value_pix=image[trans_pixel(current_el->pixel,imageprop)];
//    bit = ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold));//TODO possibilite d'enlever un abs
//    bit = get_bit(value_pix, thres_ind);	
//    add_to_stream(stream, count, (int) bit, streamlast);//SPIHT 2.1.1)
   if ((*streamlast)*8+ (*count) <= *outputsize){
      bit = read_from_stream(stream, count, streamlast);
   } else break;

  if (bit == 1) { //SPIHT 2.1.2)
//      bitsig = (value_pix > 0);
//      add_to_stream(stream, count, (int) bitsig, streamlast);
     if ((*streamlast)*8+ (*count) <= *outputsize){
        bitsig = read_from_stream(stream, count, streamlast);
     } else break;
     image[trans_pixel(current_el->pixel,imageprop)] += threshold;
     if (bitsig == 0) {
     	image[trans_pixel(current_el->pixel,imageprop)] = -image[trans_pixel(current_el->pixel,imageprop)];
     };
     (map_LIC[trans_pixel(current_el->pixel, imageprop)])--;
     (map_LSC[trans_pixel(current_el->pixel, imageprop)])++;
     remove_current_el(LIC);
     insert_el(LSC, current_el);
     current_el = LIC->current;//est passe au suivant dans le move
   } else {
      current_el=next_el(LIC);
   };
}


// ;for each entry in the LIS
current_el=first_el(LIS);
// printf("LIS processing \n");
while ((current_el != NULL)&& ((*streamlast)*8+ (*count) <= *outputsize)){ //SPIHT 2.2)
// printf("Processing in LIS: %d %d %d\n",current_el->pixel.x,current_el->pixel.y,current_el->pixel.l);
nLISloop++;
// if (LIS->current == NULL) printf("Uh Oh 2.2\n");
#ifndef NOLISTB
    if (current_el->type == 0) { //SPIHT 2.2.1);if the set is of type A
#endif
        nLISloopA++;
// 	r=spat_spec_desc_spiht(current_el->pixel, list_desc, imageprop, 0, image, thres_ind, map_LSC);
// 	bit = (r == -1);//il y a au moins un des descendants qui est significatif
//         add_to_stream(stream, count, (int) bit, streamlast); //SPIHT 2.2.1.1
        if ((*streamlast)*8+ (*count) <= *outputsize){
	   bit = read_from_stream(stream, count, streamlast);
        } else break;
        if (bit == 1) { //SPIHT 2.2.1.2
	   list_desc=list_init();//list_free(list_desc);//TODO faire juste un nettoyage de la liste sans la liberer pour eviter un malloc (voir list_free plus bas) done
	   r=spat_spec_desc_spiht(current_el->pixel, list_desc, imageprop, 1, image, thres_ind, map_LSC);//verifier qu'on va jusqu'au bout avec le onlychild a 1
	   current_child = first_el(list_desc);
 	   ngrandchild = 0;
	   list_grand_desc=list_init();
	   while ((current_child !=NULL)&& ((*streamlast)*8+ (*count) <= *outputsize)){ //SPIHT 2.2.1.2.1
	      if ((map_LSC[trans_pixel(current_child->pixel, imageprop)] == 0) && (map_LIC[trans_pixel(current_child->pixel, imageprop)] == 0)){ 
// 		value_pix=image[trans_pixel(current_child->pixel,imageprop)]; 
// 		bit = get_bit(value_pix, thres_ind);	
// 		add_to_stream(stream, count, (int) bit, streamlast); //SPIHT 2.2.1.2.1.1
		if ((*streamlast)*8+ (*count) <= *outputsize){
		   bit = read_from_stream(stream, count, streamlast);
		} else break;
		if (bit == 0){ //SPIHT 2.2.1.2.1.3
 			(map_LIC[trans_pixel(current_child->pixel, imageprop)])++;
			el=el_init(current_child->pixel);
			insert_el(LIC,el);
		} else { //SPIHT 2.2.1.2.1.2
			image[trans_pixel(current_child->pixel,imageprop)] += threshold; //le signe est vu apres
// 			bitsig = (value_pix > 0);
// 			add_to_stream(stream, count, (int) bitsig, streamlast);
			if ((*streamlast)*8+ (*count) <= *outputsize){
			   bitsig = read_from_stream(stream, count, streamlast);
			} else break;
			if (bitsig == 0){
				image[trans_pixel(current_child->pixel,imageprop)] = -image[trans_pixel(current_child->pixel,imageprop)];
			};
			(map_LSC[trans_pixel(current_child->pixel, imageprop)])++;
			el=el_init(current_child->pixel);
			insert_el(LSC,el);			
		};
		r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, imageprop, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
		if (r != 0) ngrandchild++;
	      };  
	      current_child=next_el(list_desc);
	   };
// 	   current_child = first_el(list_desc); //Verification de l'existance de petits-enfants
// 	   ngrandchild = 0;
// 	   list_grand_desc=list_init();
// 	   while ((ngrandchild == 0)&&(current_child !=NULL)){
// 		r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, imageprop, 1, image, thres_ind, map_LSC);
// 		if (r != 0) ngrandchild++; //on a au moins un descendant...
// 		current_child=next_el(list_desc);
// 	   };
	   list_free(list_grand_desc);
	   list_free(list_desc);//TODO a modifier pour eviter un malloc
	   if (ngrandchild > 0){//SPIHT 2.2.1.2.2
#ifndef INPLACE 
		tmp_el=remove_current_el(LIS); //il ne faut pas liberer la memoire ici...	
		insert_el(LIS,current_el); //Attention, l'ordre des operations est critique ici
#else
// 		insert_el_inplace(LIS,current_el);
//On ne fait rien dans ce cas, on change juste le type, le meme coefficient va etre traite (le passage au suivant est fait par remove_current_el dans l'autre cas
#endif
		current_el->type=1; // on ne peut pas le rajouter à la fin avant de le supprimer du milieu
	   } else {
		tmp_el=remove_current_el(LIS);
		(map_LIS[trans_pixel(tmp_el->pixel, imageprop)])--;
		free(tmp_el); //la par contre, il disparait...
		
	   };
	   tmp_el=NULL;
// 	   if (LIS->current == NULL) printf("Uh Oh 2.2.1.2 (if)\n");
	}else{//fin du 2.2.1.2
	   current_el=next_el(LIS);
// 	   if (LIS->current == NULL) printf("Uh Oh 2.2.1.2 (else)\n");
	};
	} else { //SPIHT 2.2.2 ;else the set is of type B
	  nLISloopB++;
		//On doit faire l'output de Sn(L(i,j))
// 	  list_desc=list_init();
// 	  r=spat_spec_desc_spiht(current_el->pixel, list_desc, imageprop, 1, image, thres_ind, map_LSC);
// 	  current_child = first_el(list_desc);
	  bit = 0;
// 	  while ((bit == 0) && (current_child !=NULL)){
// 		r=spat_spec_desc_spiht(current_child->pixel, list_desc, imageprop, 0, image, thres_ind, map_LSC);
// 		if (r == -1) {bit = 1;};
// 		current_child=next_el(list_desc);
// 	 };
// 	 add_to_stream(stream, count, (int) bit, streamlast);
	if ((*streamlast)*8+ (*count) <= *outputsize){
	   bit = read_from_stream(stream, count, streamlast);
	} else break;
	  if (bit == 1){
		list_desc=list_init();//list_free(list_desc);  //decode only done
	  	r=spat_spec_desc_spiht(current_el->pixel, list_desc, imageprop, 1, image, thres_ind, map_LSC);//decode only
		current_child = first_el(list_desc);
		while (current_child !=NULL){
			if (map_LIS[trans_pixel(current_child->pixel,imageprop)] == 0){
				el=el_init(current_child->pixel); //TODO possibilite de recuperer direct sans faire de malloc..
#ifndef INPLACE
				insert_el(LIS,el);
#else
				insert_el_inplace(LIS,el);
#endif
				(map_LIS[trans_pixel(el->pixel, imageprop)])++;
			};
			current_child=next_el(list_desc);
		};
	  	tmp_el=remove_current_el(LIS);
		(map_LIS[trans_pixel(tmp_el->pixel, imageprop)])--;
	  	free(tmp_el);
// 		if (LIS->current == NULL) printf("Uh Oh 2.2.2 (if)\n");
		list_free(list_desc);//decode only
	  } else {
	    current_el=next_el(LIS);
// 	    if (LIS->current == NULL) printf("Uh Oh 2.2.2 (else)\n");
	  };
	  //list_free(list_desc); //possibilite de les recuperer direct... (pareil dans 2.2.1.2.1)

    };
// if (check_map(map_LIS, imageprop)== 1) {printf("erreur");}; //TODO remove after
current_el=LIS->current;   
}; //endwhile


current_el=first_el(LSC);
// printf("LSC processing \n");
if (lastLSC != NULL){//Attention à la premiere boucle quand c'est encore vide
while ((current_el != lastLSC) && ((*streamlast)*8+ (*count) <= *outputsize)){
	nLSCloop++;
// 	value_pix=image[trans_pixel(current_el->pixel,imageprop)];
// 	bit = get_bit(value_pix, thres_ind);
// 	add_to_stream(stream, count, (int) bit, streamlast);
	if ((*streamlast)*8+ (*count) <= *outputsize){
	   bit = read_from_stream(stream, count, streamlast);
	   flagLSC=1;
	} else break;
	if (bit == 1){
		if (image[trans_pixel(current_el->pixel,imageprop)] > 0){
			image[trans_pixel(current_el->pixel,imageprop)] += threshold;
		} else {
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
		};
	};
	current_el=next_el(LSC);
};
if ((current_el != NULL) && ((*streamlast)*8+ (*count) <= *outputsize)){/*a priori c'est jamais le cas (== lastLSC)*/
	nLSCloop++;
// 	value_pix=image[trans_pixel(current_el->pixel,imageprop)];
// // 	bit = ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold));
// 	bit = get_bit(value_pix, thres_ind);	
// 	add_to_stream(stream, count, (int) bit, streamlast);
	if ((*streamlast)*8+ (*count) <= *outputsize){
	   bit = read_from_stream(stream, count, streamlast);
	//}; else break; 
	if (bit == 1){
		if (image[trans_pixel(current_el->pixel,imageprop)] > 0){
			image[trans_pixel(current_el->pixel,imageprop)] += threshold;
		} else {
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
		};
	};
	current_el=next_el(LSC);
	};

};
};

printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("nLSCloop: %ld\n",nLSCloop);
printf("nLICloop: %ld\n",nLICloop);
printf("nLISloop: %ld\n",nLISloop);
printf("nLISloopA: %ld\n",nLISloopA);
printf("nLISloopB: %ld\n",nLISloopB);
printf("-------------------------\n");

printf("*streamlast)*8+ (*count): %ld\n", (*streamlast)*8+ (*count));
printf("*outputsize: %ld\n", *outputsize);

if ((*streamlast)*8+ (*count) > *outputsize){
 	printf("Sortie: fin du train de bit (threshold %ld)\n", threshold);
	break;
};

};

//correction finale eventuelle 
//TODO: la sortie est calculee a l'octet pres, pas au bit
// if (thres_ind !=0){//si on a entame la boucle finale on ne va pas faire de correc
printf("Correction finale (flagLSC= %d)\n", flagLSC);
if ((*streamlast)*8+ (*count) > *outputsize){//on est sorti car le train de bit etait trop court
	if (flagLSC == 0){
		current_el=first_el(LSC);
		if (lastLSC != NULL){
		   while (current_el != lastLSC){
		      if (image[trans_pixel(current_el->pixel,imageprop)] >0){
			image[trans_pixel(current_el->pixel,imageprop)] += threshold;
		      };
		      if (image[trans_pixel(current_el->pixel,imageprop)] <0){
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
		      };
		      current_el=next_el(LSC);
		   };
		      if (image[trans_pixel(current_el->pixel,imageprop)] >0){//current_el = lastLSC donc necessairement different de NULL
			image[trans_pixel(current_el->pixel,imageprop)] += threshold;
		      };
		      if (image[trans_pixel(current_el->pixel,imageprop)] <0){
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
		      };
		   current_el=next_el(LSC);
		   while ((current_el != NULL) && (current_el !=NULL)){//cas NULL peut etre si aucun el n'a ete ajoute dans la LSC a l'etape du break.
		      if (image[trans_pixel(current_el->pixel,imageprop)] >0){
			image[trans_pixel(current_el->pixel,imageprop)] += threshold/2;
		      };
		      if (image[trans_pixel(current_el->pixel,imageprop)] <0){
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold/2;
		      };
		      current_el=next_el(LSC);
		   };
		};
	} else {
		lastprocessed=LSC->previous;
		current_el=first_el(LSC);
// 		if (lastLSC != NULL){
		printf("lastLSC %d\n", lastLSC );
		printf("lastprocessed %d\n", lastprocessed);
		   while (current_el != lastprocessed){
		      if (image[trans_pixel(current_el->pixel,imageprop)] >0){
			image[trans_pixel(current_el->pixel,imageprop)] += threshold/2;
		      };
		      if (image[trans_pixel(current_el->pixel,imageprop)] <0){
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold/2;
		      };
		      current_el=next_el(LSC);
		   };
		      if (image[trans_pixel(current_el->pixel,imageprop)] >0){//current_el = lastprocessed donc necessairement different de NULL
			image[trans_pixel(current_el->pixel,imageprop)] += threshold/2;
		      };
		      if (image[trans_pixel(current_el->pixel,imageprop)] <0){
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold/2;
		      };
		   current_el=next_el(LSC);
		   if (lastLSC != lastprocessed){//attention, un cas particulier
			while ((current_el != lastLSC) && (current_el !=NULL)){//le cas NULL arrive si on s'est arrete pile a la fin d'une etape, ex: decompression complete->NOPE
			if (image[trans_pixel(current_el->pixel,imageprop)] >0){
				image[trans_pixel(current_el->pixel,imageprop)] += threshold;
			};
			if (image[trans_pixel(current_el->pixel,imageprop)] <0){
				image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
			};
			current_el=next_el(LSC);
			};
			if (current_el !=NULL){
			if (image[trans_pixel(current_el->pixel,imageprop)] >0){
				image[trans_pixel(current_el->pixel,imageprop)] += threshold;
			};
			if (image[trans_pixel(current_el->pixel,imageprop)] <0){
				image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
			};
			current_el=next_el(LSC);
			};
		   };
		   while ((current_el != NULL)&& (current_el !=NULL)){//pas sur que le cas NULL arrive, peut etre si aucun el n'a ete ajoute dans la LSC a cette etape
		      if (image[trans_pixel(current_el->pixel,imageprop)] >0){
			image[trans_pixel(current_el->pixel,imageprop)] += threshold/2;
		      };
		      if (image[trans_pixel(current_el->pixel,imageprop)] <0){
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold/2;
		      };
		      current_el=next_el(LSC);
		   };
// 		};
	};

};

free(map_LSC);
free(map_LIC);
free(map_LIS);

free(count);
free(streamlast);
 

list_free(LSC);
// free(LSC);
list_free(LIC);
// free(LIC);
list_free(LIS);
// free(LIS);
// return, 0
return 0;
};

