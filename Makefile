# options possibles
# Taille images
# S64 pour des images de 64x64x56, 
# par defaut 256x256x224

# Structure darbre
# NEWTREE suppression du lien spectral dans la majorite des cas. (SINON overlapping tree)
# EZWTREE descendance de la LLL type EZW (attention si applique a spiht a l'init) deprecated ?
# par defaut structure redondante et LLL type spiht
# INPLACE insert element in LIS in place instead of at the end (avoid zerotrees ?)

# TIME : compute and print time information
# SIZE : compute and print size information

CC = LC_ALL=C gcc

LIBS = -lQccPack -lm
# LIBS = -lm 
# LIBS = -lm -lefence

# C_OPT = -g  -DEZWTREE -DS64 -DEZW #pour debug
# C_OPT = -g 
# C_OPT = -O3 -ansi
# C_OPT = -O3 -DNEWTREE -DS64 -DEZW
# C_OPT = -O3 -DEZWTREE -DEZW -DLATEX
# C_OPT = -O3 -DEZWTREE -DEZW -DDEBUG
# C_OPT = -O3 -DEZWTREE -DEZW -DDEBUG -DSIGNED
#  C_OPT = -O3 -DEZWTREE -DNEWTREE -DEZW  -DDEBUG 
# C_OPT = -g -DEZW

# spiht
# C_OPT = -O3  -DDEBUG
# spiht2 -> correspond �l'implementation QccPack
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
# C_OPT = -Wall -O3  -DNEWTREE -DTIME -DDEBUG2 -DSIZE
# C_OPT = -Wall -O3  -DNEWTREE -DTIME -DDEBUG2  -DSIZE -DWAV53

#SPIHT original (AT tree)
# C_OPT = -O3  -DDEBUG -DNEWTREE -DNORA
#SPIHT original (3D tree)
# C_OPT = -O3  -DDEBUG -DNORA -DINPLACE
#SPIHT RA (resolution scalable, but no separation between spatial and spectral)
# C_OPT = -Wall -O3  -DNEWTREE -DTIME -DDEBUG2 -DSIZE
#SPIHT RARS
# C_OPT = -Wall -O3  -DNEWTREE -DDEBUG2 -DTIME -DRES_SCAL -DSIZE -DLSCBEFORE
# C_OPT = -Wall -O3  -DNEWTREE -DDEBUG2 -DTIME -DRES_SCAL -DSIZE

#lossless
C_OPT = -Wall -O3  -DNEWTREE -DDEBUG2 -DTIME -DRES_SCAL -DSIZE -DLSCBEFORE  -DWAV53

PROF = 
# PROF = -pg
# PROF =  -fprofile-arcs -ftest-coverage  -pg

DEP = main.h Makefile

all : libspiht.so spihtcode

spihtcode : spiht_code_c.o desc.o desc_ezw.o ezw_code_c.o  main.o signdigit.o desc_ezw_signed.o ezw_code_signed_c.o  utils.o wavelet_c.o spiht_code_ra5.o 
	$(CC) $(PROF) $(LIBS) -o spihtcode spiht_code_c.o desc.o desc_ezw.o ezw_code_c.o  main.o signdigit.o desc_ezw_signed.o ezw_code_signed_c.o  utils.o wavelet_c.o spiht_code_ra5.o 
	
libspiht.so : spiht_code_c.o desc.o desc_ezw.o ezw_code_c.o  main.o signdigit.o utils.o  desc_ezw_signed.o ezw_code_signed_c.o   wavelet_c.o spiht_code_ra5.o 
	$(CC) $(PROF) $(LIBS) -shared -o libspiht.so spiht_code_c.o desc.o desc_ezw.o ezw_code_c.o  main.o signdigit.o desc_ezw_signed.o ezw_code_signed_c.o   utils.o wavelet_c.o spiht_code_ra5.o 

main.o : main.c $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c main.c 

utils.o : utils.c $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c utils.c 

ezw_code_c.o : ezw_code_c.c $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c ezw_code_c.c 

ezw_code_signed_c.o : ezw_code_signed_c.c $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c ezw_code_signed_c.c 

spiht_code_c.o : spiht_code_c.c $(DEP)
	$(CC) $(PROF) $(C_OPT) -Wall -c spiht_code_c.c 

# spiht_code_ra2.o : spiht_code_ra2.c $(DEP)
# 	$(CC) $(PROF) $(C_OPT) -Wall -c spiht_code_ra2.c
# 
# spiht_code_ra3.o : spiht_code_ra3.c $(DEP)
# 	$(CC) $(PROF) $(C_OPT) -Wall -c spiht_code_ra3.c

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

# desc_cumul.o : desc_cumul.c  $(DEP)
# 	$(CC) $(PROF) $(C_OPT) -Wall -c desc_cumul.c 

package:
	tar cf - Makefile README *.c *.h | gzip -9 > ../spiht3d_c.tar.gz
 
clean :
	rm -f prog *.o  
 
 