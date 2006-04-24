#include "libQccPack.h"
// #include "spiht_code_c.h"
#include "main.h"
// #include "math.h"

long int lround(double x);

int waveletDWT(long int * imagein, long int * imageout, int specdec, int spatdec){

QccVolume input_volume;
QccVolume output_volume;
// QccVolume output2_volume;
QccWAVWavelet Wavelet;

#ifdef WAV53
QccString WaveletFilename = "CohenDaubechiesFeauveau.5-3.lft";
#else
QccString WaveletFilename = QCCWAVWAVELET_DEFAULT_WAVELET;
#endif
QccString Boundary = "symmetric";

QccWAVPerceptualWeights PerceptualWeights;
// int UsePerceptualWeights = 0;



// int NumSubbands;

int i=0;
int j=0;
int k=0;
long int i_l=0;
long int npix=0;
int err=0;

// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;
// int nsmin=imageprop.nsmin;
// int nlmin=imageprop.nlmin;
// int nbmin=imageprop.nbmin;

npix= nsmax*nlmax*nbmax;

//can be derived from nsmax / nsmin
// #ifdef S64
// int NumLevels_spat = 3;
// int NumLevels_spec = 3;
// #else
// int NumLevels_spat = 5;
// int NumLevels_spec = 5;
// #endif
// int NumLevels_spat = imageprop.nresspat-1;
// int NumLevels_spec = imageprop.nresspec-1;
int NumLevels_spat = spatdec;
int NumLevels_spec = specdec;

printf("Wavelet transform using QccPack...\n");
// printf("5/3...\n");
// printf("nsmax: %d \n", nsmax);
// printf("nlmax: %d \n", nlmax);
// printf("nbmax: %d \n", nbmax);
// printf("nsmin: %d \n", nsmin);
// printf("nlmin: %d \n", nlmin);
// printf("nbmin: %d \n", nbmin);
// printf("npix: %ld \n", npix);


QccWAVWaveletInitialize(&Wavelet);
QccWAVPerceptualWeightsInitialize(&PerceptualWeights);

QccWAVWaveletCreate(&Wavelet, WaveletFilename, Boundary);

//int QccWAVWaveletPacketDWT3D(const QccVolume input_volume, QccVolume output_volume, int num_frames, int num_rows, int num_cols, int origin_frame, int origin_row, int origin_col, int subsample_pattern_frame, int subsample_pattern_row, int subsample_pattern_col, int temporal_num_scales, int spatial_num_scales, const QccWAVWavelet *wavelet); 

input_volume=QccVolumeAlloc(nbmax, nlmax, nsmax);

// output2_volume=QccVolumeAlloc(nbmax, nlmax, nsmax);

QccVolumeZero(input_volume, nbmax, nlmax, nsmax);
// printf("Sample: %f \n", (**input_volume)[0]);

for (i=0; i<nsmax; i++){
for (j=0; j<nlmax; j++){
for (k=0; k<nbmax; k++){
        i_l= i + j*nsmax + k*nsmax*nlmax;
	(*(*(input_volume+k) +j))[i] = (double) imagein[i_l];
}
}
}

free(imagein);
output_volume=QccVolumeAlloc(nbmax, nlmax, nsmax);

err = QccWAVWaveletPacketDWT3D(input_volume,output_volume, nbmax, nlmax, nsmax, 0, 0, 0, 0, 0, 0, NumLevels_spec, NumLevels_spat, &Wavelet);

QccVolumeFree(input_volume, nbmax, nlmax);
// imageout = (long int *) malloc(npix*sizeof(long int));

for (i=0; i<nsmax; i++){
for (j=0; j<nlmax; j++){
for (k=0; k<nbmax; k++){
        i_l= i + j*nsmax + k*nsmax*nlmax;
// 	imageout[i_l] = (long int) round( (*(*(output_volume+k) +j))[i] );
	imageout[i_l] = (long int) lround( (*(*(output_volume+k) +j))[i] ); //WARNING check rint()
// 	imageout[i_l] = (long int) (*(*(output_volume+k) +j))[i] ;
}
}
}

// err = QccWAVWaveletInversePacketDWT3D(output_volume,output2_volume, nbmax, nlmax, nsmax, 0, 0, 0, 0, 0, 0, NumLevels_spec, NumLevels_spat, &Wavelet);


QccVolumeFree(output_volume, nbmax, nlmax);
// QccVolumeFree(output2_volume, nbmax, nlmax);



return 0;
}



int waveletIDWT(long int * imagein, long int * imageout, int specdec, int spatdec){

QccVolume input_volume;
QccVolume output_volume;
// QccVolume output2_volume;
QccWAVWavelet Wavelet;

#ifdef WAV53
QccString WaveletFilename = "CohenDaubechiesFeauveau.5-3.lft";
#else
QccString WaveletFilename = QCCWAVWAVELET_DEFAULT_WAVELET;
#endif
QccString Boundary = "symmetric";

QccWAVPerceptualWeights PerceptualWeights;
// int UsePerceptualWeights = 0;


// int NumSubbands;

int i=0;
int j=0;
int k=0;
long int i_l=0;
long int npix=0;
int err=0;

// struct imageprop_struct imageprop={NSMAX_CONST, NLMAX_CONST, NBMAX_CONST, NSMIN_CONST, NLMIN_CONST, NBMIN_CONST};

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;
// int nsmin=imageprop.nsmin;
// int nlmin=imageprop.nlmin;
// int nbmin=imageprop.nbmin;



// #ifdef S64
// int NumLevels_spat = 3;
// int NumLevels_spec = 3;
// #else
// int NumLevels_spat = 5;
// int NumLevels_spec = 5;
// #endif
// int NumLevels_spat = imageprop.nresspat-1;
// int NumLevels_spec = imageprop.nresspec-1;
int NumLevels_spat = spatdec;
int NumLevels_spec = specdec;
double factor=1.0;

nsmax=(nsmax >> (imageprop.nresspat-1 - spatdec));
nlmax=(nlmax >> (imageprop.nresspat-1 - spatdec));
nbmax=(nbmax >> (imageprop.nresspec-1 - specdec));

npix= nsmax*nlmax*nbmax;

for (i=0;i<(imageprop.nresspat-1 - spatdec);i++) factor *= sqrt(2.0); 
for (i=0;i<(imageprop.nresspat-1 - spatdec);i++) factor *= sqrt(2.0); 
for (i=0;i<(imageprop.nresspec-1 - specdec);i++) factor *= sqrt(2.0); 

printf("Wavelet inverse transform using QccPack...\n");
// printf("5/3...\n");
// printf("nsmax: %d \n", nsmax);
// printf("nlmax: %d \n", nlmax);
// printf("nbmax: %d \n", nbmax);
// printf("nsmin: %d \n", nsmin);
// printf("nlmin: %d \n", nlmin);
// printf("nbmin: %d \n", nbmin);
// printf("npix: %ld \n", npix);


  QccWAVWaveletInitialize(&Wavelet);
  QccWAVPerceptualWeightsInitialize(&PerceptualWeights);

QccWAVWaveletCreate(&Wavelet, WaveletFilename, Boundary);

//int QccWAVWaveletPacketDWT3D(const QccVolume input_volume, QccVolume output_volume, int num_frames, int num_rows, int num_cols, int origin_frame, int origin_row, int origin_col, int subsample_pattern_frame, int subsample_pattern_row, int subsample_pattern_col, int temporal_num_scales, int spatial_num_scales, const QccWAVWavelet *wavelet); 

input_volume=QccVolumeAlloc(nbmax, nlmax, nsmax);

// output2_volume=QccVolumeAlloc(nbmax, nlmax, nsmax);

QccVolumeZero(input_volume, nbmax, nlmax, nsmax);
// printf("Sample: %f \n", (**input_volume)[0]);

for (i=0; i<nsmax; i++){
for (j=0; j<nlmax; j++){
for (k=0; k<nbmax; k++){
        i_l= i + j*nsmax + k*nsmax*nlmax;
	(*(*(input_volume+k) +j))[i] = (double) imagein[i_l];
}
}
}

free(imagein);
output_volume=QccVolumeAlloc(nbmax, nlmax, nsmax);

err = QccWAVWaveletInversePacketDWT3D(input_volume,output_volume, nbmax, nlmax, nsmax, 0, 0, 0, 0, 0, 0, NumLevels_spec, NumLevels_spat, &Wavelet);

QccVolumeFree(input_volume, nbmax, nlmax);
// imageout = (long int *) malloc(npix*sizeof(long int));

for (i=0; i<nsmax; i++){
for (j=0; j<nlmax; j++){
for (k=0; k<nbmax; k++){
        i_l= i + j*nsmax + k*nsmax*nlmax;
// 	imageout[i_l] = (long int) round( (*(*(output_volume+k) +j))[i] );
	imageout[i_l] = (long int) lround( (*(*(output_volume+k) +j))[i]/factor ); //WARNING check rint()
// 	imageout[i_l] = (long int) (*(*(output_volume+k) +j))[i];
}
}
}

// err = QccWAVWaveletInversePacketDWT3D(output_volume,output2_volume, nbmax, nlmax, nsmax, 0, 0, 0, 0, 0, 0, NumLevels_spec, NumLevels_spat, &Wavelet);


QccVolumeFree(output_volume, nbmax, nlmax);
// QccVolumeFree(output2_volume, nbmax, nlmax);

return 0;

}

int wavelet_check(long int * imagein, long int npix){

long int * image = (long int *) calloc(npix,sizeof(long int));
long int * imageori = (long int *) calloc(npix,sizeof(long int));
long int * imageidwt = (long int *) calloc(npix,sizeof(long int));
long int i_l;
long int maxerr, err;
fprintf(stderr, "Wavelet Check......\n");

for (i_l=0;i_l<npix;i_l++){
	imageori[i_l]=imagein[i_l];
}

waveletDWT(imageori,image,imageprop.nresspec-1,imageprop.nresspat-1);
waveletIDWT(image,imageidwt,imageprop.nresspec-1,imageprop.nresspat-1);

maxerr=0;
err=0;
for (i_l=0;i_l<npix;i_l++){
	if (abs(imagein[i_l]-imageidwt[i_l]) > maxerr){
		maxerr=abs(imagein[i_l]-imageidwt[i_l]);
		err=1;
	};
};

if (err) {
fprintf(stderr, "ERREUR MAX %ld\n",maxerr);
} else {
fprintf(stderr, "Decoding OK (maxerr= %ld)\n",maxerr);
}

fprintf(stderr, "End check......\n",maxerr);
return 0;
}
