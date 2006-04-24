// #include "spiht_code_c.h"
// #include "desc_ezw.h"
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

int spat_spec_desc_ezw(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC)
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
struct list_el *tmp_el=NULL;
// #endif


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
// r1=spec_desc_ezw(pixel, tmp_list, directchildonly, image, thres_ind, map_LSC);
r1=spec_desc_ezw(pixel, tmp_list, 1, image, thres_ind, map_LSC);
	if (r1 == -1) {
		if (directchildonly == 0){list_free(tmp_list);};
		return -1;
	};
} else {
r1=spec_desc_ezw(pixel, list_desc, directchildonly, image, thres_ind, map_LSC);
};
#ifdef NEWTREE
};
#endif

// #ifdef EZW
if (directchildonly == 1){
tmp_el=list_desc->last;
};
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
r2=spat_desc_ezw(pixel, list_desc, directchildonly, image, thres_ind, map_LSC);

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
  	r=spat_desc_ezw(current_el->pixel, list_desc, directchildonly, image, thres_ind, map_LSC);
  	current_el= current_el->next;
	if ((directchildonly == 0) && (r == -1)) {return -1; list_free(tmp_list);};
// 	if ((directchildonly == 1) && (r == -1)) out= -1;
  };
  list_free(tmp_list);
};


// #ifdef EZW
if (directchildonly == 1) {
   if(tmp_el !=NULL){
  current_el=list_desc->first;
  while (current_el != tmp_el){
  	r=spat_desc_ezw(current_el->pixel, list_desc, directchildonly, image, thres_ind, map_LSC);
  	current_el= current_el->next;
	if ((directchildonly == 0) && (r == -1)) {return -1; list_free(tmp_list);};
// 	if ((directchildonly == 1) && (r == -1)) out= -1;
  };
  r=spat_desc_ezw(current_el->pixel, list_desc, directchildonly, image, thres_ind, map_LSC);
  current_el= current_el->next;
  if ((directchildonly == 0) && (r == -1)) {return -1; list_free(tmp_list);};
  list_free(tmp_list);
  };
};
// #endif

return out;
};

/****************************************************
SPECTRAL
******************************************************/

int spec_desc_ezw(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC){

struct pixel_struct new_pixel;
struct pixel_struct new_pixel1;
struct pixel_struct new_pixel2;
long int value_pix;
struct list_el * current_el=NULL;

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


if (pixel.l >= nbmax / 2) return 0;


if (pixel.l < nbmin){/*lower frequency*/

       /*;Add offspring coordinate to the array*/
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=pixel.l+nbmin;
	if (directchildonly == 1){//chgt 09-04-06
	current_el=el_init(new_pixel);
	insert_el(list_desc, current_el);
	}
	
	value_pix=image[trans_pixel(new_pixel)];
        if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(new_pixel)]==0)) {
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
// 	if (directchildonly == 0){//chgt 05-04-06
		r = spec_desc_ezw(new_pixel, list_desc, directchildonly, image, thres_ind, map_LSC);
		if ((directchildonly == 0) && (r == -1)) return -1;
		if ((directchildonly == 1) && (r == -1)) out= -1;
// 	};//chgt 05-04-06
} else { /*;general case*/
	
	new_pixel.x=pixel.x;
	new_pixel.y=pixel.y;
	new_pixel.l=2*pixel.l;
	
	new_pixel2.x=pixel.x;
	new_pixel2.y=pixel.y;
	new_pixel2.l=2*pixel.l+1;

if (directchildonly == 1){//chgt 09-04-06	
	current_el1=el_init(new_pixel);	
	current_el2=el_init(new_pixel2);	
	insert_el(list_desc, current_el1);
	insert_el(list_desc, current_el2);
	}

	value_pix=image[trans_pixel(new_pixel)];
	if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(new_pixel)]==0)) {		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
	value_pix=image[trans_pixel(new_pixel2)];
	if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(new_pixel2)]==0)) {		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};	
		
	r = spec_desc_ezw(new_pixel, list_desc, directchildonly, image, thres_ind, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;
	if ((directchildonly == 1) && (r == -1)) out=-1;
	r = spec_desc_ezw(new_pixel2, list_desc, directchildonly, image, thres_ind, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;
	if ((directchildonly == 1) && (r == -1)) out=-1;

};

//on en fait un nouveau plus simple
// if (pixel.l >= nbmax / 2) return 0;
// 
// if (pixel.l < nbmin){
// 	new_pixel1.x=pixel.x;
// 	new_pixel1.y=pixel.y;
// 	new_pixel1.l=pixel.l+nbmin;
// 	if (directchildonly == 1){current_el1=el_init(new_pixel1);};
// 
// } else {
// 	new_pixel1.x=pixel.x;
// 	new_pixel1.y=pixel.y;
// 	new_pixel1.l=2*pixel.l;
// 	if (directchildonly == 1){current_el1=el_init(new_pixel1);}	
// 	new_pixel2.x=pixel.x;
// 	new_pixel2.y=pixel.y;
// 	new_pixel2.l=2*pixel.l+1;
// 	if (directchildonly == 1){current_el2=el_init(new_pixel2);}
// }
// 
// if (directchildonly == 1){/*Si on ne veut que les descendant, il faut les ajouter à la liste...*/
// 	insert_el(list_desc, current_el1);
// 	if (!(pixel.l < nbmin)){
// 		insert_el(list_desc, current_el2);
// 	}
// }
// 
// value_pix=image[trans_pixel(new_pixel1)];
// 	if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(new_pixel1)]==0)) {		
// 		if (directchildonly == 0) {return -1; };
// 		if (directchildonly == 1) {out= -1;	};
// 	};
// if (!(pixel.l < nbmin)){
// value_pix=image[trans_pixel(new_pixel2)];
// 	if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(new_pixel2)]==0)) {		
// 		if (directchildonly == 0) {return -1; };
// 		if (directchildonly == 1) {out= -1;	};
// 	};
// 
// }
// 
// 	r = spec_desc_ezw(new_pixel1, list_desc, directchildonly, image, thres_ind, map_LSC);
// 	if ((directchildonly == 0) && (r == -1)) return -1;
// if (!(pixel.l < nbmin)){
// 	r = spec_desc_ezw(new_pixel2, list_desc, directchildonly, image, thres_ind, map_LSC);
// 	if ((directchildonly == 0) && (r == -1)) return -1;
// }

return out;

};


/****************************************************
SPATIAL
******************************************************/

int spat_desc_ezw(struct pixel_struct pixel, struct list_struct * list_desc, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC){

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

long int value_pix;

int r;
int out=1;


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


if (directchildonly == 1){/*Si on ne veut que les descendant, il faut les ajouter à la liste...*/
	insert_el(list_desc, current_el1);
	insert_el(list_desc, current_el2);
	insert_el(list_desc, current_el3);
	if (!((pixel.x < nsmin) && (pixel.y < nlmin))){
		insert_el(list_desc, current_el4);
	};
}; 

value_pix=image[trans_pixel(new_pixel1)];

	if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(new_pixel1)]==0)) {		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
value_pix=image[trans_pixel(new_pixel2)];

	if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(new_pixel2)]==0)) {		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
value_pix=image[trans_pixel(new_pixel3)];

	if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(new_pixel3)]==0)) {		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
if (!((pixel.x < nsmin) && (pixel.y < nlmin))){
value_pix=image[trans_pixel(new_pixel4)];

	if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(new_pixel4)]==0)) {		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};
};


	r = spat_desc_ezw(new_pixel1, list_desc, directchildonly, image, thres_ind, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;
	r = spat_desc_ezw(new_pixel2, list_desc, directchildonly, image, thres_ind, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;
	r = spat_desc_ezw(new_pixel3, list_desc, directchildonly, image, thres_ind, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;
if (!((pixel.x < nsmin) && (pixel.y < nlmin))){
	r = spat_desc_ezw(new_pixel4, list_desc, directchildonly, image, thres_ind, map_LSC);
	if ((directchildonly == 0) && (r == -1)) return -1;//qq test en trop... ==0 supprimables
};


return out;
};


