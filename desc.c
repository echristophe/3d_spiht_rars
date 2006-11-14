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

/*
;return 0 if no offspring
;return 1 if offsrping and all non sig (according to thres_cube) (ZEROTREE)
;return -1 if one offspring at least is sig (in this case, abord further search)
*/

/*
Attention, l'option directchildonly a une signification difference si compile avec ou sans l'option EZW
sans option:
recherche uniquement les descendants directs, sans appel recursif
avec option:
recherche tous les descendant, mais pas de sortie anticip�
*/


/****************************************************
SPECTRAL - SPATIAL
******************************************************/


int spat_spec_desc_spiht(pixel_struct pixel, list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC)
{

int r1=0;
int r2=0;
int r=0;
int out=1;

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;
#ifdef NEWTREE
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
#endif

list_el *current_el=NULL;
list_struct *tmp_list=NULL;

long int threshold= 1 << (long int)thres_ind;


if (directchildonly == 0){
	tmp_list=list_init();
};



if (((pixel.x >= nsmax / 2) || (pixel.y >= nlmax / 2)) &&  (pixel.l >= nbmax / 2)) {
	if (directchildonly == 0){list_free(tmp_list);};
	return 0;
}


#ifdef NEWTREE
if ((pixel.x < nsmin) && (pixel.y < nlmin)){
#endif
if (directchildonly == 0){
r1=spec_desc_spiht(pixel, tmp_list, directchildonly, image, threshold, map_LSC);
	if (r1 == -1) {
		if (directchildonly == 0){list_free(tmp_list);};
		return -1;
	};
} else {
r1=spec_desc_spiht(pixel, list_desc, directchildonly, image, threshold, map_LSC);
};
#ifdef NEWTREE
};
#endif


/* dans tous les cas, on regarde les descendant spatiaux du pixel courant */
r2=spat_desc_spiht(pixel, list_desc, directchildonly, image, threshold, map_LSC);

if ((directchildonly == 0) && ((r1 == -1) || (r2 == -1))) {
	if (directchildonly == 0){list_free(tmp_list);};
	return -1;//sortie anticip�
};
if ((directchildonly == 1) && ((r1 == -1) || (r2 == -1))) out=-1;
if ((r1 == 0) && (r2 == 0)) {
	if (directchildonly == 0){list_free(tmp_list);};
	return 0;//pas de descendant (LF paires)
};

if (directchildonly == 0) {
  current_el=tmp_list->first;
  while (current_el != NULL){
  	r=spat_desc_spiht(current_el->pixel, list_desc, directchildonly, image, threshold, map_LSC);
  	current_el= current_el->next;
	if ((directchildonly == 0) && (r == -1)) {return -1; list_free(tmp_list);};
  };
  list_free(tmp_list);
};


return out;
};

/****************************************************
SPECTRAL
******************************************************/

int spec_desc_spiht(pixel_struct pixel, list_struct * list_desc, int directchildonly, long int *image, long int threshold, unsigned char *map_LSC){


pixel_struct new_pixel;
long int value_pix;
list_el * current_el=NULL;

int r=0;
int out=1;

int nbmax=imageprop.nbmax;
int nbmin=imageprop.nbmin;

list_el *current_el1=NULL;
list_el *current_el2=NULL;

if (pixel.l >= nbmax / 2) return 0;


if (pixel.l < nbmin){/*lower frequency*/
   if ((pixel.l %  2) == 0) {
     if (pixel.l != nbmin-1){
        return 0;/*no offspring*/
     } else {
        /*;Add offspring coordinate to the array*/
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=pixel.l+nbmin;
	current_el=el_init(new_pixel);	
	insert_el(list_desc, current_el);
	
	value_pix=image[trans_pixel(new_pixel)];

#ifndef NEWTREE
        if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(new_pixel)]==0)) {
#else
        if (isLSC2(abs(value_pix), threshold)) {
#endif
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};

	if (directchildonly == 0){
		r = spec_desc_spiht(new_pixel, list_desc, directchildonly, image, threshold, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out= -1;
	};
     };
   };
   if ((pixel.l % 2) == 1 ){
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=pixel.l+nbmin-1;
	current_el1=el_init(new_pixel);		
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=pixel.l+nbmin;
	current_el2=el_init(new_pixel);	

	insert_el(list_desc, current_el1);
	insert_el(list_desc, current_el2);
	
	value_pix=image[trans_pixel(current_el1->pixel)];

#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(current_el1->pixel)]==0)) {
#else
	if (isLSC2(abs(value_pix), threshold) ) {
#endif		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};

	value_pix=image[trans_pixel(current_el2->pixel)];

#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(current_el2->pixel)]==0)) {	
#else
	if (isLSC2(abs(value_pix), threshold) ) {
#endif		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};	
		
	/*;Recursive call on offspring*/
	if (directchildonly != 1){
		r = spec_desc_spiht(current_el1->pixel, list_desc, directchildonly, image, threshold, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out=-1;
		r = spec_desc_spiht(current_el2->pixel, list_desc, directchildonly, image, threshold, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out=-1;
	};
   };

} else {
	
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=2*pixel.l;
	current_el1=el_init(new_pixel);		
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=2*pixel.l+1;
	current_el2=el_init(new_pixel);	

	insert_el(list_desc, current_el1);
	insert_el(list_desc, current_el2);

	value_pix=image[trans_pixel(current_el1->pixel)];

#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(current_el1->pixel)]==0)) {
#else
	if (isLSC2(abs(value_pix), threshold)) {
#endif		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
	value_pix=image[trans_pixel(current_el2->pixel)];

#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(current_el2->pixel)]==0)) {
#else
	if (isLSC2(abs(value_pix), threshold)) {
#endif		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};	
		
	/*;Recursive call on offspring*/
	if (directchildonly != 1){
		r = spec_desc_spiht(current_el1->pixel, list_desc, directchildonly, image, threshold, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out=-1;
		r = spec_desc_spiht(current_el2->pixel, list_desc, directchildonly, image, threshold, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out=-1;
	};

};

return out;
};

/****************************************************
SPATIAL
******************************************************/

int spat_desc_spiht(pixel_struct pixel, list_struct * list_desc, int directchildonly, long int *image, long int threshold, unsigned char *map_LSC){

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;

list_el *current_el1=NULL;
list_el *current_el2=NULL;
list_el *current_el3=NULL;
list_el *current_el4=NULL;

pixel_struct new_pixel1;
pixel_struct new_pixel2;
pixel_struct new_pixel3;
pixel_struct new_pixel4;

long int value_pix;

int r;
int out=1;

new_pixel1 = init_pixel();
new_pixel2 = init_pixel();
new_pixel3 = init_pixel();
new_pixel4 = init_pixel();


/* Any offspring ?*/
if (pixel.x >= nsmax / 2) return 0;
if (pixel.y >= nlmax / 2) return 0;

/*Lower subband and not offspring*/
if ((pixel.x < nsmin) && (pixel.y < nlmin) && ((pixel.x % 2) == 0) && ((pixel.y % 2) == 0) ){
   if ((pixel.x != nsmin-1)&&(pixel.y != nlmin-1)){
	return 0;
   };
};


if ((pixel.x < nsmin) && (pixel.y < nlmin)){
     if ( ((pixel.x % 2) == 1) && ((pixel.y % 2) == 0) ){  /* 2 */
	new_pixel1.x=pixel.x+nsmin-1;
	new_pixel1.y=pixel.y;
	new_pixel1.l=pixel.l;
	if (directchildonly == 1){current_el1=el_init(new_pixel1);};
	new_pixel2.x=pixel.x+nsmin;
	new_pixel2.y=pixel.y;
	new_pixel2.l=pixel.l;
	if (directchildonly == 1){current_el2=el_init(new_pixel2);};
	new_pixel3.x=pixel.x+nsmin-1;
	new_pixel3.y=pixel.y+1;
	new_pixel3.l=pixel.l;
	if (directchildonly == 1){current_el3=el_init(new_pixel3);};
	new_pixel4.x=pixel.x+nsmin;
	new_pixel4.y=pixel.y+1;
	new_pixel4.l=pixel.l;
	if (directchildonly == 1){current_el4=el_init(new_pixel4);};
     };
     if ( ((pixel.x % 2) == 0) && ((pixel.y % 2) == 1) ){  /* 3 */
	new_pixel1.x=pixel.x;
	new_pixel1.y=pixel.y+nlmin-1;
	new_pixel1.l=pixel.l;
	if (directchildonly == 1){current_el1=el_init(new_pixel1);};
	new_pixel2.x=pixel.x+1;
	new_pixel2.y=pixel.y+nlmin-1;
	new_pixel2.l=pixel.l;
	if (directchildonly == 1){current_el2=el_init(new_pixel2);};
	new_pixel3.x=pixel.x;
	new_pixel3.y=pixel.y+nlmin;
	new_pixel3.l=pixel.l;
	if (directchildonly == 1){current_el3=el_init(new_pixel3);};
	new_pixel4.x=pixel.x+1;
	new_pixel4.y=pixel.y+nlmin;
	new_pixel4.l=pixel.l;
	if (directchildonly == 1){current_el4=el_init(new_pixel4);};     
     };
     if ( ((pixel.x % 2) == 1) && ((pixel.y % 2) == 1) ){  /* 4 */
	new_pixel1.x=pixel.x+nsmin-1;
	new_pixel1.y=pixel.y+nlmin-1;
	new_pixel1.l=pixel.l;
	if (directchildonly == 1){current_el1=el_init(new_pixel1);};
	new_pixel2.x=pixel.x+nsmin;
	new_pixel2.y=pixel.y+nlmin-1;
	new_pixel2.l=pixel.l;
	if (directchildonly == 1){current_el2=el_init(new_pixel2);};
	new_pixel3.x=pixel.x+nsmin-1;
	new_pixel3.y=pixel.y+nlmin;
	new_pixel3.l=pixel.l;
	if (directchildonly == 1){current_el3=el_init(new_pixel3);};
	new_pixel4.x=pixel.x+nsmin;
	new_pixel4.y=pixel.y+nlmin;
	new_pixel4.l=pixel.l;
	if (directchildonly == 1){current_el4=el_init(new_pixel4);}; 
     };

} else {
	new_pixel1.x = 2*pixel.x;
	new_pixel1.y = 2*pixel.y;
	new_pixel1.l = pixel.l;	
	if (directchildonly == 1){current_el1 = el_init(new_pixel1);};
	new_pixel2.x = 2*pixel.x;
	new_pixel2.y = 2*pixel.y+1;
	new_pixel2.l = pixel.l;
	if (directchildonly == 1){current_el2 = el_init(new_pixel2);};
	new_pixel3.x = 2*pixel.x+1;
	new_pixel3.y = 2*pixel.y;
	new_pixel3.l = pixel.l;
	if (directchildonly == 1){current_el3 = el_init(new_pixel3);};
	new_pixel4.x = 2*pixel.x+1;
	new_pixel4.y = 2*pixel.y+1;
	new_pixel4.l = pixel.l;
	if (directchildonly == 1){current_el4 = el_init(new_pixel4);}; 
};


if (directchildonly == 1){
	insert_el(list_desc, current_el1);
	insert_el(list_desc, current_el2);
	insert_el(list_desc, current_el3);
	insert_el(list_desc, current_el4);
}; 

value_pix=image[trans_pixel(new_pixel1)];

#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(new_pixel1)]==0)) {		
#else
	if (isLSC2(abs(value_pix), threshold)) {
#endif
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
value_pix=image[trans_pixel(new_pixel2)];

#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(new_pixel2)]==0)) {	
#else
	if (isLSC2(abs(value_pix), threshold)) {
#endif	
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
value_pix=image[trans_pixel(new_pixel3)];

#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(new_pixel3)]==0)) {	
#else
	if (isLSC2(abs(value_pix), threshold)) {
#endif		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};

value_pix=image[trans_pixel(new_pixel4)];

#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(new_pixel4)]==0)) {
#else
	if (isLSC2(abs(value_pix), threshold)) {
#endif		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};



if (directchildonly != 1) {/*Sinon, on fait un appel recursif pour visiter les points*/ 
	r = spat_desc_spiht(new_pixel1, list_desc, directchildonly, image, threshold, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;
	r = spat_desc_spiht(new_pixel2, list_desc, directchildonly, image, threshold, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;
	r = spat_desc_spiht(new_pixel3, list_desc, directchildonly, image, threshold, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;

	r = spat_desc_spiht(new_pixel4, list_desc, directchildonly, image, threshold, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;//qq test en trop... ==0 supprimables

};

return out;
};


