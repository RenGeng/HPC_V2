CXX=mpic++ -Wall -O3

CPPFLAGS=-Iinc

LDFLAGS=-lboost_program_options -lm

FILES=export forward init memory parse_args shalw
OBJECTS=$(addsuffix .o, $(FILES))
BIN=bin/shalw

all : $(BIN)
	make exec

$(BIN) : $(addprefix obj/, $(OBJECTS))
	$(CXX) -o $@ $^ $(LDFLAGS)

obj/%.o : src/%.cpp
	$(CXX) -c -o $@ $^ $(CPPFLAGS)

obj/%.o : src/%.c
	$(CXX) -c -o $@ $^ $(CPPFLAGS)

clean :
	rm -f bin/* obj/*

exec:
	# mpirun -n 4 -bynode ./bin/shalw -x 8192 -y 8192 -t 20
	mpirun -n 4 -bynode ./bin/shalw --export --export-path /tmp/3408406/
	# ./visu.py shalw_256x256_T1000.sav
	# diff base.sav shalw_256x256_T1000.sav
