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
#include <netinet/in.h>

#define PORT 6325

#define IP_SERVER "172.17.0.2"

int s0;
struct sockaddr_in server;
int cont, src, dest, opcao;
char arquivo[100];
int buffer [2048];

int upload(){ // Upload do cliente envia arquivo para o servidor
    src = open(arquivo,O_RDONLY);

    if (src == -1){
        printf("Impossivel abrir o arquivo %s\n", arquivo);
        exit (1); 
    }

    printf("Upload Iniciado\n");
    while ((cont = read(src, &buffer, sizeof(buffer))) > 0 ){
        send(s0, &buffer, sizeof(buffer), 0);
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
    while ((cont = recv(s0, &buffer, sizeof(buffer), 0)) > 0 ){
        write(dest, &buffer, cont);
    }
    printf("Download Concluido\n");
    return 0;
}

int main (int argc, char *argv[]) {
    
    //1:SOCKET
    s0=socket(AF_INET, SOCK_STREAM, 0);
        if (s0<0){
            perror("opening stream socket");
            exit(1);
        }
    //1:FIM SOCKET

    //2:CONEXAO COM SERVIDOR
    bzero(&server, sizeof(server)) ;
    server.sin_family = AF_INET ;
    server.sin_port = htons(PORT) ;
    server.sin_addr.s_addr = inet_addr(IP_SERVER);

    if (connect(s0, (struct sockaddr *) &server, sizeof(server))){
        perror("connectando stream socket");
        exit(0);
    }

    printf("Cliente Conectado!\n");
    
    printf("Digite a opção desejada\n");
    printf("1 - Download\n");
    printf("2 - Upload\n");
    scanf("%d", &opcao);
    send(s0, &opcao, sizeof(opcao), 0);
    printf("Digite o nome do arquivo: ");
    scanf("%s",arquivo);
    send(s0, &arquivo, sizeof(arquivo), 0);
    
    switch (opcao){
        case 1:
            download();
            break;

        case 2:
            upload();
            break;
        
        default:
            printf("Opção não tratada, tente novamente.");
            exit (1);
            break;
    }

    return 0;
}
 