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


// #include "spiht_code_c.h"
// #include "desc.h"
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
recherche tous les descendant, mais pas de sortie anticipée
*/


/****************************************************
SPECTRAL - SPATIAL
******************************************************/

/*function spat_spec_desc_spiht_fast, current_pix_x, current_pix_y, current_pix_l, desc_array_x, desc_array_y, desc_array_l, thres_cube, imageprop, directchildonly=directchildonly*/

//int spat_spec_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC)
//{
int spat_spec_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC)
{

int r1=0;
int r2=0;
int r=0;
int out=1;
/*long int threshold;*/

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;
#ifdef NEWTREE
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
#endif
// int nbmin=imageprop.nbmin;

struct list_el *current_el=NULL;
struct list_struct *tmp_list=NULL;

// #ifdef EZW
// struct list_el *tmp_el=NULL;
// #endif
long int threshold= 1 << (long int)thres_ind;


if (directchildonly == 0){
	tmp_list=list_init();
};


/*
;Is there any offspring ?
if (((current_pix_x GE nsmax / 2) || (current_pix_y GE nlmax / 2)) &&  (current_pix_l GE nbmax / 2)) then return, 0*/
if (((pixel.x >= nsmax / 2) || (pixel.y >= nlmax / 2)) &&  (pixel.l >= nbmax / 2)) {
	if (directchildonly == 0){list_free(tmp_list);};
	return 0;
}

/*
;otherwise, general case with 6 offspring

;find spectral offspring
desc_array_spec_x=current_pix_x
desc_array_spec_y=current_pix_y
desc_array_spec_l=current_pix_l*/

/*r=spec_desc_spiht_fast(current_pix_x, current_pix_y, current_pix_l, desc_array_spec_x, desc_array_spec_y, desc_array_spec_l, thres_cube, imageprop, directchildonly=directchildonly)*/
#ifdef NEWTREE
if ((pixel.x < nsmin) && (pixel.y < nlmin)){//modif 02-01-2006 chgmt structure arbre
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

// #ifdef EZW
// if (directchildonly == 1){
// tmp_el=list_desc->last;
// };
// #endif

/*if (r EQ -1) then return, -1*/
/*if (r == -1) return -1;*/

/*
ndesc=n_elements(desc_array_spec_x)

if (ndesc GT 1) then begin
	desc_array_x=[desc_array_x, desc_array_spec_x[1:ndesc-1]]
	desc_array_y=[desc_array_y, desc_array_spec_y[1:ndesc-1]]
	desc_array_l=[desc_array_l, desc_array_spec_l[1:ndesc-1]]
endif*/



/* dans tous les cas, on regarde les descendant spatiaux du pixel courant */
r2=spat_desc_spiht(pixel, list_desc, directchildonly, image, threshold, map_LSC);

if ((directchildonly == 0) && ((r1 == -1) || (r2 == -1))) {
	if (directchildonly == 0){list_free(tmp_list);};
	return -1;//sortie anticipée
};
if ((directchildonly == 1) && ((r1 == -1) || (r2 == -1))) out=-1;
if ((r1 == 0) && (r2 == 0)) {
	if (directchildonly == 0){list_free(tmp_list);};
	return 0;//pas de descendant (LF paires)
};
/*
for i=0, ndesc-1 do begin
   current_pix_spec_x=desc_array_spec_x[i]
   current_pix_spec_y=desc_array_spec_y[i]
   current_pix_spec_l=desc_array_spec_l[i]
   r=spat_desc_spiht_fast(current_pix_spec_x, current_pix_spec_y, current_pix_spec_l, desc_array_x, desc_array_y, desc_array_l, thres_cube, imageprop, directchildonly=directchildonly)
   if (r EQ -1) then return, -1

endfor
*/
if (directchildonly == 0) {
//   current_el=list_desc->first;
  current_el=tmp_list->first;
  while (current_el != NULL){
  	r=spat_desc_spiht(current_el->pixel, list_desc, directchildonly, image, threshold, map_LSC);
  	current_el= current_el->next;
	if ((directchildonly == 0) && (r == -1)) {return -1; list_free(tmp_list);};
// 	if ((directchildonly == 1) && (r == -1)) out= -1;
  };
  list_free(tmp_list);
};


// #ifdef EZW
// if (directchildonly == 1) {
//    if(tmp_el !=NULL){
//   current_el=list_desc->first;
//   while (current_el != tmp_el){
//   	r=spat_desc_spiht(current_el->pixel, list_desc, imageprop, directchildonly, image, thres_ind, map_LSC);
//   	current_el= current_el->next;
// 	if ((directchildonly == 0) && (r == -1)) {return -1; list_free(tmp_list);};
// // 	if ((directchildonly == 1) && (r == -1)) out= -1;
//   };
//   r=spat_desc_spiht(current_el->pixel, list_desc, imageprop, directchildonly, image, thres_ind, map_LSC);
//   current_el= current_el->next;
//   if ((directchildonly == 0) && (r == -1)) {return -1; list_free(tmp_list);};
//   list_free(tmp_list);
//   };
// };
// #endif

return out;
};

/****************************************************
SPECTRAL
******************************************************/
//int spec_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC){
int spec_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, long int threshold, unsigned char *map_LSC){


struct pixel_struct new_pixel;
long int value_pix;
struct list_el * current_el=NULL;

// long int threshold=0; /*WARNING temporaire*/
int r=0;
int out=1;

// int nsmax=imageprop.nsmax;
// int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;
// int nsmin=imageprop.nsmin;
// int nlmin=imageprop.nlmin;
int nbmin=imageprop.nbmin;

struct list_el *current_el1=NULL;
struct list_el *current_el2=NULL;

// printf("Processing pixel : %d, %d, %d \n",pixel.x,pixel.y,pixel.l);

/*if (current_pix_l GE nbmax / 2) then return, 0*/
if (pixel.l >= nbmax / 2) return 0;


if (pixel.l < nbmin){/*lower frequency*/
// #ifndef EZWTREE
   if ((pixel.l %  2) == 0) {
     if (pixel.l != nbmin-1){
        return 0;/*no offspring*/
     } else {
        /*;Add offspring coordinate to the array*/
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=pixel.l+nbmin;
	//TODO check...
// 	if (directchildonly == 1){/* On le rajoute à la liste seuleument si c'est pour la sortie directchild*/
		/*desc_array_x = [desc_array_x,current_pix_x]
		desc_array_y = [desc_array_y,current_pix_y]
		desc_array_l = [desc_array_l,current_pix_l+nbmin]*/
		current_el=el_init(new_pixel);	
		insert_el(list_desc, current_el);
// 	}
	
	/*if (thres_cube[current_pix_x,current_pix_y,current_pix_l+nbmin] EQ 1) then return, -1*/
	value_pix=image[trans_pixel(new_pixel)];
// 	if ((directchildonly == 0) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) return -1;
// 	if ((directchildonly == 1) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) out= -1;
// 	if ((directchildonly == 0) && get_bit(value_pix, thres_ind)) {return -1; };
// 	if ((directchildonly == 1) && get_bit(value_pix, thres_ind)) {out= -1;	};
#ifndef NEWTREE
        if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(new_pixel)]==0)) {
#else
        if (isLSC2(abs(value_pix), threshold)) {
#endif
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
	/*;Recursive call on offspring*/
// 	#ifndef EZW
	if (directchildonly == 0){
// 	#endif
		r = spec_desc_spiht(new_pixel, list_desc, directchildonly, image, threshold, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out= -1;
// 	#ifndef EZW
	};
// 	#endif
     };
   };
   if ((pixel.l % 2) == 1 ){
	/*;Add offspring coordinate to the array
	desc_array_x = [desc_array_x,current_pix_x,current_pix_x]
	desc_array_y = [desc_array_y,current_pix_y,current_pix_y]
	desc_array_l = [desc_array_l,current_pix_l+nbmin-1,current_pix_l+nbmin]
	if (thres_cube[current_pix_x,current_pix_y,current_pix_l+nbmin-1] EQ 1) then return, -1
	if (thres_cube[current_pix_x,current_pix_y,current_pix_l+nbmin] EQ 1) then return, -1*/
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=pixel.l+nbmin-1;
	current_el1=el_init(new_pixel);		
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=pixel.l+nbmin;
	current_el2=el_init(new_pixel);	
	//TODO check...		
// 	if (directchildonly == 1){
		insert_el(list_desc, current_el1);
		insert_el(list_desc, current_el2);
// 	}
	
	value_pix=image[trans_pixel(current_el1->pixel)];
// 	if ((directchildonly == 0) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) return -1;
// 	if ((directchildonly == 1) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) out=-1;
// 	if ((directchildonly == 0) && get_bit(value_pix, thres_ind)) {return -1; };
// 	if ((directchildonly == 1) && get_bit(value_pix, thres_ind)) {out= -1;	};
#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(current_el1->pixel)]==0)) {
#else
	if (isLSC2(abs(value_pix), threshold) ) {
#endif		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};

	value_pix=image[trans_pixel(current_el2->pixel)];
// 	if ((directchildonly == 0) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) return -1;
// 	if ((directchildonly == 1) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) out=-1;
// 	if ((directchildonly == 0) && get_bit(value_pix, thres_ind)) {return -1; };
// 	if ((directchildonly == 1) && get_bit(value_pix, thres_ind)) {out= -1;	};
#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(current_el2->pixel)]==0)) {	
#else
	if (isLSC2(abs(value_pix), threshold) ) {
#endif		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};	
		
	/*;Recursive call on offspring*/
// 	#ifndef EZW
	if (directchildonly != 1){
// 	#endif
		r = spec_desc_spiht(current_el1->pixel, list_desc, directchildonly, image, threshold, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out=-1;
		r = spec_desc_spiht(current_el2->pixel, list_desc, directchildonly, image, threshold, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out=-1;
// 	#ifndef EZW
	};
// 	#endif
   };
// #else
//        /*;Add offspring coordinate to the array*/
// 	new_pixel.x=pixel.x;
// 	new_pixel.y=pixel.y;
// 	new_pixel.l=pixel.l+nbmin;
// 	//TODO check...
// // 	if (directchildonly == 1){/* On le rajoute à la liste seuleument si c'est pour la sortie directchild*/
// 		/*desc_array_x = [desc_array_x,current_pix_x]
// 		desc_array_y = [desc_array_y,current_pix_y]
// 		desc_array_l = [desc_array_l,current_pix_l+nbmin]*/
// 		current_el=el_init(new_pixel);	
// 		insert_el(list_desc, current_el);
// // 	}
// 	
// 	/*if (thres_cube[current_pix_x,current_pix_y,current_pix_l+nbmin] EQ 1) then return, -1*/
// 	value_pix=image[trans_pixel(new_pixel,imageprop)];
// // 	if ((directchildonly == 0) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) return -1;
// // 	if ((directchildonly == 1) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) out= -1;
// // 	if ((directchildonly == 0) && get_bit(value_pix, thres_ind)) {return -1; };
// // 	if ((directchildonly == 1) && get_bit(value_pix, thres_ind)) {out= -1;	};
//         if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(new_pixel,imageprop)]==0)) {
// 		if (directchildonly == 0) {return -1; };
// 		if (directchildonly == 1) {out= -1;	};
// 	};
// 	/*;Recursive call on offspring*/
// 	#ifndef EZW
// 	if (directchildonly == 0){
// 	#endif
// 		r = spec_desc_spiht(new_pixel, list_desc, imageprop, directchildonly, image, thres_ind, map_LSC);
// 		if ((directchildonly == 0) && (r == -1)) return -1;
// 		if ((directchildonly == 1) && (r == -1)) out= -1;
// 	#ifndef EZW
// 	};
// 	#endif
// #endif
} else { /*;general case*/
	/*;Add offspring coordinate to the array*/
	/*desc_array_x = [desc_array_x,current_pix_x,current_pix_x]
	desc_array_y = [desc_array_y,current_pix_y,current_pix_y]
	desc_array_l = [desc_array_l,2*current_pix_l,2*current_pix_l+1]
	if (thres_cube[current_pix_x,current_pix_y,2*current_pix_l] EQ 1) then return, -1
	if (thres_cube[current_pix_x,current_pix_y,2*current_pix_l+1] EQ 1) then return, -1*/
	
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=2*pixel.l;
	current_el1=el_init(new_pixel);		
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=2*pixel.l+1;
	current_el2=el_init(new_pixel);	
	//TODO check...
// 	if (directchildonly == 1){
		insert_el(list_desc, current_el1);
		insert_el(list_desc, current_el2);
// 	};

	value_pix=image[trans_pixel(current_el1->pixel)];
/*	if ((directchildonly == 0) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) return -1;
	if ((directchildonly == 1) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) out=-1;*/	
// 	if ((directchildonly == 0) && get_bit(value_pix, thres_ind)) {return -1; };
// 	if ((directchildonly == 1) && get_bit(value_pix, thres_ind)) {out= -1;	};
#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(current_el1->pixel)]==0)) {
#else
	if (isLSC2(abs(value_pix), threshold)) {
#endif		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
	value_pix=image[trans_pixel(current_el2->pixel)];
// 	if ((directchildonly == 0) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) return -1;
// 	if ((directchildonly == 1) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) out=-1;
// 	if ((directchildonly == 0) && get_bit(value_pix, thres_ind)) {return -1; };
// 	if ((directchildonly == 1) && get_bit(value_pix, thres_ind)) {out= -1;	};
#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(current_el2->pixel)]==0)) {
#else
	if (isLSC2(abs(value_pix), threshold)) {
#endif		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};	
		
	/*;Recursive call on offspring*/
// #ifndef EZW
	if (directchildonly != 1){
// #endif
		r = spec_desc_spiht(current_el1->pixel, list_desc, directchildonly, image, threshold, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out=-1;
		r = spec_desc_spiht(current_el2->pixel, list_desc, directchildonly, image, threshold, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out=-1;
// #ifndef EZW
	};
// #endif

};

return out;
};

/****************************************************
SPATIAL
******************************************************/

//int spat_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC){
int spat_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, long int threshold, unsigned char *map_LSC){

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
// int nbmax=imageprop.nbmax;
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
// int nbmin=imageprop.nbmin;

struct list_el *current_el1=NULL;
struct list_el *current_el2=NULL;
struct list_el *current_el3=NULL;
struct list_el *current_el4=NULL;

struct pixel_struct new_pixel1;
struct pixel_struct new_pixel2;
struct pixel_struct new_pixel3;
struct pixel_struct new_pixel4;

// long int threshold=0; /*WARNING temporaire*/
long int value_pix;

int r;
int out=1;

// printf("Processing pixel : %d, %d, %d \n",pixel.x,pixel.y,pixel.l);

/* Any offspring ?*/
if (pixel.x >= nsmax / 2) return 0;
if (pixel.y >= nlmax / 2) return 0;

/*Lower subband and not offspring*/
// #ifndef EZWTREE
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
// #else
// if ((pixel.x < nsmin) && (pixel.y < nlmin)){
// 	new_pixel1.x=pixel.x+nsmin;
// 	new_pixel1.y=pixel.y;
// 	new_pixel1.l=pixel.l;
// 	if (directchildonly == 1){current_el1=el_init(new_pixel1);};
// 	new_pixel2.x=pixel.x;
// 	new_pixel2.y=pixel.y+nlmin;
// 	new_pixel2.l=pixel.l;
// 	if (directchildonly == 1){current_el2=el_init(new_pixel2);};
// 	new_pixel3.x=pixel.x+nsmin;
// 	new_pixel3.y=pixel.y+nlmin;
// 	new_pixel3.l=pixel.l;
// 	if (directchildonly == 1){current_el3=el_init(new_pixel3);};
// 
// 	if (directchildonly == 1){current_el4=NULL;};
// #endif
} else {
// 	printf("Standard case:\n");
	new_pixel1.x = 2*pixel.x;
	new_pixel1.y = 2*pixel.y;
	new_pixel1.l = pixel.l;	
	if (directchildonly == 1){current_el1 = el_init(new_pixel1);};
// 	printf("new_pixel : %d, %d, %d \n",new_pixel.x,new_pixel.y,new_pixel.l);
	new_pixel2.x = 2*pixel.x;
	new_pixel2.y = 2*pixel.y+1;
	new_pixel2.l = pixel.l;
	if (directchildonly == 1){current_el2 = el_init(new_pixel2);};
// 	printf("new_pixel : %d, %d, %d \n",new_pixel.x,new_pixel.y,new_pixel.l);
	new_pixel3.x = 2*pixel.x+1;
	new_pixel3.y = 2*pixel.y;
	new_pixel3.l = pixel.l;
	if (directchildonly == 1){current_el3 = el_init(new_pixel3);};
// 	printf("new_pixel : %d, %d, %d \n",new_pixel.x,new_pixel.y,new_pixel.l);
	new_pixel4.x = 2*pixel.x+1;
	new_pixel4.y = 2*pixel.y+1;
	new_pixel4.l = pixel.l;
	if (directchildonly == 1){current_el4 = el_init(new_pixel4);}; 
// 	printf("new_pixel : %d, %d, %d \n",new_pixel.x,new_pixel.y,new_pixel.l);
// // 	printf("--------------\n");
};


if (directchildonly == 1){/*Si on ne veut que les descendant, il faut les ajouter à la liste...*/
	insert_el(list_desc, current_el1);
	insert_el(list_desc, current_el2);
	insert_el(list_desc, current_el3);
// #ifdef EZWTREE
// 	if (!((pixel.x < nsmin) && (pixel.y < nlmin))){
// #endif
	insert_el(list_desc, current_el4);
// #ifdef EZWTREE
// 	};
// #endif
}; 

value_pix=image[trans_pixel(new_pixel1)];
// 	if ((directchildonly == 0) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) return -1;
// 	if ((directchildonly == 1) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) out=-1;
// 	if ((directchildonly == 0) && get_bit(value_pix, thres_ind)) {return -1; };
// 	if ((directchildonly == 1) && get_bit(value_pix, thres_ind)) {out= -1;	};
#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(new_pixel1)]==0)) {		
#else
	if (isLSC2(abs(value_pix), threshold)) {
#endif
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
value_pix=image[trans_pixel(new_pixel2)];
// 	if ((directchildonly == 0) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) return -1;
// 	if ((directchildonly == 1) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) out=-1;
// 	if ((directchildonly == 0) && get_bit(value_pix, thres_ind)) {return -1; };
// 	if ((directchildonly == 1) && get_bit(value_pix, thres_ind)) {out= -1;	};
#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(new_pixel2)]==0)) {	
#else
	if (isLSC2(abs(value_pix), threshold)) {
#endif	
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
value_pix=image[trans_pixel(new_pixel3)];
// 	if ((directchildonly == 0) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) return -1;
// 	if ((directchildonly == 1) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) out=-1;
// 	if ((directchildonly == 0) && get_bit(value_pix, thres_ind)) {return -1; };
// 	if ((directchildonly == 1) && get_bit(value_pix, thres_ind)) {out= -1;	};
#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(new_pixel3)]==0)) {	
#else
	if (isLSC2(abs(value_pix), threshold)) {
#endif		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
// #ifdef EZWTREE
// if (!((pixel.x < nsmin) && (pixel.y < nlmin))){
// #endif
value_pix=image[trans_pixel(new_pixel4)];
// 	if ((directchildonly == 0) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) return -1;
// 	if ((directchildonly == 1) && ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold))) out=-1;
// 	if ((directchildonly == 0) && get_bit(value_pix, thres_ind)) {return -1; };
// 	if ((directchildonly == 1) && get_bit(value_pix, thres_ind)) {out= -1;	};
#ifndef NEWTREE
	if (isLSC2(abs(value_pix), threshold) && (map_LSC[trans_pixel(new_pixel4)]==0)) {
#else
	if (isLSC2(abs(value_pix), threshold)) {
#endif		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
// #ifdef EZWTREE
// };
// #endif

//dans le cas EZW on fait l'appel recursif dans le cas directchildonly=1 aussi.
// #ifndef EZW	
if (directchildonly != 1) {/*Sinon, on fait un appel recursif pour visiter les points*/ 
// #endif
	r = spat_desc_spiht(new_pixel1, list_desc, directchildonly, image, threshold, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;
	r = spat_desc_spiht(new_pixel2, list_desc, directchildonly, image, threshold, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;
	r = spat_desc_spiht(new_pixel3, list_desc, directchildonly, image, threshold, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;
// #ifdef EZWTREE
// if (!((pixel.x < nsmin) && (pixel.y < nlmin))){
// #endif
	r = spat_desc_spiht(new_pixel4, list_desc, directchildonly, image, threshold, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;//qq test en trop... ==0 supprimables
// #ifdef EZWTREE
// };
// #endif
// #ifndef EZW
};
// #endif


return out;
};


