# options possibles
# Taille images
# S64 pour des images de 64x64x56, 
# par defaut 256x256x224

# Structure d´arbre
# NEWTREE suppression du lien spectral dans la majorite des cas.
# EZWTREE descendance de la LLL type EZW (attention si applique a spiht a l'init)
# par defaut structure redondante et LLL type spiht
# INPLACE insert element in LIS in place instead of at the end (avoid zerotrees ?)

CC = LC_ALL=C gcc

# LIBS = -lQccPack -lm
LIBS = -lm 
# LIBS = -lm -lefence

# C_OPT = -g  -DEZWTREE -DS64 -DEZW #pour debug
# C_OPT = -g 
# C_OPT = -O3 -ansi
# C_OPT = -O3 -DNEWTREE -DS64 -DEZW
#  C_OPT = -O3 -DEZWTREE  -DEZW
#  C_OPT = -O3 -DEZWTREE -DNEWTREE -DEZW
# C_OPT = -g -DEZW

# spiht
# C_OPT = -O3  -DDEBUG
# spiht2 -> correspond à l'implementation QccPack
# C_OPT = -O3 -DNEWTREE  -DDEBUG
# spiht3
# C_OPT = -g -DINPLACE -DDEBUG
# test
# C_OPT = -O3 -DNEWTREE -DINPLACE -DDEBUG
# Suppression de la liste B
# C_OPT = -g -DNOLISTB  -DNEWTREE -DDEBUG2 -DS64
C_OPT = -O3 -DNOLISTB  -DNEWTREE -DDEBUG2
# C_OPT = -g -DNOLISTB  -DNEWTREE -DDEBUG2 -DNOENC

PROF = 
# PROF = -pg
# PROF =  -fprofile-arcs -ftest-coverage  -pg

all : libspiht.so spihtcode

spihtcode : spiht_code_c.o desc.o desc_ezw.o ezw_code_c.o  main.o signdigit.o desc_ezw_signed.o ezw_code_signed_c.o spiht_code_ra.o desc_cumul.o 
	$(CC) $(PROF) $(LIBS) -o spihtcode spiht_code_c.o desc.o desc_ezw.o ezw_code_c.o  main.o signdigit.o desc_ezw_signed.o ezw_code_signed_c.o spiht_code_ra.o desc_cumul.o 
	
libspiht.so : spiht_code_c.o desc.o desc_ezw.o ezw_code_c.o  main.o signdigit.o desc_ezw_signed.o ezw_code_signed_c.o spiht_code_ra.o desc_cumul.o 
	$(CC) $(PROF) $(LIBS) -shared -o libspiht.so spiht_code_c.o desc.o desc_ezw.o ezw_code_c.o  main.o signdigit.o desc_ezw_signed.o ezw_code_signed_c.o spiht_code_ra.o desc_cumul.o 

main.o : main.c
	$(CC) $(PROF) $(C_OPT) -Wall -c main.c 

ezw_code_c.o : ezw_code_c.c
	$(CC) $(PROF) $(C_OPT) -Wall -c ezw_code_c.c 

ezw_code_signed_c.o : ezw_code_signed_c.c
	$(CC) $(PROF) $(C_OPT) -Wall -c ezw_code_signed_c.c 

spiht_code_c.o : spiht_code_c.c
	$(CC) $(PROF) $(C_OPT) -Wall -c spiht_code_c.c 

spiht_code_ra.o : spiht_code_ra.c
	$(CC) $(PROF) $(C_OPT) -Wall -c spiht_code_ra.c

desc.o : desc.c 
	$(CC) $(PROF) $(C_OPT) -Wall -c desc.c 
	
desc_ezw.o : desc_ezw.c 
	$(CC) $(PROF) $(C_OPT) -Wall -c desc_ezw.c 

signdigit.o : signdigit.c 
	$(CC) $(PROF) $(C_OPT) -Wall -c signdigit.c

desc_ezw_signed.o : desc_ezw_signed.c 
	$(CC) $(PROF) $(C_OPT) -Wall -c desc_ezw_signed.c 

# wavelet_c.o : wavelet_c.c 
# 	$(CC) $(PROF) $(C_OPT) -Wall -c wavelet_c.c 

desc_cumul.o : desc_cumul.c 
	$(CC) $(PROF) $(C_OPT) -Wall -c desc_cumul.c 
 
clean :
	rm -f prog *.o  
 
 