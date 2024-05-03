.PHONY:all
all:mqtt

mqtt:pahoclient.cpp.cpp ZLogSystem.cpp cJSON.c
	g++ -o $@ $^ -I /usr/local/include/ -lpaho-mqtt3a -lpthread

.PHONY:clean
clean:
	rm -rf *.o dmt_crypt mqtt

