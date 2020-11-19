CPPFLAGS = -Wall -g -I./sql-parser/src/ -L./
LDLIBS = -lmysqlcppconn -lsqlparser
CXX = g++
CFILES = main.cc sql_user.hh sql_proxy.hh sql_proxy.cc sql_server.hh sql_server.cc

all: main

main: $(CFILES)
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -rf main 