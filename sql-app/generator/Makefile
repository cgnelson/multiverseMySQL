CPPFLAGS = -Wall -g 
LDLIBS = -lmysqlcppconn 
CXX = g++
CFILES = generator.cc

all: generator

generator: $(CFILES)
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -rf generator