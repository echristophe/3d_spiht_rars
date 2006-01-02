#include <stdlib.h>

int spat_spec_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spec_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

int spat_desc_spiht(struct pixel_struct pixel, struct list_struct * list_desc, struct imageprop_struct imageprop, int directchildonly, long int *image, int thres_ind, unsigned char *map_LSC);

//TODO
/*
list_desc update SEULEMENT si directchildonly = 1 !!
sortie en avance SEULEMENT si directchildonly = 0 !!
*/

