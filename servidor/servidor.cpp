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

#define PORT 6325

int cont, src, dest, opcao;
char arquivo[100];
int buffer [2048];
int sock,length,s0;
struct sockaddr_in server;

int download(){
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

int upload(){
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

int main (int argc, char *argv[]) {
    int operacao;
    
    sock = socket(AF_INET, SOCK_STREAM, 0); 
        if (sock < 0) {
            perror("opening stream socket");
            exit(0);
        }
    //FIM SOCKET

    //2:BIND
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons (PORT);
    length = sizeof (server);
        if (bind(sock, (struct sockaddr *)&server, length) < 0){
            perror("binding stream socket");
            exit(0);
        }
    //FIM BIND

    //3:LISTEN
    listen(sock,5);
    printf("Servidor: Aguardando conexao!\n");  
    //FIM LISTEN

    //4:ACCEPT;
    s0 = accept(sock,(struct sockaddr *)0,0);  
    printf("Servidor: Conexões estabelecidas!\n");  

    printf("Esperando Operação\n");
    recv(s0, &operacao, sizeof(operacao), 0); 
    if(operacao == 1){
        printf("Download requisitado\n");
    } else if(operacao == 2){
        printf("Upload requisitado\n");
    }
    printf("Esperando nome do arquivo\n");
    recv(s0, &arquivo, sizeof(arquivo), 0); 
    printf("Nome de arquivo %s recebido\n", arquivo);

    switch (operacao){
        case 1:
            upload();
            break;

        case 2:
            download();
            break;
        
        default:
            printf("Opção não tratada, tente novamente.");
            break;
    }

    return 0;
}
 