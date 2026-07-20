#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUF_SIZE 30
#define FILENAME 1024
#define FILEPATH 4096
#define MAXFILES 50

struct file
{
    int fileSize;
    char filePath[FILEPATH];
    char fileName[FILENAME];
};

// 몇개 파일이 있는지 확인
// 갯수를 보냄 (int) size of 4
// 파일 리스트 보냄 (filePath * 갯수)
// 파일 입력 받아 (fileNameSize)
// 파일 보내 (30 size buffer * buffer횟수-1 + 마지막 찌꺼기)

void search_directory(struct file **fileList, char path[FILEPATH]);
void error_handling(char *message);

int fileCount = 0;

int main(int argc, char *argv[])
{
    int serv_sd, clnt_sd;
    FILE *fp;
    char buf[BUF_SIZE];
    char path[FILEPATH] = "/root/assignment1";
    int read_cnt;
    struct file **fileList = malloc(sizeof(struct file *) * MAXFILES);

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    search_directory(fileList, path);

    fp = fopen("file_server.c", "rb");
    serv_sd = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    bind(serv_sd, (struct sockaddr *)&serv_adr, sizeof(serv_adr));
    listen(serv_sd, 5);

    clnt_adr_sz = sizeof(clnt_adr);
    clnt_sd = accept(serv_sd, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
    int flag = fcntl(clnt_sd, F_GETFL, 0);
    fcntl(clnt_sd, F_SETFL, flag | O_NONBLOCK);

    int ack = 0;
    int i = 0;

    write(clnt_sd, &fileCount, sizeof(int));

    while (!ack)
    {
        if (read(clnt_sd, &ack, sizeof(int)) > 0)
        {
            printf("Received ACK for fileCount: %d\n", ack);
        }
    }
    ack = 0;

    for (int i = 0; i < fileCount; i++)
    {
        ack = 0; 
        printf("Sending: %s\n", fileList[i]->fileName);
        
        write(clnt_sd, fileList[i]->fileName, FILENAME); 
        
        while (!ack)
        {
            if (read(clnt_sd, &ack, sizeof(int)) > 0) {
                break; 
            }
        }
    }

    shutdown(clnt_sd, SHUT_WR);
    read(clnt_sd, buf, BUF_SIZE);
    printf("Message from client: %s \n", buf);

    for (int i = 0; i < fileCount; i++)
    {
        free(fileList[i]);
    }
    free(fileList);
    fclose(fp);
    close(clnt_sd);
    close(serv_sd);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void search_directory(struct file **fileList, char path[FILEPATH])
{
    DIR *dir = opendir(path);
    if (!dir)
        return;

    struct dirent *entry;
    struct stat statbuf;
    char tempPath[FILEPATH];

    while ((entry = readdir(dir)) != NULL)
    {
        // skip . and .. files
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        // combine dirpath and the read the name of the file or directory
        snprintf(tempPath, FILEPATH, "%s/%s", path, entry->d_name);
        // if nothing is read, skip
        if (stat(tempPath, &statbuf) == -1)
            continue;

        // if it is directory recursively call the function again
        if (S_ISDIR(statbuf.st_mode))
        {
            search_directory(fileList, tempPath);
        }
        // if it is regular file, add to the directory
        else if (S_ISREG(statbuf.st_mode) && fileCount < MAXFILES)
        {
            fileList[fileCount] = malloc(sizeof(struct file));
            // copy the path
            strncpy(fileList[fileCount]->filePath, tempPath, FILEPATH - 1);
            fileList[fileCount]->filePath[FILEPATH - 1] = '\0';

            // copy the file name
            strncpy(fileList[fileCount]->fileName, entry->d_name, FILENAME - 1);
            fileList[fileCount]->fileName[FILENAME - 1] = '\0';

            // copy the file size
            fileList[fileCount]->fileSize = statbuf.st_size;

            fileCount++;
        }
    }
    closedir(dir);
}