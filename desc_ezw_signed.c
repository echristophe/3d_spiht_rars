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


int spat_spec_desc_ezw_signed(pixel_struct pixel, list_struct * list_desc, int directchildonly, char *image_signed, int thres_ind , unsigned char *map_LSC)
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

list_el *tmp_el=NULL;


if (directchildonly == 0){
	tmp_list=list_init();
};


#ifdef NEWTREE
if ((pixel.x >= nsmax / 2) || (pixel.y >= nlmax / 2)) {
	if (directchildonly == 0){list_free(tmp_list);};
	return 0;
}
#else
   #ifdef NEWTREE2
if ((pixel.x >= nsmax / 2) || (pixel.y >= nlmax / 2)) {
	if (directchildonly == 0){list_free(tmp_list);};
	return 0;
}
   #else
if (((pixel.x >= nsmax / 2) || (pixel.y >= nlmax / 2)) &&  (pixel.l >= nbmax / 2)) {
	if (directchildonly == 0){list_free(tmp_list);};
	return 0;
}
   #endif
#endif

#ifdef NEWTREE
if ((pixel.x < nsmin) && (pixel.y < nlmin)){
#endif
#ifdef NEWTREE2
if ((pixel.x < nsmax/2) && (pixel.y < nlmax/2)){
#endif 
if (directchildonly == 0){
r1=spec_desc_ezw_signed(pixel, tmp_list, 1, image_signed, thres_ind, map_LSC);
	if (r1 == -1) {
		if (directchildonly == 0){list_free(tmp_list);};
		return -1;
	};
} else {
r1=spec_desc_ezw_signed(pixel, list_desc, directchildonly, image_signed, thres_ind, map_LSC);
};
#ifdef NEWTREE
};
#endif
#ifdef NEWTREE2
};
#endif

if (directchildonly == 1){
tmp_el=list_desc->last;
};


r2=spat_desc_ezw_signed(pixel, list_desc, directchildonly, image_signed, thres_ind, map_LSC);

if ((directchildonly == 0) && ((r1 == -1) || (r2 == -1))) {
	if (directchildonly == 0){list_free(tmp_list);};
	return -1;
};
if ((directchildonly == 1) && ((r1 == -1) || (r2 == -1))) out=-1;
if ((r1 == 0) && (r2 == 0)) {
	if (directchildonly == 0){list_free(tmp_list);};
	return 0;
};

if (directchildonly == 0) {
  current_el=tmp_list->first;
  while (current_el != NULL){
  	r=spat_desc_ezw_signed(current_el->pixel, list_desc, directchildonly, image_signed, thres_ind, map_LSC);
  	current_el= current_el->next;
	if ((directchildonly == 0) && (r == -1)) {return -1; list_free(tmp_list);};
  };
  list_free(tmp_list);
};


if (directchildonly == 1) {
   if(tmp_el !=NULL){
  current_el=list_desc->first;
  while (current_el != tmp_el){
  	r=spat_desc_ezw_signed(current_el->pixel, list_desc, directchildonly, image_signed, thres_ind, map_LSC);
  	current_el= current_el->next;
	if ((directchildonly == 0) && (r == -1)) {return -1; list_free(tmp_list);};
  };
  r=spat_desc_ezw_signed(current_el->pixel, list_desc, directchildonly, image_signed, thres_ind, map_LSC);
  current_el= current_el->next;
  if ((directchildonly == 0) && (r == -1)) {return -1; list_free(tmp_list);};
  list_free(tmp_list);
  };
};

return out;
};

/****************************************************
SPECTRAL
******************************************************/

int spec_desc_ezw_signed(pixel_struct pixel, list_struct * list_desc, int directchildonly, char *image_signed, int thres_ind, unsigned char *map_LSC){

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

       /*;Add offspring coordinate to the array*/
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=pixel.l+nbmin;

	current_el=el_init(new_pixel);	
#ifdef EZWREF
	if((image_signed[NBITS*trans_pixel(new_pixel)+thres_ind] !=0) && (map_LSC[trans_pixel(new_pixel)]==0)) {
#else
	if (image_signed[NBITS*trans_pixel(new_pixel)+thres_ind] !=0){
#endif
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	} 
		insert_el(list_desc, current_el);

	/*;Recursive call on offspring*/
		r = spec_desc_ezw_signed(new_pixel, list_desc, directchildonly, image_signed, thres_ind, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out= -1;
} else { /*;general case*/
	/*;Add offspring coordinate to the array*/
	
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=2*pixel.l;
	current_el1=el_init(new_pixel);		
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=2*pixel.l+1;
	current_el2=el_init(new_pixel);	

#ifdef EZWREF
	if((image_signed[NBITS*trans_pixel(current_el1->pixel)+thres_ind] !=0) && (map_LSC[trans_pixel(current_el1->pixel)]==0)) {
#else
	if (image_signed[NBITS*trans_pixel(current_el1->pixel)+thres_ind] !=0){
#endif
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	} 
		insert_el(list_desc, current_el1);
#ifdef EZWREF
	if((image_signed[NBITS*trans_pixel(current_el2->pixel)+thres_ind] !=0) && (map_LSC[trans_pixel(current_el2->pixel)]==0)) {
#else
	if (image_signed[NBITS*trans_pixel(current_el2->pixel)+thres_ind] !=0){	
#endif
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	}
		insert_el(list_desc, current_el2);
		
	/*;Recursive call on offspring*/
		r = spec_desc_ezw_signed(current_el1->pixel, list_desc, directchildonly, image_signed, thres_ind, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out=-1;
		r = spec_desc_ezw_signed(current_el2->pixel, list_desc, directchildonly, image_signed, thres_ind, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out=-1;
};

return out;
};

/****************************************************
SPATIAL
******************************************************/

int spat_desc_ezw_signed(pixel_struct pixel, list_struct * list_desc, int directchildonly, char *image_signed, int thres_ind, unsigned char *map_LSC){

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

if ((pixel.x < nsmin) && (pixel.y < nlmin)){
	new_pixel1.x=pixel.x+nsmin;
	new_pixel1.y=pixel.y;
	new_pixel1.l=pixel.l;
	if (directchildonly == 1){current_el1=el_init(new_pixel1);};
	new_pixel2.x=pixel.x;
	new_pixel2.y=pixel.y+nlmin;
	new_pixel2.l=pixel.l;
	if (directchildonly == 1){current_el2=el_init(new_pixel2);};
	new_pixel3.x=pixel.x+nsmin;
	new_pixel3.y=pixel.y+nlmin;
	new_pixel3.l=pixel.l;
	if (directchildonly == 1){current_el3=el_init(new_pixel3);};

	if (directchildonly == 1){current_el4=NULL;};

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
	if (!((pixel.x < nsmin) && (pixel.y < nlmin))){
		insert_el(list_desc, current_el4);
	};
}; 

#ifdef EZWREF
	if((image_signed[NBITS*trans_pixel(new_pixel1)+thres_ind] !=0) && (map_LSC[trans_pixel(new_pixel1)]==0)) {
#else
	if (image_signed[NBITS*trans_pixel(new_pixel1)+thres_ind] !=0){
#endif
		if (directchildonly == 0) {return -1;};
		if (directchildonly == 1) {out= -1;};
	};
#ifdef EZWREF
	if((image_signed[NBITS*trans_pixel(new_pixel2)+thres_ind] !=0) && (map_LSC[trans_pixel(new_pixel2)]==0)) {
#else
	if (image_signed[NBITS*trans_pixel(new_pixel2)+thres_ind] !=0){	
#endif
		if (directchildonly == 0) {return -1;};
		if (directchildonly == 1) {out= -1;};
	};
#ifdef EZWREF
	if((image_signed[NBITS*trans_pixel(new_pixel3)+thres_ind] !=0) && (map_LSC[trans_pixel(new_pixel3)]==0)) {
#else
	if (image_signed[NBITS*trans_pixel(new_pixel3)+thres_ind] !=0){	
#endif
		if (directchildonly == 0) {return -1;};
		if (directchildonly == 1) {out= -1;};
	};


if (!((pixel.x < nsmin) && (pixel.y < nlmin))){
#ifdef EZWREF
	if((image_signed[NBITS*trans_pixel(new_pixel4)+thres_ind] !=0) && (map_LSC[trans_pixel(new_pixel4)]==0)) {
#else
	if (image_signed[NBITS*trans_pixel(new_pixel4)+thres_ind] !=0){	
#endif	
		if (directchildonly == 0) {return -1;};
		if (directchildonly == 1) {out= -1;};
	};
};


	r = spat_desc_ezw_signed(new_pixel1, list_desc, directchildonly, image_signed, thres_ind, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;
	r = spat_desc_ezw_signed(new_pixel2, list_desc, directchildonly, image_signed, thres_ind, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;
	r = spat_desc_ezw_signed(new_pixel3, list_desc, directchildonly, image_signed, thres_ind, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;

if (!((pixel.x < nsmin) && (pixel.y < nlmin))){
	r = spat_desc_ezw_signed(new_pixel4, list_desc, directchildonly, image_signed, thres_ind, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;//qq test en trop... ==0 supprimables
};


return out;
};


