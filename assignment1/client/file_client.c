// ack 사라지면 어쩔건데? 기도해야죠
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
#define FILENAME 1024
#define MAXFILES 50

void error_handling(char *message);

//don't need path here
struct file
{
    int fileSize;
    char fileName[FILENAME];
};

int main(int argc, char *argv[])
{
    int sd;
    FILE *fp;

    char buf[BUF_SIZE];
    int read_cnt, totalRead = 0;
    
    struct sockaddr_in serv_adr;
    int fileCount = 0;
    int ack = 1;

    if (argc != 3)
    {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sd = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    connect(sd, (struct sockaddr *)&serv_adr, sizeof(serv_adr));

    // receving file count
    while (totalRead < sizeof(int))
    {
        read_cnt = read(sd, &fileCount + totalRead, sizeof(int) - totalRead);
        totalRead += read_cnt;
    }
    printf("file numbers: %d\n", fileCount);
    struct file **fileList = malloc(sizeof(struct file *) * fileCount);
    totalRead = 0;

    // dynamically allocate memory of files in filelist
    for (int i = 0; i < fileCount; i++)
    {
        fileList[i] = malloc(sizeof(struct file));
        totalRead = 0;
        // read the name of the file and initialize the data
        while (totalRead < FILENAME)
        {
            // it starts reading from the part where it left off
            // and read remaining file name
            read_cnt = read(sd, fileList[i]->fileName + totalRead, FILENAME - totalRead);
            totalRead += read_cnt;
        }
        totalRead = 0;
        // read the size of the file and initialize it
        while (totalRead < sizeof(int))
        {
            read_cnt = read(sd, (&fileList[i]->fileSize) + totalRead, sizeof(int) - totalRead);
            totalRead += read_cnt;
        }

        printf("%d. %s (%d)\n", i + 1, fileList[i]->fileName, fileList[i]->fileSize);
    }
    totalRead = 0;

    int index = -1;
    //requesting a file to send
    while (index < 1 || index > fileCount)
    {
        printf("Which file do you want? ");
        scanf("%d", &index);
    }
    index--;
    // send the index of file to receive
    write(sd, &index, sizeof(int));

    //let's assume the file was opened successfully
    fp = fopen(fileList[index]->fileName, "wb");
    printf("opened the file \n");

    //getting the input until total read becomes same as fileSize
    while (totalRead < fileList[index]->fileSize)
    {
        if ((read_cnt = read(sd, buf, BUF_SIZE)) == 0)
            break;
        buf[read_cnt] = 0;
        fwrite((void *)buf, 1, read_cnt, fp);
        totalRead += read_cnt;
    }

    puts("Received file data");
    //포항 야호~
    write(sd, "Yaho", 5);
    fclose(fp);

    close(sd);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}