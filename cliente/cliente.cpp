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
#define BUFLEN 12  //Max length of buffer
#define PORT 8888   //The port on which to send data
#define PORTSENDACK 8890   //The port on which to listen for incoming data
#define PORTRECIVEACK 8889
#define BUFFERSIZE 5

struct sockaddr_in si_other;
int cont, src, dest;
char arquivo[100];
int s, slen = sizeof(si_other) , recv_len;
pthread_t tid0,tid1;
int verificaEnvioAck = 0;
char idBufferAck[1];
struct bufferStruct{
    int id; // identifica a ordem do bloco
    int ack = 1; // 1 pra livre 0 pra ocupado
    char conteudo[BUFLEN]; // conteudo do bloco
};

bufferStruct buffer[BUFFERSIZE];

int upload(){ // Upload do cliente envia arquivo para o servidor
    int i = 0; // index do array de struct
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

int download(){ // Download do cliente, recebe arquivo do servidor
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

void die(char *s) {
    perror(s);
    exit(1);
}

void *enviaAck(void *){
    struct sockaddr_in si_other;
    int s, slen = sizeof(si_other) , recv_len;
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket");
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));
    
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORTSENDACK);
     
    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
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
    struct sockaddr_in si_other;
    int s, slen = sizeof(si_other) , recv_len;
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket");
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));
    
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORTRECIVEACK);
     
    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
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
    char opcao[1];
 
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
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
    pthread_create(&tid0,NULL,recebeAck,NULL);
    pthread_create(&tid1,NULL,enviaAck,NULL);
    
    printf("Digite a opção desejada\n");
    printf("1 - Download\n");
    printf("2 - Upload\n");
    scanf("%c", &opcao);

    sendto(s, opcao, strlen(opcao) , 0 , (struct sockaddr *) &si_other, slen);

    printf("Digite o nome do arquivo: ");
    scanf("%s",arquivo);
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

    pthread_join(tid0,NULL);
    pthread_join(tid1,NULL);
    return 0;
}
 
