.PHONY:all
all:mqtt

mqtt:mqtt.cpp ZLogSystem.cpp cJSON.c
	g++ -o $@ $^ -I /usr/local/include/ -lpaho-mqtt3a -lpthread

.PHONY:clean
clean:
	rm -rf *.o mqtt

