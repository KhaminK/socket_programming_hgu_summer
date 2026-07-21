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

// 파일 갯수를 보냄 size of int
// 파일 리스트 보냄 (filePath + fileSize) (1024+4096)* fileCount
// 보낼 파일을 입력 받아 (fileName) 1024
// 파일 보내 (30 size buffer * buffer횟수-1 + 마지막 찌꺼기) fileSize

void search_directory(struct file **fileList, char path[FILEPATH]);
void error_handling(char *message);

int fileCount = 0;

int main(int argc, char *argv[])
{
    int serv_sd, clnt_sd;
    FILE *fp;
    char buf[BUF_SIZE];
    char path[FILEPATH] = "/root/assignment1";
    int read_cnt, totalRead = 0;
    struct file **fileList = malloc(sizeof(struct file *) * MAXFILES);

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    search_directory(fileList, path);

    serv_sd = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    bind(serv_sd, (struct sockaddr *)&serv_adr, sizeof(serv_adr));
    listen(serv_sd, 5);

    clnt_adr_sz = sizeof(clnt_adr);
    clnt_sd = accept(serv_sd, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);

    int ack = 0;

    // sending number of file
    write(clnt_sd, &fileCount, sizeof(int));

    // sending all file names in the list
    for (int i = 0; i < fileCount; i++)
    {
        printf("Sending: %s\n", fileList[i]->fileName);
        write(clnt_sd, fileList[i]->fileName, FILENAME);
        write(clnt_sd, &fileList[i]->fileSize, sizeof(int));
    }

    int index;
    while (totalRead < sizeof(int))
    {
        read_cnt = read(clnt_sd, &index + totalRead, sizeof(int) - totalRead);
        totalRead += read_cnt;
    }
    printf("Received Index: %d\n", index);

    // open requested file
    fp = fopen(fileList[index]->filePath, "rb");

    //send the data of the file
    while (1)
    {
        read_cnt = fread((void *)buf, 1, BUF_SIZE, fp);
        if (read_cnt < BUF_SIZE)
        {
            write(clnt_sd, buf, read_cnt);
            break;
        }
        write(clnt_sd, buf, BUF_SIZE);
    }

    //shutdown writing
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