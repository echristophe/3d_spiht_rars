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


/**********************************
Interface with QccPack wavelet transform
************************************/
#include "libQccPack.h"
#include "main.h"

long int lround(double x);

int waveletDWT(long int * imagein, long int * imageout, int specdec, int spatdec, coder_param_struct coder_param){

QccVolumeInt input_volume_int=NULL;
QccVolume input_volume=NULL;

QccWAVWavelet Wavelet;

QccString WaveletFilename;

QccString Boundary = "symmetric";

QccWAVPerceptualWeights PerceptualWeights;




int i=0;
int j=0;
int k=0;
long int i_l=0;
long int npix=0;
int err=0;

int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;


npix= nsmax*nlmax*nbmax;

int NumLevels_spat = spatdec;
int NumLevels_spec = specdec;

printf("Wavelet transform using QccPack...\n");


if (*(coder_param.flag_wavelet_int)){
//   QccConvertToQccString(WaveletFilename, "CohenDaubechiesFeauveau.5-3.int.lft");
  QccConvertToQccString(WaveletFilename,  "CohenDaubechiesFeauveau.5-3.lft");
} else {
  QccConvertToQccString(WaveletFilename, QCCWAVWAVELET_DEFAULT_WAVELET);
//   QccConvertToQccString(WaveletFilename,  "CohenDaubechiesFeauveau.5-3.lft");
}

QccWAVWaveletInitialize(&Wavelet); 
QccWAVPerceptualWeightsInitialize(&PerceptualWeights);

QccWAVWaveletCreate(&Wavelet, WaveletFilename, Boundary);

// if (*(coder_param.flag_wavelet_int)){
//   input_volume_int=QccVolumeIntAlloc(nbmax, nlmax, nsmax);
//   QccVolumeIntZero(input_volume_int, nbmax, nlmax, nsmax);
// } else {
  input_volume=QccVolumeAlloc(nbmax, nlmax, nsmax);
  QccVolumeZero(input_volume, nbmax, nlmax, nsmax);
// }

// if (*(coder_param.flag_wavelet_int)){
//   for (i=0; i<nsmax; i++){
//     for (j=0; j<nlmax; j++){
//       for (k=0; k<nbmax; k++){
//         i_l= i + j*nsmax + k*nsmax*nlmax;
//         (*(*(input_volume_int+k) +j))[i] = (double) imagein[i_l];
//       }
//     }
//   }
// } else {
  for (i=0; i<nsmax; i++){
    for (j=0; j<nlmax; j++){
      for (k=0; k<nbmax; k++){
          i_l= i + j*nsmax + k*nsmax*nlmax;
          (*(*(input_volume+k) +j))[i] = (double) imagein[i_l];
      }
    }
  }
// }

free(imagein);imagein=NULL;

// if (*(coder_param.flag_wavelet_int)){
//   err = QccWAVWaveletPacketDWT3DInt(input_volume_int, nbmax, nlmax, nsmax, 0, 0, 0, 0, 0, 0, NumLevels_spec, NumLevels_spat, &Wavelet);
// } else {
  err = QccWAVWaveletPacketDWT3D(input_volume, nbmax, nlmax, nsmax, 0, 0, 0, 0, 0, 0, NumLevels_spec, NumLevels_spat, &Wavelet);
// }

// if (*(coder_param.flag_wavelet_int)){
//   for (i=0; i<nsmax; i++){
//     for (j=0; j<nlmax; j++){
//       for (k=0; k<nbmax; k++){
//         i_l= i + j*nsmax + k*nsmax*nlmax;
//         imageout[i_l] = (long int) lround( (*(*(input_volume_int+k) +j))[i] ); //WARNING check rint()
//       }
//     }
//   }
// } else {
  for (i=0; i<nsmax; i++){
    for (j=0; j<nlmax; j++){
      for (k=0; k<nbmax; k++){
          i_l= i + j*nsmax + k*nsmax*nlmax;
          imageout[i_l] = (long int) lround( (*(*(input_volume+k) +j))[i] ); //WARNING check rint()
      }
    }
  }
// }



// if (*(coder_param.flag_wavelet_int)){
//   QccVolumeIntFree(input_volume_int, nbmax, nlmax);
// } else {
  QccVolumeFree(input_volume, nbmax, nlmax);
// }



return 0;
}



int waveletIDWT(long int * imagein, long int * imageout, int specdec, int spatdec, coder_param_struct coder_param){


QccVolumeInt input_volume_int=NULL;
QccVolume input_volume=NULL;

QccWAVWavelet Wavelet;
 QccString WaveletFilename;
 


QccString Boundary = "symmetric";

QccWAVPerceptualWeights PerceptualWeights;


int i=0;
int j=0;
int k=0;
long int i_l=0;
long int npix=0;
int err=0;


int nsmax=imageprop.nsmax;
int nlmax=imageprop.nlmax;
int nbmax=imageprop.nbmax;


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


if (*(coder_param.flag_wavelet_int)){
//   QccConvertToQccString(WaveletFilename, "CohenDaubechiesFeauveau.5-3.int.lft");
  QccConvertToQccString(WaveletFilename, "CohenDaubechiesFeauveau.5-3.lft");
} else {
  QccConvertToQccString(WaveletFilename, QCCWAVWAVELET_DEFAULT_WAVELET);
//   QccConvertToQccString(WaveletFilename, "CohenDaubechiesFeauveau.5-3.lft");
}

  QccWAVWaveletInitialize(&Wavelet);
  QccWAVPerceptualWeightsInitialize(&PerceptualWeights);

QccWAVWaveletCreate(&Wavelet, WaveletFilename, Boundary);

// if (*(coder_param.flag_wavelet_int)){
//   input_volume_int=QccVolumeIntAlloc(nbmax, nlmax, nsmax);
//   QccVolumeIntZero(input_volume_int, nbmax, nlmax, nsmax);
// } else {
  input_volume=QccVolumeAlloc(nbmax, nlmax, nsmax);
  QccVolumeZero(input_volume, nbmax, nlmax, nsmax);
// }
// if (*(coder_param.flag_wavelet_int)){
//   for (i=0; i<nsmax; i++){
//     for (j=0; j<nlmax; j++){
//       for (k=0; k<nbmax; k++){
//         i_l= i + j*nsmax + k*nsmax*nlmax;
//         (*(*(input_volume_int+k) +j))[i] = (double) imagein[i_l];
//       }
//     }
//   }
// } else {
  for (i=0; i<nsmax; i++){
    for (j=0; j<nlmax; j++){
      for (k=0; k<nbmax; k++){
          i_l= i + j*nsmax + k*nsmax*nlmax;
          (*(*(input_volume+k) +j))[i] = (double) imagein[i_l];
      }
    }
  }
// }

free(imagein);imagein=NULL;

// if (*(coder_param.flag_wavelet_int)){
//   err = QccWAVWaveletInversePacketDWT3DInt(input_volume_int, nbmax, nlmax, nsmax, 0, 0, 0, 0, 0, 0, NumLevels_spec, NumLevels_spat, &Wavelet);
// } else {
  err = QccWAVWaveletInversePacketDWT3D(input_volume, nbmax, nlmax, nsmax, 0, 0, 0, 0, 0, 0, NumLevels_spec, NumLevels_spat, &Wavelet);
// }
// if (*(coder_param.flag_wavelet_int)){
//   for (i=0; i<nsmax; i++){
//     for (j=0; j<nlmax; j++){
//       for (k=0; k<nbmax; k++){
//         i_l= i + j*nsmax + k*nsmax*nlmax;
//         imageout[i_l] = (long int) lround( (*(*(input_volume_int+k) +j))[i]/factor ); //WARNING check rint()
//       }
//     }
//   }
// } else {
  for (i=0; i<nsmax; i++){
    for (j=0; j<nlmax; j++){
      for (k=0; k<nbmax; k++){
          i_l= i + j*nsmax + k*nsmax*nlmax;
          imageout[i_l] = (long int) lround( (*(*(input_volume+k) +j))[i]/factor ); //WARNING check rint()
      }
    }
  }
// }



// if (*(coder_param.flag_wavelet_int)){
//   QccVolumeIntFree(input_volume_int, nbmax, nlmax);
// } else {
  QccVolumeFree(input_volume, nbmax, nlmax);
// }

return 0;

}

int wavelet_check(long int * imagein, long int npix, coder_param_struct coder_param){

long int * image = (long int *) calloc(npix,sizeof(long int));
long int * imageori = (long int *) calloc(npix,sizeof(long int));
long int * imageidwt = (long int *) calloc(npix,sizeof(long int));
long int i_l;
long int maxerr, err;
fprintf(stderr, "Wavelet Check......\n");

for (i_l=0;i_l<npix;i_l++){
	imageori[i_l]=imagein[i_l];
}

waveletDWT(imageori,image,imageprop.nresspec-1,imageprop.nresspat-1, coder_param);
waveletIDWT(image,imageidwt,imageprop.nresspec-1,imageprop.nresspat-1, coder_param);

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

fprintf(stderr, "End check......\n");
return 0;
}
