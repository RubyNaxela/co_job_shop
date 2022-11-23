CC = g++
SOURCES = main.cpp convert.cpp test.cpp dataset.hpp heuristics.hpp platform.hpp schedule.hpp timer.hpp

job_shop: $(SOURCES)
	$(CC) main.cpp -o job_shop -std=gnu++2a -O3
	$(CC) convert.cpp -o convert -std=gnu++2a -O3
	$(CC) test.cpp -o test -std=gnu++2a -O3

exe: $(SOURCES)
	$(CC) main.cpp -o job_shop.exe -std=gnu++2a -O3
	$(CC) convert.cpp -o convert.exe -std=gnu++2a -O3
	$(CC) test.cpp -o test.exe -std=gnu++2a -O3

clean:
	rm -f job_shop
	rm -f convert
	rm -f test
	rm -f *.exe