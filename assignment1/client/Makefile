# 기본 실행: make run (172.17.0.2의 9000번 포트로 접속)

# IP 및 포트 변경해서 실행: make run IP=172.17.0.3 PORT=9001

CC = gcc
TARGET = file_client
SRC = file_client.c
# 기본 IP와 포트 설정
IP ?= 172.17.0.2
PORT ?= 9000

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) $(IP) $(PORT)

clean:
	rm -f $(TARGET)