// ack 사라지면 어쩔건데? 기도해야죠
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
#define FILEPATH 4096
#define FILENAME 1024
#define MAXFILES 50

void error_handling(char *message);

struct file
{
    int fileSize;
    char filePath[FILEPATH];
    char fileName[FILENAME];
};

int main(int argc, char *argv[])
{
    int sd;
    FILE *fp;

    char buf[BUF_SIZE];
    int read_cnt, totalRead = 0;
    struct file **fileList = malloc(sizeof(struct file *) * MAXFILES);
    struct sockaddr_in serv_adr;
    int fileCount = 0;
    int ack = 1;

    if (argc != 3)
    {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    // fp = fopen("receive.dat", "wb");
    sd = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    connect(sd, (struct sockaddr *)&serv_adr, sizeof(serv_adr));

    // receving file count
    while (totalRead < sizeof(int))
    {
        read_cnt = read(sd, &fileCount, sizeof(int));
        totalRead += read_cnt;
    }
    write(sd, &ack, sizeof(int));
    printf("file numbers: %d\n", fileCount);
    totalRead = 0;

    for (int i = 0; i < fileCount; i++)
    {
        fileList[i] = malloc(sizeof(struct file));
        int nameRead = 0;
        
        while (nameRead < FILENAME) {
            read_cnt = read(sd, fileList[i]->fileName + nameRead, FILENAME - nameRead);
            nameRead += read_cnt;
        }
        
        write(sd, &ack, sizeof(int)); 
        printf("%d. %s\n", i + 1, fileList[i]->fileName);
    }
        



    puts("Received file data");
    close(sd);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}