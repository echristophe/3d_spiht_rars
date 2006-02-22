// #include "spiht_code_c.h"
// #include "desc.h"
#include "main.h"



/****************************************************
SPECTRAL - SPATIAL
******************************************************/

int spat_spec_desc_spiht_cumul(struct pixel_struct pixel, struct imageprop_struct imageprop, long int *image, int thres_ind, long long int * dist)
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
long long int value_err=0;
int directchildonly = 0;

struct list_el *current_el=NULL;
struct list_struct *tmp_list=NULL;
struct list_struct *tmp_list2=NULL;

if (directchildonly == 0){
	tmp_list=list_init();
};
tmp_list2=list_init();

/*
;Is there any offspring ?
*/
if (((pixel.x >= nsmax / 2) || (pixel.y >= nlmax / 2)) &&  (pixel.l >= nbmax / 2)) {
	if (directchildonly == 0){list_free(tmp_list);};
	value_err = abs(image[trans_pixel(pixel,imageprop)]);
	value_err = value_err - ((value_err >> thres_ind)<<thres_ind);
	*dist += value_err * value_err;
	return 0;
}


//otherwise, general case with 6 offspring

//find spectral offspring

#ifdef NEWTREE
if ((pixel.x < nsmin) && (pixel.y < nlmin)){//modif 02-01-2006 chgmt structure arbre
#endif

r1=spec_desc_spiht_cumul(pixel, tmp_list, imageprop, directchildonly, image, thres_ind, dist);

#ifdef NEWTREE
};
#endif


/* dans tous les cas, on regarde les descendant spatiaux du pixel courant */
r2=spat_desc_spiht_cumul(pixel, tmp_list2, imageprop, directchildonly, image, thres_ind, dist);

if ((r1 == 0) && (r2 == 0)) {
	if (directchildonly == 0){list_free(tmp_list);};
	value_err = abs(image[trans_pixel(pixel,imageprop)]);
	value_err = value_err - ((value_err >> thres_ind)<<thres_ind);
	*dist += value_err * value_err;
	return 0;//pas de descendant (LF paires)
};


  current_el=tmp_list->first;
  while (current_el != NULL){
  	r=spat_desc_spiht_cumul(current_el->pixel, tmp_list2, imageprop, directchildonly, image, thres_ind, dist);
  	current_el= current_el->next;
  };
  list_free(tmp_list);
  list_free(tmp_list2);

value_err = abs(image[trans_pixel(pixel,imageprop)]);
value_err = value_err - ((value_err >> thres_ind)<<thres_ind);
*dist += value_err * value_err;

return out;
};

/****************************************************
SPECTRAL
******************************************************/

int spec_desc_spiht_cumul(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, long long int * dist){

struct pixel_struct new_pixel;
long int value_pix;
struct list_el * current_el=NULL;

// long int threshold=0; /*WARNING temporaire*/
int r=0;
int out=1;
long long int value_err=0;

int nbmax=imageprop.nbmax;

int nbmin=imageprop.nbmin;

struct list_el *current_el1=NULL;
struct list_el *current_el2=NULL;

// printf("Processing pixel : %d, %d, %d \n",pixel.x,pixel.y,pixel.l);

/*if (current_pix_l GE nbmax / 2) then return, 0*/
if (pixel.l >= nbmax / 2) {
return 0;
}

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
	
// 	value_pix=image[trans_pixel(new_pixel,imageprop)];
//         if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(new_pixel,imageprop)]==0)) {
// 		if (directchildonly == 0) {return -1; };
// 		if (directchildonly == 1) {out= -1;	};
// 	};
	value_err = abs(image[trans_pixel(new_pixel,imageprop)]);
	value_err = value_err - ((value_err >> thres_ind)<<thres_ind);
	*dist += value_err * value_err;

	/*;Recursive call on offspring*/
	r = spec_desc_spiht_cumul(new_pixel, list_desc, imageprop, directchildonly, image, thres_ind, dist);


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
	
// 	value_pix=image[trans_pixel(current_el1->pixel,imageprop)];
// 
// 	if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(current_el1->pixel,imageprop)]==0)) {		
// 		if (directchildonly == 0) {return -1; };
// 		if (directchildonly == 1) {out= -1;	};
// 	};
	value_err = abs(image[trans_pixel(current_el1->pixel,imageprop)]);
	value_err = value_err - ((value_err >> thres_ind)<<thres_ind);
	*dist += value_err * value_err;

/*	value_pix=image[trans_pixel(current_el2->pixel,imageprop)];

	if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(current_el2->pixel,imageprop)]==0)) {		
		if (directchildonly == 0) {return -1; };
		if (directchildonly == 1) {out= -1;	};
	};*/	
	value_err = abs(image[trans_pixel(current_el2->pixel,imageprop)]);
	value_err = value_err - ((value_err >> thres_ind)<<thres_ind);
	*dist += value_err * value_err;	


		r = spec_desc_spiht_cumul(current_el1->pixel, list_desc, imageprop, directchildonly, image, thres_ind, dist);
		r = spec_desc_spiht_cumul(current_el2->pixel, list_desc, imageprop, directchildonly, image, thres_ind, dist);
   };

} else { /*;general case*/
	
	
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


// 	value_pix=image[trans_pixel(current_el1->pixel,imageprop)];
// 
// 	if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(current_el1->pixel,imageprop)]==0)) {		
// 		if (directchildonly == 0) {return -1; };
// 		if (directchildonly == 1) {out= -1;	};
// 	};
	value_err = abs(image[trans_pixel(current_el1->pixel,imageprop)]);
	value_err = value_err - ((value_err >> thres_ind)<<thres_ind);
	*dist += value_err * value_err;

// 	value_pix=image[trans_pixel(current_el2->pixel,imageprop)];
// 
// 	if (isLSC(value_pix, thres_ind) && (map_LSC[trans_pixel(current_el2->pixel,imageprop)]==0)) {		
// 		if (directchildonly == 0) {return -1; };
// 		if (directchildonly == 1) {out= -1;	};
// 	};
	value_err = abs(image[trans_pixel(current_el2->pixel,imageprop)]);
	value_err = value_err - ((value_err >> thres_ind)<<thres_ind);
	*dist += value_err * value_err;
		
	/*;Recursive call on offspring*/
		r = spec_desc_spiht_cumul(current_el1->pixel, list_desc, imageprop, directchildonly, image, thres_ind, dist);
		r = spec_desc_spiht_cumul(current_el2->pixel, list_desc, imageprop, directchildonly, image, thres_ind, dist);

};

return out;
};

/****************************************************
SPATIAL
******************************************************/

int spat_desc_spiht_cumul(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, long long int * dist){

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
// int nbmax=imageprop.nbmax;
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
// int nbmin=imageprop.nbmin;
long long int value_err=0;
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
	insert_el(list_desc, current_el4);

}; 



	value_err = abs(image[trans_pixel(new_pixel1,imageprop)]);
	value_err = value_err - ((value_err >> thres_ind)<<thres_ind);
	*dist += value_err * value_err;
	value_err = abs(image[trans_pixel(new_pixel2,imageprop)]);
	value_err = value_err - ((value_err >> thres_ind)<<thres_ind);
	*dist += value_err * value_err;
	value_err = abs(image[trans_pixel(new_pixel3,imageprop)]);
	value_err = value_err - ((value_err >> thres_ind)<<thres_ind);
	*dist += value_err * value_err;
	value_err = abs(image[trans_pixel(new_pixel4,imageprop)]);
	value_err = value_err - ((value_err >> thres_ind)<<thres_ind);
	*dist += value_err * value_err;


	r = spat_desc_spiht_cumul(new_pixel1, list_desc, imageprop, directchildonly, image, thres_ind, dist);
	r = spat_desc_spiht_cumul(new_pixel2, list_desc, imageprop, directchildonly, image, thres_ind, dist);
	r = spat_desc_spiht_cumul(new_pixel3, list_desc, imageprop, directchildonly, image, thres_ind, dist);
	r = spat_desc_spiht_cumul(new_pixel4, list_desc, imageprop, directchildonly, image, thres_ind, dist);



return out;
};


