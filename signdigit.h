
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

int check_zero(char * a, int t);

int add_bin(char * a, char * b, char * c);

int min_representation(char * a);

long int value_signed(char * a);

int bin_value(long int value, char * a);

