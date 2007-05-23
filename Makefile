
#  Hyperspectral compression program
# 
#  Name:		Makefile	
#  Author:		Emmanuel Christophe	
#  Contact:		e.christophe at melaneum.com
#  Description:		Utility functions for hyperspectral image compression
#  Version:		v1.1 - 2006-10	
 

# options possibles
# Taille images
# S64 pour des images de 64x64x56, 
# par defaut 256x256x224

# Structure darbre
# NEWTREE suppression du lien spectral dans la majorite des cas. (SINON overlapping tree)
# EZWTREE descendance de la LLL type EZW (attention si applique a spiht a l'init) deprecated ?
# par defaut structure redondante et LLL type spiht
# INPLACE insert element in LIS in place instead of at the end (avoid zerotrees ?)

# SKIPWAV: skip wavelet transform (assuming that the file provided is already a transform)
# WAV53 : use the 5/3 wavelet transform (with the modified QccPack, corresponds to lossless transform)
# NORA: disable random access (to correspond to the traditionnal SPIHT)
# RES_SCAL: resolution scalability ordering (all bitplanes are processed for one resolution before going to the next one)
# LSC_BEFORE: put the LSC processing before the LIS to be able to jump the last LIS for partial decoding (save few hundrends bits)
# OLDRATE: trigger the original distortion computation in SPIHT which does not consider the deadzone quantizer


# Information printing
# TIME : compute and print time information
# SIZE : compute and print size information
# DEBUG : many details
# DEBUG2: more synthetic information
# OUTPUT: output intermediates wavelet hypercubes

#WARNING:
#Do not use the -DEZW option anymore, it is now automatically included

CC = LC_ALL=C gcc

LIBS = -lQccPack -lm
# LIBS = -lQccPack051 -lm
# LIBS = -lm 
# LIBS = -lm -lefence

# C_OPT = -g  -DEZWTREE -DS64 -DEZW #pour debug
# C_OPT = -g 
# C_OPT = -O3 -ansi
# C_OPT = -O3 -DNEWTREE -DS64 -DEZW
# C_OPT = -O3 -DEZWTREE -DEZW -DLATEX
# C_OPT = -O3 -DEZWTREE -DEZW -DDEBUG
# C_OPT = -O3 -DEZWTREE -DEZW -DDEBUG -DOUTPUT -DMEANSUB
# C_OPT = -g -DEZWTREE -DEZW -DDEBUG -DSIGNED
# C_OPT = -O3 -DEZWTREE -DEZW -DDEBUG -DSIGNED -DEZW_ARITH -DOUTPUT -DMEANSUB -DOUTPUTSIGNED
# C_OPT = -O3 -DEZWTREE -DNEWTREE -DEZW  -DDEBUG 
# C_OPT = -g -DEZW

#EZW AT (spat tree)
# C_OPT = -O3 -DNEWTREE -DEZW -DDEBUG
# C_OPT = -O3 -DNEWTREE -DEZW -DDEBUG -DEZWUSEZ
#EZW OT (3D tree)
# C_OPT = -O3 -DEZW -DDEBUG 
# C_OPT = -O3 -DEZW -DDEBUG -DEZWUSEZ

#EZW SIGNED AT (spat tree) -101
# C_OPT = -O3 -DNEWTREE -DEZW -DSIGNED -DDEBUG 
# C_OPT = -O3 -DNEWTREE -DEZW -DSIGNED -DDEBUG -DEZWUSEZ
#EZW SIGNED OT (3D tree)
# C_OPT = -O3 -DEZW -DSIGNED -DDEBUG 
# C_OPT = -O3 -DEZW -DSIGNED -DDEBUG -DEZWUSEZ

#EZW SIGNED AT (spat tree) 011
# C_OPT = -O3 -DNEWTREE -DEZW -DSIGNED -DSIGNED011 -DDEBUG 
# C_OPT = -O3 -DNEWTREE -DEZW -DSIGNED -DSIGNED011 -DDEBUG -DEZWUSEZ
#EZW SIGNED OT (3D tree)
# C_OPT = -O3 -DEZW -DSIGNED -DSIGNED011 -DDEBUG 
# C_OPT = -O3 -DEZW -DSIGNED -DSIGNED011 -DDEBUG -DEZWUSEZ

#best perf, test avec ARITH
# C_OPT = -O3 -DNEWTREE -DEZW -DDEBUG -DEZW_ARITH

#test 
# C_OPT = -O3 -DEZW -DDEBUG -DEZWUSEZ
# C_OPT = -O3 -DEZW -DDEBUG -DEZWUSEZ -DEZW_ARITH -DMEANSUB -DEZW_ARITH_RESET_MODEL -DEZWNOREF
# C_OPT = -O3 -DEZW -DDEBUG  -DNEWTREE2
# C_OPT = -O3 -DEZW -DDEBUG -DNEWTREE -DEZWUSEZ -DEZWREFAFTER -DEZW_ARITH 
#-DEZW_ARITH_RESET_MODEL
# C_OPT = -O3 -DEZW -DDEBUG -DNEWTREE -DEZWUSEZ -DEZW_ARITH -DEZW_ARITH_RESET_MODEL -DEZWNOREF

# C_OPT = -O3 -DNEWTREE -DEZW -DSIGNED -DDEBUG -DEZWUSEZ -DEZWREF
# C_OPT = -O3 -DEZW -DSIGNED -DSIGNED011 -DDEBUG -DEZWUSEZ -DEZWREF
# C_OPT = -O3 -DEZW  -DNEWTREE2 -DSIGNED -DDEBUG -DEZWUSEZ -DEZWREF -DEZWREFAFTER -DEZW_ARITH
# C_OPT = -O3 -DEZW  -DNEWTREE -DSIGNED -DDEBUG -DEZWUSEZ -DEZWREF -DEZW_ARITH -DEZW_ARITH_CONTEXT
# C_OPT = -O3 -DEZW -DDEBUG -DSIGNED -DEZWUSEZ -DEZW_ARITH -DEZW_ARITH_CONTEXT -DEZW_ARITH_RESET_MODEL
# C_OPT = -O3 -DEZW -DDEBUG -DNEWTREE2 -DSIGNED -DEZWUSEZ -DEZW_ARITH -DEZW_ARITH_CONTEXT -DEZW_ARITH_RESET_MODEL  -DEZWREF
# C_OPT = -O3 -DEZW -DDEBUG -DNEWTREE2 -DEZWUSEZ -DEZW_ARITH -DEZWREFAFTER -DEZW_ARITH_RESET_MODEL -DWAV53

#Classic EZW without ref pass
# C_OPT = -O3 -DEZW -DDEBUG -DNEWTREE2 -DEZWUSEZ -DEZWNOREF -DEZW_ARITH -DEZW_ARITH_RESET_MODEL 
#Signed EZW without ref pass and without arith context
# C_OPT = -O3 -DEZW -DDEBUG -DNEWTREE2 -DSIGNED -DEZWUSEZ -DEZW_ARITH -DEZW_ARITH_RESET_MODEL
#Get statistics
# C_OPT = -O3 -DEZW -DDEBUG -DNEWTREE2 -DSIGNED -DOUTPUTSIGNED
#Signed EZW without ref pass and WITH arith context
# C_OPT = -O3 -DEZW -DDEBUG -DNEWTREE2 -DSIGNED -DEZWUSEZ -DEZW_ARITH -DEZW_ARITH_CONTEXT -DEZW_ARITH_RESET_MODEL

#REFERENCES
#produce slightly better results than EZW on barbara (result on paper)
# C_OPT = -g -DDEBUG -DEZWUSEZ -DEZW_ARITH -DEZWREFAFTER -DEZW_ARITH_RESET_MODEL 
#signed representation without ref pass, perform almost as well as EZW
# C_OPT = -g -DDEBUG -DNEWTREE -DSIGNED -DEZWUSEZ -DEZW_ARITH -DEZW_ARITH_CONTEXT -DEZW_ARITH_RESET_MODEL
#en 3D, on rajoute en plus l'arbre...
# C_OPT = -O3 -DDEBUG -DNEWTREE2 -DEZWUSEZ -DEZW_ARITH -DEZWREFAFTER -DEZW_ARITH_RESET_MODEL 
# C_OPT = -O3 -DDEBUG -DNEWTREE2 -DSIGNED -DEZWUSEZ -DEZW_ARITH -DEZW_ARITH_CONTEXT -DEZW_ARITH_RESET_MODEL



# spiht
# C_OPT = -O3  -DDEBUG
# spiht2 -> correspond l'implementation QccPack
# C_OPT = -O3 -DNEWTREE  -DDEBUG
# spiht3
# C_OPT = -g -DINPLACE -DDEBUG
# test
# C_OPT = -O3 -DNEWTREE -DINPLACE -DDEBUG
# Suppression de la liste B
# C_OPT = -g -DNOLISTB  -DNEWTREE -DDEBUG2 -DS64
# C_OPT = -g -DNOLISTB  -DNEWTREE -DDEBUG2 -DRES_SCAL -DTIME -DS64
# C_OPT = -Wall -O3 -DNOLISTB  -DNEWTREE -DDEBUG2 -DTIME  
# C_OPT = -Wall -O3  -DNEWTREE -DDEBUG2 -DTIME -DS512  -DRES_SCAL -DRES_RATE
# C_OPT = -g -DNOLISTB  -DNEWTREE -DDEBUG2 -DNOENC
# C_OPT = -Wall -O3  -DNEWTREE -DDEBUG2 -DTIME -DRES_SCAL -DRES_RATE
# C_OPT = -Wall -O3  -DNEWTREE -DTIME -DDEBUG2 -DRES_SCAL -DLSCBEFORE -DSIZE
# C_OPT = -Wall -g  -DNEWTREE -DTIME -DDEBUG2  -DSIZE -DCHECKEND
# C_OPT = -Wall -g  -DNEWTREE -DTIME -DDEBUG2 -DSIZE
# C_OPT = -Wall -O3  -DNEWTREE -DTIME -DDEBUG2  -DSIZE -DWAV53

#SPIHT original (AT tree)
# C_OPT = -g  -DDEBUG -DNEWTREE -DNORA
#SPIHT original (3D tree)
# C_OPT = -O3  -DDEBUG -DNORA
#SPIHT RA (resolution scalable, but no separation between spatial and spectral)
# C_OPT = -Wall -O3  -DNEWTREE -DTIME -DDEBUG2 -DSIZE

#for testing only
# C_OPT = -Wall -g -DNEWTREE -DSIZE -DDEBUG2 



#SPIHT RARS
C_OPT = -Wall -O3  -DNEWTREE -DDEBUG2 -DTIME -DRES_SCAL -DSIZE -DLSCBEFORE
# C_OPT = -Wall -O3  -DNEWTREE -DDEBUG2 -DTIME -DRES_SCAL -DSIZE
# C_OPT = -Wall -O3  -DNEWTREE -DDEBUG2 -DTIME -DSIZE -DOLDRATE0 
# C_OPT = -Wall -g -DNEWTREE -DDEBUG2 -DTIME -DSIZE -DOUTPUT -DTEMPWEIGHTCHECKING
# C_OPT = -Wall -g -DNEWTREE -DDEBUG2 -DTIME -DSIZE -DRES_SCAL
# C_OPT = -Wall -g -DNEWTREE -DDEBUG2 -DTIME -DSIZE

# C_OPT = -Wall -g  -DNEWTREE -DDEBUG2 -DTIME -DSIZE  -DNORA

#lossless
# C_OPT = -Wall -O3  -DNEWTREE -DDEBUG2 -DTIME -DRES_SCAL -DSIZE -DLSCBEFORE  -DWAV53 -DOUTPUT
# C_OPT = -Wall -O3  -DNEWTREE -DDEBUG2 -DTIME -DSIZE  -DWAV53 -DOUTPUT
# C_OPT = -Wall -O3  -DNEWTREE -DTIME -DDEBUG2 -DSIZE -DSKIPWAV
# C_OPT = -O3 -DEZW -DDEBUG -DSKIPWAV
# C_OPT = -O3 -DEZW -DDEBUG  -DWAV53

PROF = 
# PROF = -pg
# PROF =  -fprofile-arcs -ftest-coverage  -pg

DEP = main.h Makefile

all : spihtcode ezwcode

spihtcode : spiht_code_c.o desc.o  main_spiht.o signdigit.o utils.o wavelet_c.o spiht_code_ra5.o 
	$(CC) $(PROF) $(LIBS) -o spihtcode spiht_code_c.o desc.o  main_spiht.o signdigit.o  utils.o wavelet_c.o spiht_code_ra5.o 

ezwcode : desc_ezw.o ezw_code_c.o main_ezw.o signdigit.o desc_ezw_signed.o ezw_code_signed_c.o  utils.o wavelet_c.o 
	$(CC) $(PROF) $(LIBS) -o ezwcode desc_ezw.o ezw_code_c.o  main_ezw.o signdigit.o desc_ezw_signed.o ezw_code_signed_c.o  utils.o wavelet_c.o 

#just in case a dynamic librairy is required	
libspiht.so : spiht_code_c.o desc.o desc_ezw.o ezw_code_c.o   signdigit.o utils.o  desc_ezw_signed.o ezw_code_signed_c.o wavelet_c.o spiht_code_ra5.o 
	$(CC) $(PROF) $(LIBS) -shared -o libspiht.so spiht_code_c.o desc.o desc_ezw.o ezw_code_c.o  signdigit.o desc_ezw_signed.o ezw_code_signed_c.o utils.o wavelet_c.o spiht_code_ra5.o 

main_spiht.o : main.c $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c  -o main_spiht.o main.c

main_ezw.o : main.c $(DEP)
	$(CC) $(PROF) $(C_OPT) -DEZW -Wall -c -o main_ezw.o main.c

utils.o : utils.c $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c utils.c 

ezw_code_c.o : ezw_code_c.c $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c ezw_code_c.c 

ezw_code_signed_c.o : ezw_code_signed_c.c $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c ezw_code_signed_c.c 

spiht_code_c.o : spiht_code_c.c $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c spiht_code_c.c 

spiht_code_ra5.o : spiht_code_ra5.c $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c spiht_code_ra5.c

desc.o : desc.c  $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c desc.c 
	
desc_ezw.o : desc_ezw.c  $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c desc_ezw.c 

signdigit.o : signdigit.c  $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c signdigit.c

desc_ezw_signed.o : desc_ezw_signed.c  $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c desc_ezw_signed.c 

wavelet_c.o : wavelet_c.c  $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c wavelet_c.c 


package:
	tar cf - Makefile README *.c *.h | gzip -9 > ../spiht3d_c.tar.gz
 
clean :
	rm -f prog *.o  
 
 