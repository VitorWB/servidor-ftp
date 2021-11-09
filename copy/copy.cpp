#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char *argv[]) {

    int cont, src, dest, opcao;
    char arquivo[50];
    char novoArquivo[50] = "teste.txt";
    int buffer [2048];
    
    printf ("Comecei a copia\n");    

    printf("Digite a opção desejada\n");
    printf("1 - Download\n");
    printf("2 - Upload\n");
    scanf("%d", &opcao);
    printf("Digite o nome do arquivo: ");
    scanf("%s",arquivo);
    printf("O nome armazenado foi: %s", arquivo);
    
    src = open(arquivo,O_RDONLY);

    if (src == -1){
        printf("Impossivel abrir o arquivo %s\n", arquivo);
        exit (1); 
    }

    dest = creat(novoArquivo, 0666);
    if (dest == -1){
        printf(" Impossivel criar o arquivo %s\n", novoArquivo); 
        exit (1);
    }
    
    while ((cont = read(src, &buffer, sizeof(buffer))) > 0 ){
        write(dest, &buffer, cont) ;
        // send
    }
    
    printf(" Terminei a copia\n");
    return 0;
}
 