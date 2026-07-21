# 컴파일 및 기본 실행: make run (자동으로 9000번 포트로 실행됩니다)

# 포트 변경해서 실행: make run PORT=9001

# 실행 파일 삭제: make clean

CC = gcc
TARGET = file_server
SRC = file_server.c
# 기본 포트 설정 (실행 시 덮어쓰기 가능)
PORT ?= 9000

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) $(PORT)

clean:
	rm -f $(TARGET)