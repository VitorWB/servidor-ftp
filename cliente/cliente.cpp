#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

// #define PORT 6325
// #define IP_SERVER "172.20.20.206"

#define SERVER "127.0.0.1"
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to send data

int s0;
struct sockaddr_in si_other;
int cont, src, dest;
char arquivo[100];
char confirma[1];
char buffer[2048];
int s, i, slen = sizeof(si_other) , recv_len;
char buf[BUFLEN];

int upload(){ // Upload do cliente envia arquivo para o servidor
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

int download(){ // Download do cliente, recebe arquivo do servidor
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

    printf("Download Concluido\n");
    return 0;
}

void die(char *s) {
    perror(s);
    exit(1);
}

int main (int argc, char *argv[]) {
    char message[BUFLEN];
    char opcao[1];
    confirma[0] = 1;
 
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));
    
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
     
    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    printf("Cliente Conectado!\n");
    
    printf("Digite a opção desejada\n");
    printf("1 - Download\n");
    printf("2 - Upload\n");
    scanf("%c", &opcao);

    sendto(s, opcao, strlen(opcao) , 0 , (struct sockaddr *) &si_other, slen);

    printf("Digite o nome do arquivo: ");
    scanf("%s",arquivo);
    memset(buf,'\0', BUFLEN);
    sendto(s, arquivo, strlen(arquivo) , 0 , (struct sockaddr *) &si_other, slen);
    
    switch (opcao[0]){
        case '1':
            download();
            break;

        case '2':
            upload();
            break;
        
        default:
            printf("Opção não tratada, tente novamente.");
            exit (1);
            break;
    }

    return 0;
}
 
