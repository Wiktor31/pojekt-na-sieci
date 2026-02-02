#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <unistd.h> 
#include<pthread.h>
#include <stdbool.h>
#include <ctype.h>
#include <signal.h>

#define N 10
#define size 8

char board[N][size][size];
int games[N];
int turns[N];
bool ended[N];

bool good_style(char msg[256])
{
    if (msg[0]<='h' && msg[0]>='a' && msg[1]<='8' && msg[1]>='1' && msg[3]<='h' && msg[3]>='a' && msg[4]<='8' && msg[4]>='1')
        return true;
    return false;
}

void init(int n) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i < 3 && (i + j) % 2 == 1) {
                board[n][i][j] = 'b';
            }
            else if (i > 4 && (i + j) % 2 == 1) {
                board[n][i][j] = 'w';
            }
            else {
                board[n][i][j] = '.';
            }
        }
    }
    turns[n]=1;
}

void init_fast_end(int n) {
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            board[n][i][j] = '.';

    board[n][6][1] = 'W';

    board[n][5][2] = 'B';
    board[n][3][4] = 'B';
    board[n][1][6] = 'B';
    turns[n]=1;
}

void quins_only(int n) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i < 1 && (i + j) % 2 == 1) {
                board[n][i][j] = 'B';
            }
            else if (i > 6 && (i + j) % 2 == 1) {
                board[n][i][j] = 'W';
            }
            else {
                board[n][i][j] = '.';
            }
        }
    }
    turns[n]=1;
}


char* send_table(int k) {
    char* checkers = malloc(65);
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            checkers[i*8+j] = board[k][i][j];
    checkers[64] = '\0';
    return checkers;
}
bool can_take1(int id,int turn,int sx,int sy)
{
    char opponent;
    char color;
    color=(turn==1 ? 'w' : 'b');
    if (tolower(board[id][sx][sy])!=color) return false;
    opponent=(turn==1 ? 'b' : 'w');
    if (board[id][sx][sy]==tolower(board[id][sx][sy]))
    {
        int side;
        if (turn==1) side=-2;
        else side=2;  
        if (sx+side>=0 && sx+side<size)
        {
            if (sy-2>=0)
            {
                if(board[id][sx+side][sy-2]=='.' && tolower(board[id][sx+(side/2)][sy-1])==opponent)
                return true;
            }
            if (sy+2<size)
            {
                if(board[id][sx+side][sy+2]=='.' && tolower(board[id][sx+(side/2)][sy+1])==opponent)
                return true;
            }
        }
    }
    else if (board[id][sx][sy]==toupper(board[id][sx][sy]))
    {
        int sides1[4]={-1,-1,1,1};
        int sides2[4]={-1,1,1,-1};
        for (int i=0;i<4;i++)
        {
            int j=1;
            while (sx+(sides1[i]*j)<size && sx+(sides1[i]*j)>=0 && sy+(sides2[i]*j)<size && sy+(sides2[i]*j)>=0 && board[id][sx+(sides1[i]*j)][sy+(sides2[i]*j)]=='.') 
            {
                
                j+=1;
            }
            int k=j+1;
            if(tolower(board[id][sx+(sides1[i]*j)][sy+(sides2[i]*j)])==opponent && sx+(sides1[i]*k)<size && sx+(sides1[i]*k)>=0 && sy+(sides2[i]*k)<size && sy+(sides2[i]*k)>=0
             && board[id][sx+(sides1[i]*k)][sy+(sides2[i]*k)]=='.') 
                {
                    return true;
                }
        }
    }
    return false;
}
bool takes2(int id,int turn,int sx,int sy,int dx,int dy)
{
    char opponent;
    char color;
    color=(turn==1 ? 'w' : 'b');
    if (tolower(board[id][sx][sy])!=color) return false;
    opponent=(turn==1 ? 'b' : 'w');
    if (board[id][sx][sy]==tolower(board[id][sx][sy]))
    {
        int side;
        if (turn==1) side=-2;
        else side=2;  
        if (sx+side>=0 && sx+side<size)
        {
            if (sy-2>=0)
            {
                if(board[id][sx+side][sy-2]=='.' && tolower(board[id][sx+(side/2)][sy-1])==opponent && sx+(side)==dx && sy-2== dy)
                {
                    board[id][sx+side][sy-2]=color;
                    board[id][sx][sy]='.';
                    board[id][sx+(side/2)][sy-1]='.';
                    return true;
                }
            }
            if (sy+2<size)
            {
                if(board[id][sx+side][sy+2]=='.' && tolower(board[id][sx+(side/2)][sy+1])==opponent && sx+(side)==dx && sy+2== dy)
                {
                    board[id][sx+side][sy+2]=color;
                    board[id][sx][sy]='.';
                    board[id][sx+(side/2)][sy+1]='.';
                    return true;
                }
            }
        }
    }
    else if (board[id][sx][sy]==toupper(board[id][sx][sy]))
    {
        int sides1[4]={-1,-1,1,1};
        int sides2[4]={-1,1,1,-1};
        for (int i=0;i<4;i++)
        {
            int j=1;
            while (sx+(sides1[i]*j)<size && sx+(sides1[i]*j)>=0 && sy+(sides2[i]*j)<size && sy+(sides2[i]*j)>=0 && board[id][sx+(sides1[i]*j)][sy+(sides2[i]*j)]=='.') 
            {
                
                j+=1;
            }
            int k=j+1;
            if(tolower(board[id][sx+(sides1[i]*j)][sy+(sides2[i]*j)])==opponent && sx+(sides1[i]*k)<size && sx+(sides1[i]*k)>=0 && sy+(sides2[i]*k)<size && sy+(sides2[i]*k)>=0
             && board[id][sx+(sides1[i]*k)][sy+(sides2[i]*k)]=='.' && sx+(sides1[i]*k)==dx && sy+(sides2[i]*k) == dy) 
                {
                    board[id][dx][dy]=board[id][sx][sy];
                    board[id][sx+(sides1[i]*j)][sy+(sides2[i]*j)]='.';
                    board[id][sx][sy]='.';
                    return true;
                }
        }
    }
    return false;
}

bool can_take(int id,int turn)
{
    for (int i=0;i<size;i++)
    {
        for (int j=0;j<size;j++)
        {
            if (can_take1(id,turn,i,j)) return true;
        }
    }
    return false;
}

bool do_normal_move(int id,int turn,int sx,int sy,int dx,int dy)
{
    char color;
    color=(turn==1 ? 'w' : 'b');
    if (tolower(board[id][sx][sy])!=color) return false;
    if (board[id][sx][sy]==tolower(board[id][sx][sy]))
    {
        int side;
        if (turn==1) side=-1;
        else side=1;  
        if (sx+side>=0 && sx+side<size)
        {
            if (sy-1>=0)
            {
                if(board[id][sx+side][sy-1]=='.' && tolower(board[id][sx+(side)][sy-1])=='.' && sx+(side)==dx && sy-1== dy)
                {
                    board[id][sx+side][sy-1]=color;
                    board[id][sx][sy]='.';
                    return true;
                }
            }
            if (sy+1<size)
            {
                if(board[id][sx+side][sy+1]=='.' && tolower(board[id][sx+(side)][sy+1])=='.' && sx+(side)==dx && sy+1== dy)
                {
                    board[id][sx+side][sy+1]=color;
                    board[id][sx][sy]='.';
                    return true;
                }
            }
        }
        return false;
    }
    else if (board[id][sx][sy]==toupper(board[id][sx][sy]))
    {
        int sides1[4]={-1,-1,1,1};
        int sides2[4]={-1,1,1,-1};
        for (int i=0;i<4;i++)
        {
            int j=1;
            while (sx+(sides1[i]*j)<size && sx+(sides1[i]*j)>=0 && sy+(sides2[i]*j)<size && sy+(sides2[i]*j)>=0 && board[id][sx+(sides1[i]*j)][sy+(sides2[i]*j)]=='.') 
            {
                if(sx+(sides1[i]*j)==dx && sy+(sides2[i]*j)==dy) 
                {
                    board[id][sx+(sides1[i]*j)][sy+(sides2[i]*j)]=board[id][sx][sy];
                    board[id][sx][sy]='.';
                    return true;
                }
                j+=1;
            }

        }
        
        
        
    }
    return false;
}

int do_move(char mes[256],int id,int turn)
{
    if (!good_style(mes)) return -1;
    int sx=mes[0]-'a',sy=7-mes[1]+'1',dx=mes[3]-'a',dy=7-mes[4]+'1';
    int true_turn;
    if (tolower(board[id][sy][sx])=='w') true_turn=1;
    else if (tolower(board[id][sy][sx])=='b') true_turn=2;
    else return 0;

    if (true_turn!=turn) return 0;
    
    if (can_take(id,turn))
    {
        if (takes2(id,turn,sy,sx,dy,dx))
        {
            if (can_take1(id,turn,dy,dx)) return 2;
            else return 3;
        }
        else return 1;
    }
    if (do_normal_move(id,turn,sy,sx,dy,dx)) return 3;
    else return 0;
    return 3;
    
}
int do_next_move(char mes[256],int id,int turn,char for_next[2])
{
    if (for_next[0] != mes[0] || for_next[1] != mes[1]) return -1;
    int sx=mes[0]-'a',sy=7-mes[1]+'1',dx=mes[3]-'a',dy=7-mes[4]+'1';
    if (takes2(id,turn,sy,sx,dy,dx))
        {
            if (can_take1(id,turn,dy,dx)) return 1;
            else return 2;
        }
        else return 0;

}

void upgrade(int id,int turn)
{
    char color = (turn == 1 ? 'b' : 'w');
    int j=(turn == 1 ? 7 : 0);
    for (int i=0;i<size;i++)
    {
        if (board[id][j][i]==color)
        {
            board[id][j][i]=toupper(color);
        }
    }
}

bool loser(int id,int turn)
{
    char color = (turn == 1 ? 'w' : 'b');
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (tolower(board[id][i][j]) == color) {
                return false;
            }
            
        }
    }
    return true;
}

void * socketThread(void *arg)
{
    void ** table=(void**)arg;
    int klient = *((int *)table[0]);
    int game_id = *((int *)table[1]);
    int turn;
    printf("game_id %d\n", game_id);
    if (games[game_id]==1) 
    {
        turn=1;
        send(klient,"you are plaing white\n",256,0);
    }    
    else 
    { 
        turn=2;
        send(klient,"you are plaing black\n",256,0);
    }
    int opponent=(turn==1 ? 2 : 1);
    int n;
    while(games[game_id]!=2)
    {
        sleep(1);
    }
    char* board_line1=send_table(game_id);

    send(klient,board_line1,256,0);
    free(board_line1);
    int wal=-1;
    char for_next[2];
    for_next[0] = '.';
    for_next[1] = '.';
    for(;;){
        if(opponent == turns[game_id])
        {
            sleep(1);
        }
        else
        {
        if (loser(game_id,turn))
        {
            turns[game_id]=turn*100;
            
        }
        //jak turns[game_id] = 20 to bialy wygral bo czarny wyszedl
        //jak turns[game_id] = 10 to czarny wygral bo bialy wyszedl
        //jak turns[game_id] = 200 to bialy wygral bo czarny nie ma pionkow
        //jak turns[game_id] = 100 to czarny wygral bo bialy nie ma pionkow

        if (turns[game_id]>5)
        {
            if (turns[game_id] == 20) {
                send(klient, "white wins black disconnected\n", 256, 0);
                char* board_line1=send_table(game_id);
                n=send(klient,board_line1,256,0);
                free(board_line1);
            }
            else if (turns[game_id] == 10) {
                send(klient, "black wins white disconnected\n", 256, 0);
                char* board_line1=send_table(game_id);
                n=send(klient,board_line1,256,0);
                free(board_line1);
            }
            else if (turns[game_id] == 200) {
                send(klient, "white wins black has no pieces left\n", 256, 0);
                char* board_line1=send_table(game_id);
                n=send(klient,board_line1,256,0);
                free(board_line1);
            }
            else if (turns[game_id] == 100) {
                send(klient, "black wins white has no pieces left\n", 256, 0);
                char* board_line1=send_table(game_id);
                n=send(klient,board_line1,256,0);
                free(board_line1);
            }
            ended[game_id]=true;
            break;
        }
        
        
        upgrade(game_id,turn);

        send(klient,"your turn now\n",256,0);
        char* board_line1=send_table(game_id);
        n=send(klient,board_line1,256,0);
        free(board_line1);
        wal=-2;  
        while (wal!=3)
        {
            
            if (wal==-1)
            {
                send(klient,"wrong style of move example of good style for writing move \"a1 b2\"\n",256,0);
                char* board_line=send_table(game_id);
                send(klient,board_line,256,0);
                free(board_line);
                
                    
            }
            if (wal==0)
            {
                send(klient,"wrong move\n",256,0);
                char* board_line=send_table(game_id);
                send(klient,board_line,256,0);
                free(board_line);
                
                    
            }
            else if (wal==1)
            {
                send(klient,"can take, wrong move\n",256,0);
                char* board_line=send_table(game_id);
                send(klient,board_line,256,0);
                free(board_line);
                   
            }
            else if (wal==2)
            {
                int wal1=-2;
                char msg2[256];
                snprintf(msg2, 256, "next move for %c%c\n", for_next[0], for_next[1]);
                send(klient, msg2, 256, 0);
                char* board_line=send_table(game_id);
                send(klient,board_line,256,0);
                free(board_line);
                while (wal1!=2)
                {
                    
                    char mes1[256];
                    bzero(mes1,256);
                    n=recv(klient , mes1 , 256 , 0);
                    wal1=do_next_move(mes1,game_id,turn,for_next);
                    if (wal1==-1)
                    {
                        char msg3[256];
                        bzero(msg3,256);
                        snprintf(msg3, 256, "wrong base move, next move for %c%c\n", for_next[0], for_next[1]);
                        send(klient, msg3, 256, 0);
                        char* board_line=send_table(game_id);
                        send(klient,board_line,256,0);
                        free(board_line);
                    }
                    else if (wal1==0)
                    {
                        char msg3[256];
                        bzero(msg3,256);
                        snprintf(msg3, 256, "doesn't take, next move for %c%c\n", for_next[0], for_next[1]);
                        send(klient, msg3, 256, 0);
                        char* board_line=send_table(game_id);
                        send(klient,board_line,256,0);
                        free(board_line);
                    }
                    else if (wal1==1)
                    {
                        for_next[0] = mes1[3];
                        for_next[1] = mes1[4];
                        char msg3[256];
                        bzero(msg3,256);
                        snprintf(msg3, 256, "good move, next move for %c%c\n", for_next[0], for_next[1]);
                        send(klient, msg3, 256, 0);
                        char* board_line=send_table(game_id);
                        send(klient,board_line,256,0);
                        free(board_line);
                    }
                    if(n<=1) break;
                }
                turns[game_id]=(turns[game_id]==2 ? 1 : 2);
                wal=-1;
                break;
                
                   
            }
            char mes[256];
            bzero(mes,256);
            n=recv(klient , mes , 256 , 0);
            for_next[0] = mes[3];
            for_next[1] = mes[4];

            if(n<1){
                break;
            }
            wal=do_move(mes,game_id,turn);
            if (wal==3)
            {
                for_next[0] = '.';
                for_next[1] = '.';
                
                send(klient, "now enemy moves", 256, 0);
                char* board_line1=send_table(game_id);
                n=send(klient,board_line1,256,0);
                free(board_line1);
                turns[game_id]=(turns[game_id]==2 ? 1 : 2);
                wal=-1;
                break;
            }
        }
            if (n<1) 
            {
                
                turns[game_id]=turn*10;
                break;
            }
        }
    }
    close(klient);
    printf("Exited Thread \n");
    free(table[0]);
    free(table[1]);
    free(table);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){

    int what_game=atoi(argv[1]);

    signal(SIGPIPE, SIG_IGN); //so end of connection to klient in phread doesnt close the server
    int serverSocket;
    struct sockaddr_in serverAddr;

    serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1100);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //so it will be avaliable imiedietly

    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    for (int i=0;i<N;i++)
    {
        games[i]=0;
        ended[i]=false;
    }

    if(listen(serverSocket,50)==0)
        printf("Listening\n");
    else
        printf("Error\n");

    pthread_t thread_id;
    while(1)
    {
        int *klient=malloc(sizeof(int));
        *klient = accept(serverSocket, NULL, NULL);

        int* game_id = malloc(sizeof(int));
        *game_id = -1;
        for (int i=0;i<N;i++)
        {
            if(ended[i])
            {
                games[i]=0;
                ended[i]=false;
            }
        }
        while (*game_id == -1) {
            for (int i = 0; i < N; i++) {
                if (games[i] == 0) {
                    games[i] = 1;
                    *game_id = i;
                    break;
                } else if (games[i] == 1) {
                    games[i] = 2;
                    *game_id = i;
                    if (what_game==0)  init(i);
                    else if (what_game==1) init_fast_end(i);
                    else quins_only(i);
                    break;
                }
            }
        }
        void** table = malloc(2 * sizeof(void*));

        table[0] = klient;
        table[1] = game_id;

        if( pthread_create(&thread_id, NULL, socketThread, table) != 0 ) printf("Failed to create thread\n");
        pthread_detach(thread_id);
    }
    close(serverSocket);

    return 0;
}
