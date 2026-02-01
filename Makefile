CC = g++-14
all:
		$(CC) -O3 -march=native -std=c++11 -o "SSGA" SSGA.h
		$(CC) -O3 -march=native -std=c++11 -o "KTNS" KTNS.h
		$(CC) -O3 -march=native -std=c++11 -o "SampleDecoder" SampleDecoder.h
		$(CC) -O3 -march=native -std=c++11 -o "samplecode" samplecode.cpp
		$(CC) -O3 -march=native -std=c++11 -o "run" run.cpp