// udp가 한 번에 여러번 보낼 수 있다는 장점을 최대한 살려보려고
// buffer를 5개를 만든다음에 싹 다 보내는 형식으로 했습니다
// 그리고 순서가 뒤죽박죽으로 와도 array에 저장함으로써 다시 보내는 수고를 줄였습니다
// 만약 받아야하는 seq의 ack가 3초 이상 안 오면 다시 보냅니다
// array가 싹 다 비어있으면 Fin 보내고 끝납니다
// ack가 안 와도 되는 이유는 어차피 server는 일정시간 신호 안 받으면 꺼지게 했기 때문입니다
// 구체적인 구현은 아래 주석을 확인해주시면 감사하겠습니다

// 예시처럼 한 번 주고 한 번 기다리면 그냥 신뢰성이 없는 tcp가 되는 것 같아서 이렇게 구현했습니다
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define BUF_SIZE 30
#define FILE_NAME 1024

struct buffer
{
    int seq;
    int time;
    char message[BUF_SIZE];
};
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int sock, gloablSeq = 1;
    socklen_t adr_sz;

    // struct buffer *sender = malloc(sizeof(struct buffer));
    struct sockaddr_in serv_adr, from_adr;
    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        error_handling("socket() error");
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    // got it from stackoverflow code somewhere
    FILE *fp = NULL;
    char fileName[FILE_NAME] = "";

    printf("Enter a filename: ");

    // couldn't implement getting the input from the user
    // if (fscanf(stdin, " %1023[^\n]", fileName) != 1)
    // {
    //     fputs("user canceled input.\n", stderr);
    //     return 1;
    // }

    // file we are sending, yep, the user has no choice
    strcpy(fileName, "uecho_client.c");

    if ((fp = fopen(fileName, "r")) == NULL)
    {
        perror("fopen-a");
        return 1;
    }

    printf("file opened: %s\n\n", fileName);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int currentTime = 0, read_cnt = -1, flag = 0;
    struct buffer sender[5];
    int recSeq;
    // emptying every data
    for (int i = 0; i < 5; i++)
    {
        sender[i].seq = -1;
        sender[i].time = -1;
    }

    int location = 0;

    while (1)
    {
        // printf("checkpoint 1\n");
        // set current time
        currentTime = time(NULL);
        // if something was received, erase that index's time
        adr_sz = sizeof(from_adr);
        // printf("checkpoint 2\n");
        if (recvfrom(sock, &recSeq, sizeof(int), MSG_DONTWAIT,
                     (struct sockaddr *)&from_adr, &adr_sz) != -1)
        {
            // calculate index and empty the data of the index
            int idx = recSeq % 5;
            if (sender[idx].seq == recSeq)
            {
                sender[idx].time = -1;
            }
            sender[idx].time = -1;
        }
        // printf("checkpoint 3\n");
        // loop through array (don't care if client received or not)
        for (int i = 0; i < 5; i++)
        {
            // if the array is not empty
            if (sender[i].time != -1)
            {

                // if there was no response longer than 3 seconds
                if (currentTime - sender[i].time >= 3)
                {
                    // resend the data
                    sendto(sock, &sender[i], sizeof(struct buffer), 0,
                           (struct sockaddr *)&serv_adr, sizeof(serv_adr));
                    // reset the time to current time
                    sender[i].time = currentTime;
                }
            }
        }
        // filling in the empty spaces until it faces filled one
        while (sender[gloablSeq % 5].time == -1)
        {
            int idx = gloablSeq % 5;
            // read some bytes from the file and save to globalSeq's message
            read_cnt = fread((void *)sender[idx].message, 1, BUF_SIZE - 1, fp);
            // if it reads something from the file
            if (read_cnt != 0)
            {
                // putting 0 at the end of the message
                sender[idx].message[read_cnt] = 0;
                // set the time to current time
                sender[idx].time = currentTime;
                // set the index
                sender[idx].seq = gloablSeq;
                // send the file data to the server
                sendto(sock, &sender[idx], sizeof(struct buffer), 0,
                       (struct sockaddr *)&serv_adr, sizeof(serv_adr));
                // increase global seq
                gloablSeq++;
                printf("%d. %s", sender[idx].seq, sender[idx].message);
            }
            // if it reads nothing from the file
            else
            {
                // count how many times the process read the empty file (just in case)
                flag++;
                break;
            }
        }
        // if all files are read, and the flag is up
        if (read_cnt == 0 && flag >= 1)
        {
            int alll_clear = 1;
            // check if the array is all empty
            for (int i = 0; i < 5; i++)
            {
                if (sender[i].time != -1)
                    alll_clear = 0;
            }
            // if array is empty
            if (alll_clear)
            {
                //initialize fin buffer
                struct buffer finisher;
                //setting fin seq
                finisher.seq = -1;
                // printf("checkpoint -1");
                //send FIN, don't care about ack because server will shutdown eventually when it receives nothing
                sendto(sock, &finisher, sizeof(struct buffer), 0, (struct sockaddr *)&serv_adr, sizeof(serv_adr));
                break;
            }
        }
    }

    // first draft
    // while (1)
    // {
    //     sendto(sock, fileName, FILE_NAME, 0,
    //            (struct sockaddr *)&serv_adr, sizeof(serv_adr));
    //     if (recvfrom(sock, &sender->seq, sizeof(int), 0,
    //                  (struct sockaddr *)&from_adr, &adr_sz) == -1)
    //     {
    //         printf("Timeout: File name\n");
    //         continue;
    //     }
    //     if (sender->seq == 0)
    //     {
    //         printf("Successfully sent the name of the file: seq = %d\n", sender->seq);
    //         break;
    //     }
    // }

    // while (1)
    // {
    //     read_cnt = fread((void *)sender->message, 1, BUF_SIZE, fp);
    //     sender->message[read_cnt] = 0;
    //     sendto(sock, sender, sizeof(struct buffer), 0,
    //            (struct sockaddr *)&serv_adr, sizeof(serv_adr));

    //     if (read_cnt == 0)
    //     {
    //         sendto(sock, "", 0, 0,
    //                (struct sockaddr *)&serv_adr, sizeof(serv_adr));
    //         break;
    //     }

    //     // timeout
    //     if (recvfrom(sock, &sender->seq, sizeof(int), 0,
    //                  (struct sockaddr *)&from_adr, &adr_sz) == -1)
    //     {
    //         printf("*Timeout: Message*\n");
    //         continue;
    //     }
    //     // if the last file is sent successfully
    //     else if (sender->seq == -1)
    //     {
    //         printf("**Done Sending the messages: %d**\n", sender->seq);
    //         break;
    //     }
    //     // received wrong seq
    //     else if (sender->seq != gloablSeq)
    //     {
    //         printf("***Wrong seq came in***\n");
    //     }
    //     // received correct seq
    //     else
    //     {
    //         sender->seq++;
    //         gloablSeq++;
    //     }
    // }

    fclose(fp);
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}