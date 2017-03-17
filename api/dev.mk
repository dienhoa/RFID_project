../dev/src/read.o: $(HEADERS) $(LIB)
../dev/bin/read: ../dev/src/read.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread $(LTKC_LIBS)

../dev/src/phase_detect.o: $(HEADERS) $(LIB)
../dev/bin/phase_detect: ../dev/src/phase_detect.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread $(LTKC_LIBS)

../dev/src/test_config.o: $(HEADERS) $(LIB)
../dev/bin/test_config: ../dev/src/test_config.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread $(LTKC_LIBS)

../dev/src/readasync.o: $(HEADERS) $(LIB)
../dev/bin/readasync: ../dev/src/readasync.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread $(LTKC_LIBS)

../dev/src/readasyncfilter.o: $(HEADERS) $(LIB)
../dev/bin/readasyncfilter: ../dev/src/readasyncfilter.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread $(LTKC_LIBS)