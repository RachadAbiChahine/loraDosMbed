run: build
	mv BUILD/NUCLEO_F401RE/GCC_ARM/transiver.bin /media/$(USER)/NUCLEO/

clean:
	rm -rf BUILD

build:
	mbed compile -m NUCLEO_F401RE -t GCC_ARM
		