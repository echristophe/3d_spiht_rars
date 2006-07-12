/*
 *
 *  Hyperspectral compression program
 *
 * Name:		main.c	
 * Author:		Emmanuel Christophe	
 * Contact:		e.christophe at melaneum.com
 * Description:		Utility functions for hyperspectral image compression
 * Version:		v1.0 - 2006-04	
 * 
 */

#include "main.h"



// long int value_signed(char * a);

/*int main(void) {

int t=0;
long int value=46;

char * a=(char *) malloc(NBITS*sizeof(char));
char * b=(char *) malloc(NBITS*sizeof(char));
char * c=(char *) malloc(NBITS*sizeof(char));

for (t=0;t<NBITS;t++){
	a[t] =0;
	b[t] =0;
	c[t] =0;
}

a[1]=1;
a[2]=1;
a[3]=1;
a[4]=0;
a[5]=1;

b[1]=-2;
b[2]=1;

printf("check_zero(a,0): %d\n",check_zero(a,0));
printf("check_zero(a,1): %d\n",check_zero(a,1));
printf("check_zero(a,5): %d\n",check_zero(a,5));
printf("check_zero(a,6): %d\n",check_zero(a,6));


printf("add_bin(a,b,c): %d\n",add_bin(a,b,c));

for (t=0;t<NBITS;t++){
printf("%d",a[t]);
}
printf("\n");

for (t=0;t<NBITS;t++){
printf("%d",b[t]);
}
printf("\n");

for (t=0;t<NBITS;t++){
printf("%d",c[t]);
}
printf("\n");

printf("Real test :\n");


bin_value(value, a);
for (t=0;t<NBITS;t++){
printf("%d",a[t]);
}
printf("\n");

printf("Representation minimale :\n");

min_representation(a);

for (t=0;t<NBITS;t++){
printf("%d",a[t]);
}
printf("\n");
value=value_signed(a);
printf("%ld\n",value);

}*/

//return 0 if (..., a[t+2], a[t+1], a[t]) = (0,0,..,0)
// return 1 otherwise
int check_zero(char * a, int t){
	int i=t;
	if (t >= NBITS-1) return 0;
	for (i=t;i<NBITS-1;i++){
		if (a[i] != 0) return 1;
	}
	return 0;
}


int add_bin(char * a, char * b, char * c){
	int t=0;
// 	char * c=(char *) malloc(NBITS*sizeof(char));
	for (t=0;t<NBITS;t++){
		c[t] =0;
	}
	for (t=0;t<NBITS-1;t++){
		c[t] += a[t]+b[t];
		if (c[t] > 1) {
			if (t+1 == NBITS-1) return 1;
			c[t+1] +=1;
			c[t] -= 2;
		}
		if (c[t] < -1){
			if (t+1 == NBITS-1) return 1;
			c[t+1] -=1;
			c[t] += 2;
		}
	}
	return 0;
}

int min_representation(char * a){
	int t=0;
	int i=0;
	int err=0;
	char * b=(char *) malloc(NBITS*sizeof(char));
	char * c=(char *) malloc(NBITS*sizeof(char));	
	while (check_zero(a,t)) {
	    if (a[t] != 0){
		for (i=0;i<NBITS-1;i++){
			b[i] =0;
			c[i] =0;
		}
		b[t] = -2*(a[t]);//normalement, juste sign(a), mais en base 2 c'est pareil
		b[t+1] = a[t];//meme remarque
		err=add_bin(a,b,c);

		if (err) printf("***** WARNING Depassement de dynamique *****\n");
		if (c[t+1] == 0){
			for (i=0;i<NBITS-1;i++){
				a[i] =c[i];
			}	
		}
	   }
	   t++;
	}
	if (a[NBITS-1] == -1){
		for (i=0;i<NBITS-1;i++){
			a[i] *= -1 ;
		}
	}

	free(b);
	free(c);
	return 0;
}

long int value_signed(char * a){
	int i=0;
	long int value=0;
	for (i=0; i<NBITS; i++){
		value += (long int) a[i] * (1<<i);
	}
	return value;
}

//convert a number into its binary representation
// with 1 or -1 according to value sign
int bin_value(long int value, char * a){
	int i=0;
	int sign=0;
	if (value >= 0) {
		sign= 1;
	} else {
		sign= -1;
	}
// 	for (i=0; i<NBITS; i++){
// 		if (i+1 == NBITS){
// 			a[i]=sign*(abs(value)>>i);
// 		} else {
// 			a[i]=sign*((((abs(value)>>i)<<i) - ((abs(value)>>(i+1))<<(i+1))) >>i);
// 		}
// 	}
	for (i=0; i<NBITS-1; i++){
		if (i+1 == NBITS){
			a[i]=(abs(value)>>i);
		} else {
			a[i]=((((abs(value)>>i)<<i) - ((abs(value)>>(i+1))<<(i+1))) >>i);
		}
	}
	a[NBITS-1] = sign;

	return 0;
}

//we don't want to count the 0s above the first sig value
long int count_zero(char * image){
	long int nzero = 0;
	int flagsig=0;
	double moysign =0.0;
	int i;
	long int pix = 0;
	long int npix = imageprop.nsmax*imageprop.nlmax*imageprop.nbmax;
	for (pix = 0; pix< npix; pix++){
		flagsig = 0;
		for (i=NBITS-2; i>=0; i--){
			if ((flagsig ==0) && (image[i+pix*NBITS] != 0)) {
				flagsig = 1;
				if (pix !=0) {
					moysign = moysign + 1.0/pix *(i-moysign);
				} else {
					moysign=i;
				}
			}
			if ((flagsig ==1) && (image[i+pix*NBITS] == 0)) nzero++;
	
		}
	}
	printf("Moyenne du premier bit significatif: %f\n", moysign);
	return nzero;
}

//change 1,0,-1 to 0,1,1 and -1,0,1 to 0,-1,-1
int change_rep(char * a){ 
	int t=0;
	for (t=NBITS-2;t>=2;t--){
		if ((a[t] == 1) && (a[t-1] == 0) && (a[t-2] == -1)){
			a[t] = 0; a[t-1] = 1; a[t-2] = 1;
		}
		if ((a[t] == -1) && (a[t-1] == 0) && (a[t-2] == 1)){
			a[t] = 0; a[t-1] = -1; a[t-2] = -1;
		}
	}
	return 0;
}

//change 0,1,1 to 1,0,-1 and 0,-1,-1 to -1,0,1
int change_rep2(char * a){
	int t=0;
	for (t=2;t<NBITS-1;t++){
		if ((a[t] == 0) && (a[t-1] == -1) && (a[t-2] == -1)){
			a[t] = -1; a[t-1] = 0; a[t-2] = 1;
		}
		if ((a[t] == 0) && (a[t-1] == 1) && (a[t-2] == 1)){
			a[t] = 1; a[t-1] = 0; a[t-2] = -1;
		}
	}
	return 0;
}
