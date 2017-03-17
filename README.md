## Synopsis
This project is about tracking RFID tag by using 2x2 miniature antenna.

Reader Used: ThingMagic M6e

API: mercuryapi-1.29.2.10

OS: Ubuntu 16.04

For more information about original MercuryAPI SDK please visit: http://www.thingmagic.com/manuals-firmware#Universal_Reader_Assistant

### Prerequisites

xsltproc

### Project Structure and Compiling

Get all the repository:

```
git clone https://github.com/dienhoa/RFID_project.git
```
Path of user source code: /RFID_project/dev/src


Path of executable file: /RFID_project/dev/bin 

If you want to compile your code that you have created in /dev/src . Please modified in /api files dev.mk (the make file specified for user code) and Makefile (main makefile)

For Example if I wrote a source code phase_detect.c in /dev/src then I need to add in dev.mk
```
../dev/src/phase_detect.o: $(HEADERS) $(LIB)
../dev/bin/phase_detect: ../dev/src/phase_detect.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread $(LTKC_LIBS)
```
and in Makefile
```
EXE += ../dev/bin/phase_detect
```
To Compile Project:`
```
cd RFID_project/api
make
```
If you don't want to compile the samples files (Samples that created by ThingMagic found in MercuryAPI SDK)
```
make SKIP_SAMPLES=1
```

### Running bin file
```
../dev/bin/phase_detect tmr:///dev/ttyUSB0 --ant 1,2
```
