# -----------------------------------------------------------------------------
# 
# 
# -----------------------------------------------------------------------------

MAKEFLAGS += --no-print-directory

main: main.cpp

CPPFLAGS = -o2 -Wall

% : %.cpp
	gcc $< $(CPPFLAGS) -o $@ -lm

all: main

clean:
	rm -rf ./main

iterate:
	clear && make all && ./main
