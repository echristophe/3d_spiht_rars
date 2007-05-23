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



#include "main.h"

//command line parsing
int main(int argc, char *argv[]){	

int i;
int filename_supplied = 0;
int output_filename_supplied = 0;
int size_supplied = 0;
int ndecomp_supplied = 0;

coder_param_struct coder_param;

int selected = -1;
// float rate=0.0;

preinit_coder_param(&coder_param);
#ifdef EZW
*(coder_param.flag_spiht)=0;
#else
*(coder_param.flag_spiht)=1;
#endif

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
			if (sscanf(argv[i],"%f",&(coder_param.rate)) == 0) usage(argv[0]);
			i++;
			break;
		case 'm':		/* use mean substraction */
			i++;
			*(coder_param.flag_meansub)=1;
			break;
		case 'i':		/* use integer wavelet */
			i++;
			*(coder_param.flag_wavelet_int)=1;
			break;
		case 'a':		/* use arith coder (only EZW) */
			i++;
			*(coder_param.flag_arith)=1;
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
			if (sscanf(argv[i],"%d",&(coder_param.type)) == 0) usage(argv[0]);
			i++;
			break;
		case 'o':  /*output filename*/
			output_filename_supplied = 1;
			i++;
			coder_param.output_filename = argv[i];
			i++;
			break;
		case 'f':  /*roi filename*/
			coder_param.flag_roi = 1;
			i++;
			coder_param.roi_filename = argv[i];
			i++;
			break;
		default:
			usage(argv[0]);
		}
	
	
	} else {
		//read filename
		if (filename_supplied) usage(argv[0]);
		coder_param.filename = argv[i];
		filename_supplied = 1;
		i++;
	}
	}


	if ( !filename_supplied){
		if (selected == ENCODE){
		//Default filename
		coder_param.filename = "/home/christop/Boulot/images/hyper_test/moffett3-lsb.img";
		} else {
		coder_param.filename = "output.dat";
		}
	}

	if ( !output_filename_supplied){
	    if(selected == ENCODE){
		coder_param.output_filename = "output.dat";
	    } else {
		coder_param.output_filename = "output.img";
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

        if (sizeChecking()){
                usage(argv[0]);
        }

	//print selected option to tell what is going to be done
	switch(selected) {
		case ENCODE:
			printf("\n     ****** Encoding ******\n\n");
			break;
		case DECODE:
			printf("\n     ****** Decoding ******\n\n");
			break;
		default:
			usage(argv[0]);
	}

	printf("File: %s\n", coder_param.filename);

	if (coder_param.rate != 0){
		printf("at rate %f\n", coder_param.rate);
	}

	//do the encoding or decoding
	switch(selected) {
		case ENCODE:
			encode(coder_param);
			break;
		case DECODE:
			decode(coder_param);
			break;
		default:
			usage(argv[0]);
	}
	return 0;
}

int sizeChecking(){
//the current implemencation need image dimension to be 
//- k*2^(ndecomp+1) for spatial
//- k*2^(ndecomp) for spectral
// this come from the tree definition
   int tmpValue;
   tmpValue = (imageprop.nsmax/(1<<(imageprop.nresspat)));
   if ( tmpValue*(1<<(imageprop.nresspat)) != imageprop.nsmax ) return 1;
   tmpValue = (imageprop.nlmax/(1<<(imageprop.nresspat)));
   if ( tmpValue*(1<<(imageprop.nresspat)) != imageprop.nlmax ) return 1;
   tmpValue = (imageprop.nbmax/(1<<(imageprop.nresspec-1)));
   if ( tmpValue*(1<<(imageprop.nresspec-1)) != imageprop.nbmax ) return 1;
   return 0;
}

void usage(char *str1){
#ifdef EZW
	fprintf(stderr,
		"\nHyperspectral image (or 2D image) coding using a EZW-like scheme\n"
		"Options includes: \n"
		"  - Asymetric tree, overlapping tree or mixed structure\n"
		"  - Direct coefficients output or arithmetically coded\n"
	);
#else
	fprintf(stderr,
		"Hyperspectral image (or 2D image) coding using a SPIHT-like scheme\n"
		"Options includes: \n"
		"  - Asymetric tree, overlapping tree\n"
		"  - For asymetric tree, random access and resolution scalability\n"
	);
#endif
	fprintf(stderr,
		"\nUsage:\n"
		"%s [-e | -d] [-r rate] [-s ns nl nb] [-n d1 d2] [-t datatype] [-m] [-i] [-a] [-f roifile] [-o outputfile] [filename]\n"
		"ns: # samples                 [256]\n"
		"nl: # lines                   [256]\n"
		"nb: # bands                   [224]\n"
		"d1: # spectral decompositions [5]\n"
		"d2: # spatial decompositions  [5]\n"
		"-t datatype: 2 short int (default), 1 unsigned char, 3 unsigned short int, 4 long int\n"
                "-i: use 5/3 integer transform instead of default 9/7\n"
		"-m: use mean substraction for every spectral band prior to wavelet transform\n"
                "-a: use arithmetic coding (still experimental)\n"
                "  Default value in []\n\n"

                "WARNING: Image size has to be of the type: k1*2^(d2+1) , k2*2^(d2+1), k3*2^(d1)\n\n",
	       str1
	       );
	exit(1);

}

//****************************************************
//        ENCODING
//****************************************************

int encode(coder_param_struct coder_param){

char * filename = coder_param.filename;
char * output_filename = coder_param.output_filename;
int type = coder_param.type;
float rate = coder_param.rate;


long int *imageori=NULL;

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

FILE *output_file;

// coder_param_struct coder_param;


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
stream = (unsigned char *) calloc(type*npix,sizeof(unsigned char));//with margin


nbitswritten=0;
nbitswrittenheader=0;
//coeff in one block: (n1+1)(3 n2 +1)
//n1 being the number of spec desc : n1 = (2^(ndecomp+1)-1)
//n2 being the number of spat desc : n2 = (2^(2(ndecomp+1))-1)/3
//
sizeblockstream = type * (1<< imageprop.nresspec) * ( 1<< (2*imageprop.nresspat));
printf("Sizeblock stream: %ld\n",sizeblockstream);


imageori = read_hyper(filename, npix, type);

if (*(coder_param.flag_meansub)){
// mean substraction
printf("Be aware: using mean substraction\n");
mean = (long int *) calloc(imageprop.nbmax,sizeof(long int));
compute_mean(imageori, mean);
substract_mean(imageori, mean);
}

waveletDWT(imageori,image,imageprop.nresspec-1,imageprop.nresspat-1,coder_param);


#ifdef OUTPUT
status = write_hyper("/home/christop/Boulot/images/output_stream/output-dwt-ori.img", image, npix, 4);
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
if (coder_param.flag_roi){
  read_roi_spec(coder_param); //tmp to do properly
} else {
  for (i_l=0; i_l<nblock; i_l++){
          coder_param.maxres[i_l]=imageprop.nres;
  // 	coder_param.maxres[i_l]=4;
//   #ifdef RES_SCAL
          coder_param.maxresspat[i_l]=imageprop.nresspat;
          coder_param.maxresspec[i_l]=imageprop.nresspec;
  // 	coder_param.maxresspat[i_l]=4;
  // 	coder_param.maxresspec[i_l]=4;
//   #endif
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
}

coder_param.rate = rate;

print_imageprop();
print_coderparam(coder_param);

streamstruct.stream=stream;
streamstruct.streamlast=streamlast;
streamstruct.count=count;
streamstruct.headerlength=headerlength;

//Image header coding
status = write_magic(streamstruct, coder_param);

status=write_header(streamstruct);
if (*(coder_param.flag_meansub)){
status=write_header_mean(streamstruct, mean);
}

#ifdef TEMPWEIGHTCHECKING
imageweight = calloc(npix,sizeof(long int));
imageweightcount = calloc(npix,sizeof(long int));
#endif

#ifdef TIME
start = clock();
#endif


#ifdef EZW
#ifdef SIGNED
ezw_code_signed_c(image, streamstruct, outputsize, coder_param);
#else
ezw_code_c(image, streamstruct, outputsize, coder_param);
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

#ifdef TEMPWEIGHTCHECKING
status = write_hyper("imageweight.img", imageweight, npix, 4);
status = write_hyper("imageweightcount.img", imageweightcount, npix, 4);
#endif

printf("Outputsize: %ld bits\n", *outputsize);


//output coded stream
// #ifdef OUTPUT
// output_file = fopen("/home/christop/Boulot/images/output_stream/output.dat","w");
// #else
output_file = fopen(output_filename,"w");
// #endif
if (output_file == NULL) fprintf(stderr, "Error opening file...\n");
status = fwrite(stream, 1, (*outputsize+7)/8, output_file);
status = fclose(output_file);

#ifdef SIZE
fprintf(stderr, "Wrote %ld bits (code+block header-image header)\n",nbitswritten);
fprintf(stderr, "Wrote %ld header bits\n",nbitswrittenheader);
#endif

return 0;
}


//****************************************************
//        DECODING
//****************************************************


int decode(coder_param_struct coder_param){

char * filename = coder_param.filename;
char * output_filename = coder_param.output_filename;
int type = coder_param.type;
float rate = coder_param.rate;

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

#ifdef TIME
clock_t start, end;
double elapseddecomp;
#endif

int *maxquantvalue=(int*) malloc(sizeof(int));
long int * outputsize =(long int*) malloc(sizeof(int)) ;
long int npix;

int status;
FILE *stream_file;
long int stream_size;
int niloc, njloc, nkloc, nblock;
int iloc, jloc, kloc, blockind;
// coder_param_struct coder_param;

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

stream_file = fopen(filename,"r");
if (stream_file == NULL) fprintf(stderr, "Error opening file...\n");
stream_size = file_size(stream_file);
printf("Reading %ld bytes\n",stream_size);
stream = (unsigned char *) malloc(stream_size*sizeof(unsigned char));
status = fread(stream, 1, stream_size, stream_file);//same as for writing...
status = fclose(stream_file);

*outputsize = stream_size * 8;

streamstruct.stream=stream;
streamstruct.streamlast=streamlast;
streamstruct.count=count;
streamstruct.headerlength=headerlength;

status= read_magic(streamstruct, coder_param);

status=read_header(streamstruct);

if (*(coder_param.flag_meansub)){
printf("Be aware: using mean substraction\n");
mean = (long int *) calloc(imageprop.nbmax,sizeof(long int));
status=read_header_mean(streamstruct, mean);
}


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
init_coder_param(&coder_param, nblock);

if (coder_param.flag_roi){
  read_roi_spec(coder_param); //tmp to do properly
  //warning ! resolutions = decomp +1
  spatdec=imageprop.nresspat-1;
  // spatdec=4;
  specdec=imageprop.nresspec-1;
  coder_param.nlayer=100;// decoder tous les layers pare defaut
} else {

  
  
  //Change here if you want a ROI decoding
  
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
}

npix=imageprop.nsmax*imageprop.nlmax*imageprop.nbmax;
image = (long int *) calloc(npix,sizeof(long int));

#ifdef CHECKEND
imageoriglobal = read_hyper("/home/christop/Boulot/images/output_stream/output-dwt-ori.img", npix, 4);
#endif

#ifdef TIME
start = clock();
#endif

#ifdef EZW
#ifdef SIGNED
ezw_decode_signed_c(image, streamstruct, outputsize, coder_param);
#else
ezw_decode_c(image, streamstruct, outputsize, coder_param);
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


//output decoded dwt
#ifdef OUTPUT

status=write_hyper("/home/christop/Boulot/images/output_stream/output-dwt.img", image, npix, 4);

imagedwtori = read_hyper("/home/christop/Boulot/images/output_stream/output-dwt-ori.img", npix, 4);

err=0;
if (imagedwtori == NULL) fprintf(stderr, "Are you sure you generated the reference dwt during the encoding ?\n");
err = compare_hyper_long(image, imagedwtori);

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

waveletIDWT(image,imageidwt,specdec,spatdec,coder_param);

// mean substraction
if (*(coder_param.flag_meansub)){
add_mean(imageidwt, mean);
}

// #ifdef OUTPUT
// 
// 
// status=write_hyper("/home/christop/Boulot/images/output_stream/output.img", imageidwt, npix, type);
// free(imageidwt);imageidwt=NULL;
// 
// #else

status=write_hyper(output_filename, imageidwt, npix, type);
free(imageidwt);imageidwt=NULL;

// #endif



#ifdef OUTPUT
//processing partial IDWT for imageori dwt also
if (1){


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

waveletDWT(imageoritmp,imagedwtori,imageprop.nresspec-1-specdec,imageprop.nresspat-1-spatdec, coder_param);

imagepartori=(short int *) calloc(npix,sizeof(short int));

//Normalization (to compensate the gain of the low-pass)
for (i=0;i<(imageprop.nresspat-1 - spatdec);i++) factor *= sqrt(2.0); 
for (i=0;i<(imageprop.nresspat-1 - spatdec);i++) factor *= sqrt(2.0); 
for (i=0;i<(imageprop.nresspec-1 - specdec);i++) factor *= sqrt(2.0); 

for (i_l=0;i_l<npix;i_l++){
	pixel.x = i_l % nsnewmax;
	pixel.y = (i_l / nsnewmax) % nlnewmax;
	pixel.l = (i_l / (nsnewmax*nlnewmax));
	imagepartori[i_l]=(short int) ROUND( (imagedwtori[trans_pixel(pixel)]/factor));
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
#endif


#ifdef SIZE
fprintf(stderr, "Read %ld bits\n",nbitsread);
#endif

free(maxquantvalue);
free(outputsize);
free(stream);

return 0;
}


