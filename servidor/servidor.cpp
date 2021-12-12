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
#include <iostream>
#include <netinet/in.h>

// #define PORT 6325
#define BUFLEN 12  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data
#define PORTSENDACK 8889   //The port on which to listen for incoming data
#define PORTRECIVEACK 8890
#define BUFFERSIZE 5

int cont, src, dest, opcao;
char arquivo[100];
struct sockaddr_in si_me, si_other;
int s, slen = sizeof(si_other) , recv_len;
char operacao[1];
int verificaEnvioAck = 0;
char idBufferAck[1];

pthread_t tid0, tid1;
struct bufferStruct{
    int id; // identifica a ordem do bloco
    int ack = 1; // 1 pra livre 0 pra ocupado
    char conteudo[BUFLEN]; // conteudo do bloco
};

bufferStruct buffer[BUFFERSIZE];

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

        // printf("id: %d\n", buffer[i].id);
        // printf("ack: %d\n", buffer[i].ack);
        printf("Conteudo: %s\n", buffer[i].conteudo);
        write(dest, &buffer[i].conteudo, strlen(buffer[i].conteudo));
        idBufferAck[0] = buffer[i].id;
        verificaEnvioAck = 1;
        i++;
    } while (recv_len > 0);

    printf("Download Concluido\n");
    return 0;
}

int upload(){
    int i = 0;
    int n = 0; // numero de blocos
    src = open(arquivo,O_RDONLY);

    if (src == -1){
        printf("Impossivel abrir o arquivo %s\n", arquivo);
        exit (1); 
    }

    printf("Upload Iniciado\n");
    int verify = 1;
    while ((cont = read(src, &buffer[i].conteudo, sizeof(buffer[i].conteudo))) > 0 ){
        if(verify){
            if(n >= 4){
                verify = 0;
            }
            buffer[i].id = n;
            buffer[i].ack = 0;
            printf("%s\n", buffer[i].conteudo);
            sendto(s,&buffer[i], sizeof(buffer[i]) , 0 , (struct sockaddr *) &si_other, slen);
            i++;
            n++;
        } else{
            while(verify == 0){
                sleep(1);
                for (size_t k = 0; k < 5; k++){
                    if(buffer[i].ack){
                        i = k;
                        verify = 1;
                        break;
                    }
                }
            }
            
        }
    }
    sendto(s, 0, 0, 0, (struct sockaddr *) &si_other, slen);
    printf("Upload Concluido\n");

    return 0;
}

void die(char *s){
    perror(s);
    exit(1);
}

void *enviaAck(void *){
    struct sockaddr_in si_me, si_other;
    int s, slen = sizeof(si_other) , recv_len;
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORTSENDACK);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }

    while(1){
        if(verificaEnvioAck){
            printf("Mandando ack\n");
            sendto(s, idBufferAck, strlen(idBufferAck) , 0 , (struct sockaddr *) &si_other, slen);
            printf("Id de buffer enviado: %c\n", idBufferAck[0]);
            verificaEnvioAck = 0;
        }
        sleep(1);
    }
}

void *recebeAck(void *){
    struct sockaddr_in si_me, si_other;
    int s, slen = sizeof(si_other) , recv_len;
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORTRECIVEACK);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }

    char id[1];
    while(1){
        printf("esperando ack\n");
        recvfrom(s, id, BUFLEN, 0, (struct sockaddr *) &si_other, (socklen_t*) &slen);
        printf("id: %c\n", id[0]);

        for (size_t i = 0; i < BUFFERSIZE; i++){
            if(buffer[i].id == id[0]){
                buffer[i].ack = 1;
                memset(buffer[i].conteudo,'\0', BUFLEN);
            }
        }

        sleep(1);
    }
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

    pthread_create(&tid0,NULL,recebeAck,NULL);
    pthread_create(&tid1,NULL,enviaAck,NULL);

    fflush(stdout);
        
    printf("Esperando Operação\n");

    recv_len = recvfrom(s, operacao, BUFLEN, 0, (struct sockaddr *) &si_other, (socklen_t*) &slen);
    printf("operacao: %c\n", operacao[0]);
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

    pthread_join(tid0,NULL);
    pthread_join(tid1,NULL);

    return 0;
}
 