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


/*********************************
***          CODER             ***
**********************************/


int spiht_code_ra5(long int *image, struct stream_struct streamstruct, long int *outputsize, struct coder_param_struct coder_param){

// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
int nbmin=imageprop.nbmin;

// struct list_struct * LSC[NRES];
// struct list_struct * LIC[NRES];
// struct list_struct * LIS[NRES];
struct list_struct ** LSC = (struct list_struct **) calloc(imageprop.nres, sizeof(struct list_struct *));
struct list_struct ** LIC = (struct list_struct **) calloc(imageprop.nres, sizeof(struct list_struct *));
struct list_struct ** LIS = (struct list_struct **) calloc(imageprop.nres, sizeof(struct list_struct *));
struct pixel_struct pixel;

struct list_el * el=NULL;
struct list_el * current_el=NULL;
struct list_el * tmp_el=NULL;
struct list_el * current_child=NULL;
struct list_el * lastLIScurrentthres=NULL;

struct list_struct * list_desc=NULL;
struct list_struct * list_grand_desc=NULL;

long int value_pix=0;
unsigned char bit=255;
unsigned char bitsig=255;
int r=0;
int ngrandchild=0;
// unsigned char * count = (unsigned char *) malloc(sizeof(unsigned char));
// long int *streamlast = (long int *) malloc(sizeof(long int));
unsigned char * count;
long int *streamlast;
unsigned char * stream;
long int threshold=0;
int thres_ind=0;
int i,j,k;
int iloc,jloc,kloc;
int niloc, njloc, nkloc;
int ii,ji,ki;
int ie,je,ke;
int blockind;
int nblock;

long long int ratebefore=0;
		
long int npix=nsmax*nbmax*nlmax;

int res=0;
int islastloop=0;
int * flagfirst = (int *) malloc (sizeof(int));


//debit-distortion
long long int dist=0;
float alpha, beta, rate_i, rate_e;
struct datablock_struct * datablock;
float lambda=0.0;

int maxres=imageprop.nres;
#ifdef RES_SCAL
int maxresspec=imageprop.nresspec;
int maxresspat=imageprop.nresspat;
int resspat=0;
int resspec=0;
#endif	

#ifdef NOLISTB
int is_accessible=0;
struct parents_struct parents;
#endif

#ifdef DEBUG
long int nLICloop;
long int nLSCloop;
long int nLISloop;
long int nLISloopA;
long int nLISloopB;
#endif


unsigned char *map_LSC = NULL;
#ifndef NEWTREE
unsigned char *map_LIC = NULL;
unsigned char *map_LIS = NULL;

map_LSC=(unsigned char *) calloc(nsmax*nbmax*nlmax,sizeof(unsigned char));
map_LIC= (unsigned char *) calloc(nsmax*nbmax*nlmax,sizeof(unsigned char));
map_LIS= (unsigned char *) calloc(nsmax*nbmax*nlmax,sizeof(unsigned char));
#endif

// *count=0;
// *streamlast=0;
stream = streamstruct.stream;
count = streamstruct.count;
streamlast = streamstruct.streamlast;


for (i=0; i<maxres; i++){
LSC[i] = list_init();
LIC[i] = list_init();
LIS[i] = list_init();
}


//This is just to get NUMRD point with exp spacing between rate_i and rate_e
rate_i = 200.;
// rate_i = 1.;
rate_e = 3000000.;
alpha = expf(1./NUMRD * logf(rate_e/rate_i));
beta = NUMRD * logf(rate_i) / logf(rate_e/rate_i);

for (i=0;i<imageprop.maxquant-1;i++){
   outputsize[i]=0;
}


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
#ifdef RES_RATE
	for (j=0;j<imageprop.nres;j++){
	(datablock[blockind].rddata[j]).ptcourant=0;
	for (i=0;i<NUMRD;i++){
// 		(((datablock[blockind]).rddata[j]).reval)[i]= (long long int) powf(alpha, beta+i);
		(((datablock[blockind]).rddata[j]).reval)[i]= 0; //we want to keep one cutting point per threshold i from 0 to maxquant, the rest won't be used...
		(((datablock[blockind]).rddata[j]).r)[i]=0; //faster to use separate loops for memory access ?
		(((datablock[blockind]).rddata[j]).d)[i]=0;
	}
	}
#else
	datablock[blockind].rddata.ptcourant=0;
	for (i=0;i<NUMRD;i++){
		((datablock[blockind]).rddata.reval)[i]= (long long int) powf(alpha, beta+i);
		((datablock[blockind]).rddata.r)[i]=0; //faster to use separate loops for memory access ?
		((datablock[blockind]).rddata.d)[i]=0;
	}	
#endif
      }
   }
}


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
	pixel.x=i;
	pixel.y=j;
	pixel.l=k;
	el=el_init(pixel);
	el->thres=(coder_param.maxquant[blockind])+1;
	insert_el(LIC[0],el);
#ifndef NEWTREE
	(map_LIC[trans_pixel(pixel)])++;
#endif
	if ((i % 2) || (j % 2) || (k % 2) || (((nbmin % 2) == 1) && (k == nbmin-1) && (k!=0))){ 
		el=el_init(pixel);
		el->thres=(coder_param.maxquant[blockind])+1;
		insert_el(LIS[0],el);
#ifndef NEWTREE
		(map_LIS[trans_pixel(pixel)])++;
#endif
	 };
      };
   };
};


//the distance is processed once for each block then updated for tracking

// dist = eval_dist_grp(iloc, jloc, kloc, image, (coder_param.maxquant[blockind])+1);
dist = 0; //en fait on n'a pas besoin de connaitre la valeur initiale...


//SPIHT 2)
#ifdef RES_SCAL
for (res=0; res<(coder_param.maxres[blockind]); res++){	
nbitswritten=0;
#ifdef SPATFIRST
resspat=res % maxresspat;
resspec=res / maxresspat;
#else
resspat=res / maxresspec;
resspec=res % maxresspec;
#endif


if( (resspat < coder_param.maxresspat[blockind]) &&  (resspec < coder_param.maxresspec[blockind])){
#ifdef RES_RATE
dist=0;
ratebefore= *(datablock[blockind].streamlast)*8+*(datablock[blockind].count);
#endif
for (thres_ind=(coder_param.maxquant[blockind]); thres_ind >= (coder_param.minquant[blockind]); thres_ind--){	
#endif

#ifndef RES_SCAL
for (thres_ind=(coder_param.maxquant[blockind]); thres_ind >= (coder_param.minquant[blockind]); thres_ind--){	
nbitswritten=0;
for (res=0; res<(coder_param.maxres[blockind]); res++){	
#endif


threshold= 1 << (long int)thres_ind;

#ifdef DEBUG
printf("Processing for thres_ind %d (threshold: %ld) at resolution %d...\n",thres_ind, threshold, res);
#endif

#ifdef DEBUG
nLSCloop=0;
nLICloop=0;
nLISloop=0;
nLISloopA=0;
nLISloopB=0;
#endif


/*******************************
LIC
********************************/
current_el=first_el(LSC[res]); //right positionning for adding from LIC
while ((current_el != NULL) && (current_el->thres > thres_ind)){
   	current_el=next_el(LSC[res]);
}
if ((current_el != NULL) && (current_el->thres <= thres_ind)) {
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

while ((current_el != NULL) && (current_el->thres > thres_ind)){
#ifdef DEBUG
   nLICloop++;
#endif
   value_pix=image[trans_pixel(current_el->pixel)];
   bit = get_bit(value_pix, thres_ind);	
   add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));//SPIHT 2.1.1)
  if (bit == 1) { //SPIHT 2.1.2)
     bitsig = (value_pix > 0);
     add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bitsig, (datablock[blockind].streamlast));
     update_dist(current_el->pixel, thres_ind, &dist, image);
#ifdef RES_RATE
#else
     add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
#endif

#ifndef NEWTREE
     (map_LIC[trans_pixel(current_el->pixel)])--;
     (map_LSC[trans_pixel(current_el->pixel)])++;
#endif
     remove_current_el(LIC[res]);
     insert_el_inplace(LSC[res], current_el);
	if (LSC[res]->current == NULL){
		LSC[res]->current=LSC[res]->first;
	}else {
		next_el(LSC[res]);
	}
	current_el->thres=thres_ind;
     current_el = LIC[res]->current;//est passe au suivant dans le move
     
   } else {
      current_el=next_el(LIC[res]);
   };
}

/*******************************
LSC before LIS
********************************/
#ifdef LSCBEFORE
current_el=first_el(LSC[res]);

while ((current_el != NULL) && (current_el->thres > thres_ind)){
#ifdef DEBUG
	nLSCloop++;
#endif
	value_pix=image[trans_pixel(current_el->pixel)];
	bit = get_bit(value_pix, thres_ind);
	add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));
	if (bit ==1) {
		update_dist(current_el->pixel, thres_ind, &dist, image);
#ifdef RES_RATE
#else
		add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
#endif
	}
	current_el=next_el(LSC[res]);
};

#ifdef RES_RATE
add_to_rddata(&((datablock[blockind].rddata)[res]), *(datablock[blockind].streamlast)*8+*(datablock[blockind].count)-ratebefore, dist);//we want to keep one point per threshold.
#endif

#endif

// 
// ;for each entry in the LIS
/*******************************
LIS
********************************/
current_el=first_el(LIS[res]); //right positionning for adding from LIS
while ((current_el != NULL) && (current_el->thres >= thres_ind)){
	current_el=next_el(LIS[res]);
}
if ((current_el != NULL) && (current_el->thres < thres_ind)) {
	LIS[res]->current = LIS[res]->previous;
}
if (current_el == NULL){
//si on arrive la, ca veut dire que soit la liste est vide soit on est alle jusqu'a la fin
	LIS[res]->current = LIS[res]->last;
}
lastLIScurrentthres=LIS[res]->current;

current_el=first_el(LIS[res]);
islastloop=0;
// printf("LIS processing \n");

while ((current_el != NULL) && (current_el->thres >= thres_ind)){ //SPIHT 2.2)
#ifdef DEBUG
nLISloop++;
#endif
#ifndef NOLISTB
    if (current_el->type == 0) { //SPIHT 2.2.1);if the set is of type A
#endif
#ifdef DEBUG
        nLISloopA++;
#endif

	r=spat_spec_desc_spiht(current_el->pixel, /*tmp_list*/list_desc, 0, image, thres_ind, map_LSC);
	
	bit = (r == -1);//il y a au moins un des descendants qui est significatif
        add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast)); //SPIHT 2.2.1.1
        if (bit == 1) { //SPIHT 2.2.1.2
	   list_desc=list_init();//list_free(list_desc);//TODO faire juste un nettoyage de la liste sans la liberer pour eviter un malloc (voir list_free plus bas) done
	   r=spat_spec_desc_spiht(current_el->pixel, list_desc, 1, image, thres_ind, map_LSC);//verifier qu'on va jusqu'au bout avec le onlychild a 1
	   current_child = first_el(list_desc);
 	   ngrandchild = 0;
	   list_grand_desc=list_init();
	   while (current_child !=NULL){ //SPIHT 2.2.1.2.1
#ifndef NEWTREE
	      if ((map_LSC[trans_pixel(current_child->pixel)] == 0) && (map_LIC[trans_pixel(current_child->pixel)] == 0)){ 
#endif
        	value_pix=image[trans_pixel(current_child->pixel)]; 
		bit = get_bit(value_pix, thres_ind);	
		add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast)); //SPIHT 2.2.1.2.1.1
		if (bit == 0){ //SPIHT 2.2.1.2.1.3
#ifndef NEWTREE
 			(map_LIC[trans_pixel(current_child->pixel)])++;
#endif
			el=el_init(current_child->pixel);
			el->thres = thres_ind;
#ifdef RES_SCAL
			if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
#ifdef SPATFIRST
				insert_el(LIC[res+1],el);
#else
				insert_el(LIC[res+maxresspec],el);
#endif
			} else {//this is a spectral children
#ifdef SPATFIRST
				insert_el(LIC[res+maxresspat],el);
#else
				insert_el(LIC[res+1],el);
#endif			
			}
#else
			insert_el(LIC[res+1],el);
#endif
		} else { //SPIHT 2.2.1.2.1.2
			bitsig = (value_pix > 0);
			add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bitsig, (datablock[blockind].streamlast));
			update_dist(current_child->pixel, thres_ind, &dist, image);
#ifdef RES_RATE
#else
			add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
#endif
#ifndef NEWTREE
			(map_LSC[trans_pixel(current_child->pixel)])++;
#endif
			el=el_init(current_child->pixel);
			el->thres = thres_ind;
#ifdef RES_SCAL
			if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
				#ifdef SPATFIRST
				insert_el(LSC[res+1],el);
				#else
				insert_el(LSC[res+maxresspec],el);
				#endif
			} else {//this is a spectral children
				#ifdef SPATFIRST
				insert_el(LSC[res+maxresspat],el);
				#else
				insert_el(LSC[res+1],el);
				#endif				
			}
#else			
			insert_el(LSC[res+1],el);
#endif			
		};
#ifndef NOLISTB
		r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
		if (r != 0) ngrandchild++;
#else
#ifndef NEWTREE
	 if (map_LIS[trans_pixel(current_child->pixel)] == 0){
#endif
	  r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
	  if (r != 0) {	 
#ifndef NEWTREE
	 	if (bit == 1) { 
#endif
		el=el_init(current_child->pixel);
		el->thres = thres_ind;

#ifdef RES_SCAL
		if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
#ifdef SPATFIRST
				insert_el(LIS[res+1],el);
#else
				insert_el(LIS[res+maxresspec],el);
#endif
		} else {//this is a spectral children
#ifdef SPATFIRST
				insert_el(LIS[res+maxresspat],el);
#else
				insert_el(LIS[res+1],el);
#endif				
		}
#else
		insert_el(LIS[res+1],el);
#endif
#ifndef NEWTREE
		(map_LIS[trans_pixel(el->pixel)])++;
#endif
#ifndef NEWTREE
		} else {//si on met k,l dans la LIC on verifie si il est accessible
			is_accessible=0;
			parents=find_parents(current_child->pixel);
			if ((parents.spat.x != current_el->pixel.x) ||
				(parents.spat.y != current_el->pixel.y) ||
				(parents.spat.l != current_el->pixel.l) ){//on s'arrure qu'on n'est pas arrive par le spatial
			is_accessible=is_accessible_from(parents.spat,map_LIS);
			}
			if ((parents.spec.x != current_el->pixel.x) ||
				(parents.spec.y != current_el->pixel.y) ||
				(parents.spec.l != current_el->pixel.l) ){//on s'arrure qu'on n'est pas arrive par le spatial
			if (is_accessible ==1) printf("********** UH OH*******\n");
			is_accessible=is_accessible_from(parents.spec,map_LIS);
			}
			if (is_accessible ==0){
				el=el_init(current_child->pixel);
				el->thres = thres_ind;
				#ifndef INPLACE
				insert_el(LIS[res+1],el);
				#else
				insert_el_inplace(LIS[res+1],el);
				#endif
#ifndef NEWTREE
				(map_LIS[trans_pixel(el->pixel)])++;
#endif
			}
		}
#endif
		
	  }
#ifndef NEWTREE
	}
#endif
#endif

#ifndef NEWTREE
	      };//end of if on maps
#endif
	      current_child=next_el(list_desc);
	   };

	   list_free(list_grand_desc);
	   list_free(list_desc);//TODO a modifier pour eviter un malloc
#ifndef NOLISTB
	   if (ngrandchild > 0){//SPIHT 2.2.1.2.2
	      if (current_el != lastLIScurrentthres){
		tmp_el=remove_current_el(LIS[res]); //il ne faut pas liberer la memoire ici
		insert_el_after(LIS[res],current_el,&lastLIScurrentthres); //Attention, l'ordre des operations est critique ici
	      }

		current_el->type=1; // on ne peut pas le rajouter à la fin avant de le supprimer du milieu
	   } else {
#endif
		tmp_el=remove_current_el(LIS[res]);
#ifndef NEWTREE		
		(map_LIS[trans_pixel(tmp_el->pixel)])--;
#endif
		free(tmp_el); //la par contre, il disparait...
		
#ifndef NOLISTB
	   };
#endif
	   tmp_el=NULL;
	}else{//fin du 2.2.1.2
	   current_el=next_el(LIS[res]);
	};
#ifndef NOLISTB
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
#ifndef NEWTREE
		if(map_LIS[trans_pixel(current_child->pixel)] == 0){//attention, ca c'est fin... (i512)
#endif
		   r=spat_spec_desc_spiht(current_child->pixel, /*tmp_list*/list_desc, 0, image, thres_ind, map_LSC);
		   if (r == -1) {bit = 1;};
#ifndef NEWTREE
		};
#endif
		current_child=next_el(list_desc);
	 };
	 add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));

	 if (bit == 1){
		current_child = first_el(list_desc);
		while (current_child !=NULL){
#ifndef NEWTREE
			if (map_LIS[trans_pixel(current_child->pixel)] == 0){
#endif
				el=el_init(current_child->pixel); //TODO possibilite de recuperer direct sans faire de malloc..
				el->thres = thres_ind;
						
#ifdef RES_SCAL
//WARNING WARNING WARNING ne pas utiliser insert_el_after
				if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
					#ifdef SPATFIRST
					insert_el(LIS[res+1],el);
					#else
					insert_el(LIS[res+maxresspec],el);
					#endif
				} else {//this is a spectral children
					#ifdef SPATFIRST
					insert_el(LIS[res+maxresspat],el);
					#else
					insert_el(LIS[res+1],el);
					#endif				
				}
#else 
				insert_el(LIS[res+1],el);
#endif
#ifndef NEWTREE
				(map_LIS[trans_pixel(el->pixel)])++;
			};
#endif
			current_child=next_el(list_desc);
		};
	  	tmp_el=remove_current_el(LIS[res]);
#ifndef NEWTREE
		(map_LIS[trans_pixel(tmp_el->pixel)])--;
#endif
	  	free(tmp_el);
							
	  } else {
	    current_el=next_el(LIS[res]);
	  };
	  list_free(list_desc); //possibilite de les recuperer direct... (pareil dans 2.2.1.2.1)

    };
#endif
current_el=LIS[res]->current;
}; //endwhile
// };

/*******************************
LSC
********************************/
#ifndef LSCBEFORE
current_el=first_el(LSC[res]);

while ((current_el != NULL) && (current_el->thres > thres_ind)){
#ifdef DEBUG
	nLSCloop++;
#endif
	value_pix=image[trans_pixel(current_el->pixel)];
	bit = get_bit(value_pix, thres_ind);
	add_to_stream((datablock[blockind].stream), (datablock[blockind].count), (int) bit, (datablock[blockind].streamlast));
	if (bit ==1) {
		update_dist(current_el->pixel, thres_ind, &dist, image);
#ifdef RES_RATE
#else
		add_to_rddata(&datablock[blockind].rddata, *(datablock[blockind].streamlast)*8+*(datablock[blockind].count), dist);
#endif
	}
	current_el=next_el(LSC[res]);
};

#ifdef RES_RATE
add_to_rddata(&((datablock[blockind].rddata)[res]), *(datablock[blockind].streamlast)*8+*(datablock[blockind].count)-ratebefore, dist);//we want to keep one point per threshold.
#endif

#endif

/********* END LSC  ************/




#ifndef RES_SCAL
};//fin resolution
(datablock[blockind].partsize)[thres_ind] = nbitswritten;
#endif

//is it still correct ?
outputsize[thres_ind] += (*(datablock[blockind].streamlast))*8 + (*(datablock[blockind].count));
};//fin threshold

#ifdef RES_SCAL
}//end of if on resspat and resspec
(datablock[blockind].partsize)[res] = nbitswritten;
#ifdef RES_RATE
// (datablock[blockind].rddata[res].r)[NUMRD-1]=(*(datablock[blockind].streamlast))*8 + (*(datablock[blockind].count));
// (datablock[blockind].rddata[res].d)[NUMRD-1]=dist;
#endif
};//fin resolution
#endif


#ifndef RES_RATE
(datablock[blockind].rddata.r)[NUMRD-1]=(*(datablock[blockind].streamlast))*8 + (*(datablock[blockind].count));//CHANGE 14-03-06 parenth
(datablock[blockind].rddata.d)[NUMRD-1]=dist;
#endif

for (i=0; i<imageprop.nres; i++){
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

// #ifdef SIZE
// 
// #ifdef RES_SCAL
// for (i=0;i<imageprop.nres;i++){
// #else
// for (i=0;i<imageprop.maxquant+1;i++){
// #endif
// 
// 
// 	printf("%ld, ",(datablock[blockind].partsize)[i]);
// 
// }
// printf("\n");
// #endif	

//test
// if (blockind ==136){
// 	for (i=0;i<imageprop.nres;i++){
// 	printf("%ld, ",(datablock[blockind].partsize)[i]);
// 	}
// 	printf("\n");
// }
//fin test


}// Fin du 
}// parcours 
}// en bloc


*flagfirst = 1; //this command the output of the secondary header for the first layer

// lambda=compute_lambda(datablock, 0.5*(npix/64), nblock);
// lambda=compute_lambda(datablock, 0.5*npix, nblock);
// printf("Interleaving for lambda=%f\n",lambda);
// interleavingblocks(datablock, nblock, stream, count, streamlast, lambda);
// 
// #ifdef DEBUG2
// printf("Stream size: %ld \n",*streamlast);
// printf("count:       %uc \n",*count);
// printf("Size in bit: %ld \n", *streamlast*8+*count);
// printf("Rate: %f \n", (float) (*streamlast*8+*count)/((long int) nsmax*nlmax*nbmax));
// #endif

nbitswritten=0;// to count the total (all are rewritten)
if (coder_param.rate != 0.0){

	lambda=compute_lambda(datablock, coder_param.rate*npix, nblock);
	printf("Interleaving for lambda=%f\n",lambda);
	interleavingblocks(datablock, nblock, stream, count, streamlast, lambda, flagfirst);
	
	#ifdef DEBUG2
	printf("Stream size: %ld \n",*streamlast);
	printf("count:       %uc \n",*count);
	printf("Size in bit: %ld \n", *streamlast*8+*count);
	printf("Rate: %f \n", (float) (*streamlast*8+*count)/((long int) nsmax*nlmax*nbmax));
	#endif
} else {
	printf("Interleaving for lambda=0 (lossless)\n");
	lambda=0.;
	interleavingblocks(datablock, nblock, stream, count, streamlast, lambda, flagfirst);
			
	#ifdef DEBUG2
	printf("Stream size: %ld \n",*streamlast);
	printf("count:       %uc \n",*count);
	printf("Size in bit: %ld \n", *streamlast*8+*count);
	printf("Rate: %f \n", (float) (*streamlast*8+*count)/((long int) nsmax*nlmax*nbmax));
	#endif		
		
}

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


// lambda=compute_lambda(datablock, 2.0*npix, nblock);
// printf("Interleaving for lambda=%f\n",lambda);
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
// printf("Interleaving for lambda=0 (lossless)\n");
// lambda=0.;
// interleavingblocks(datablock, nblock, stream, count, streamlast, lambda, flagfirst);
// 
// #ifdef DEBUG2
// printf("Stream size: %ld \n",*streamlast);
// printf("count:       %uc \n",*count);
// printf("Size in bit: %ld \n", *streamlast*8+*count);
// printf("Rate: %f \n", (float) (*streamlast*8+*count)/((long int) nsmax*nlmax*nbmax));
// #endif

//WARNING no more full size...
*outputsize = *streamlast*8+*count;


#ifndef NEWTREE
free(map_LSC);
free(map_LIC);
free(map_LIS);
#endif

//do not forget to free each block separatly first
for (i=0;i<nblock;i++){
	datablock_free(&(datablock[i]));
}

free(datablock);

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



/*********************************
***         DECODER            ***
**********************************/

int spiht_decode_ra5(long int *image, struct stream_struct streamstruct, long int *outputsize, struct coder_param_struct coder_param){

// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};

// int nsmax=imageprop.nsmax;
// int nlmax=imageprop.nlmax;
// int nbmax=imageprop.nbmax;
int nsmin=imageprop.nsmin;
int nlmin=imageprop.nlmin;
int nbmin=imageprop.nbmin;

// struct list_struct * LSC[NRES];
// struct list_struct * LIC[NRES];
// struct list_struct * LIS[NRES];
struct list_struct ** LSC = (struct list_struct **) calloc(imageprop.nres, sizeof(struct list_struct *));
struct list_struct ** LIC = (struct list_struct **) calloc(imageprop.nres, sizeof(struct list_struct *));
struct list_struct ** LIS = (struct list_struct **) calloc(imageprop.nres, sizeof(struct list_struct *));
struct pixel_struct pixel;
struct parents_struct parents;
int is_accessible=0;


struct list_el * el=NULL;
struct list_el * current_el=NULL;
struct list_el * tmp_el=NULL;
struct list_el * current_child=NULL;
struct list_el * lastLIScurrentthres=NULL;



struct list_el * lastprocessed=NULL;

struct list_struct * list_desc=NULL;
struct list_struct * list_grand_desc=NULL;
unsigned char bit=255;
unsigned char bitsig=255;
int r=0;
int ngrandchild=0;
// unsigned char * count = (unsigned char *) malloc(sizeof(unsigned char));
// long int *streamlast = (long int *) malloc(sizeof(long int));
unsigned char * stream;
unsigned char * count;
long int *streamlast;

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

long int endpos=0;

long int err=0;
long int maxerr=0;

int minquant=0; //curieux qu'on ait besoin de ca... a verifier...

// #ifdef RES_SCAL
// int maxresspat=6;//WARNING WARNING WARNING
// int maxresspec=6;
// //int maxresspat=4;
// //int maxresspec=4;
// int maxres=maxresspat*maxresspec;//attention, influence non verifiee, specialement sur for...
// #else
// int maxres=imageprop.nres;
// #endif

 #ifdef RES_SCAL
int maxresspec=imageprop.nresspec;
int maxresspat=imageprop.nresspat;
#endif
int maxres=imageprop.nres;	
	

int lastres=0;
int flagLSClastres=0;
int LSCprocessed = 0;

#ifdef CHECKEND
int flagLIS = 0;
#endif

struct datablock_struct * datablock;

int res=0;
#ifdef RES_SCAL
int resspat=0;
int resspec=0;
#endif

#ifdef DEBUG
long int nLICloop;
long int nLSCloop;
long int nLISloop;
long int nLISloopA;
long int nLISloopB;
#endif

char flagLSC=0;

unsigned char *map_LSC = NULL;
#ifndef NEWTREE
unsigned char *map_LIC = NULL;
unsigned char *map_LIS = NULL;

map_LSC=(unsigned char *) calloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax,sizeof(unsigned char));
map_LIC= (unsigned char *) calloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax,sizeof(unsigned char));
map_LIS= (unsigned char *) calloc(imageprop.nsmax*imageprop.nbmax*imageprop.nlmax,sizeof(unsigned char));
#endif

// *count=0;
// *streamlast=0;
stream = streamstruct.stream;
count = streamstruct.count;
streamlast = streamstruct.streamlast;

nbitsread=0;//warning, we put it back to zero later
nbitsreadheader=0;

for (i=0;i<maxres;i++){//warning, have to be correct with maxresspat et maxresspec
LSC[i] = list_init();
LIC[i] = list_init();
LIS[i] = list_init();
}

#ifndef NEWTREE
for (i_l=0;i_l<imageprop.nsmax*imageprop.nlmax*imageprop.nbmax;i_l++)
{
	map_LSC[i_l]=0;
	map_LIC[i_l]=0;
	map_LIS[i_l]=0;
}
#endif


niloc=(nsmin+1)/2;
njloc=(nlmin+1)/2;
nkloc=(nbmin+1)/2;
nblock=niloc* njloc* nkloc;

datablock=(struct datablock_struct *) malloc(nblock* sizeof(struct datablock_struct));

for (i=0; i<nblock; i++){
	datablock_init(&(datablock[i]));
}

//Desinterleaving stream
err=desinterleavingblocks(datablock, nblock, streamstruct, *outputsize, coder_param.nlayer);//carefull for the bit counting later...
if (err) {fprintf(stderr, "******** ERROR desinterleavingblocks (truncation)\n");};

//test
// for (i=0; i<coder_param.maxres[136]; i++){
// 	printf("%ld  ",datablock[136].partsize[i]);
// }
// printf("\n");
//fin test


for (i=0; i<nblock; i++){
	datablock[i].currentpos=(*(datablock[i].streamlast))*8+ *(datablock[i].count);
	*(datablock[i].streamlast)=0;
	*(datablock[i].count)=0;
}

nbitsread=0;

//******************************************
//Parcours des differentes localisations
//******************************************

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
	pixel.x=i;
	pixel.y=j;
	pixel.l=k;
	el=el_init(pixel);
	el->thres=(coder_param.maxquant[blockind])+1;
	insert_el(LIC[0],el);
#ifndef NEWTREE
	(map_LIC[trans_pixel(pixel)])++;
#endif
	if ((i % 2) || (j % 2) || (k % 2) || (((nbmin % 2) == 1) && (k == nbmin-1) && (k!=0))){ 
		el=el_init(pixel);
		el->thres=(coder_param.maxquant[blockind])+1;
		insert_el(LIS[0],el);
#ifndef NEWTREE
		(map_LIS[trans_pixel(pixel)])++;
#endif
	 };
      };
   };
};

endpos =0;

#ifdef RES_SCAL
for (res=0; res<(coder_param.maxres[blockind]); res++){	
	
#ifdef SPATFIRST
resspat=res % maxresspat;
resspec=res / maxresspat;
#else
resspat=res / maxresspec;
resspec=res % maxresspec;
#endif
#ifdef DEBUG
printf("Processing for res %d ...\n",res);
#endif	
if( (resspat < coder_param.maxresspat[blockind]) &&  (resspec < coder_param.maxresspec[blockind])){
	
// 	for (thres_ind=(coder_param.maxquant[blockind]); thres_ind >= minquant; thres_ind--){	
		for (thres_ind=(coder_param.maxquant[blockind]); thres_ind >= (coder_param.minquant[blockind]); thres_ind--){

#endif
//SPIHT 2)


#ifndef RES_SCAL
for (thres_ind=(coder_param.maxquant[blockind]); thres_ind >= (coder_param.minquant[blockind]); thres_ind--){	
	if (thres_ind>10){
		maxres =imageprop.nres; //WARNING valid ONLY if encoding was full !!!
	}else{
		maxres=(coder_param.maxres[blockind]);
	}
#ifdef DEBUG
printf("Processing for thres_ind %d (threshold: %ld)...\n",thres_ind, threshold);
#endif
	for (res=0; res<maxres; res++){

#endif


threshold= 1 << (long int)thres_ind;


flagLSC=0;
#ifdef CHECKEND
flagLIS=0;
#endif

#ifdef DEBUG
nLSCloop=0;
nLICloop=0;
nLISloop=0;
nLISloopA=0;
nLISloopB=0;
#endif

/*******************************
LIC
********************************/


// Used for Significant pass
current_el=first_el(LSC[res]); //right positionning for adding from LIC
while ((current_el != NULL) && (current_el->thres > thres_ind)){
   	current_el=next_el(LSC[res]);
}
if ((current_el != NULL) && (current_el->thres <= thres_ind)) {
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
while ((current_el != NULL) && (current_el->thres > thres_ind) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize)){
// printf(".");
#ifdef DEBUG
   nLICloop++;
#endif

   if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
      bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
   } else {
	   current_el=first_el(LSC[res]); //correct positionning for the final correction
	   break;
   }

  if (bit == 1) { //SPIHT 2.1.2)
     if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
        bitsig = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
     } else{
	     current_el=first_el(LSC[res]); //correct positionning for the final correction
	     break;
     }
     image[trans_pixel(current_el->pixel)] += threshold;
     if (bitsig == 0) {
     	image[trans_pixel(current_el->pixel)] = -image[trans_pixel(current_el->pixel)];
     };
#ifndef NEWTREE
     (map_LIC[trans_pixel(current_el->pixel)])--;
     (map_LSC[trans_pixel(current_el->pixel)])++;
#endif
     remove_current_el(LIC[res]);
     insert_el_inplace(LSC[res], current_el);
	if (LSC[res]->current == NULL){
		LSC[res]->current=LSC[res]->first;
	}else {
		next_el(LSC[res]);
	}

     current_el->thres = thres_ind;
     current_el = LIC[res]->current;//est passe au suivant dans le move
   } else {
      current_el=next_el(LIC[res]);
   };
}

/*******************************
LSC before LIS
********************************/
#ifdef LSCBEFORE
current_el=first_el(LSC[res]);
while ((current_el != NULL) && (current_el->thres > thres_ind) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize)){
#ifdef DEBUG
	nLSCloop++;
#endif
	if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
	bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
	flagLSC=1;
	} else break;
	if (bit == 1){
		if (image[trans_pixel(current_el->pixel)] > 0){
			image[trans_pixel(current_el->pixel)] += threshold;
		} else {
			image[trans_pixel(current_el->pixel)] -= threshold;
		};
	};
	current_el=next_el(LSC[res]);
};
#endif

/*******************************
LIS
********************************/
#ifdef LSCBEFORE
#ifdef RES_SCAL
if( (thres_ind == coder_param.minquant[blockind]) && 
     ((resspat == coder_param.maxresspat[blockind]-1) && (coder_param.maxresspat[blockind] !=1 )) ){//TODO check that give a gain
#ifdef DEBUG
	printf("Skipping LIS for res %d thres %d (spec: %d, spat: %d)\n", res, thres_ind, resspec, resspat);	
#endif
}else 
#endif  
#endif
{	

// if( (resspat != coder_param.maxresspat[blockind]-1) 
//       ||  (resspec != coder_param.maxresspec[blockind]-1)){//We want to process the LIS for this resolution only if this is not the last one
#ifdef CHECKEND
flagLIS=1;
#endif
	
// ;for each entry in the LIS
current_el=first_el(LIS[res]); //right positionning for adding from LIS
while ((current_el != NULL) && (current_el->thres >= thres_ind)){
	current_el=next_el(LIS[res]);
}
if ((current_el != NULL) && (current_el->thres < thres_ind)) {
	LIS[res]->current = LIS[res]->previous;
}
if (current_el == NULL){
//si on arrive la, ca veut dire que soit la liste est vide soit on est alle jusqu'a la fin
	LIS[res]->current = LIS[res]->last;
}
lastLIScurrentthres=LIS[res]->current;


current_el=first_el(LIS[res]);
// printf("LIS processing \n");
while ((current_el != NULL)&& (current_el->thres >= thres_ind) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize)){ //SPIHT 2.2)
#ifdef DEBUG
nLISloop++;
#endif
#ifndef NOLISTB
    if (current_el->type == 0) { //SPIHT 2.2.1);if the set is of type A
#endif
#ifdef DEBUG
        nLISloopA++;
#endif
//          //SPIHT 2.2.1.1
        if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
	   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
        } else break;
        if (bit == 1) { //SPIHT 2.2.1.2
	   list_desc=list_init();//list_free(list_desc);//TODO faire juste un nettoyage de la liste sans la liberer pour eviter un malloc (voir list_free plus bas) done
	   r=spat_spec_desc_spiht(current_el->pixel, list_desc, 1, image, thres_ind, map_LSC);//verifier qu'on va jusqu'au bout avec le onlychild a 1
	   current_child = first_el(list_desc);
 	   ngrandchild = 0;
	   list_grand_desc=list_init();
	   while ((current_child !=NULL)&& ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize)){ //SPIHT 2.2.1.2.1
#ifndef NEWTREE
	      if ((map_LSC[trans_pixel(current_child->pixel)] == 0) && (map_LIC[trans_pixel(current_child->pixel)] == 0)){ 
#endif
		if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
		   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
		} else break;
		if (bit == 0){ //SPIHT 2.2.1.2.1.3
#ifndef NEWTREE
 			(map_LIC[trans_pixel(current_child->pixel)])++;
#endif
			el=el_init(current_child->pixel);
			el->thres=thres_ind;
#ifdef RES_SCAL
			if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
#ifdef SPATFIRST
				insert_el(LIC[res+1],el);
#else
				insert_el(LIC[res+maxresspec],el);
#endif
			} else {//this is a spectral children
#ifdef SPATFIRST
				insert_el(LIC[res+maxresspat],el);
#else
				insert_el(LIC[res+1],el);
#endif				
			}
#else
			insert_el(LIC[res+1],el);
#endif
		} else { //SPIHT 2.2.1.2.1.2
			if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
			   bitsig = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
			} else break;
			image[trans_pixel(current_child->pixel)] += threshold; //we don't want to add it UNLESS we have the sign
			if (bitsig == 0){
				image[trans_pixel(current_child->pixel)] = -image[trans_pixel(current_child->pixel)];
			};
#ifndef NEWTREE
			(map_LSC[trans_pixel(current_child->pixel)])++;
#endif
			el=el_init(current_child->pixel);
			el->thres=thres_ind;
#ifdef RES_SCAL
			if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
#ifdef SPATFIRST
				insert_el(LSC[res+1],el);
#else
				insert_el(LSC[res+maxresspec],el);
#endif
			} else {//this is a spectral children
#ifdef SPATFIRST
				insert_el(LSC[res+maxresspat],el);
#else
				insert_el(LSC[res+1],el);
#endif				
			}
#else
			insert_el(LSC[res+1],el);			
#endif
		};
#ifndef NOLISTB
		r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
		if (r != 0) ngrandchild++;
#else

#ifndef NEWTREE
	 if (map_LIS[trans_pixel(current_child->pixel)] == 0){
#endif
	  r=spat_spec_desc_spiht(current_child->pixel, list_grand_desc, 1, image, thres_ind, map_LSC);//pour L(x,y,l)
	  if (r != 0) {	 
#ifndef NEWTREE
	 	if (bit == 1) { 
#endif
		el=el_init(current_child->pixel);
		el->thres=thres_ind;
#ifdef RES_SCAL
		if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
#ifdef SPATFIRST
				insert_el(LIS[res+1],el);
#else
				insert_el(LIS[res+maxresspec],el);
#endif
		} else {//this is a spectral children
#ifdef SPATFIRST
				insert_el(LIS[res+maxresspat],el);
#else
				insert_el(LIS[res+1],el);
#endif				
		}
#else
		insert_el(LIS[res+1],el);
#endif

#ifndef NEWTREE
		(map_LIS[trans_pixel(el->pixel)])++;

		} else {//si on met k,l dans la LIC on verifie si il est accessible
			is_accessible=0;
			parents=find_parents(current_child->pixel);
			if ((parents.spat.x != current_el->pixel.x) ||
				(parents.spat.y != current_el->pixel.y) ||
				(parents.spat.l != current_el->pixel.l) ){//on s'arrure qu'on n'est pas arrive par le spatial
			is_accessible=is_accessible_from(parents.spat,map_LIS);
			}
			if ((parents.spec.x != current_el->pixel.x) ||
				(parents.spec.y != current_el->pixel.y) ||
				(parents.spec.l != current_el->pixel.l) ){//on s'arrure qu'on n'est pas arrive par le spatial
			if (is_accessible ==1) printf("********** UH OH*******\n");
			is_accessible=is_accessible_from(parents.spec,map_LIS);
			}
			if (is_accessible ==0){
				el=el_init(current_child->pixel);
				el->thres=thres_ind;
				(map_LIS[trans_pixel(el->pixel)])++;
			}
		}
#endif
		
	  }
#ifndef NEWTREE
	}
#endif
#endif
#ifndef NEWTREE
	      };//fin cdt on maps
#endif
	      current_child=next_el(list_desc);
	   };

	   list_free(list_grand_desc);
	   list_free(list_desc);//TODO a modifier pour eviter un malloc
#ifndef NOLISTB
	   if (ngrandchild > 0){//SPIHT 2.2.1.2.2
	      if (current_el != lastLIScurrentthres){
		tmp_el=remove_current_el(LIS[res]); //il ne faut pas liberer la memoire ici...	
		insert_el_after(LIS[res],current_el,&lastLIScurrentthres); //Attention, l'ordre des operations est critique ici
	      }

		current_el->type=1; // on ne peut pas le rajouter à la fin avant de le supprimer du milieu
	   } else {
#endif
		tmp_el=remove_current_el(LIS[res]);
#ifndef NEWTREE		
		(map_LIS[trans_pixel(tmp_el->pixel)])--;
#endif
		free(tmp_el); //la par contre, il disparait...
		
#ifndef NOLISTB
	   };
#endif
	   tmp_el=NULL;
	}else{//fin du 2.2.1.2
	   current_el=next_el(LIS[res]);
	};
#ifndef NOLISTB
	} else { //SPIHT 2.2.2 ;else the set is of type B
#ifdef DEBUG
	  nLISloopB++;
#endif
		//On doit faire l'output de Sn(L(i,j))
	  bit = 0;
	if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
	   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
	} else break;
	  if (bit == 1){
		list_desc=list_init();//list_free(list_desc);  //decode only done
	  	r=spat_spec_desc_spiht(current_el->pixel, list_desc, 1, image, thres_ind, map_LSC);//decode only
		current_child = first_el(list_desc);
		while (current_child !=NULL){
#ifndef NEWTREE		
			if (map_LIS[trans_pixel(current_child->pixel)] == 0){
#endif
				el=el_init(current_child->pixel); //TODO possibilite de recuperer direct sans faire de malloc..

				el->thres = thres_ind;
						
		#ifdef RES_SCAL
				if (el->pixel.l == current_el->pixel.l){//we are going down in spat resolution
					#ifdef SPATFIRST
					insert_el(LIS[res+1],el);
					#else
					insert_el(LIS[res+maxresspec],el);
					#endif
				} else {//this is a spectral children
					#ifdef SPATFIRST
					insert_el(LIS[res+maxresspat],el);
					#else
					insert_el(LIS[res+1],el);
					#endif				
				}
		#else 
				insert_el(LIS[res+1],el);
		#endif
#ifndef NEWTREE
				(map_LIS[trans_pixel(el->pixel)])++;
			};
#endif			
			current_child=next_el(list_desc);
		};
	  	tmp_el=remove_current_el(LIS[res]);
#ifndef NEWTREE
		(map_LIS[trans_pixel(tmp_el->pixel)])--;
#endif
	  	free(tmp_el);
		list_free(list_desc);//decode only
	  } else {
	    current_el=next_el(LIS[res]);
	  };

    };
#endif
current_el=LIS[res]->current;   
}; //endwhile
#ifdef RES_SCAL
}//; //endif on jump LIS if last resolution
// else{
// 	printf("Skipping LIS for res %d thres %d (spec: %d, spat: %d)\n", res, thres_ind, resspec, resspat);
// }
#endif

/*******************************
LSC
********************************/
#ifndef LSCBEFORE
current_el=first_el(LSC[res]);
while ((current_el != NULL) && (current_el->thres > thres_ind) && ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize)){
#ifdef DEBUG
	nLSCloop++;
#endif
	if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) < *outputsize){
	   bit = read_from_stream((datablock[blockind].stream), (datablock[blockind].count), (datablock[blockind].streamlast));
	   flagLSC=1;
	} else break;
	if (bit == 1){
		if (image[trans_pixel(current_el->pixel)] > 0){
			image[trans_pixel(current_el->pixel)] += threshold;
		} else {
			image[trans_pixel(current_el->pixel)] -= threshold;
		};
	};
	current_el=next_el(LSC[res]);
};
#endif

if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) >= *outputsize){
#ifdef DEBUG
 	printf("Sortie: fin du train de bit (threshold %ld)\n", threshold);
#endif
#ifdef CHECKEND
	 printf("Sortie: fin du train de bit (blk: %d) (threshold %ld, res %d, flagLSC:%d, flagLIS:%d)\n", blockind, threshold, res, flagLSC, flagLIS);
#endif
	break;
};

};//fin threshold


//we are between the internal and the external loop in any case (RES_SCAL or not) time to check if a jump is necessary (case of partial decoding)...
#ifdef DEBUG
printf("Before jump %ld %d\n",*(datablock[blockind].streamlast),*(datablock[blockind].count));
#endif
#ifdef RES_SCAL
}
// if (res >=3){
endpos += (datablock[blockind].partsize)[res];
*(datablock[blockind].streamlast) = endpos /8;
*(datablock[blockind].count) = endpos % 8;
// }
#else
if (thres_ind <= 10){
endpos += (datablock[blockind].partsize)[thres_ind];
*(datablock[blockind].streamlast) = endpos /8;
*(datablock[blockind].count) = endpos % 8;
}
#endif
#ifdef DEBUG
printf("After jump %ld %d\n",*(datablock[blockind].streamlast),*(datablock[blockind].count));
#endif

// if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) >= *outputsize){//le = pose probleme pour la derniere iteration
if ((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) > *outputsize){//temporaire, il faudrait faire plus fin
#ifdef DEBUG
 	printf("Sortie: fin du train de bit (threshold %ld)\n", threshold);
#endif
	break;
};

#ifndef RES_SCAL



};//fin res
#endif





//correction finale eventuelle 
//TODO: la sortie est calculee a l'octet pres, pas au bit

//WARNING the final correction is not always correct with the resolution scalability... to improve later...
#ifdef DEBUG
printf("Correction finale (flagLSC= %d)\n", flagLSC);
#endif

#ifndef RES_SCAL
lastres=res;
flagLSClastres=flagLSC;
for (res=0;res<(coder_param.maxres[blockind]
	       );res++){//Hum... comment gerer le LSC ici...

if (res <= lastres) { //Hum is it usefull ???
	flagLSC = flagLSClastres;
} else {
	flagLSC=0;
}

if (res <= lastres) {
	LSCprocessed = 1;
}else{
	LSCprocessed = 0;	
}
#endif

if (thres_ind <= (coder_param.minquant[blockind])) {//when we go out at the very end (thres_ind == -1)
	thres_ind=coder_param.minquant[blockind];
	flagLSC=0;
	LSCprocessed = 1; //correction 21-04-06
	threshold= 1 << (long int)thres_ind;
}
#ifdef CHECKEND
err=0; maxerr=0;
printf("res: %d\n", res);
#endif

if (((*(datablock[blockind].streamlast))*8+ (*(datablock[blockind].count)) >= *outputsize) || (thres_ind >0)){ //on est sorti car le train de bit etait trop court

	if (LSCprocessed){
		lastprocessed=LSC[res]->current;
		current_el=first_el(LSC[res]);
		   while (current_el != lastprocessed){
		      if (image[trans_pixel(current_el->pixel)] >0){
			image[trans_pixel(current_el->pixel)] += threshold/2;
		      };
		      if (image[trans_pixel(current_el->pixel)] <0){
			image[trans_pixel(current_el->pixel)] -= threshold/2;
		      };
#ifdef CHECKEND
			err= abs(image[trans_pixel(current_el->pixel)]-imageoriglobal[trans_pixel(current_el->pixel)]);
			if (err > maxerr) maxerr=err;
#endif		      
		      current_el=next_el(LSC[res]);
		   };
#ifdef CHECKEND
			printf("-> %ld\n", maxerr);maxerr=0;
#endif		   
		   while (current_el != NULL){
			if (current_el->thres > thres_ind){//this was added before and not refined yet, 
				if (image[trans_pixel(current_el->pixel)] >0){
					image[trans_pixel(current_el->pixel)] += threshold;
				};
				if (image[trans_pixel(current_el->pixel)] <0){
					image[trans_pixel(current_el->pixel)] -= threshold;
				};
			} else{
				if (image[trans_pixel(current_el->pixel)] >0){
					image[trans_pixel(current_el->pixel)] += threshold/2;
				};
				if (image[trans_pixel(current_el->pixel)] <0){
					image[trans_pixel(current_el->pixel)] -= threshold/2;
				};
			}
#ifdef CHECKEND
			err= abs(image[trans_pixel(current_el->pixel)]-imageoriglobal[trans_pixel(current_el->pixel)]);
			if (err > maxerr) maxerr=err;
#endif	
			current_el=next_el(LSC[res]);
		   }
#ifdef CHECKEND
			printf("--> %ld\n", maxerr);maxerr=0;
#endif		   
	} else {
		current_el=first_el(LSC[res]);
		while (current_el != NULL){
			if (current_el->thres == thres_ind){
				if (image[trans_pixel(current_el->pixel)] >0){
					image[trans_pixel(current_el->pixel)] += threshold/2;
				};
				if (image[trans_pixel(current_el->pixel)] <0){
					image[trans_pixel(current_el->pixel)] -= threshold/2;
				};
			} else {
				if (image[trans_pixel(current_el->pixel)] >0){
					image[trans_pixel(current_el->pixel)] += threshold;
				};
				if (image[trans_pixel(current_el->pixel)] <0){
					image[trans_pixel(current_el->pixel)] -= threshold;
				};	
			}
#ifdef CHECKEND
			err= abs(image[trans_pixel(current_el->pixel)]-imageoriglobal[trans_pixel(current_el->pixel)]);
			if (err > maxerr) maxerr=err;
#endif
			current_el=next_el(LSC[res]);
		};
#ifdef CHECKEND
			printf("---> %ld\n", maxerr);maxerr=0;
#endif
	}


};

#ifndef RES_SCAL
};//fin boucle for
#endif

#ifdef RES_SCAL
};//fin res
#endif

#ifdef CHECKEND
check_end(image, LSC, coder_param, blockind);
if (0) 1+2;
#endif


for (i=0; i<imageprop.nres; i++){
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

#ifndef NEWTREE
free(map_LSC);
free(map_LIC);
free(map_LIS);
#endif

free(count);
free(streamlast);
 
return 0;
};

