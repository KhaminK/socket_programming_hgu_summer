# 컴파일: make

# 실행: make run

# 실행 파일 삭제: make clean

CC = gcc
TARGET = uecho_server
SRCS = uecho_server.c
PORT = 9000

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET) $(PORT)

clean:
	rm -f $(TARGET)