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
#define BUFLEN 12  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

int cont, src, dest, opcao;
char arquivo[100];
struct sockaddr_in si_me, si_other;
int s, slen = sizeof(si_other) , recv_len;
char operacao[1];
struct bufferStruct{
    int id; // identifica a ordem do bloco
    int ack = 0;
    int flag = 0; // se o bloco esta livre ou ocupado
    char conteudo[BUFLEN]; // conteudo do bloco
};

bufferStruct buffer[5];

int download(){
    int i = 0;
    dest = creat(arquivo, 0666);
    if (dest == -1){
        printf(" Impossivel criar o arquivo %s\n", arquivo); 
        exit (1);
    }

    printf("Download Iniciado\n");
    do {
        recv_len = recvfrom(s, &buffer[i], sizeof(buffer[i]), 0, (struct sockaddr *) &si_other, (socklen_t*) &slen);

        printf("id: %d\n", buffer[i].id);
        printf("flag: %d\n", buffer[i].flag);
        printf("ack: %d\n", buffer[i].ack);
        printf("Conteudo: %s\n", buffer[i].conteudo);

        write(dest, &buffer[i].conteudo, strlen(buffer[i].conteudo));
        i++;
    } while (recv_len > 0);

    printf("Download Concluido\n");
    return 0;
}

int upload(){
    int i = 0;
    src = open(arquivo,O_RDONLY);

    if (src == -1){
        printf("Impossivel abrir o arquivo %s\n", arquivo);
        exit (1); 
    }

    printf("Upload Iniciado\n");
    while ((cont = read(src, &buffer[i].conteudo, sizeof(buffer[i].conteudo))) > 0 ){
        buffer[i].id = i;
        buffer[i].flag = 1;
        printf("%s\n", buffer[i].conteudo);
        sendto(s,&buffer[i], sizeof(buffer[i]) , 0 , (struct sockaddr *) &si_other, slen);
        i++;
    }
    sendto(s, 0, 0 , 0 , (struct sockaddr *) &si_other, slen);
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
 