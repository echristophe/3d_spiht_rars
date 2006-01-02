
struct pixel_struct{
     short int x;
     short int y;
     short int l;
};

struct list_el{
     struct list_el *next;
     struct pixel_struct pixel;
     unsigned char type; /*0 type A and 1 type B*/
};


struct list_struct{
     struct list_el *current;
     struct list_el *previous;
     struct list_el *first;
     struct list_el *last;
};


struct imageprop_struct{
   int nsmax;
   int nlmax;
   int nbmax;
   int nsmin;
   int nlmin;
   int nbmin;
};

struct list_el * el_init(struct pixel_struct pixel);
struct list_struct * list_init(void);
void list_free(struct list_struct * list);

struct list_el * first_el(struct list_struct * list);
struct list_el * next_el(struct list_struct * list);
void insert_el(struct list_struct * list, struct list_el * el);
struct list_el * remove_current_el(struct list_struct * list);
int check_list(struct list_struct * list);
int count_list(struct list_struct * list);
int print_list(struct list_struct * list);

int check_map(unsigned char * map, struct imageprop_struct imageprop);

long int trans_pixel(struct pixel_struct pixel, struct imageprop_struct imageprop);

unsigned char get_bit(long int value,int thres_ind);

void add_to_stream(unsigned char *stream, unsigned char *count, int input, long int *streamlast);
unsigned char read_from_stream(unsigned char * stream, unsigned char * count, long int *streamlast);

int spiht_code_c(long int *image, unsigned char *stream,long int * outputsize);
int spiht_decode_c(long int *image, unsigned char *stream, long int *outputsize);

