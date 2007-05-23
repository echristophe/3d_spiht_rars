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

int spiht_code_c(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param)
{


int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
int nbmin=imageprop.nbmin;

list_struct * LSC=NULL;
list_struct * LIC=NULL;
list_struct * LIS=NULL;
pixel_struct pixel;
// parents_struct parents;
// int is_accessible=0;

int maxquant = (int) *coder_param.maxquant;

int minquant=0;
list_el * el=NULL;
list_el * current_el=NULL;
list_el * tmp_el=NULL;
list_el * current_child=NULL;
list_el * lastLSC=NULL;


list_struct * list_desc=NULL;
list_struct * list_grand_desc=NULL;

long int value_pix=0;
unsigned char bit=255;
unsigned char bitsig=255;
int r=0;
int ngrandchild=0;
unsigned char *stream = streamstruct.stream;
unsigned char * count = streamstruct.count;
long int *streamlast = streamstruct.streamlast;

long int threshold=0;
int thres_ind=0;
int i,j,k;
long int i_l;

#ifdef DEBUG
long int nLICloop;
long int nLSCloop;
long int nLISloop;
long int nLISloopA;
long int nLISloopB;
#endif

unsigned char *map_LSC = NULL;
unsigned char *map_LIC = NULL;
unsigned char *map_LIS = NULL;

map_LSC=(unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));
map_LIC= (unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));
map_LIS= (unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));


LSC = list_init();
LIC = list_init();
LIS = list_init();


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
	(map_LIC[trans_pixel(pixel)])++;
	if ((i % 2) || (j % 2) || (k % 2) || (((nbmin % 2) == 1) && (k == nbmin-1) && (k!=0))){ 
		el=el_init(pixel);
		insert_el(LIS,el);
		(map_LIS[trans_pixel(pixel)])++;
	 };
      };
   };
};


//SPIHT 2)
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){

threshold= 1 << (long int)thres_ind;

#ifdef DEBUG
printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
// printf("Sorting pass for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);


nLSCloop=0;
nLICloop=0;
nLISloop=0;
nLISloopA=0;
nLISloopB=0;
#endif

// Used for Significant pass
lastLSC = LSC->last;

// ;for each entry in the LIC
//SPIHT 2.1)
current_el=first_el(LIC);
// printf("LIC processing \n");
while (current_el != NULL){
// printf(".");
#ifdef DEBUG
   nLICloop++;
#endif
   value_pix=image[trans_pixel(current_el->pixel)];
   bit = get_bit(value_pix, thres_ind);	
   add_to_stream(stream, count, (int) bit, streamlast);//SPIHT 2.1.1)
  if (bit == 1) { //SPIHT 2.1.2)
     bitsig = (value_pix > 0);
     add_to_stream(stream, count, (int) bitsig, streamlast);
     (map_LIC[trans_pixel(current_el->pixel)])--;
     (map_LSC[trans_pixel(current_el->pixel)])++;
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
#ifdef DEBUG
nLISloop++;
#endif
    if (current_el->type == 0) { //SPIHT 2.2.1);if the set is of type A
#ifdef DEBUG
        nLISloopA++;
#endif
	r=spat_spec_desc_spiht(current_el->pixel, list_desc, 0, image, thres_ind, map_LSC);
	bit = (r == -1);//At least one descendant is significant
        add_to_stream(stream, count, (int) bit, streamlast); //SPIHT 2.2.1.1
        if (bit == 1) { //SPIHT 2.2.1.2
	   list_desc=list_init();
	   r=spat_spec_desc_spiht(current_el->pixel, list_desc, 1, image, thres_ind, map_LSC);
	   current_child = first_el(list_desc);
 	   ngrandchild = 0;
	   list_grand_desc=list_init();
	   while (current_child !=NULL){ //SPIHT 2.2.1.2.1
	      if ((map_LSC[trans_pixel(current_child->pixel)] == 0) && (map_LIC[trans_pixel(current_child->pixel)] == 0)){ 
        	value_pix=image[trans_pixel(current_child->pixel)]; 
		bit = get_bit(value_pix, thres_ind);	
		add_to_stream(stream, count, (int) bit, streamlast); //SPIHT 2.2.1.2.1.1
		if (bit == 0){ //SPIHT 2.2.1.2.1.3
 			(map_LIC[trans_pixel(current_child->pixel)])++;
			el=el_init(current_child->pixel);
			insert_el(LIC,el);
		} else { //SPIHT 2.2.1.2.1.2
			bitsig = (value_pix > 0);
			add_to_stream(stream, count, (int) bitsig, streamlast);
			(map_LSC[trans_pixel(current_child->pixel)])++;
			el=el_init(current_child->pixel);
			insert_el(LSC,el);			
		};
		r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
		if (r != 0) ngrandchild++;

	      };
	      current_child=next_el(list_desc);
	   };

	   list_free(list_grand_desc);
	   list_free(list_desc);//TODO a modifier pour eviter un malloc
	   if (ngrandchild > 0){//SPIHT 2.2.1.2.2
		tmp_el=remove_current_el(LIS); //il ne faut pas liberer la memoire ici
		insert_el(LIS,current_el); //Attention, l'ordre des operations est critique ici

		current_el->type=1; // on ne peut pas le rajouter à la fin avant de le supprimer du milieu
	   } else {
		tmp_el=remove_current_el(LIS);
		(map_LIS[trans_pixel(tmp_el->pixel)])--;
		free(tmp_el); //la par contre, il disparait...
	   };
	   tmp_el=NULL;

	}else{//fin du 2.2.1.2
	   current_el=next_el(LIS);

	};
	} else { //SPIHT 2.2.2 ;else the set is of type B
#ifdef DEBUG
	  nLISloopB++;
#endif
		//On doit faire l'output de Sn(L(i,j))
	  list_desc=list_init();//list_free(list_desc);//done
	  r=spat_spec_desc_spiht(current_el->pixel, list_desc, 1, image, thres_ind, map_LSC);
	  current_child = first_el(list_desc);
	  bit = 0;
	  while ((bit == 0) && (current_child !=NULL)){
		if(map_LIS[trans_pixel(current_child->pixel)] == 0){//attention, ca c'est fin... (i512)
		   r=spat_spec_desc_spiht(current_child->pixel, /*tmp_list*/list_desc, 0, image, thres_ind, map_LSC);
		   if (r == -1) {bit = 1;};
		};
		current_child=next_el(list_desc);
	 };
	 add_to_stream(stream, count, (int) bit, streamlast);

	 if (bit == 1){
		current_child = first_el(list_desc);
		while (current_child !=NULL){
			if (map_LIS[trans_pixel(current_child->pixel)] == 0){
				el=el_init(current_child->pixel); //TODO possibilite de recuperer direct sans faire de malloc..
				insert_el(LIS,el);
				(map_LIS[trans_pixel(el->pixel)])++;
			};
			current_child=next_el(list_desc);
		};
	  	tmp_el=remove_current_el(LIS);
		(map_LIS[trans_pixel(tmp_el->pixel)])--;
	  	free(tmp_el);
	  } else {
	    current_el=next_el(LIS);
	  };
	  list_free(list_desc); //possibilite de les recuperer direct... (pareil dans 2.2.1.2.1)

    };
current_el=LIS->current;
}; //endwhile


current_el=first_el(LSC);
// printf("LSC processing \n");
if (lastLSC != NULL){//Attention à la premiere boucle quand c'est encore vide
while (current_el != lastLSC){
#ifdef DEBUG
	nLSCloop++;
#endif
	value_pix=image[trans_pixel(current_el->pixel)];
	bit = get_bit(value_pix, thres_ind);
	add_to_stream(stream, count, (int) bit, streamlast);
	current_el=next_el(LSC);
};
if (current_el != NULL){/*a priori c'est jamais le cas (== lastLSC)*/
#ifdef DEBUG
	nLSCloop++;
#endif
	value_pix=image[trans_pixel(current_el->pixel)];
	bit = get_bit(value_pix, thres_ind);	
	add_to_stream(stream, count, (int) bit, streamlast);
};
};
#ifdef DEBUG
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


printf("-------------------------\n");
#endif

outputsize[thres_ind]=(*streamlast)*8 + (*count); //add a +1 ???

};

free(map_LSC);
free(map_LIC);
free(map_LIS);

free(count);
free(streamlast);
list_free(LSC);
list_free(LIC);
list_free(LIS);


if (coder_param.rate != 0){
outputsize[0] = (long int) (coder_param.rate * nsmax*nbmax*nlmax); //Il faudrait faire ca bien et le mettre dans la compression
}

return 0;
};




int spiht_decode_c(long int *image, stream_struct streamstruct, long int *outputsize, coder_param_struct coder_param)
{


int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
int nbmin=imageprop.nbmin;

list_struct * LSC=NULL;
list_struct * LIC=NULL;
list_struct * LIS=NULL;
pixel_struct pixel;
// parents_struct parents;
// int is_accessible=0;
int maxquant=(int) *coder_param.maxquant;

int minquant=0;
list_el * el=NULL;
list_el * current_el=NULL;
list_el * tmp_el=NULL;
list_el * current_child=NULL;

list_el * lastLSC=NULL;
// list_el * lastprocessed=NULL;

list_struct * list_desc=NULL;
list_struct * list_grand_desc=NULL;
unsigned char bit=255;
unsigned char bitsig=255;
int r=0;
int ngrandchild=0;
unsigned char *stream = streamstruct.stream;
unsigned char * count = streamstruct.count;
long int *streamlast = streamstruct.streamlast;

long int threshold=0;
int thres_ind=0;
int i,j,k;
long int i_l;

#ifdef DEBUG
long int nLICloop;
long int nLSCloop;
long int nLISloop;
long int nLISloopA;
long int nLISloopB;
#endif

unsigned char *map_LSC = NULL;
unsigned char *map_LIC = NULL;
unsigned char *map_LIS = NULL;
char flagLSC=0;

map_LSC=(unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));
map_LIC= (unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));
map_LIS= (unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));

LSC = list_init();
LIC = list_init();
LIS = list_init();


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
	(map_LIC[trans_pixel(pixel)])++;
	if ((i % 2) || (j % 2) || (k % 2) || (((nbmin % 2) == 1) && (k == nbmin-1) && (k!=0))){ 
		el=el_init(pixel);
		insert_el(LIS,el);
		(map_LIS[trans_pixel(pixel)])++;
	 };
      };
   };
};


//SPIHT 2)
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){

threshold= 1 << (long int)thres_ind;
#ifdef DEBUG
printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
#endif

flagLSC=0;

#ifdef DEBUG
nLSCloop=0;
nLICloop=0;
nLISloop=0;
nLISloopA=0;
nLISloopB=0;
#endif

// Used for Significant pass
lastLSC = LSC->last;

// ;for each entry in the LIC
//SPIHT 2.1)
current_el=first_el(LIC);
// printf("LIC processing \n");
while ((current_el != NULL) && ((*streamlast)*8+ (*count) <= *outputsize)){
#ifdef DEBUG
   nLICloop++;
#endif
   if ((*streamlast)*8+ (*count) <= *outputsize){
      bit = read_from_stream(stream, count, streamlast);
   } else break;

  if (bit == 1) { //SPIHT 2.1.2)
     if ((*streamlast)*8+ (*count) <= *outputsize){
        bitsig = read_from_stream(stream, count, streamlast);
     } else break;
     image[trans_pixel(current_el->pixel)] += threshold + threshold/2;
     if (bitsig == 0) {
     	image[trans_pixel(current_el->pixel)] = -image[trans_pixel(current_el->pixel)];
     };
     (map_LIC[trans_pixel(current_el->pixel)])--;
     (map_LSC[trans_pixel(current_el->pixel)])++;
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
#ifdef DEBUG
nLISloop++;
#endif

    if (current_el->type == 0) { //SPIHT 2.2.1);if the set is of type A
#ifdef DEBUG
        nLISloopA++;
#endif
        if ((*streamlast)*8+ (*count) <= *outputsize){
	   bit = read_from_stream(stream, count, streamlast);
        } else break;
        if (bit == 1) { //SPIHT 2.2.1.2
	   list_desc=list_init();//TODO faire juste un nettoyage de la liste sans la liberer pour eviter un malloc (voir list_free plus bas) done
	   r=spat_spec_desc_spiht(current_el->pixel, list_desc, 1, image, thres_ind, map_LSC);//verifier qu'on va jusqu'au bout avec le onlychild a 1
	   current_child = first_el(list_desc);
 	   ngrandchild = 0;
	   list_grand_desc=list_init();
	   while ((current_child !=NULL)&& ((*streamlast)*8+ (*count) <= *outputsize)){ //SPIHT 2.2.1.2.1
	      if ((map_LSC[trans_pixel(current_child->pixel)] == 0) && (map_LIC[trans_pixel(current_child->pixel)] == 0)){ 
		if ((*streamlast)*8+ (*count) <= *outputsize){
		   bit = read_from_stream(stream, count, streamlast);
		} else break;
		if (bit == 0){ //SPIHT 2.2.1.2.1.3
 			(map_LIC[trans_pixel(current_child->pixel)])++;
			el=el_init(current_child->pixel);
			insert_el(LIC,el);
		} else { //SPIHT 2.2.1.2.1.2
			image[trans_pixel(current_child->pixel)] += threshold + threshold/2; //le signe est vu apres
			if ((*streamlast)*8+ (*count) <= *outputsize){
			   bitsig = read_from_stream(stream, count, streamlast);
			} else break;
			if (bitsig == 0){
				image[trans_pixel(current_child->pixel)] = -image[trans_pixel(current_child->pixel)];
			};
			(map_LSC[trans_pixel(current_child->pixel)])++;
			el=el_init(current_child->pixel);
			insert_el(LSC,el);			
		};
		r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
		if (r != 0) ngrandchild++;
	      };  
	      current_child=next_el(list_desc);
	   };

	   list_free(list_grand_desc);
	   list_free(list_desc);//TODO a modifier pour eviter un malloc
	   if (ngrandchild > 0){//SPIHT 2.2.1.2.2
		tmp_el=remove_current_el(LIS); //il ne faut pas liberer la memoire ici...	
		insert_el(LIS,current_el); //Attention, l'ordre des operations est critique ici
		current_el->type=1; // on ne peut pas le rajouter �la fin avant de le supprimer du milieu
	   } else {
		tmp_el=remove_current_el(LIS);
		(map_LIS[trans_pixel(tmp_el->pixel)])--;
		free(tmp_el); //la par contre, il disparait...
		
	   };
	   tmp_el=NULL;
	}else{//fin du 2.2.1.2
	   current_el=next_el(LIS);
	};
	} else { //SPIHT 2.2.2 ;else the set is of type B
#ifdef DEBUG
	  nLISloopB++;
#endif
		//On doit faire l'output de Sn(L(i,j))

	  bit = 0;

	if ((*streamlast)*8+ (*count) <= *outputsize){
	   bit = read_from_stream(stream, count, streamlast);
	} else break;
	  if (bit == 1){
		list_desc=list_init();//list_free(list_desc);  //decode only done
	  	r=spat_spec_desc_spiht(current_el->pixel, list_desc, 1, image, thres_ind, map_LSC);//decode only
		current_child = first_el(list_desc);
		while (current_child !=NULL){
			if (map_LIS[trans_pixel(current_child->pixel)] == 0){
				el=el_init(current_child->pixel); //TODO possibilite de recuperer direct sans faire de malloc..
				insert_el(LIS,el);
				(map_LIS[trans_pixel(el->pixel)])++;
			};
			current_child=next_el(list_desc);
		};
	  	tmp_el=remove_current_el(LIS);
		(map_LIS[trans_pixel(tmp_el->pixel)])--;
	  	free(tmp_el);
		list_free(list_desc);//decode only
	  } else {
	    current_el=next_el(LIS);
	  };

    };
current_el=LIS->current;   
}; //endwhile


current_el=first_el(LSC);

// printf("LSC processing \n");
if (lastLSC != NULL){//Attention a la premiere boucle quand c'est encore vide
while ((current_el != lastLSC) && ((*streamlast)*8+ (*count) <= *outputsize)){
#ifdef DEBUG
	nLSCloop++;
#endif
	if ((*streamlast)*8+ (*count) <= *outputsize){
	   bit = read_from_stream(stream, count, streamlast);
	   flagLSC=1;
	} else break;
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
	current_el=next_el(LSC);
};
//TODO check if this is happening and necessary
if ((current_el != NULL) && ((*streamlast)*8+ (*count) <= *outputsize)){/*a priori c'est jamais le cas (== lastLSC)*/
#ifdef DEBUG
	nLSCloop++;
#endif

	if ((*streamlast)*8+ (*count) <= *outputsize){
	   bit = read_from_stream(stream, count, streamlast);
	//}; else break; 
	if (bit == 1){
		if (image[trans_pixel(current_el->pixel)] > 0){
			image[trans_pixel(current_el->pixel)] += threshold/2;
		} else {
			image[trans_pixel(current_el->pixel)] -= threshold/2;
		};
	} else {//in this case, we need to remove the excess we put before
		if (image[trans_pixel(current_el->pixel)] > 0){
			image[trans_pixel(current_el->pixel)] -= threshold/2;
		} else {
			image[trans_pixel(current_el->pixel)] += threshold/2;
		};	
	}
	current_el=next_el(LSC);
	};

};
};

#ifdef DEBUG
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("nLSCloop: %ld\n",nLSCloop);
printf("nLICloop: %ld\n",nLICloop);
printf("nLISloop: %ld\n",nLISloop);
printf("nLISloopA: %ld\n",nLISloopA);
printf("nLISloopB: %ld\n",nLISloopB);
printf("-------------------------\n");

printf("(*streamlast)*8+ (*count): %ld\n", (*streamlast)*8+ (*count));
printf("*outputsize: %ld\n", *outputsize);
#endif

if ((*streamlast)*8+ (*count) > *outputsize){
#ifdef DEBUG
 	printf("Sortie: fin du train de bit (threshold %ld)\n", threshold);
#endif
	break;
};

};


free(map_LSC);
free(map_LIC);
free(map_LIS);

free(count);
free(streamlast);

list_free(LSC);
list_free(LIC);
list_free(LIS);

return 0;
};

