parade : parade.c
	gcc -Wall -std=c11 -I/home/walker/MyroC/include -L/home/walker/MyroC/lib -lm -lMyroC -lbluetooth -ljpeg -leSpeakPackage parade.c -o parade

.PHONY : clean
clean :
	rm -rf parade *.o *.core
