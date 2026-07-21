// 자세한 설명은 client 확인
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define FILE_NAME 1024

void error_handling(char *message);
int countChar(char *message);
struct buffer
{
    int seq;
    int time;
    int size;
    char message[BUF_SIZE];
};

int main(int argc, char *argv[])
{
    int serv_sock;
    char fileName[BUF_SIZE];
    int str_len;
    // 10 seconds without response = kill the process
    struct timeval optVal = {10, 0};
    socklen_t clnt_adr_sz;

    // struct buffer *receiver = malloc(sizeof(struct buffer));
    struct sockaddr_in serv_adr, clnt_adr;
    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (serv_sock == -1)
        error_handling("UDP socket creation error");
    int optLen = sizeof(optVal);
    setsockopt(serv_sock, SOL_SOCKET, SO_RCVTIMEO, &optVal, optLen);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    FILE *fp = NULL;
    // couldn't implement getting file name from the client
    strcpy(fileName, "binary.jpeg");
    // strcpy(fileName, "uecho_client.c");

    // opening the file as write mode
    if ((fp = fopen(fileName, "wb")) == NULL)
    {
        perror("fopen-a");
        return 1;
    }
    printf("file opened: %s\n\n", fileName);

    struct buffer sender[5];
    struct buffer temp;
    int recvCnt;
    int location = 0;
    for (int i = 0; i < 5; i++)
    {
        sender[i].seq = -1;
        sender[i].time = -1;
    }

    int flag = 0;
    while (1)
    {
        clnt_adr_sz = sizeof(clnt_adr);
        // receive the buffer
        // if nothing is received for 10 seconds, terminates
        recvCnt = recvfrom(serv_sock, &temp, sizeof(struct buffer), 0,
                           (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        if (recvCnt == -1)
        {
            break;
        }
        else
        {
            // if it receives terminating seq, terminate
            if (temp.seq == -1)
            {
                printf("received terminating seq\n");
            }
            printf("received %d\n", temp.seq); 
            // send client exact same seq
            sendto(serv_sock, &temp.seq, sizeof(int), 0,
                   (struct sockaddr *)&clnt_adr, clnt_adr_sz);
            // if wrong seq was received (smaller than the location or bigger than array index)
            if (temp.seq < location || temp.seq >= location + 5)
                continue;
            int idx = temp.seq % 5;
            sender[idx] = temp;

            // writing until it's stucked
            while (sender[location % 5].seq == location)
            {
                int tempmIdx = location % 5;
                // printf("%d. %d\n", sender[tempmIdx].seq, sender[tempmIdx].size);
                fwrite((void *)sender[tempmIdx].message, 1, sender[tempmIdx].size, fp);
                // fwrite((void *)sender[tempmIdx].message, 1, strlen(sender[tempmIdx].message), fp);
                sender[tempmIdx].seq = -1;
                if (temp.size < BUF_SIZE)
                    flag++;
                location++;
            }
        }
        if (flag)
        {
            printf("terminating\n");
            break;
        }
    }
    fclose(fp);
    close(serv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}