../dev/src/read.o: $(HEADERS) $(LIB)
../dev/bin/read: ../dev/src/read.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread $(LTKC_LIBS)

../dev/src/phase_detect.o: $(HEADERS) $(LIB)
../dev/bin/phase_detect: ../dev/src/phase_detect.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread $(LTKC_LIBS)