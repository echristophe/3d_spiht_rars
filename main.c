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


//TODO
// Si on veux une gestion pour des stream de longueur > 4Gb, passer les rate, currentpos et autres en long long int (64bits)

#include "main.h"



//command line parsing
int main(int argc, char *argv[]){	

int i;
int filename_supplied = 0;
int output_filename_supplied = 0;
int size_supplied = 0;
int ndecomp_supplied = 0;

coder_option_struct coder_option;

int selected = -1;
float rate=0.0;

init_coder_option(&coder_option);

	for (i = 1; i < argc;) {
	if (argv[i][0] == '-') 	{
		switch(argv[i][1]) {
		case 'e':		/* do encode */
			selected = ENCODE;
			i++;
			break;
		case 'd':		/* do decode */
			selected = DECODE;
			i++;
			break;
		case 'r':		/* use rate */
			i++;
			if (i>=argc) usage(argv[0]);
			if (sscanf(argv[i],"%f",&rate) == 0) usage(argv[0]);
			i++;
			break;
		case 'm':		/* use mean substraction */
			i++;
			coder_option.flag_meansub=1;
			break;
		case 's':	/* image size */
			size_supplied = 1;
			i++;
			if (i>=argc) usage(argv[0]);
			if (sscanf(argv[i],"%d",&(imageprop.nsmax)) == 0) usage(argv[0]);
			i++;
			if (i>=argc) usage(argv[0]);
			if (sscanf(argv[i],"%d",&(imageprop.nlmax)) == 0) usage(argv[0]);
			i++;
			if (i>=argc) usage(argv[0]);
			if (sscanf(argv[i],"%d",&(imageprop.nbmax)) == 0) usage(argv[0]);
			i++;
			break;
		case 'n':	/* number of decomposition*/
			ndecomp_supplied = 1;
			i++;
			if (i>=argc) usage(argv[0]);
			if (sscanf(argv[i],"%d",&(imageprop.nresspec)) == 0) usage(argv[0]);
			imageprop.nresspec++;
			i++;
			if (i>=argc) usage(argv[0]);
			if (sscanf(argv[i],"%d",&(imageprop.nresspat)) == 0) usage(argv[0]);
			imageprop.nresspat++;
			i++;
			break;
		case 't':	/*data type*/
			i++;
			if (i>=argc) usage(argv[0]);
			if (sscanf(argv[i],"%d",&(coder_option.type)) == 0) usage(argv[0]);
			i++;
			break;
		case 'o':  /*output filename*/
			output_filename_supplied = 1;
			i++;
			coder_option.output_filename = argv[i];
			i++;
			break;
		default:
			usage(argv[0]);
		}
	
	
	} else {
		//read filename
		if (filename_supplied) usage(argv[0]);
		coder_option.filename = argv[i];
		filename_supplied = 1;
		i++;
	}
	}


	if ( !filename_supplied){
		if (selected == ENCODE){
// 		filename = "/home/christop/Boulot/images/hyper_test/moffett3-ani-lsb.img";
// 		#ifdef S64
// 		coder_option.filename = "/home/christop/Boulot/images/hyper_test/moffett3-64-lsb.img";
// 		#else
		coder_option.filename = "/home/christop/Boulot/images/hyper_test/moffett3-lsb.img";
// 		#endif
		} else {
// 		#ifdef OUTPUT
// 		coder_option.filename = "/home/christop/Boulot/images/output_stream/output.dat";
// 		#else
		coder_option.filename = "output.dat";
// 		#endif
		}
	}

	if ( !output_filename_supplied){
	    if(selected == ENCODE){
		coder_option.output_filename = "output.dat";
	    } else {
		coder_option.output_filename = "output.img";
	    }
	}


//default values
	if ((selected == ENCODE) && (!size_supplied)){
		imageprop.nsmax = 256;
		imageprop.nlmax = 256;
		imageprop.nbmax = 224;
	}
	if ((selected == ENCODE) && (!ndecomp_supplied)){
		imageprop.nresspec = 6;
		imageprop.nresspat = 6;
	}



	//print selected option to tell what is going to be done
	switch(selected) {
		case ENCODE:
			printf("Encoding:\n");
			break;
		case DECODE:
			printf("Decoding:\n");
			break;
		default:
			usage(argv[0]);
	}

	printf("File: %s\n", coder_option.filename);

	if (rate != 0){
		printf("at rate %f\n", rate);
	}

	//do the encoding or decoding
	switch(selected) {
		case ENCODE:
			encode(coder_option);
			break;
		case DECODE:
			decode(coder_option);
			break;
		default:
			usage(argv[0]);
	}
	return 0;
}


/*
 * usage(argv[0])
 */

void usage(char *str1){
	fprintf(stderr,
		"\nUsage:"
		"%s [-e | -d] [-r rate] [-s ns nl nb] [-n d1 d2] [-t datatype] [-m] [-o outputfile] [filename]\n"
		"ns: # samples                 [256]\n"
		"nl: # lines                   [256]\n"
		"nb: # bands                   [224]\n"
		"d1: # spectral decompositions [5]\n"
		"d2: # spatial decompositions  [5]\n"
		"-t datatype: 2 short int (default), 1 unsigned char\n"
		"-m: use mean substraction for every spectral band prior to wavelet transform\n",
	       str1
	       );
	exit(1);

}

//****************************************************
//        ENCODING
//****************************************************

//add the option here later...
int encode(coder_option_struct coder_option){


char * filename = coder_option.filename;
char * output_filename = coder_option.output_filename;
int type = coder_option.type;
float rate = coder_option.rate;

// short *imageoritmp=NULL;
// unsigned char *imageoritmpbyte=NULL;
long int *imageori=NULL;
// long int *imagedwt;
// long long int err=0;
long int * mean=NULL;
long int *image=NULL;
unsigned char * stream=NULL;
long int * streamlast=(long int *) calloc(1,sizeof(long int));
unsigned char * count=(unsigned char *) calloc(1,sizeof(unsigned char *));
int *maxquantvalue=(int*) malloc(sizeof(int));
int * headerlength = (int *) malloc(sizeof(int));
long int * outputsize=NULL;
long int npix;
long int i_l;
int niloc, njloc,nkloc, nblock;
int iloc, jloc, kloc, blockind;
stream_struct streamstruct;

int status;
// FILE *data_file;
FILE *output_file;

coder_param_struct coder_param;


#ifdef TIME
clock_t start, end;
double elapsedcomp;
#endif


#ifdef RES_SCAL
imageprop.nres = imageprop.nresspec * imageprop.nresspat;
#else
imageprop.nres =  imageprop.nresspec + imageprop.nresspat -1;
#endif
imageprop.nsmin=imageprop.nsmax;
imageprop.nlmin=imageprop.nlmax;
imageprop.nbmin=imageprop.nbmax;
for (i_l=0;i_l<imageprop.nresspat-1;i_l++){
	imageprop.nsmin = ceil(imageprop.nsmin / 2.0);//This is to prepare the case for odd sizes...
	imageprop.nlmin = ceil(imageprop.nlmin / 2.0);//has to be done accordingly to the wavelet transform
}
for (i_l=0;i_l<imageprop.nresspec-1;i_l++){
	imageprop.nbmin = ceil(imageprop.nbmin / 2.0);
}

npix=imageprop.nsmax*imageprop.nlmax*imageprop.nbmax;
image = (long int *) malloc(npix*sizeof(long int));
stream = (unsigned char *) calloc(type*npix,sizeof(unsigned char));//on prend une marge... a faire en finesse plus tard...
// outputsize = (long int *) calloc((*maxquantvalue+1), sizeof(long int));

nbitswritten=0;
nbitswrittenheader=0;
//coeff in one block: (n1+1)(3 n2 +1)
//n1 being the number of spec desc : n1 = (2^(ndecomp+1)-1)
//n2 being the number of spat desc : n2 = (2^(2(ndecomp+1))-1)/3
//
sizeblockstream = type * (1<< imageprop.nresspec) * ( 1<< (2*imageprop.nresspat));
printf("Sizeblock stream: %ld\n",sizeblockstream);



//possibility to skip the wavelet processing (to allow external transform)
#ifdef SKIPWAV
// image = (long int *) calloc(npix,sizeof(long int));
// data_file = fopen(filename, "r");
// if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
// status = fread(image, 4, npix, data_file);
// status = fclose(data_file);
image = read_hyper(filename, npix, 4);

#else
//replace by one call to read_hyper
// imageori = (long int *) calloc(npix,sizeof(long int));
// image = (long int *) calloc(npix,sizeof(long int));
// data_file = fopen(filename, "r");
// if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
// if (type == 2){
// imageoritmp = (short *) calloc(npix,sizeof(short));
// status = fread(imageoritmp, type, npix, data_file);
// status = fclose(data_file);
// for (i_l=0;i_l<npix;i_l++){
// 	imageori[i_l] = (long int) imageoritmp[i_l];
// }
// free(imageoritmp);
// } else {
// imageoritmpbyte = (unsigned char *) calloc(npix,sizeof(short));
// status = fread(imageoritmpbyte, type, npix, data_file);
// status = fclose(data_file);
// for (i_l=0;i_l<npix;i_l++){
// 	imageori[i_l] = (long int) imageoritmpbyte[i_l];
// }
// free(imageoritmpbyte);
// }
imageori = read_hyper(filename, npix, type);

//perform wavelet checking
// if (! wavelet_check(imageori,npix)) return 0;

if (coder_option.flag_meansub){
// mean substraction
printf("Be aware: using mean substraction\n");
mean = (long int *) calloc(imageprop.nbmax,sizeof(long int));
compute_mean(imageori, mean);
substract_mean(imageori, mean);
}

waveletDWT(imageori,image,imageprop.nresspec-1,imageprop.nresspat-1);
#endif
//end of wavelet skipping

#ifdef OUTPUT
// output_file = fopen("/home/christop/Boulot/images/output_stream/output-dwt-ori.img","w");
// if (output_file == NULL) fprintf(stderr, "Error opening file...\n");
// status = fwrite(image, 4, npix, output_file);
// status = fclose(output_file);
status = write_hyper("/home/christop/Boulot/images/output_stream/output-dwt-ori.img", image, npix, 4);
#endif

#ifdef WAV53
// waveletscaling(image, imageprop.nresspat-1, imageprop.nresspec-1,0);//perform the wavelet scaling (finally, not necessary for lossless)
#endif

imageprop.maxquant=(int) floor(log((double) find_max(image, npix))/log(2.0));
printf("Max quant: %d\n",imageprop.maxquant);
outputsize = (long int *) calloc((imageprop.maxquant+1), sizeof(long int));
//coder parameter initialization



niloc=(imageprop.nsmin+1)/2;
njloc=(imageprop.nlmin+1)/2;
nkloc=(imageprop.nbmin+1)/2;
nblock=niloc* njloc* nkloc;
// define encoding parameter for each block
init_coder_param(&coder_param, nblock);//encoder FULL !!!
for (i_l=0; i_l<nblock; i_l++){
	coder_param.maxres[i_l]=imageprop.nres;
// 	coder_param.maxres[i_l]=4;
#ifdef RES_SCAL
	coder_param.maxresspat[i_l]=imageprop.nresspat;
	coder_param.maxresspec[i_l]=imageprop.nresspec;
// 	coder_param.maxresspat[i_l]=4;
// 	coder_param.maxresspec[i_l]=4;
#endif
	coder_param.maxquant[i_l]=imageprop.maxquant;
	coder_param.minquant[i_l]=0; //warning, not used yet for EZW signed
}
//Example of ROI for encoding
// for (kloc=0;kloc<nkloc;kloc++){
//    for (jloc=1;jloc<=1;jloc++){
//       for (iloc=2;iloc<=2;iloc++){
// 	blockind = iloc + jloc*niloc + kloc * niloc *njloc;
// 	coder_param.maxres[blockind]=NRES;
// 	coder_param.minquant[blockind]=0;
//       }
//    }
// }

coder_param.rate = rate;

print_imageprop();


streamstruct.stream=stream;
streamstruct.streamlast=streamlast;
streamstruct.count=count;
streamstruct.headerlength=headerlength;

//Image header coding
status=write_header(streamstruct);
if (coder_option.flag_meansub){
status=write_header_mean(streamstruct, mean);
}
// add_to_stream_number(imageprop.nsmax, stream, count, streamlast, 16);
// add_to_stream_number(imageprop.nlmax, stream, count, streamlast, 16);
// add_to_stream_number(imageprop.nbmax, stream, count, streamlast, 16);
// add_to_stream_number(imageprop.maxquant, stream, count, streamlast, 8);
// add_to_stream_number(imageprop.nresspec, stream, count, streamlast, 4);
// add_to_stream_number(imageprop.nresspat, stream, count, streamlast, 4);
// #ifdef MEANSUB
// for (i_l=0;i_l<imageprop.nbmax;i_l++){
// add_to_stream_number(mean[i_l], stream, count, streamlast, 16);
// }
// #endif

// wavelet_check(image, npix);

#ifdef TIME
start = clock();
#endif


#ifdef EZW
#ifdef SIGNED
ezw_code_signed_c(image, streamstruct, outputsize, imageprop.maxquant);
#else
ezw_code_c(image, streamstruct, outputsize, imageprop.maxquant);
#endif
#else
#ifdef NORA
spiht_code_c(image, streamstruct, outputsize, coder_param);
#else
spiht_code_ra5(image, streamstruct, outputsize, coder_param);
#endif
#endif


#ifdef TIME
end = clock();
elapsedcomp = ((double) (end - start)) / CLOCKS_PER_SEC;
printf("Compression time: %f \n", elapsedcomp);
#endif


printf("Outputsize: %ld bits\n", *outputsize);


//output coded stream
#ifdef OUTPUT
output_file = fopen("/home/christop/Boulot/images/output_stream/output.dat","w");
#else
output_file = fopen(output_filename,"w");
#endif
if (output_file == NULL) fprintf(stderr, "Error opening file...\n");
status = fwrite(stream, 1, (*outputsize+7)/8, output_file);
status = fclose(output_file);

// free(maxquantvalue);
// free(outputsize);
// free(image);
// free(stream);

#ifdef SIZE
fprintf(stderr, "Wrote %ld bits (code+block header-image header)\n",nbitswritten);
fprintf(stderr, "Wrote %ld header bits\n",nbitswrittenheader);
#endif

return 0;
}


//****************************************************
//        DECODING
//****************************************************


int decode(coder_option_struct coder_option){

char * filename = coder_option.filename;
char * output_filename = coder_option.output_filename;
int type = coder_option.type;
float rate = coder_option.rate;

long int * imageidwt=NULL;
short * imageitmp=NULL;
unsigned char * imageitmpbyte=NULL;
long int * imageoritmp=NULL;
long int *imagedwtori=NULL;

long int * image=NULL;
long int * imagepart=NULL;
long int * imagesav=NULL;
long int * imagepartori1=NULL;
short int * imagepartori=NULL;
long int * imageidwtori=NULL;
unsigned char * stream=NULL;
long int * streamlast=(long int *) calloc(1,sizeof(long int));
unsigned char * count=(unsigned char *) calloc(1,sizeof(unsigned char *));
int * headerlength = (int *) malloc(sizeof(int));
stream_struct streamstruct;
// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};

#ifdef TIME
clock_t start, end;
double elapseddecomp;
#endif

int *maxquantvalue=(int*) malloc(sizeof(int));
long int * outputsize =(long int*) malloc(sizeof(int)) ;
long int npix;

int status;
FILE *stream_file;
// FILE *output_file;
long int stream_size;
int niloc, njloc, nkloc, nblock;
int iloc, jloc, kloc, blockind;
coder_param_struct coder_param;

// long int *imageori;
short *imageori;

FILE *data_file;
int err=0;
long int maxerr=0;
long int maxerrnz=0;
long int i_l=0;
long int * mean=NULL;
int i;
pixel_struct pixel;
long int nnewpix;
int nsnewmax, nbnewmax, nlnewmax;
int spatdec, specdec;
double factor=1.0;
//decoder parameter initialization

// imageprop=(struct imageprop_struct) {NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST, NRES, NRESSPEC, NRESSPAT,MAXQUANT_CONST};


stream_file = fopen(filename,"r");
if (stream_file == NULL) fprintf(stderr, "Error opening file...\n");
stream_size = file_size(stream_file);
// stream_size =64566;
printf("Reading %ld bytes\n",stream_size);
stream = (unsigned char *) malloc(stream_size*sizeof(unsigned char));
status = fread(stream, 1, stream_size, stream_file);//same as for writing...
status = fclose(stream_file);

*outputsize = stream_size * 8;

streamstruct.stream=stream;
streamstruct.streamlast=streamlast;
streamstruct.count=count;
streamstruct.headerlength=headerlength;

status=read_header(streamstruct);

if (coder_option.flag_meansub){
printf("Be aware: using mean substraction\n");
mean = (long int *) calloc(imageprop.nbmax,sizeof(long int));
status=read_header_mean(streamstruct, mean);
}

// imageprop.nsmax = read_from_stream_number(stream, count, streamlast, 16);
// imageprop.nlmax = read_from_stream_number(stream, count, streamlast, 16);
// imageprop.nbmax = read_from_stream_number(stream, count, streamlast, 16);
// imageprop.maxquant = read_from_stream_number(stream, count, streamlast, 8);
// imageprop.nresspec = read_from_stream_number(stream, count, streamlast, 4);
// imageprop.nresspat = read_from_stream_number(stream, count, streamlast, 4);
// #ifdef MEANSUB
// printf("Be aware: using mean substraction\n");
// mean = (long int *) calloc(imageprop.nbmax,sizeof(long int));
// for (i_l=0;i_l<imageprop.nbmax;i_l++){
// 	mean[i_l] = read_from_stream_number(stream, count, streamlast, 16);
// }
// #endif

//coeff in one block: (n1+1)(3 n2 +1)
//n1 being the number of spec desc : n1 = (2^(ndecomp+1)-1)
//n2 being the number of spat desc : n2 = (2^(2(ndecomp+1))-1)/3
//
sizeblockstream = type * ( (1<< imageprop.nresspec)-1) * (( 1<< (2*imageprop.nresspat))-1);
printf("Sizeblock stream: %ld\n",sizeblockstream);

#ifdef RES_SCAL
imageprop.nres = imageprop.nresspec * imageprop.nresspat;
#else
imageprop.nres =  imageprop.nresspec + imageprop.nresspat -1;
#endif
imageprop.nsmin=imageprop.nsmax;
imageprop.nlmin=imageprop.nlmax;
imageprop.nbmin=imageprop.nbmax;
for (i_l=0;i_l<imageprop.nresspat-1;i_l++){
	imageprop.nsmin = ceil(imageprop.nsmin / 2.0);//This is to prepare the case for odd sizes...
	imageprop.nlmin = ceil(imageprop.nlmin / 2.0);//has to be done accordingly to the wavelet transform
}
for (i_l=0;i_l<imageprop.nresspec-1;i_l++){
	imageprop.nbmin = ceil(imageprop.nbmin / 2.0);
}
niloc=(imageprop.nsmin+1)/2;
njloc=(imageprop.nlmin+1)/2;
nkloc=(imageprop.nbmin+1)/2;
nblock=niloc* njloc* nkloc;
//Decoder parameter
//Change here if you want a ROI decoding
init_coder_param(&coder_param, nblock);
for (i_l=0; i_l<nblock; i_l++){
	coder_param.maxres[i_l]=imageprop.nres;
// 	coder_param.maxres[i_l]=4;
#ifdef RES_SCAL
	coder_param.maxresspat[i_l]=imageprop.nresspat;
	coder_param.maxresspec[i_l]=imageprop.nresspec;
// 	coder_param.maxresspat[i_l]=5;
// 	coder_param.maxresspec[i_l]=1;
#endif
	coder_param.maxquant[i_l]=imageprop.maxquant;
	coder_param.minquant[i_l]=0;
}
//warning ! resolutions = decomp +1
spatdec=imageprop.nresspat-1;
// spatdec=4;
specdec=imageprop.nresspec-1;
coder_param.nlayer=100;// decoder tous les layers pare defaut

// Example of ROI decoding (article example on moffett3)
// resspat/resspec above should be 3
// spatdec/specdec above should be 2
// #ifdef RES_SCAL
// for (kloc=0;kloc<nkloc;kloc++){
// coder_param.maxresspec[18+kloc*64]=6;
// coder_param.maxresspec[19+kloc*64]=6;
// 
// coder_param.maxresspat[21+kloc*64]=6;
// coder_param.maxresspat[22+kloc*64]=6;
// coder_param.maxresspat[29+kloc*64]=6;
// coder_param.maxresspat[30+kloc*64]=6;
// 
// coder_param.maxresspec[46+kloc*64]=6;
// coder_param.maxresspat[46+kloc*64]=6;
// coder_param.maxresspec[54+kloc*64]=6;
// coder_param.maxresspat[54+kloc*64]=6;
// }
// #endif
// *maxquantvalue=MAXQUANT_CONST;
//ROI
// for (kloc=0;kloc<nkloc;kloc++){
//    for (jloc=1;jloc<=1;jloc++){
//       for (iloc=2;iloc<=2;iloc++){
// 	blockind = iloc + jloc*niloc + kloc * niloc *njloc;
// 	coder_param.maxres[blockind]=NRES;
// 	coder_param.minquant[blockind]=0;
//       }
//    }
// }

npix=imageprop.nsmax*imageprop.nlmax*imageprop.nbmax;
image = (long int *) calloc(npix,sizeof(long int));

#ifdef CHECKEND
// imageoriglobal = (long int *) calloc(npix,sizeof(long int));
// data_file = fopen("/home/christop/Boulot/images/output_stream/output-dwt-ori.img","r");
// if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
// status = fread(imageoriglobal, 4, npix, data_file);
// status = fclose(data_file);
imageoriglobal = read_hyper("/home/christop/Boulot/images/output_stream/output-dwt-ori.img", npix, 4);
#endif

#ifdef TIME
start = clock();
#endif

#ifdef EZW
#ifdef SIGNED
ezw_decode_signed_c(image, streamstruct, outputsize, imageprop.maxquant);
#else
ezw_decode_c(image, streamstruct, outputsize, imageprop.maxquant);
#endif
#else
#ifdef NORA
spiht_decode_c(image, streamstruct, outputsize, coder_param);
#else
spiht_decode_ra5(image, streamstruct, outputsize, coder_param);
#endif
#endif


#ifdef TIME
end = clock();
elapseddecomp = ((double) (end - start)) / CLOCKS_PER_SEC;
printf("Decompression time: %f \n", elapseddecomp);
#endif

#ifdef WAV53
// waveletscaling(image, imageprop.nresspat-1, imageprop.nresspec-1,1);
#endif

#ifdef SKIPWAV
// output_file = fopen(output_filename,"w");
// if (output_file == NULL) fprintf(stderr, "Error opening file...\n");
// status = fwrite(image, 4, npix,output_file);
// status = fclose(output_file);
status=write_hyper(output_filename, image, npix, 4);
#else
//output decoded dwt
#ifdef OUTPUT
// output_file = fopen("/home/christop/Boulot/images/output_stream/output-dwt.img","w");
// if (output_file == NULL) fprintf(stderr, "Error opening file...\n");
// status = fwrite(image, 4, npix,output_file);
// status = fclose(output_file);
status=write_hyper("/home/christop/Boulot/images/output_stream/output-dwt.img", image, npix, 4);
#endif

#ifdef OUTPUT
// imagedwtori = (long int *) calloc(npix,sizeof(long int));
// data_file = fopen("/home/christop/Boulot/images/output_stream/output-dwt-ori.img","r");
// if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
// status = fread(imagedwtori, 4, npix, data_file);
// status = fclose(data_file);
imagedwtori = read_hyper("/home/christop/Boulot/images/output_stream/output-dwt-ori.img", npix, 4);

err=0;
if (imagedwtori == NULL) fprintf(stderr, "Are you sure you generated the reference dwt during the encoding ?\n");
err = compare_hyper_long(image, imagedwtori);
// maxerr=0;
// maxerrnz=0;
// for (i_l=0;i_l<npix;i_l++){
// 	if (abs(imagedwtori[i_l]-image[i_l]) > maxerr){
// 		maxerr=abs(imagedwtori[i_l]-image[i_l]);
// 		err=1;
// 	};
// 	if (image[i_l] != 0){
// 		if (abs(imagedwtori[i_l]-image[i_l]) > maxerrnz){
// 			maxerrnz=abs(imagedwtori[i_l]-image[i_l]);
// 			err=1;
// 		};
// 	};
// };
// 
// if (err) {
// fprintf(stderr, "ERREUR MAX (on DWT) %ld\n",maxerr);
// fprintf(stderr, "ERREUR MAX (on DWT)(NZ) %ld\n",maxerrnz);
// } else {
// fprintf(stderr, "Decoding OK (maxerr= %ld)\n",maxerr);
// }
free(imagedwtori);imagedwtori=NULL;
#endif

//processing IDWT and writing file
nsnewmax=(imageprop.nsmax >> (imageprop.nresspat-1 - spatdec));
nlnewmax=(imageprop.nlmax >> (imageprop.nresspat-1 - spatdec));
nbnewmax=(imageprop.nbmax >> (imageprop.nresspec-1 - specdec));

// nnewpix= nsnewmax*nlnewmax*nbnewmax;
npix=nsnewmax*nlnewmax*nbnewmax;
imagepart=(long int *) calloc(npix,sizeof(long int));

for (i_l=0;i_l<npix;i_l++){
	pixel.x = i_l % nsnewmax;
	pixel.y = (i_l / nsnewmax) % nlnewmax;
	pixel.l = (i_l / (nsnewmax*nlnewmax));
	imagepart[i_l]=image[trans_pixel(pixel)];
}
imagesav=image;
image=imagepart;
imagepart=imagesav;
free(imagepart);imagepart=NULL;

imageidwt = (long int *) calloc(npix,sizeof(long int));

waveletIDWT(image,imageidwt,specdec,spatdec);
// free(image); //was freed inside the IDWT

// mean substraction
if (coder_option.flag_meansub){
add_mean(imageidwt, mean);
}

#ifdef OUTPUT
// if (type ==2){
// imageitmp = (short *) calloc(npix,sizeof(short));
// for (i_l=0;i_l<npix;i_l++){
// 	imageitmp[i_l] = (short) imageidwt[i_l];
// }
// free(imageidwt);imageidwt=NULL;
// data_file = fopen("/home/christop/Boulot/images/output_stream/output.img", "w");
// if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
// status = fwrite(imageitmp, type, npix, data_file);
// status = fclose(data_file);
// 
// }else{
// imageitmpbyte = (unsigned char *) calloc(npix,sizeof(unsigned char));
// if (imageitmpbyte == NULL) fprintf(stderr, "Allocation error...\n");
// for (i_l=0;i_l<npix;i_l++){
// 	imageitmpbyte[i_l] = (unsigned char) imageidwt[i_l];
// }
// free(imageidwt);imageidwt=NULL;
// data_file = fopen("/home/christop/Boulot/images/output_stream/output.img", "w");
// if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
// status = fwrite(imageitmpbyte, type, npix, data_file);
// if (status != npix) fprintf(stderr, "Error writing output file...\n");
// status = fclose(data_file);
// }

status=write_hyper("/home/christop/Boulot/images/output_stream/output.img", imageidwt, npix, type);
free(imageidwt);imageidwt=NULL;

#else
// if (type ==2){
// imageitmp = (short *) calloc(npix,sizeof(short));
// for (i_l=0;i_l<npix;i_l++){
// 	imageitmp[i_l] = (short) imageidwt[i_l];
// }
// free(imageidwt);imageidwt=NULL;
// data_file = fopen(output_filename, "w");
// if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
// status = fwrite(imageitmp, type, npix, data_file);
// status = fclose(data_file);
// 
// }else{
// imageitmpbyte = (unsigned char *) calloc(npix,sizeof(unsigned char));
// for (i_l=0;i_l<npix;i_l++){
// 	imageitmpbyte[i_l] = (unsigned char) imageidwt[i_l];
// }
// free(imageidwt);imageidwt=NULL;
// data_file = fopen(output_filename, "w");
// if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
// status = fwrite(imageitmpbyte, type, npix, data_file);
// if (status != npix) fprintf(stderr, "Error writing output file...\n");
// status = fclose(data_file);
// }

status=write_hyper(output_filename, imageidwt, npix, type);
free(imageidwt);imageidwt=NULL;


#endif



#ifdef OUTPUT
//processing partial IDWT for imageori dwt also
if (1){
//debut image recomp
// imagedwtori = (long int *) calloc(imageprop.nsmax*imageprop.nlmax*imageprop.nbmax,sizeof(long int));
// data_file = fopen("/home/christop/Boulot/images/output_stream/output-dwt-ori.img","r");
// status = fread(imagedwtori, 4, imageprop.nsmax*imageprop.nlmax*imageprop.nbmax, data_file);
// status = fclose(data_file);
// 
// imagepartori1=(long int *) calloc(npix,sizeof(long int));
// 
// for (i_l=0;i_l<npix;i_l++){
// 	pixel.x = i_l % nsnewmax;
// 	pixel.y = (i_l / nsnewmax) % nlnewmax;
// 	pixel.l = (i_l / (nsnewmax*nlnewmax));
// 	imagepartori1[i_l]=imagedwtori[trans_pixel(pixel)];
// }
// imagesav=imagedwtori;
// imagedwtori=imagepartori1;
// imagepartori1=imagesav;
// free(imagepartori1);imagepartori=NULL;
// 
// imageidwtori = (long int *) calloc(npix,sizeof(long int));
// waveletIDWT(imagedwtori,imageidwtori,specdec,spatdec);
// 
// imageitmp = (short *) calloc(npix,sizeof(short));
// for (i_l=0;i_l<npix;i_l++){
// 	imageitmp[i_l] = (short) imageidwtori[i_l];
// }
// free(imageidwtori);imageidwtori=NULL;
// data_file = fopen("/home/christop/Boulot/images/output_stream/output-ori.img", "w");
// status = fwrite(imageitmp, type, npix, data_file);
// status = fclose(data_file);
//fin image recomp

//debut image originale
imageori = (short int *) calloc(imageprop.nsmax*imageprop.nlmax*imageprop.nbmax,sizeof(short int));
imageoritmp = (long *) calloc(imageprop.nsmax*imageprop.nlmax*imageprop.nbmax,sizeof(long));
if (imageprop.nsmax == 64){
data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett3-64-lsb.img","r");
if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
}
if (imageprop.nsmax == 512){
data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett512.rawl","r");
// data_file = fopen("/home/christop/Boulot/images/hyper_test/jasper1-rfl.img","r");
if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
}
if (imageprop.nsmax == 256){
data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett3-lsb.img","r");
if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
}
status = fread(imageori, 2, imageprop.nsmax*imageprop.nlmax*imageprop.nbmax, data_file);
status = fclose(data_file);

for (i_l=0;i_l<imageprop.nsmax*imageprop.nlmax*imageprop.nbmax;i_l++){
	imageoritmp[i_l] = (long int) imageori[i_l];
}
free(imageori);

imagedwtori=(long int *) calloc(imageprop.nsmax*imageprop.nlmax*imageprop.nbmax,sizeof(long int));

waveletDWT(imageoritmp,imagedwtori,imageprop.nresspec-1-specdec,imageprop.nresspat-1-spatdec);

imagepartori=(short int *) calloc(npix,sizeof(short int));

//Normalization (to compensate the gain of the low-pass)
for (i=0;i<(imageprop.nresspat-1 - spatdec);i++) factor *= sqrt(2.0); 
for (i=0;i<(imageprop.nresspat-1 - spatdec);i++) factor *= sqrt(2.0); 
for (i=0;i<(imageprop.nresspec-1 - specdec);i++) factor *= sqrt(2.0); 

for (i_l=0;i_l<npix;i_l++){
	pixel.x = i_l % nsnewmax;
	pixel.y = (i_l / nsnewmax) % nlnewmax;
	pixel.l = (i_l / (nsnewmax*nlnewmax));
	imagepartori[i_l]=(short int) lround( (imagedwtori[trans_pixel(pixel)]/factor));
}

data_file = fopen("/home/christop/Boulot/images/output_stream/output-ori.img", "w");
if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
status = fwrite(imagepartori, type, npix, data_file);
status = fclose(data_file);
//fin image originale
}
#endif

#ifdef OUTPUT
//comparaison dwt
imageori = (short *) calloc(npix,sizeof(short));
if (imageprop.nsmax == 64){
data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett3-64-lsb.img","r");
if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
}
if (imageprop.nsmax == 512){
data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett512.rawl","r");
// data_file = fopen("/home/christop/Boulot/images/hyper_test/jasper1-rfl.img","r");
if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
}
if (imageprop.nsmax == 256){
data_file = fopen("/home/christop/Boulot/images/hyper_test/moffett3-lsb.img","r");
if (data_file == NULL) fprintf(stderr, "Error opening file...\n");
}


status = fread(imageori, type, npix, data_file);
if (status != npix) fprintf(stderr, "Error reading file...\n");
status = fclose(data_file);


err=0;

imageidwt=read_hyper("/home/christop/Boulot/images/output_stream/output.img", npix, type);
err = compare_hyper_short(imageidwt, imageori);//TODO change to right type
// maxerr=0;
// maxerrnz=0;
// if (type == 2){
// for (i_l=0;i_l<npix;i_l++){
// 	if (abs(imageori[i_l]-imageitmp[i_l]) > maxerr){
// 		maxerr=abs(imageori[i_l]-imageitmp[i_l]);
// 		err=1;
// 	};
// };
// 
// free(imageitmp);
// } else {
// for (i_l=0;i_l<npix;i_l++){
// 	if (abs(imageori[i_l]-imageitmpbyte[i_l]) > maxerr){
// 		maxerr=abs(imageori[i_l]-imageitmpbyte[i_l]);
// 		err=1;
// 	};
// };
// 
// free(imageitmpbyte);
// }
// 
// 
// if (err) {
// fprintf(stderr, "ERREUR MAX %ld\n",maxerr);
// } else {
// fprintf(stderr, "Decoding OK (maxerr= %ld)\n",maxerr);
// }
#endif

#endif
//fin du skipwav

#ifdef SIZE
fprintf(stderr, "Read %ld bits\n",nbitsread);
#endif

free(maxquantvalue);
free(outputsize);
// free(image);
free(stream);

return 0;
}


