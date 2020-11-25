export LD_LIBRARY_PATH=/usr/local/lib

CPPFLAGS = -Wall -g -I../sql-parser/src/ -L../ -L/usr/local/lib
LDLIBS = -lmysqlcppconn -lsqlparser
CXX = g++
CFILES = ../sql_user.hh ../sql_proxy.hh ../sql_proxy.cc ../sql_server.hh ../sql_server.cc

all: main  

main: main.cc $(CFILES)
	$(CXX) $(CPPFLAGS)  $^ -o $@ $(LDLIBS)

clean:
	rm -rf main  