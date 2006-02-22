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
// #define NRES 11
// #define NRES 1

int spiht_code_ra(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue)
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
// struct list_struct * LSC[NRES];
// struct list_struct * LIC[NRES];
// struct list_struct * LIS[NRES];
// struct list_struct * LSC[NRES+1];
// struct list_struct * LIC[NRES+1];
// struct list_struct * LIS[NRES+1];
struct pixel_struct pixel;
struct parents_struct parents;
int is_accessible=0;
// int maxquant=imageprop.maxquant;
// int maxquant=MAXQUANT_CONST;
int maxquant= (int) (*maxquantvalue);
int minquant=0;
struct list_el * el=NULL;
struct list_el * current_el=NULL;
struct list_el * tmp_el=NULL;
struct list_el * current_child=NULL;
struct list_el * lastLSC=NULL;
// struct list_el * lastLSC[NRES];
// struct list_el * lastLIC[NRES];

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
long int i_l;
int blockind;
int nblock;

int res=0;

//debit-distortion
long long int dist=0;
// struct rddata_struct rddata;
float alpha, beta, rate_i, rate_e;
struct datablock_struct * datablock;
float lambda=0.0;

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

*count=0;
*streamlast=0;


LSC = list_init();
LIC = list_init();
LIS = list_init();
// for (i=0; i<NRES; i++){
// LSC[i] = list_init();
// LIC[i] = list_init();
// LIS[i] = list_init();
// }

//temp
// LSC[1] = LSC[0];
// LIC[1] = LIC[0];
// LIS[1] = LIS[0];

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



//**********************************************
//Parcours des differentes localisations
//**********************************************

for (kloc=0;kloc<nkloc;kloc++){
   for (jloc=0;jloc<njloc;jloc++){
      for (iloc=0;iloc<niloc;iloc++){

#ifdef DEBUG2
printf("Processing grp: %d %d %d\n",iloc, jloc, kloc);
#endif

blockind = iloc + jloc*niloc + kloc * niloc *njloc;
datablock_init(&(datablock[blockind]));

datablock[blockind].rddata.ptcourant=0;
for (i=0;i<NUMRD;i++){
datablock[blockind].rddata.reval[i]= (long long int) powf(alpha, beta+i);
datablock[blockind].rddata.r[i]=0; //fastest to use separate loops for memory access ?
datablock[blockind].rddata.d[i]=0;
};
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

// #ifdef DEBUG
// if (check_accessibility_of_all(imageprop, map_LSC, map_LIC, map_LIS) == 0) {
//     printf("Probleme d'accessibilite");
// }
// #endif


//SPIHT 2)
for (thres_ind=maxquant; thres_ind >= minquant; thres_ind--){
dist = eval_dist_grp(iloc, jloc, kloc, image, imageprop, thres_ind+1);

//remember what was the last one (only in case a of p25
// for (res=0; res<NRES; res++){
lastLSC= LSC->last;
// lastLIC[res] = LIC[res]->last;
// }
// 
// for (res=0; res<NRES; res++){//possible to put it later...

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
// printf("Dist before eval (from updating): %lld\n",dist);
// dist = eval_dist_grp(iloc, jloc, kloc, image, imageprop, thres_ind+1);
// printf("Dist after eval (direct processing): %lld\n",dist);

// Used for Significant pass
// lastLSC = LSC[res]->last;

// ;for each entry in the LIC
//SPIHT 2.1)
current_el=first_el(LIC );
// printf("LIC processing \n");
// if (lastLIC[res] != NULL){//Attention à la premiere boucle quand c'est encore vide
// while (current_el != lastLIC[res]){
while (current_el != NULL){
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
     remove_current_el(LIC );
     insert_el(LSC , current_el);
     current_el = LIC ->current;//est passe au suivant dans le move
     
   } else {
      current_el=next_el(LIC );
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
//      remove_current_el(LIC );
//      insert_el(LSC , current_el);
//      current_el = LIC ->current;//est passe au suivant dans le move
//      
//    } else {
//       current_el=next_el(LIC );
//    };
// }


// ;for each entry in the LIS
current_el=first_el(LIS );
// printf("LIS processing \n");
while (current_el != NULL){ //SPIHT 2.2)
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
			insert_el(LIC ,el);
		} else { //SPIHT 2.2.1.2.1.2
			bitsig = (value_pix > 0);
			add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bitsig, (datablock[blockind].streamlast));
			update_dist(current_child->pixel, thres_ind, &dist, image, imageprop);
			add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
			(map_LSC[trans_pixel(current_child->pixel, imageprop)])++;
			el=el_init(current_child->pixel);
			insert_el(LSC ,el);			
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
		#ifndef INPLACE
		insert_el(LIS ,el);
		#else
		insert_el_inplace(LIS ,el);
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
				#ifndef INPLACE
				insert_el(LIS ,el);
				#else
				insert_el_inplace(LIS ,el);
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
		tmp_el=remove_current_el(LIS );
		(map_LIS[trans_pixel(tmp_el->pixel, imageprop)])--;
		free(tmp_el); //la par contre, il disparait...
		
#ifndef NOLISTB
	   };
#endif
	   tmp_el=NULL;
// 	   if (LIS->current == NULL) printf("Uh Oh 2.2.1.2 (if)\n");
	}else{//fin du 2.2.1.2
	   current_el=next_el(LIS );
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
current_el=LIS ->current;
}; //endwhile


current_el=first_el(LSC );
// printf("LSC processing \n");
// if (lastLSC[res] != NULL){//Attention à la premiere boucle quand c'est encore vide
// while (current_el != lastLSC[res]){
if (lastLSC != NULL){//Attention à la premiere boucle quand c'est encore vide
while (current_el != NULL){
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
	current_el=next_el(LSC );
};
if (current_el != NULL){/*a priori c'est jamais le cas (== lastLSC)*/
#ifdef DEBUG
	nLSCloop++;
#endif
	value_pix=image[trans_pixel(current_el->pixel,imageprop)];
// 	bit = ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold));
	bit = get_bit(value_pix, thres_ind);	
	add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));
	if (bit ==1) {
		update_dist(current_el->pixel, thres_ind, &dist,image,  imageprop);
		add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
	}
};
};
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
outputsize[thres_ind] += (*(datablock[blockind].streamlast))*8 + (*(datablock[blockind].count)); 

};//fin threshold

// };//fin resolution

datablock[blockind].rddata.r[NUMRD-1]=(*(datablock[blockind].streamlast))*8 + (*(datablock[blockind].count));
datablock[blockind].rddata.d[NUMRD-1]=dist;

// for (i=0; i<NRES; i++){
// list_flush(LSC[i]);
// list_flush(LIC[i]);
// list_flush(LIS[i]);
// }
list_flush(LSC);
list_flush(LIC);
list_flush(LIS);

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

printf("Interleaving for lambda=200\n");
lambda=200.;
interleavingblocks(datablock, nblock, stream, count, streamlast, lambda);

#ifdef DEBUG2
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("Size in bit: %ld \n", *streamlast*8+*count);
#endif

printf("Interleaving for lambda=100\n");
lambda=100.;
interleavingblocks(datablock, nblock, stream, count, streamlast, lambda);

#ifdef DEBUG2
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("Size in bit: %ld \n", *streamlast*8+*count);
#endif

printf("Interleaving for lambda=0 (lossless)\n");
lambda=0.;
interleavingblocks(datablock, nblock, stream, count, streamlast, lambda);

#ifdef DEBUG2
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
printf("Size in bit: %ld \n", *streamlast*8+*count);
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



int spiht_decode_ra(long int *image, unsigned char *stream, long int *outputsize, int *maxquantvalue)
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
// struct list_struct * LSC[NRES];
// struct list_struct * LIC[NRES];
// struct list_struct * LIS[NRES];
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

struct list_el * lastLSC=NULL;
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

map_LSC=(unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));
map_LIC= (unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));
map_LIS= (unsigned char *) malloc(nsmax*nbmax*nlmax*sizeof(unsigned char));

*count=0;
*streamlast=0;

LSC = list_init();
LIC = list_init();
LIS = list_init();

// for (i=0;i<NRES;i++){
// LSC[i] = list_init();
// LIC[i] = list_init();
// LIS[i] = list_init();
// }

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
desinterleavingblocks(datablock, nblock, stream, *outputsize);

for (i=0; i<nblock; i++){
	datablock[i].currentpos=(*(datablock[i].streamlast))*8+ *(datablock[i].count);
	*(datablock[i].streamlast)=0;
	*(datablock[i].count)=0;
}



//******************************************
//Parcours des differentes localisations
//******************************************

for (kloc=0;kloc<nkloc;kloc++){
   for (jloc=0;jloc<njloc;jloc++){
      for (iloc=0;iloc<niloc;iloc++){

#ifdef DEBUG2
printf("Processing grp: %d %d %d\n",iloc, jloc, kloc);
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

// for (res=0; res<NRES; res++){
lastLSC  = LSC ->last;
// lastLIC[res] = LIC[res]->last;
// }

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

// ;for each entry in the LIC
//SPIHT 2.1)
current_el=first_el(LIC );
// printf("LIC processing \n");
// if (lastLIC[res] != NULL){//Attention à la premiere boucle quand c'est encore vide
// while ((current_el != lastLIC[res]) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize)){
while ((current_el != NULL) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize)){
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
     remove_current_el(LIC );
     insert_el(LSC , current_el);
     current_el = LIC ->current;//est passe au suivant dans le move
   } else {
      current_el=next_el(LIC );
   };
}
// // printf(".");
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
//      remove_current_el(LIC );
//      insert_el(LSC , current_el);
//      current_el = LIC ->current;//est passe au suivant dans le move
//    } else {
//       current_el=next_el(LIC );
//    };
// 
// }


// ;for each entry in the LIS
current_el=first_el(LIS );
// printf("LIS processing \n");
while ((current_el != NULL)&& ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize)){ //SPIHT 2.2)
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
			insert_el(LIC ,el);
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
			insert_el(LSC ,el);			
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
		#ifndef INPLACE
		insert_el(LIS ,el);
		#else
		insert_el_inplace(LIS ,el);
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
				#ifndef INPLACE
				insert_el(LIS ,el);
				#else
				insert_el_inplace(LIS ,el);
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
		tmp_el=remove_current_el(LIS ); //il ne faut pas liberer la memoire ici...	
		insert_el(LIS ,current_el); //Attention, l'ordre des operations est critique ici
#else
// 		insert_el_inplace(LIS,current_el);
//On ne fait rien dans ce cas, on change juste le type, le meme coefficient va etre traite (le passage au suivant est fait par remove_current_el dans l'autre cas
#endif
		current_el->type=1; // on ne peut pas le rajouter à la fin avant de le supprimer du milieu
	   } else {
#endif
		tmp_el=remove_current_el(LIS );
		(map_LIS[trans_pixel(tmp_el->pixel, imageprop)])--;
		free(tmp_el); //la par contre, il disparait...
		
#ifndef NOLISTB
	   };
#endif
	   tmp_el=NULL;
// 	   if (LIS->current == NULL) printf("Uh Oh 2.2.1.2 (if)\n");
	}else{//fin du 2.2.1.2
	   current_el=next_el(LIS );
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
current_el=LIS ->current;   
}; //endwhile


current_el=first_el(LSC );
// printf("LSC processing \n");
// if (lastLSC[res] != NULL){//Attention à la premiere boucle quand c'est encore vide
// while ((current_el != lastLSC[res]) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize)){
if (lastLSC != NULL){//Attention à la premiere boucle quand c'est encore vide
while ((current_el != lastLSC) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize)){
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
	current_el=next_el(LSC );
};
if ((current_el != NULL) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize)){/*a priori c'est jamais le cas (== lastLSC)*/
#ifdef DEBUG
	nLSCloop++;
#endif
// 	value_pix=image[trans_pixel(current_el->pixel,imageprop)];
// // 	bit = ((abs(value_pix) >= threshold) && (abs(value_pix) < 2*threshold));
// 	bit = get_bit(value_pix, thres_ind);	
// 	add_to_stream(stream, count, (int) bit, streamlast);
	if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) <= *outputsize){
	   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
	//}; else break; 
	if (bit == 1){
		if (image[trans_pixel(current_el->pixel,imageprop)] > 0){
			image[trans_pixel(current_el->pixel,imageprop)] += threshold;
		} else {
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
		};
	};
	current_el=next_el(LSC );
	};

};
};

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
// };//fin res

// for (i=0; i<NRES; i++){
// list_flush(LSC[i]);
// list_flush(LIC[i]);
// list_flush(LIS[i]);
// }
list_flush(LSC);
list_flush(LIC);
list_flush(LIS);

}// Fin codage
}// des 
}// groupes

#ifdef DEBUG2
printf("Stream size: %ld \n",*streamlast);
printf("count:       %uc \n",*count);
#endif


//correction finale eventuelle 
//TODO: la sortie est calculee a l'octet pres, pas au bit
// if (thres_ind !=0){//si on a entame la boucle finale on ne va pas faire de correc

//WARNING the final correction is not always correct with the resolution scalability... to improve later...
// for (res=0;res<NRES;res++){
#ifdef DEBUG
printf("Correction finale (flagLSC= %d)\n", flagLSC);
#endif
if ((*streamlast)*8+ (*count) > *outputsize){//on est sorti car le train de bit etait trop court
	if (flagLSC == 0){
		current_el=first_el(LSC );
		if (lastLSC != NULL){
		   while (current_el != lastLSC){
		      if (image[trans_pixel(current_el->pixel,imageprop)] >0){
			image[trans_pixel(current_el->pixel,imageprop)] += threshold;
		      };
		      if (image[trans_pixel(current_el->pixel,imageprop)] <0){
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
		      };
		      current_el=next_el(LSC );
		   };
		      if (image[trans_pixel(current_el->pixel,imageprop)] >0){//current_el = lastLSC donc necessairement different de NULL
			image[trans_pixel(current_el->pixel,imageprop)] += threshold;
		      };
		      if (image[trans_pixel(current_el->pixel,imageprop)] <0){
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
		      };
		   current_el=next_el(LSC );
		   while ((current_el != NULL) && (current_el !=NULL)){//cas NULL peut etre si aucun el n'a ete ajoute dans la LSC a l'etape du break.
		      if (image[trans_pixel(current_el->pixel,imageprop)] >0){
			image[trans_pixel(current_el->pixel,imageprop)] += threshold/2;
		      };
		      if (image[trans_pixel(current_el->pixel,imageprop)] <0){
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold/2;
		      };
		      current_el=next_el(LSC );
		   };
		};
	} else {
		lastprocessed=LSC ->previous;
		current_el=first_el(LSC );
// 		if (lastLSC != NULL){
		   while (current_el != lastprocessed){
		      if (image[trans_pixel(current_el->pixel,imageprop)] >0){
			image[trans_pixel(current_el->pixel,imageprop)] += threshold/2;
		      };
		      if (image[trans_pixel(current_el->pixel,imageprop)] <0){
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold/2;
		      };
		      current_el=next_el(LSC );
		   };
		      if (image[trans_pixel(current_el->pixel,imageprop)] >0){//current_el = lastprocessed donc necessairement different de NULL
			image[trans_pixel(current_el->pixel,imageprop)] += threshold/2;
		      };
		      if (image[trans_pixel(current_el->pixel,imageprop)] <0){
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold/2;
		      };
		   current_el=next_el(LSC );
		   if (lastLSC != lastprocessed){//attention, un cas particulier
			while ((current_el != lastLSC) && (current_el !=NULL)){//le cas NULL arrive si on s'est arrete pile a la fin d'une etape, ex: decompression complete->NOPE
			if (image[trans_pixel(current_el->pixel,imageprop)] >0){
				image[trans_pixel(current_el->pixel,imageprop)] += threshold;
			};
			if (image[trans_pixel(current_el->pixel,imageprop)] <0){
				image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
			};
			current_el=next_el(LSC );
			};
			if (current_el !=NULL){
			if (image[trans_pixel(current_el->pixel,imageprop)] >0){
				image[trans_pixel(current_el->pixel,imageprop)] += threshold;
			};
			if (image[trans_pixel(current_el->pixel,imageprop)] <0){
				image[trans_pixel(current_el->pixel,imageprop)] -= threshold;
			};
			current_el=next_el(LSC );
			};
		   };
		   while ((current_el != NULL)&& (current_el !=NULL)){//pas sur que le cas NULL arrive, peut etre si aucun el n'a ete ajoute dans la LSC a cette etape
		      if (image[trans_pixel(current_el->pixel,imageprop)] >0){
			image[trans_pixel(current_el->pixel,imageprop)] += threshold/2;
		      };
		      if (image[trans_pixel(current_el->pixel,imageprop)] <0){
			image[trans_pixel(current_el->pixel,imageprop)] -= threshold/2;
		      };
		      current_el=next_el(LSC );
		   };
// 		};
	};

};
// };
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

