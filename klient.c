#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <unistd.h> 
#include<pthread.h>
#include <stdbool.h>
#include <string.h>

#define size 8



void printBoard(char board[256]) {
    printf("  a b c d e f g h\n");
    for (int i = 0; i < size; i++) {
        printf("%d ", size-i);
        for (int j = 0; j < size; j++)
            printf("%c ", board[i*8+j]);
        printf("%d\n", size-i);
    }
    printf("  a b c d e f g h\n");
}


int main(int argc, char *argv[]){

    char board[256];
    char buff[256];
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    clientSocket = socket(PF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    /*serverAddr.sin_port = htons(1100);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");*/
    serverAddr.sin_port = htons(atoi(argv[1]));
    serverAddr.sin_addr.s_addr = inet_addr(argv[2]);

    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    addr_size = sizeof serverAddr;
    connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
    bzero(buff,256);
    recv(clientSocket,buff,256,0);
    printf("%s",buff);
    
    bzero(board,256);
    recv(clientSocket,board,256,0);
    if (buff[15]=='b')
    {
        printBoard(board);
    }
    int n;
    int sent=1;
    char board1[256];
    while(true){
        if(sent==1)
        {
            bzero(buff,256);
            n=recv(clientSocket,buff,256,0);
            if(n<=0) break;
            printf("%s",buff);
            
            bzero(board1,256);
            n=recv(clientSocket,board1,256,0);
            if(n<=0) break;
            if (!strcmp(board1,"terminate")) break;
            printBoard(board1);  
            sent=0;
        }

        char msg[256];
        fgets(msg, sizeof(msg), stdin);
        if(!strcmp(msg,"exit\n"))
        {
            printf("Are you sure you want to exit\n");
            printf("1 yes, anything else no\n");
            scanf("%255s",msg);
            if(!strcmp(msg,"1"))
            {
                printf("Exiting\n");
                printf("exited\n");
                break;
            }
        }

        n=send(clientSocket,msg,sizeof(msg),0);
        

        bzero(buff,256);
        n=recv(clientSocket,buff,256,0);
        if(n<=0) break;
        printf("%s",buff);
        bzero(board1,256);
        n=recv(clientSocket,board1,256,0);
        if (buff[0]=='n' && buff[1]=='o') 
        {
            sent=1;
        }
        if(n<=0) break;
        if (!strcmp(board1,"terminate")) break;
        printBoard(board1);  
    }
    if(n<=0) printf("No connection from server\n");
    close(clientSocket);

    return 0;
}
