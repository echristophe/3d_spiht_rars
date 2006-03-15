/*
 *
 *  Hyperspectral compression program
 *
 * Name:		spiht_code_ra.c	
 * Author:		Emmanuel Christophe	
 * Contact:		e.christophe at melaneum.com
 * Description:		Random access based coding and decoding for hyperspectral image
 * Version:		v1.0 - 2006-02	
 * 
 */


#include "main.h"

//number of resolutions

// #define NRES 1

int spiht_code_ra2(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue)
{

// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, 8, 8, 7};
struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
int nbmin=imageprop.nbmin;

// struct list_struct * LSC=NULL;
// struct list_struct * LIC=NULL;
// struct list_struct * LIS=NULL;
struct list_struct * LSC[NRES];
struct list_struct * LIC[NRES];
struct list_struct * LIS[NRES];
// struct list_struct * LSC[NRES+1];
// struct list_struct * LIC[NRES+1];
// struct list_struct * LIS[NRES+1];
struct pixel_struct pixel;
// struct parents_struct parents;
// int is_accessible=0;
// int maxquant=imageprop.maxquant;
// int maxquant=MAXQUANT_CONST;
int maxquant= (int) (*maxquantvalue);

int minquant=4;

struct list_el * el=NULL;
struct list_el * current_el=NULL;
struct list_el * tmp_el=NULL;
struct list_el * current_child=NULL;

// struct list_el * lastLIC=NULL;
// struct list_el * lastLSC=NULL;
// struct list_el * lastLIS=NULL;
// struct list_el * lastLSC[NRES];
// struct list_el * lastLIC[NRES];
// struct list_el * lastLIS[NRES];
// struct list_el * lastLSCm[NRES][MAXQUANT_CONST+1];
// struct list_el * lastLICm[NRES][MAXQUANT_CONST+1];
// struct list_el * lastLISm[NRES][MAXQUANT_CONST+1];

struct list_struct * list_desc=NULL;
// struct list_struct * tmp_list=NULL;
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
int iloc,jloc,kloc;
int niloc, njloc, nkloc;
int ii,ji,ki;
int ie,je,ke;
// long int i_l;
int blockind;
int nblock;

long int npix=nsmax*nbmax*nlmax;

int res=0;
// int restmp=0;
int islastloop=0;

// char * unused[64];

//debit-distortion
long long int dist=0;
// long long int distup=0;
// struct rddata_struct rddata;
float alpha, beta, rate_i, rate_e;
struct datablock_struct * datablock;
float lambda=0.0;

int maxres=NRES;
// int maxres=2;

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

map_LSC=(unsigned char *) calloc(nsmax*nbmax*nlmax,sizeof(unsigned char));
map_LIC= (unsigned char *) calloc(nsmax*nbmax*nlmax,sizeof(unsigned char));
map_LIS= (unsigned char *) calloc(nsmax*nbmax*nlmax,sizeof(unsigned char));

*count=0;
*streamlast=0;

for (i=0; i<NRES; i++){
LSC[i] = list_init();
LIC[i] = list_init();
LIS[i] = list_init();
}

//temp
// LSC[1] = LSC[0];
// LIC[1] = LIC[0];
// LIS[1] = LIS[0];

// for (i=0; i<NRES; i++){
// for (j=0; j<MAXQUANT_CONST+1; j++){
// 	lastLSCm[i][j] = NULL;
// 	lastLICm[i][j] = NULL;
// 	lastLISm[i][j] = NULL;
// }
// }

// list_desc=list_init();
// tmp_list=list_init();

//This is just to get NUMRD point with exp spacing between rate_i and rate_e
rate_i = 200.;
rate_e = 3000000.;
alpha = expf(1./NUMRD * logf(rate_e/rate_i));
beta = NUMRD * logf(rate_i) / logf(rate_e/rate_i);

for (i=0;i<MAXQUANT_CONST-1;i++){
   outputsize[i]=0;
}

// for (i_l=0;i_l<nsmax*nlmax*nbmax;i_l++)
// {
// 	map_LSC[i_l]=0;
// 	map_LIC[i_l]=0;
// 	map_LIS[i_l]=0;
// }

niloc=(nsmin+1)/2;
njloc=(nlmin+1)/2;
nkloc=(nbmin+1)/2;
nblock=niloc* njloc* nkloc;

datablock=(struct datablock_struct *) malloc(nblock* sizeof(struct datablock_struct));



//**********************************************
//Parcours des differentes localisations
//**********************************************

// block initialization
for (kloc=0;kloc<nkloc;kloc++){
   for (jloc=0;jloc<njloc;jloc++){
      for (iloc=0;iloc<niloc;iloc++){
	blockind = iloc + jloc*niloc + kloc * niloc *njloc;
	datablock_init(&(datablock[blockind]));
	datablock[blockind].rddata.ptcourant=0;
	for (i=0;i<NUMRD;i++){
		((datablock[blockind]).rddata.reval)[i]= (long long int) powf(alpha, beta+i);
		((datablock[blockind]).rddata.r)[i]=0; //faster to use separate loops for memory access ?
		((datablock[blockind]).rddata.d)[i]=0;
	}	
      }
   }
}


// nkloc=1;njloc=1;//TEMP !!!!


for (kloc=0;kloc<nkloc;kloc++){
   for (jloc=0;jloc<njloc;jloc++){
      for (iloc=0;iloc<niloc;iloc++){

#ifdef DEBUG2
printf("Coding grp: %d %d %d\n",iloc, jloc, kloc);
#endif

blockind = iloc + jloc*niloc + kloc * niloc *njloc;

//SPIHT 1)
// printf("Initialization...\n");
ki=2*kloc;
ji=2*jloc;
ii=2*iloc;
ke= (2*kloc+2 < nbmin ? 2*kloc+2 : nbmin);
je= (2*jloc+2 < nlmin ? 2*jloc+2 : nlmin);
ie= (2*iloc+2 < nsmin ? 2*iloc+2 : nsmin);

for (k=ki;k<ke;k++){
   for (j=ji;j<je;j++){
      for (i=ii;i<ie;i++){
//       for (j=0;j<nlmin;j++){
//       for (k=0;k<nbmin;k++){
	pixel.x=i;
	pixel.y=j;
	pixel.l=k;
	el=el_init(pixel);
	el->type=maxquant+1;
	insert_el(LIC[0],el);
	(map_LIC[trans_pixel(pixel, imageprop)])++;
	if ((i % 2) || (j % 2) || (k % 2) || (((nbmin % 2) == 1) && (k == nbmin-1) && (k!=0))){ 
		el=el_init(pixel);
		el->type=maxquant+1;
		insert_el(LIS[0],el);
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

// #ifdef DEBUG
// if (check_accessibility_of_all(imageprop, map_LSC, map_LIC, map_LIS) == 0) {
//     printf("Probleme d'accessibilite");
// }
// #endif

//the distance is processed once for each block then updated for tracking
dist = eval_dist_grp(iloc, jloc, kloc, image, imageprop, maxquant+1);

// for (res=0; res<NRES; res++){
// lastLICm[res][maxquant]=NULL;
// lastLSCm[res][maxquant]=NULL;
// }
// for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){
// lastLICm[0][thres_ind]=LIC[0]->last;
// lastLSCm[0][thres_ind]=LSC[0]->last;
// lastLISm[0][thres_ind]=LIS[0]->last;
// }


// //SPIHT 2)
// for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){
// 
// //remember what was the last one (only in case a of p25
// for (restmp=0; restmp<NRES; restmp++){
// lastLSC[restmp] = LSC[restmp]->last;
// lastLIC[restmp] = LIC[restmp]->last;
// }
// 
// for (res=0; res<NRES; res++){//possible to put it later...

// lastLIC=LIC[0]->last;

//SPIHT 2)
#ifdef RES_SCAL
for (res=0; res<maxres; res++){//possible to put it later...
#endif

for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){

#ifndef RES_SCAL
for (res=0; res<NRES; res++){//possible to put it later...
#endif

// lastLIC=lastLICm[thres_ind];
// lastLSC=lastLSCm[thres_ind];
// lastLIS=lastLISm[thres_ind];
// if (thres_ind == maxquant){
// LSC[res]->current = NULL; //inplace understand that as putting the element at the front
// } else {
// LSC[res]->current = lastLSCm[res][thres_ind+1];
// }

threshold= 1 << (long int)thres_ind;

#ifdef DEBUG
printf("Processing for thres_ind %d (threshold: %ld) at resolution %d...\n",thres_ind, threshold, res);
// printf("Sorting pass for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
#endif

#ifdef DEBUG
nLSCloop=0;
nLICloop=0;
nLISloop=0;
nLISloopA=0;
nLISloopB=0;
#endif
//debit distortion
// printf("Dist (blk %d %d %d) t=%d, res=%d: %lld\n",iloc, jloc, kloc, thres_ind, res, dist);
// dist = eval_dist_grp(iloc, jloc, kloc, image, imageprop, thres_ind+1);
// printf("Dist after eval (direct processing): %lld\n",dist);

// Used for Significant pass
// lastLSC = LSC[res]->last;

current_el=first_el(LSC[res]); //right positionning for adding from LIC
while ((current_el != NULL) && (current_el->type > thres_ind)){
   	current_el=next_el(LSC[res]);
}
if ((current_el != NULL) && (current_el->type <= thres_ind)) {
LSC[res]->current = LSC[res]->previous;
}
if (current_el == NULL){
//si on arrive la, ca veut dire que soit la liste est vide soit on est alle jusqu'a la fin
LSC[res]->current = LSC[res]->last;
}


// ;for each entry in the LIC
//SPIHT 2.1)
current_el=first_el(LIC[res]);
// printf("LIC processing \n");
// if (lastLIC[res] != NULL){//Attention à la premiere boucle quand c'est encore vide
// while (current_el != lastLIC[res]){

// if (lastLICm[res][thres_ind] != NULL){//Attention à la premiere boucle quand c'est encore vide
// while (current_el != lastLICm[res][thres_ind]){
while ((current_el != NULL) && (current_el->type > thres_ind)){
// printf(".");
#ifdef DEBUG
   nLICloop++;
#endif
   value_pix=image[trans_pixel(current_el->pixel,imageprop)];
//    bit = ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold));//TODO possibilite d'enlever un abs
   bit = get_bit(value_pix, thres_ind);	
   add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));//SPIHT 2.1.1)
  if (bit == 1) { //SPIHT 2.1.2)
     bitsig = (value_pix > 0);
     add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bitsig, (datablock[blockind].streamlast));
     update_dist(current_el->pixel, thres_ind, &dist, image, imageprop);
     add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
     (map_LIC[trans_pixel(current_el->pixel, imageprop)])--;
     (map_LSC[trans_pixel(current_el->pixel, imageprop)])++;
     remove_current_el(LIC[res]);
//      insert_el(LSC[res], current_el);
     insert_el_inplace(LSC[res], current_el);
	if (LSC[res]->current == NULL){
		LSC[res]->current=LSC[res]->first;
	}else {
		next_el(LSC[res]);
	}
	current_el->type=thres_ind;
     current_el = LIC[res]->current;//est passe au suivant dans le move
     
   } else {
      current_el=next_el(LIC[res]);
   };
}
// while (current_el != NULL){
// printf(".");
// #ifdef DEBUG
//    nLICloop++;
// #endif
//    value_pix=image[trans_pixel(current_el->pixel,imageprop)];
// //    bit = ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold));//TODO possibilite d'enlever un abs
//    bit = get_bit(value_pix, thres_ind);	
//    add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));//SPIHT 2.1.1)
//   if (bit == 1) { //SPIHT 2.1.2)
//      bitsig = (value_pix > 0);
//      add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bitsig, (datablock[blockind].streamlast));
//      update_dist(current_el->pixel, thres_ind, &dist, image, imageprop);
//      add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
//      (map_LIC[trans_pixel(current_el->pixel, imageprop)])--;
//      (map_LSC[trans_pixel(current_el->pixel, imageprop)])++;
//      remove_current_el(LIC[res]);
// //      insert_el(LSC[res], current_el);
//      insert_el_inplace(LSC[res], current_el);
//      if (LSC[res]->current == NULL){
// 		LSC[res]->current=LSC[res]->first;
// 	}else {
// 		next_el(LSC[res]);
// 	}
// 	current_el->type=thres_ind;
//      current_el = LIC[res]->current;//est passe au suivant dans le move
//      
//    } else {
//       current_el=next_el(LIC[res]);
//    };
// }

// if (thres_ind !=0){
// 	if (lastLSCm[res][thres_ind-1] == NULL) {
// 		lastLSCm[res][thres_ind-1]=LSC[res]->last;
// 	}
// }

// 
// ;for each entry in the LIS
current_el=first_el(LIS[res]);
islastloop=0;
// printf("LIS processing \n");
// if (lastLISm[res][thres_ind] != NULL){//Attention à la premiere boucle quand c'est encore vide
// while (islastloop ==0) {

// if (current_el == lastLISm[res][thres_ind]){
// islastloop=1;
// }
while ((current_el != NULL) && (current_el->type >= thres_ind)){ //SPIHT 2.2)
// printf("Processing in LIS: %d %d %d\n",current_el->pixel.x,current_el->pixel.y,current_el->pixel.l);
#ifdef DEBUG
nLISloop++;
#endif
// if (LIS->current == NULL) printf("Uh Oh 2.2\n");
#ifndef NOLISTB
    if (current_el->type == 0) { //SPIHT 2.2.1);if the set is of type A
#endif
#ifdef DEBUG
        nLISloopA++;
#endif
	r=spat_spec_desc_spiht(current_el->pixel, /*tmp_list*/list_desc, imageprop, 0, image, thres_ind, map_LSC);
// 	list_free(tmp_list);//test
	bit = (r == -1);//il y a au moins un des descendants qui est significatif
        add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast)); //SPIHT 2.2.1.1
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
		add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast)); //SPIHT 2.2.1.2.1.1
		if (bit == 0){ //SPIHT 2.2.1.2.1.3
 			(map_LIC[trans_pixel(current_child->pixel, imageprop)])++;
			el=el_init(current_child->pixel);
			el->type = thres_ind;
			insert_el(LIC[res+1],el);
		} else { //SPIHT 2.2.1.2.1.2
			bitsig = (value_pix > 0);
			add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bitsig, (datablock[blockind].streamlast));
			update_dist(current_child->pixel, thres_ind, &dist, image, imageprop);
			add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
			(map_LSC[trans_pixel(current_child->pixel, imageprop)])++;
			el=el_init(current_child->pixel);
			el->type = thres_ind;
			insert_el(LSC[res+1],el);			
		};
#ifndef NOLISTB
		r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, imageprop, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
		if (r != 0) ngrandchild++;
#else
	 if (map_LIS[trans_pixel(current_child->pixel, imageprop)] == 0){
	  r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, imageprop, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
	  if (r != 0) {	 
#ifndef NEWTREE
	 	if (bit == 1) { 
#endif
		el=el_init(current_child->pixel);
		el->type = thres_ind;
		#ifndef INPLACE
		insert_el(LIS[res+1],el);
		#else
		insert_el_inplace(LIS[res+1],el);
		#endif
		(map_LIS[trans_pixel(el->pixel, imageprop)])++;
#ifndef NEWTREE
		} else {//si on met k,l dans la LIC on verifie si il est accessible
			is_accessible=0;
			parents=find_parents(current_child->pixel, imageprop);
			if ((parents.spat.x != current_el->pixel.x) ||
				(parents.spat.y != current_el->pixel.y) ||
				(parents.spat.l != current_el->pixel.l) ){//on s'arrure qu'on n'est pas arrive par le spatial
			is_accessible=is_accessible_from(parents.spat,imageprop,map_LIS);
			}
			if ((parents.spec.x != current_el->pixel.x) ||
				(parents.spec.y != current_el->pixel.y) ||
				(parents.spec.l != current_el->pixel.l) ){//on s'arrure qu'on n'est pas arrive par le spatial
			if (is_accessible ==1) printf("********** UH OH*******\n");
			is_accessible=is_accessible_from(parents.spec,imageprop,map_LIS);
			}
			if (is_accessible ==0){
				el=el_init(current_child->pixel);
				el->type = thres_ind;
				#ifndef INPLACE
				insert_el(LIS[res+1],el);
				#else
				insert_el_inplace(LIS[res+1],el);
				#endif
				(map_LIS[trans_pixel(el->pixel, imageprop)])++;
			}
		}
#endif
		
	  }
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
		tmp_el=remove_current_el(LIS[res]);
		(map_LIS[trans_pixel(tmp_el->pixel, imageprop)])--;
		free(tmp_el); //la par contre, il disparait...
		
#ifndef NOLISTB
	   };
#endif
	   tmp_el=NULL;
// 	   if (LIS->current == NULL) printf("Uh Oh 2.2.1.2 (if)\n");
	}else{//fin du 2.2.1.2
	   current_el=next_el(LIS[res]);
// 	   if (LIS->current == NULL) printf("Uh Oh 2.2.1.2 (else)\n");
	};
#ifndef NOLISTB
	} else { //SPIHT 2.2.2 ;else the set is of type B
#ifdef DEBUG
	  nLISloopB++;
#endif
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
	 add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));

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
current_el=LIS[res]->current;
}; //endwhile
// };

current_el=first_el(LSC[res]);
// printf("LSC processing \n");
// if (lastLSCm[res][thres_ind] != NULL){//Attention à la premiere boucle quand c'est encore vide
// while (current_el != lastLSCm[res][thres_ind]){
while ((current_el != NULL) && (current_el->type > thres_ind)){
#ifdef DEBUG
	nLSCloop++;
#endif
	value_pix=image[trans_pixel(current_el->pixel,imageprop)];
	bit = get_bit(value_pix, thres_ind);
	add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));
	if (bit ==1) {
		update_dist(current_el->pixel, thres_ind, &dist, image, imageprop);
		add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
	}
	current_el=next_el(LSC[res]);
};
// if (current_el != NULL){/*a priori c'est jamais le cas (== lastLSC)*/
// #ifdef DEBUG
// 	nLSCloop++;
// #endif
// 	value_pix=image[trans_pixel(current_el->pixel,imageprop)];
// // 	bit = ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold));
// 	bit = get_bit(value_pix, thres_ind);	
// 	add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));
// 	if (bit ==1) {
// 		update_dist(current_el->pixel, thres_ind, &dist,image,  imageprop);
// 		add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
// 	}
// };
// };
#ifdef DEBUG
// if (res == NRES-1){
printf("Stream size: %ld \n",*(datablock[blockind].streamlast));
printf("count:       %uc \n",*(datablock[blockind].count));
printf("Size in bit: %ld \n", *(datablock[blockind].streamlast)*8+*(datablock[blockind].count));
printf("nLSCloop: %ld\n",nLSCloop);
printf("nLICloop: %ld\n",nLICloop);
printf("nLISloop: %ld\n",nLISloop);
printf("nLISloopA: %ld\n",nLISloopA);
printf("nLISloopB: %ld\n",nLISloopB);

// if (check_map(map_LSC, imageprop)) printf("Error in LSC\n");
// if (check_map(map_LIC, imageprop)) printf("Error in LIC\n");
// if (check_map(map_LIS, imageprop)) printf("Error in LIS\n");

printf("nLSCmap: %ld\n",count_map(map_LSC,nsmax*nbmax*nlmax));
printf("nLICmap: %ld\n",count_map(map_LIC,nsmax*nbmax*nlmax));
printf("nLISmap: %ld\n",count_map(map_LIS,nsmax*nbmax*nlmax));


printf("-------------------------\n");


// printf("Size for thres %d : %ld \n",thres_ind, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count));
// }
#endif
 
// if (thres_ind == thres_ind){//tout ca pour les ajouter a la bonne place lors du passage dans la LIC
// LSC[res]->current=LSC[res]->first;
// } else {
// LSC[res]->current=lastLSCm[thres_ind];//c'est de la voltige ici... sans filet.
// }
// if (res != NRES-1){
// if (thres_ind !=0){
// lastLSC=lastLSCm[thres_ind-1];
// lastLIC=lastLICm[thres_ind-1];
// lastLSCm[thres_ind-1]=LSC[res+1]->last;
// lastLICm[thres_ind-1]=LIC[res+1]->last;
// } else {
// lastLSC = NULL;
// lastLIC=NULL;
// }
// lastLISm[thres_ind]=LIS[res+1]->last;
// }

// if ((res == 0) && (thres_ind !=0)){
// lastLICm[res][thres_ind-1]=LIC[res]->last;//because some elements may have been deleted
// lastLISm[res][thres_ind-1]=LIS[res]->last;
// }
// if ((res != 0) && (thres_ind !=0)){
//    if (LIC[res]->last ==NULL){
// 	lastLICm[res][thres_ind-1]==NULL;
//    }
//    if (LIS[res]->last ==NULL){
// 	lastLISm[res][thres_ind-1]==NULL;
//    }
// }
// 
// if (res != NRES-1){
// 	if (thres_ind !=0){
// 		lastLSCm[res+1][thres_ind-1]=LSC[res+1]->last;
// 		lastLICm[res+1][thres_ind-1]=LIC[res+1]->last;
// 	}
// 	lastLISm[res+1][thres_ind]=LIS[res+1]->last;
// }
// 
// lastLSCm[res][thres_ind]=NULL;
// lastLICm[res][thres_ind]=NULL;
// lastLISm[res][thres_ind]=NULL;

#ifndef RES_SCAL
};//fin resolution
#endif

//is it still correct ?
outputsize[thres_ind] += (*(datablock[blockind].streamlast))*8 + (*(datablock[blockind].count));
};//fin threshold

#ifdef RES_SCAL
};//fin resolution
#endif

datablock[blockind].rddata.r[NUMRD-1]=(*(datablock[blockind].streamlast))*8 + (*(datablock[blockind].count));
datablock[blockind].rddata.d[NUMRD-1]=dist;

for (i=0; i<NRES; i++){
list_flush(LSC[i]);
list_flush(LIC[i]);
list_flush(LIS[i]);
}

/*lambda=100.;
compute_cost(&(datablock[blockind].rddata),lambda)*/;

#ifdef DEBUG2
//print the rd admissible points
// printf("Rate  --- Distortion (reval)   ------> Cost (J)\n");
// for (i=0; i<NUMRD; i++){
// printf("%lld --- %lld (%lld)  ------> %f\n", datablock[blockind].rddata.r[i], datablock[blockind].rddata.d[i], datablock[blockind].rddata.reval[i], datablock[blockind].rddata.cost_j[i]);
// }
// printf("Optimal for lambda=%f at %d\n",lambda,datablock[blockind].rddata.ptcourant);

#endif


}// Fin du 
}// parcours 
}// en bloc

// printf("Interleaving for lambda=200\n");
// lambda=200.;
// interleavingblocks(datablock, nblock, stream, count, streamlast, lambda);
// 
// #ifdef DEBUG2
// printf("Stream size: %ld \n",*streamlast);
// printf("count:       %uc \n",*count);
// printf("Size in bit: %ld \n", *streamlast*8+*count);
// printf("Rate: %f \n", (float) (*streamlast*8+*count)/((long int) nsmax*nlmax*nbmax));
// #endif
// 
// printf("Interleaving for lambda=100\n");
// lambda=100.;
// interleavingblocks(datablock, nblock, stream, count, streamlast, lambda);
// 
// #ifdef DEBUG2
// printf("Stream size: %ld \n",*streamlast);
// printf("count:       %uc \n",*count);
// printf("Size in bit: %ld \n", *streamlast*8+*count);
// printf("Rate: %f \n", (float) (*streamlast*8+*count)/((long int) nsmax*nlmax*nbmax));
// #endif
// 
// 
// printf("Interleaving for lambda=50\n");
// lambda=50.;
// interleavingblocks(datablock, nblock, stream, count, streamlast, lambda);
// 
// #ifdef DEBUG2
// printf("Stream size: %ld \n",*streamlast);
// printf("count:       %uc \n",*count);
// printf("Size in bit: %ld \n", *streamlast*8+*count);
// printf("Rate: %f \n", (float) (*streamlast*8+*count)/((long int) nsmax*nlmax*nbmax));
// #endif

// lambda=compute_lambda(datablock, 1.0*npix, nblock);
// printf("Interleaving for lambda=%f\n",lambda);
// interleavingblocks(datablock, nblock, stream, count, streamlast, lambda);
// 
// #ifdef DEBUG2
// printf("Stream size: %ld \n",*streamlast);
// printf("count:       %uc \n",*count);
// printf("Size in bit: %ld \n", *streamlast*8+*count);
// printf("Rate: %f \n", (float) (*streamlast*8+*count)/((long int) nsmax*nlmax*nbmax));
// #endif


printf("Interleaving for lambda=0 (lossless)\n");
lambda=0.;
interleavingblocks(datablock, nblock, stream, count, streamlast, lambda);

#ifdef DEBUG2
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("Size in bit: %ld \n", *streamlast*8+*count);
printf("Rate: %f \n", (float) (*streamlast*8+*count)/((long int) nsmax*nlmax*nbmax));
#endif

//WARNING no more full size...
*outputsize = *streamlast*8+*count;


free(map_LSC);
free(map_LIC);
free(map_LIS);

//do not forget to free each block separatly first
for (i=0;i<nblock;i++){
	datablock_free(&(datablock[i]));
}

free(datablock);

// return, 0
// outputsize[thres_ind]=*streamlast+1; //add a +1 ???

free(count);
free(streamlast);

// for (i=0; i<NRES; i++){
// free(LSC[res]);
// free(LIC[res]);
// free(LIS[res]);
// }

return 0;
};



int spiht_decode_ra2(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue)
{

// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, 8, 8, 7};
struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
int nbmin=imageprop.nbmin;

struct list_struct * LSC[NRES];
struct list_struct * LIC[NRES];
struct list_struct * LIS[NRES];
struct pixel_struct pixel;
struct parents_struct parents;
int is_accessible=0;
// int maxquant=imageprop.maxquant;
// int maxquant=MAXQUANT_CONST;
int maxquant=(int) *maxquantvalue;

int minquant=0;

struct list_el * el=NULL;
struct list_el * current_el=NULL;
struct list_el * tmp_el=NULL;
struct list_el * current_child=NULL;

// struct list_el * lastLSC=NULL;
// struct list_el * lastLSC[NRES];
// struct list_el * lastLIC[NRES];

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
int iloc,jloc,kloc;
int niloc, njloc, nkloc;
int ii,ji,ki;
int ie,je,ke;
long int i_l;
int blockind;
int nblock;

int err=0;

int maxres=NRES;
// int maxres=2;

int lastres=0;
int flagLSClastres=0;

//debit-distortion
// long long int dist=0;
// struct rddata_struct rddata;
// float alpha, beta, rate_i, rate_e;
struct datablock_struct * datablock;
// float lambda=0.0;

int res=0;

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

map_LSC=(unsigned char *) calloc(nsmax*nbmax*nlmax,sizeof(unsigned char));
map_LIC= (unsigned char *) calloc(nsmax*nbmax*nlmax,sizeof(unsigned char));
map_LIS= (unsigned char *) calloc(nsmax*nbmax*nlmax,sizeof(unsigned char));

*count=0;
*streamlast=0;

for (i=0;i<NRES;i++){
LSC[i] = list_init();
LIC[i] = list_init();
LIS[i] = list_init();
}

// list_desc = list_init();

for (i_l=0;i_l<nsmax*nlmax*nbmax;i_l++)
{
	map_LSC[i_l]=0;
	map_LIC[i_l]=0;
	map_LIS[i_l]=0;
}


niloc=(nsmin+1)/2;
njloc=(nlmin+1)/2;
nkloc=(nbmin+1)/2;
nblock=niloc* njloc* nkloc;

datablock=(struct datablock_struct *) malloc(nblock* sizeof(struct datablock_struct));

for (i=0; i<nblock; i++){
	datablock_init(&(datablock[i]));
}

//Desinterlacing stream
err=desinterleavingblocks(datablock, nblock, stream, *outputsize);
if (err) {fprintf(stderr, "******** ERROR desinterleavingblocks (truncation)\n");};

for (i=0; i<nblock; i++){
	datablock[i].currentpos=(*(datablock[i].streamlast))*8+ *(datablock[i].count);
	*(datablock[i].streamlast)=0;
	*(datablock[i].count)=0;
}



//******************************************
//Parcours des differentes localisations
//******************************************

// nkloc=1;njloc=1;//TEMP !!!!

for (kloc=0;kloc<nkloc;kloc++){
   for (jloc=0;jloc<njloc;jloc++){
      for (iloc=0;iloc<niloc;iloc++){

#ifdef DEBUG2
printf("Decoding grp: %d %d %d\n",iloc, jloc, kloc);
#endif

blockind = iloc + jloc*niloc + kloc * niloc *njloc;
*outputsize = datablock[blockind].currentpos;
//SPIHT 1)
// printf("Initialization...\n");
ki=2*kloc;
ji=2*jloc;
ii=2*iloc;
ke= (2*kloc+2 < nbmin ? 2*kloc+2 : nbmin);
je= (2*jloc+2 < nlmin ? 2*jloc+2 : nlmin);
ie= (2*iloc+2 < nsmin ? 2*iloc+2 : nsmin);

for (k=ki;k<ke;k++){
   for (j=ji;j<je;j++){
      for (i=ii;i<ie;i++){
//       for (j=0;j<nlmin;j++){
//       for (k=0;k<nbmin;k++){
	pixel.x=i;
	pixel.y=j;
	pixel.l=k;
	el=el_init(pixel);
	el->type=maxquant+1;
	insert_el(LIC[0],el);
	(map_LIC[trans_pixel(pixel, imageprop)])++;
	if ((i % 2) || (j % 2) || (k % 2) || (((nbmin % 2) == 1) && (k == nbmin-1) && (k!=0))){ 
		el=el_init(pixel);
		el->type=maxquant+1;
		insert_el(LIS[0],el);
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
#ifdef RES_SCAL
for (res=0; res<maxres; res++){
#endif
//SPIHT 2)
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){

#ifndef RES_SCAL
for (res=0; res<NRES; res++){
#endif

// for (res=0; res<NRES; res++){
// lastLSC[res] = LSC[res]->last;
// lastLIC[res] = LIC[res]->last;
// }
// 
// for (res=0; res<NRES; res++){

threshold= 1 << (long int)thres_ind;
#ifdef DEBUG
printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
// printf("Sorting pass for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
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
// lastLSC[res] = LSC[res]->last;
current_el=first_el(LSC[res]); //right positionning for adding from LIC
while ((current_el != NULL) && (current_el->type > thres_ind)){
   	current_el=next_el(LSC[res]);
}
if ((current_el != NULL) && (current_el->type <= thres_ind)) {
LSC[res]->current = LSC[res]->previous;
}
if (current_el == NULL){
//si on arrive la, ca veut dire que soit la liste est vide soit on est alle jusqu'a la fin
LSC[res]->current = LSC[res]->last;
}



// ;for each entry in the LIC
//SPIHT 2.1)
current_el=first_el(LIC[res]);
// printf("LIC processing \n");
// if (lastLIC[res] != NULL){//Attention à la premiere boucle quand c'est encore vide
while ((current_el != NULL) && (current_el->type > thres_ind) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize)){
// printf(".");
#ifdef DEBUG
   nLICloop++;
#endif
//    value_pix=image[trans_pixel(current_el->pixel,imageprop)];
//    bit = ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold));//TODO possibilite d'enlever un abs
//    bit = get_bit(value_pix, thres_ind);	
//    add_to_stream(stream, count, (int) bit, streamlast);//SPIHT 2.1.1)
   if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize){
      bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
   } else break;

  if (bit == 1) { //SPIHT 2.1.2)
//      bitsig = (value_pix > 0);
//      add_to_stream(stream, count, (int) bitsig, streamlast);
     if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize){
        bitsig = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
     } else break;
     image[trans_pixel(current_el->pixel,imageprop)] += threshold;
     if (bitsig == 0) {
     	image[trans_pixel(current_el->pixel,imageprop)] = -image[trans_pixel(current_el->pixel,imageprop)];
     };
     (map_LIC[trans_pixel(current_el->pixel, imageprop)])--;
     (map_LSC[trans_pixel(current_el->pixel, imageprop)])++;
     remove_current_el(LIC[res]);
     insert_el_inplace(LSC[res], current_el);
	if (LSC[res]->current == NULL){
		LSC[res]->current=LSC[res]->first;
	}else {
		next_el(LSC[res]);
	}

     current_el->type = thres_ind;
     current_el = LIC[res]->current;//est passe au suivant dans le move
   } else {
      current_el=next_el(LIC[res]);
   };
}
// printf(".");
// #ifdef DEBUG
//    nLICloop++;
// #endif
// //    value_pix=image[trans_pixel(current_el->pixel,imageprop)];
// //    bit = ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold));//TODO possibilite d'enlever un abs
// //    bit = get_bit(value_pix, thres_ind);	
// //    add_to_stream(stream, count, (int) bit, streamlast);//SPIHT 2.1.1)
//    if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize){
//       bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
//    } else break;
// 
//   if (bit == 1) { //SPIHT 2.1.2)
// //      bitsig = (value_pix > 0);
// //      add_to_stream(stream, count, (int) bitsig, streamlast);
//      if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize){
//         bitsig = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
//      } else break;
//      image[trans_pixel(current_el->pixel,imageprop)] += threshold;
//      if (bitsig == 0) {
//      	image[trans_pixel(current_el->pixel,imageprop)] = -image[trans_pixel(current_el->pixel,imageprop)];
//      };
//      (map_LIC[trans_pixel(current_el->pixel, imageprop)])--;
//      (map_LSC[trans_pixel(current_el->pixel, imageprop)])++;
//      remove_current_el(LIC[res]);
//      insert_el(LSC[res], current_el);
//      current_el = LIC[res]->current;//est passe au suivant dans le move
//    } else {
//       current_el=next_el(LIC[res]);
//    };
// 
// }


// ;for each entry in the LIS
current_el=first_el(LIS[res]);
// printf("LIS processing \n");
while ((current_el != NULL)&& (current_el->type >= thres_ind) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize)){ //SPIHT 2.2)
// printf("Processing in LIS: %d %d %d\n",current_el->pixel.x,current_el->pixel.y,current_el->pixel.l);
#ifdef DEBUG
nLISloop++;
#endif
// if (LIS->current == NULL) printf("Uh Oh 2.2\n");
#ifndef NOLISTB
    if (current_el->type == 0) { //SPIHT 2.2.1);if the set is of type A
#endif
#ifdef DEBUG
        nLISloopA++;
#endif
// 	r=spat_spec_desc_spiht(current_el->pixel, list_desc, imageprop, 0, image, thres_ind, map_LSC);
// 	bit = (r == -1);//il y a au moins un des descendants qui est significatif
//         add_to_stream(stream, count, (int) bit, streamlast); //SPIHT 2.2.1.1
        if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize){
	   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
        } else break;
        if (bit == 1) { //SPIHT 2.2.1.2
	   list_desc=list_init();//list_free(list_desc);//TODO faire juste un nettoyage de la liste sans la liberer pour eviter un malloc (voir list_free plus bas) done
	   r=spat_spec_desc_spiht(current_el->pixel, list_desc, imageprop, 1, image, thres_ind, map_LSC);//verifier qu'on va jusqu'au bout avec le onlychild a 1
	   current_child = first_el(list_desc);
 	   ngrandchild = 0;
	   list_grand_desc=list_init();
	   while ((current_child !=NULL)&& ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize)){ //SPIHT 2.2.1.2.1
	      if ((map_LSC[trans_pixel(current_child->pixel, imageprop)] == 0) && (map_LIC[trans_pixel(current_child->pixel, imageprop)] == 0)){ 
// 		value_pix=image[trans_pixel(current_child->pixel,imageprop)]; 
// 		bit = get_bit(value_pix, thres_ind);	
// 		add_to_stream(stream, count, (int) bit, streamlast); //SPIHT 2.2.1.2.1.1
		if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize){
		   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
		} else break;
		if (bit == 0){ //SPIHT 2.2.1.2.1.3
 			(map_LIC[trans_pixel(current_child->pixel, imageprop)])++;
			el=el_init(current_child->pixel);
			el->type=thres_ind;
			insert_el(LIC[res+1],el);
		} else { //SPIHT 2.2.1.2.1.2
			image[trans_pixel(current_child->pixel,imageprop)] += threshold; //le signe est vu apres
// 			bitsig = (value_pix > 0);
// 			add_to_stream(stream, count, (int) bitsig, streamlast);
			if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize){
			   bitsig = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
			} else break;
			if (bitsig == 0){
				image[trans_pixel(current_child->pixel,imageprop)] = -image[trans_pixel(current_child->pixel,imageprop)];
			};
			(map_LSC[trans_pixel(current_child->pixel, imageprop)])++;
			el=el_init(current_child->pixel);
			el->type=thres_ind;
			insert_el(LSC[res+1],el);			
		};
#ifndef NOLISTB
		r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, imageprop, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
		if (r != 0) ngrandchild++;
#else
	//Si l'enfant k,l a lui meme des enfants, il faut le rajouter à la LIS (seulement si il n'y est pas déjà...
// 	if (map_LIS[trans_pixel(current_child->pixel, imageprop)] == 0){
// 	r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, imageprop, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
// 	if (r != 0) {	 
// 	 
// 		el=el_init(current_child->pixel);
// 		#ifndef INPLACE
// 		insert_el(LIS,el);
// 		#else
// 		insert_el_inplace(LIS,el);
// 		#endif
// 		(map_LIS[trans_pixel(el->pixel, imageprop)])++;
// 	}
// 	}
	 if (map_LIS[trans_pixel(current_child->pixel, imageprop)] == 0){
	  r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, imageprop, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
	  if (r != 0) {	 
#ifndef NEWTREE
	 	if (bit == 1) { 
#endif
		el=el_init(current_child->pixel);
		el->type=thres_ind;
		#ifndef INPLACE
		insert_el(LIS[res+1],el);
		#else
		insert_el_inplace(LIS[res+1],el);
		#endif
		(map_LIS[trans_pixel(el->pixel, imageprop)])++;
#ifndef NEWTREE
		} else {//si on met k,l dans la LIC on verifie si il est accessible
			is_accessible=0;
			parents=find_parents(current_child->pixel, imageprop);
			if ((parents.spat.x != current_el->pixel.x) ||
				(parents.spat.y != current_el->pixel.y) ||
				(parents.spat.l != current_el->pixel.l) ){//on s'arrure qu'on n'est pas arrive par le spatial
			is_accessible=is_accessible_from(parents.spat,imageprop,map_LIS);
			}
			if ((parents.spec.x != current_el->pixel.x) ||
				(parents.spec.y != current_el->pixel.y) ||
				(parents.spec.l != current_el->pixel.l) ){//on s'arrure qu'on n'est pas arrive par le spatial
			if (is_accessible ==1) printf("********** UH OH*******\n");
			is_accessible=is_accessible_from(parents.spec,imageprop,map_LIS);
			}
			if (is_accessible ==0){
				el=el_init(current_child->pixel);
				el->type=thres_ind;
				#ifndef INPLACE
				insert_el(LIS[res+1],el);
				#else
				insert_el_inplace(LIS[res+1],el);
				#endif
				(map_LIS[trans_pixel(el->pixel, imageprop)])++;
			}
		}
#endif
		
	  }
	}
#endif
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
#ifndef NOLISTB
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
#endif
		tmp_el=remove_current_el(LIS[res]);
		(map_LIS[trans_pixel(tmp_el->pixel, imageprop)])--;
		free(tmp_el); //la par contre, il disparait...
		
#ifndef NOLISTB
	   };
#endif
	   tmp_el=NULL;
// 	   if (LIS->current == NULL) printf("Uh Oh 2.2.1.2 (if)\n");
	}else{//fin du 2.2.1.2
	   current_el=next_el(LIS[res]);
// 	   if (LIS->current == NULL) printf("Uh Oh 2.2.1.2 (else)\n");
	};
#ifndef NOLISTB
	} else { //SPIHT 2.2.2 ;else the set is of type B
#ifdef DEBUG
	  nLISloopB++;
#endif
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
	if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize){
	   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
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
#endif
// if (check_map(map_LIS, imageprop)== 1) {printf("erreur");}; //TODO remove after
current_el=LIS[res]->current;   
}; //endwhile


current_el=first_el(LSC[res]);
// printf("LSC processing \n");
// if (lastLSC[res] != NULL){//Attention à la premiere boucle quand c'est encore vide
// while ((current_el != lastLSC[res]) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize)){
while ((current_el != NULL) && (current_el->type > thres_ind) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize)){
#ifdef DEBUG
	nLSCloop++;
#endif
// 	value_pix=image[trans_pixel(current_el->pixel,imageprop)];
// 	bit = get_bit(value_pix, thres_ind);
// 	add_to_stream(stream, count, (int) bit, streamlast);
	if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize){
	   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
	   flagLSC=1;
	} else break;
	if (bit == 1){
		if (image[trans_pixel(current_el->pixel,imageprop)] > 0){
			image[trans_pixel(current_el->pixel,imageprop)] += threshold;
		} else {
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
		};
	};
	current_el=next_el(LSC[res]);
};
// if ((current_el != NULL) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize)){/*a priori c'est jamais le cas (== lastLSC)*/
// #ifdef DEBUG
// 	nLSCloop++;
// #endif
// // 	value_pix=image[trans_pixel(current_el->pixel,imageprop)];
// // // 	bit = ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold));
// // 	bit = get_bit(value_pix, thres_ind);	
// // 	add_to_stream(stream, count, (int) bit, streamlast);
// 	if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize){
// 	   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
// 	//}; else break; 
// 	if (bit == 1){
// 		if (image[trans_pixel(current_el->pixel,imageprop)] > 0){
// 			image[trans_pixel(current_el->pixel,imageprop)] += threshold;
// 		} else {
// 			image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
// 		};
// 	};
// 	current_el=next_el(LSC[res]);
// 	};
// 
// };
// };

#ifdef DEBUG
printf("Stream size: %ld \n",*(datablock[blockind].streamlast));
printf("count:       %uc \n",*(datablock[blockind].count));
printf("nLSCloop: %ld\n",nLSCloop);
printf("nLICloop: %ld\n",nLICloop);
printf("nLISloop: %ld\n",nLISloop);
printf("nLISloopA: %ld\n",nLISloopA);
printf("nLISloopB: %ld\n",nLISloopB);
printf("-------------------------\n");

printf("*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)): %ld\n", (*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)));
printf("*outputsize: %ld\n", *outputsize);
#endif

if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) > *outputsize){
#ifdef DEBUG
 	printf("Sortie: fin du train de bit (threshold %ld)\n", threshold);
#endif
	break;
};

};//fin threshold

if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) > *outputsize){
#ifdef DEBUG
 	printf("Sortie: fin du train de bit (threshold %ld)\n", threshold);
#endif
	break;
};

#ifndef RES_SCAL

};//fin res
#endif

/*
for (i=0; i<NRES; i++){
list_flush(LSC[i]);
list_flush(LIC[i]);
list_flush(LIS[i]);
}*/



//correction finale eventuelle 
//TODO: la sortie est calculee a l'octet pres, pas au bit
// if (thres_ind !=0){//si on a entame la boucle finale on ne va pas faire de correc

//WARNING the final correction is not always correct with the resolution scalability... to improve later...
// for (res=0;res<maxres;res++){
#ifdef DEBUG
printf("Correction finale (flagLSC= %d)\n", flagLSC);
#endif

#ifndef RES_SCAL
lastres=res;
flagLSClastres=flagLSC;
for (res=0;res<maxres;res++){//Hum... comment gerer le LSC ici...

if (res == lastres) {
	flagLSC = flagLSClastres;
} else {
	flagLSC=0;
}
#endif

// if ((thres_ind <= minquant) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize)){
if (thres_ind <= minquant) {
flagLSC=0;
threshold= 1 << (long int)thres_ind;
}

if (((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) > *outputsize) || (thres_ind >0)){ //on est sorti car le train de bit etait trop court

	if (flagLSC == 0){ //we didn't exit during the refinment pass
		current_el=first_el(LSC[res]);
		   while (current_el != NULL){
			if (current_el->type > thres_ind){//this was added before and not refined yet, 
				if (image[trans_pixel(current_el->pixel,imageprop)] >0){
					image[trans_pixel(current_el->pixel,imageprop)] += threshold;
				};
				if (image[trans_pixel(current_el->pixel,imageprop)] <0){
					image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
				};
			} else{
				if (image[trans_pixel(current_el->pixel,imageprop)] >0){
					image[trans_pixel(current_el->pixel,imageprop)] += threshold/2;
				};
				if (image[trans_pixel(current_el->pixel,imageprop)] <0){
					image[trans_pixel(current_el->pixel,imageprop)] -= threshold/2;
				};
			}
			current_el=next_el(LSC[res]);
		   };
	} else { //we exit during the refinment pass
		lastprocessed=LSC[res]->current;
		current_el=first_el(LSC[res]);
		if (lastprocessed != NULL){
		   while (current_el != lastprocessed){
		      if (image[trans_pixel(current_el->pixel,imageprop)] >0){
			image[trans_pixel(current_el->pixel,imageprop)] += threshold/2;
		      };
		      if (image[trans_pixel(current_el->pixel,imageprop)] <0){
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold/2;
		      };
		      current_el=next_el(LSC[res]);
		   };
		}
		   while (current_el != NULL){
			if (current_el->type > thres_ind){//this was added before and not refined yet, 
				if (image[trans_pixel(current_el->pixel,imageprop)] >0){
					image[trans_pixel(current_el->pixel,imageprop)] += threshold;
				};
				if (image[trans_pixel(current_el->pixel,imageprop)] <0){
					image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
				};
			} else{
				if (image[trans_pixel(current_el->pixel,imageprop)] >0){
					image[trans_pixel(current_el->pixel,imageprop)] += threshold/2;
				};
				if (image[trans_pixel(current_el->pixel,imageprop)] <0){
					image[trans_pixel(current_el->pixel,imageprop)] -= threshold/2;
				};
			}
			current_el=next_el(LSC[res]);
		   }
	};


};

#ifndef RES_SCAL
};//fin boucle for
#endif

#ifdef RES_SCAL
};//fin res
#endif

for (i=0; i<NRES; i++){
list_flush(LSC[i]);
list_flush(LIC[i]);
list_flush(LIS[i]);
}

}// Fin codage
}// des 
}// groupes

#ifdef DEBUG2
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
#endif


free(map_LSC);
free(map_LIC);
free(map_LIS);

free(count);
free(streamlast);
 
// for (i=0; i<NRES; i++){
// list_free(LSC[i]);
// // free(LSC);
// list_free(LIC[i]);
// // free(LIC);
// list_free(LIS[i]);
// // free(LIS);
// };

// return, 0
return 0;
};

