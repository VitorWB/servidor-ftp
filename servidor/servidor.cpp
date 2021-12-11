#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

// #define PORT 6325
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

int cont, src, dest, opcao;
char arquivo[100];
char buffer [2048];
int sock,length,s0;
struct sockaddr_in si_me, si_other;
int s, i, slen = sizeof(si_other) , recv_len;
char buf[BUFLEN];
char operacao[1];

int download(){
    dest = creat(arquivo, 0666);
    if (dest == -1){
        printf(" Impossivel criar o arquivo %s\n", arquivo); 
        exit (1);
    }

    printf("Download Iniciado\n");

    do {
        recv_len = recvfrom(s, buffer, BUFLEN, 0, (struct sockaddr *) &si_other, (socklen_t*) &slen);
        fflush(stdout);
        memset(buf,'\0', BUFLEN);
        printf("%s\n", buffer);
        printf("%d\n", recv_len);
        write(dest, &buffer, recv_len);
    } while (recv_len > 0);
    
    return 0;
}

int upload(){
    src = open(arquivo,O_RDONLY);

    if (src == -1){
        printf("Impossivel abrir o arquivo %s\n", arquivo);
        exit (1); 
    }

    printf("Upload Iniciado\n");
    while ((cont = read(src, &buffer, sizeof(buffer))) > 0 ){
        printf("%s\n", buffer);
        sendto(s, buffer, strlen(buffer) , 0 , (struct sockaddr *) &si_other, slen);
    }
    printf("Upload Concluido\n");

    return 0;
}

void die(char *s){
    perror(s);
    exit(1);
}

int main (int argc, char *argv[]) {
     
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }

    fflush(stdout);
        
    printf("Esperando Operação\n");

    recv_len = recvfrom(s, operacao, BUFLEN, 0, (struct sockaddr *) &si_other, (socklen_t*) &slen);

    if(operacao[0] == '1'){
        printf("Download requisitado\n");
    } else if(operacao[0] == '2'){
        printf("Upload requisitado\n");
    }

    fflush(stdout);
    memset(buf,'\0', BUFLEN);

    printf("Esperando nome do arquivo\n");
    recv_len = recvfrom(s, arquivo, BUFLEN, 0, (struct sockaddr *) &si_other, (socklen_t*) &slen);
    printf("Nome de arquivo %s recebido\n", arquivo);

    switch (operacao[0]){
        case '1':
            upload();
            break;

        case '2':
            download();
            break;
        
        default:
            printf("Opção não tratada, tente novamente.");
            break;
    }

    return 0;
}
 