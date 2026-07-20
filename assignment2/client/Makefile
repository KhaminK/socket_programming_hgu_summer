# 서버 IP가 172.17.0.3일 때 실행:
# make run IP=172.17.0.3

# 서버 IP가 172.17.0.4로 바뀌었을 때 실행:
# make run IP=172.17.0.4

# 내 컴퓨터(로컬)에서 테스트할 때 (기본값 사용):
# make run

CC = gcc
TARGET = uecho_client
SRCS = uecho_client.c
PORT = 9000
# 기본 IP 설정 (입력 안 하면 127.0.0.1로 동작)
IP ?= 127.0.0.1

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET) $(IP) $(PORT)

clean:
	rm -f $(TARGET)