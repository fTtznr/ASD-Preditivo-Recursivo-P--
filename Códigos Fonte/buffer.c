#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*Constantes para manipulação de avanços e retrocessos na leitura dos Buffers*/
#define RETROCEDER 1
#define ADIANTE 0


typedef struct
{

    FILE *arquivo;                  /*Ponteiro do Arquivo Fonte*/

    int  tamanhoBuffer;             /*Tamanho de cada Buffer*/
    
    char *buffer;                   /*Par de Buffers*/
    
    char *sentinelaPrimeiroBuffer;  /*Posição do Sentinela do Primeiro Buffer*/
    char *sentinelaSegundoBuffer;   /*Posição do Sentinela do Segundo Buffer*/
    
    /*Flag que indica se houve um retrocesso na leitura do Buffer, utilizado para não recarregar os Buffers desnecessariamente quando houver um retrocesso */
    int  retroceder; 
    
    char *beginning;                /*Posição inicial da cadeia atual*/
    char *forward;                  /*Posição do próximo caractere a ser lido do Buffer*/

    int contadorLinhas;             /*Contador de linhas do arquivo de entrada*/

} Buffer;


Buffer* initBuffer(char *path, int tamanhoBuffer);
char lerBuffer(Buffer *buffer);
void restaurarBuffer(Buffer *buffer);
void avancarInicioBuffer(Buffer *buffer);
void retrocederBuffer(Buffer *buffer);
char* getToken(Buffer *buffer);
int leituraFinalizada(Buffer *buffer);
FILE* initArquivo(char *path, char *modo);
void fecharArquivo(FILE *file);
void imprimirArquivo(char *path);
void destruirBuffer(Buffer *buffer);


/*
        Descrição:
            Criação e Inicialização de um Par de Buffers com Sentinelas sobre um arquivo

        Parâmetros:
            path: caminho do arquivo sobre o qual se criará o Buffer
            int: tamanho de cada Buffer do par de Buffers

        Retorno:
            Um par de Buffers com o primeiro deles carregado com os caracteres do arquivo de entrada

*/ 
Buffer* initBuffer(char *path, int tamanhoBuffer)
{

    int i;  /*Iterador*/

    /*Criação do Buffer*/
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));

    if(buffer != NULL)
    {
            /*Inicialização do arquivo bufferizado*/
            buffer->arquivo = initArquivo(path, "rb");
            
            if(buffer->arquivo != NULL)
            {

                buffer->tamanhoBuffer = tamanhoBuffer;

                /*Pares de Buffers e Sentinelas*/
                buffer->buffer = (char *)malloc(sizeof(char)*(buffer->tamanhoBuffer)*2+2);

                /*Sentinelas ao final de cada um dos Buffers*/
                buffer->sentinelaPrimeiroBuffer = buffer->buffer + buffer->tamanhoBuffer;
                buffer->sentinelaSegundoBuffer = buffer->buffer + 2*buffer->tamanhoBuffer + 1;
                *(buffer->sentinelaPrimeiroBuffer) = EOF;
                *(buffer->sentinelaSegundoBuffer) = EOF;
                
                /*Inicialmente, a identificação de tokens começa no primeiro caractere do primeiro Buffer*/
                buffer->beginning = buffer->buffer;
                buffer->forward = buffer->buffer;

                /*Carregamento do Primeiro Buffer*/
                for (i = 0; i < buffer->tamanhoBuffer; i++)
                    *(buffer->buffer+i) = fgetc(buffer->arquivo);

                buffer->retroceder = ADIANTE;
                buffer->contadorLinhas = 1;
            
            }

            //else apagarBuffer(buffer);

    }
    
    return buffer;

}


/*
        Descrição:
            Leitura de um caractere do Buffer

        Parâmetros:
            buffer: Buffer do Arquivo de Entrada

        Retorno:    
            Caractere lido do Buffer

*/
char lerBuffer(Buffer *buffer)
{
    int i;                          /*iterador*/
    char c = *(buffer-> forward);   /*Leitura do caractere do Buffer*/

    /*Atualização do contador de linhas*/
    if(c == '\n') buffer->contadorLinhas++;

    /*Atualização da posição do próximo caractere a ser lido*/

    buffer->forward++;

    if(*(buffer->forward) == EOF)
    {
        /*Apontador adiante está no fim do Primeiro Buffer*/
        if(buffer->forward == buffer->sentinelaPrimeiroBuffer)
        {
            buffer->forward++;
        }
        
        /*Apontador adiante está no fim do Segundo Buffer*/
        else if(buffer->forward == buffer->sentinelaSegundoBuffer)
        {
            buffer->forward = buffer->buffer;
        }

        /*Se não houve um retrocesso carrega o buffer*/
        if(buffer->retroceder) buffer->retroceder = ADIANTE;
        else 
        {
            for (i = 0; i < buffer->tamanhoBuffer; i++)
                *(buffer->forward+i) = fgetc(buffer->arquivo);
           
        }
    
    }

    /* Se o Apontador adiante não estiver em nenhuma sentinela e o caracter lido foi EOF, retornará EOF, indicando o fim da Análise Léxica*/
    return c;

}


/*
        Descrição:
            Restaura a leitura do Buffer para o início da cadeia atual

        Parâmetros:
            buffer: Buffer do Arquivo de Entrada

        Retorno:

*/
void restaurarBuffer(Buffer *buffer)
{
    int i, j; /*iteradores*/

    /*Verificação da posição relativa entre o início e fim da cadeia atual*/
    if(buffer-> forward > buffer->beginning) j = buffer-> forward - buffer->beginning;
    else j = buffer->beginning - buffer-> forward; 

    for(i=0; i < j; i++) retrocederBuffer(buffer);
    
}


/*
        Descrição:  
            Avança o início de cadeia para o próximo caractere a ser lido do Buffer

        Parâmetros:
            buffer: Buffer do Arquivo de Entrada

        Retorno:

*/
void avancarInicioBuffer(Buffer *buffer)
{
    buffer-> beginning = buffer->forward;
}


/*
        Descrição:
            Reverte a leitura do último caractere lido do Buffer

        Parâmetros:
            buffer: Buffer do Arquivo de Entrada

        Retorno:

*/
void retrocederBuffer(Buffer *buffer)
{

    /*
        Verificação dos limites inferiores de cada Buffer antes de um retrocesso. Além disso, a flag de retrocesso é sinalizada
        para que não haja carregamanto desnecessário do Buffer
    */

    if ((buffer->forward - 1) == buffer->sentinelaPrimeiroBuffer) 
    {
        buffer->forward -= 2;
        buffer->retroceder = RETROCEDER;
    }
    
    else if (buffer->forward == buffer->buffer)
    {
        buffer->forward = buffer->sentinelaSegundoBuffer-1;
        buffer->retroceder = RETROCEDER;
    }
    
    else  buffer->forward--;

    /*Atualização do contador de linhas*/
    if(*(buffer->forward) == '\n') buffer->contadorLinhas--;
}


/*
        Descrição:
            Leitura da Cadeia de caracteres atual baseada na posição relativa dos apontadores de início e adiante

        Parâmetros:
            buffer: Buffer do Arquivo de Entrada

        Retorno:
            Cadeia de caracteres que será classificada pelo Analisador Léxico

*/
char* getToken(Buffer *buffer)
{
    char *c;                                /*Cadeia de Caracteres*/
    int i, j;                               /*Iteradores*/
    int eof;                                /*Quantidade de EOFs entre os apontadores de início e adiante*/
    int tamanhoString, tamanhoString1;      /*Tamanho das cadeias delimitadas entre os apontadores de início e adiante e os sentinelas*/
    
    /*Verificação da posição relativa entre os apontadores início e adiante*/
    if(buffer->beginning <= buffer-> forward)
    {
        /*Tamanho da cadeia*/
        tamanhoString = buffer->forward - buffer->beginning;
        
        c = (char *)malloc(sizeof(char)*tamanhoString)+1;

        eof=0;

        /*Leitura dos caracteres ignorando EOFs*/
        for(i=0; i < tamanhoString; i++)
        {
            if (*(buffer->beginning+i) != EOF) c[i-eof] = *(buffer->beginning+i);
            else eof++;
        }
        
        /*Finalização da cadeia*/
        c[tamanhoString-eof] = '\0';

    }

    else
    {
        /*Cálculo do tamanho das duas subcadeias em que a cadeia está dividida*/
        tamanhoString = buffer->sentinelaSegundoBuffer - buffer->beginning;
        tamanhoString1 = buffer->forward - buffer->buffer;
        
        c = (char *)malloc(sizeof(char)*(tamanhoString+tamanhoString1));

        eof=0;

        /*Lendo caracteres da primeira subcadeia e ignorando EOFs*/
        for(i=0; i < tamanhoString; i++)
        {
            if (*(buffer->beginning+i) != EOF) c[i-eof] = *(buffer->beginning+i);
            else eof++;
        }
        
        /*Lendo caracteres da segunda subcadeia e ignorando EOFs*/
        for(j=0; j < tamanhoString1; j++)
        {
            if (*(buffer->buffer+j) != EOF) c[i+j-eof] = *(buffer->buffer+j);
            else eof++;
        }

        /*Finalização da cadeia*/
        c[tamanhoString+tamanhoString1-eof] = '\0';
    
    }

    /*O início da próxima cadeia é o próximo caractere a ser lido*/
    avancarInicioBuffer(buffer);

    return c;

}


/*
        Descrição:  
            Verificação do fim de leitura do arquivo no Buffer

        Parâmetros:
            buffer: Buffer do Arquivo de Entrada 

        Retorno:
            Retorna 0 - falso - se o último caractere do arquivo ainda não foi lido do Buffer 

*/
int leituraFinalizada(Buffer *buffer)
{
    return (*(buffer->forward) == EOF);
}


/*
        Descrição:
            Abre um arquivo com o modo especificado

        Parâmetros:
            path: caminho do arquivo
            modo: modo de abertura do arquivo

        Retorno:
            Descritor do arquivo

*/
FILE* initArquivo(char *path, char *modo)
{

    FILE *file; /*Descritor do arquivo*/

    /*Abertura ou criação do arquivo*/
    file = fopen(path, modo);
    
    /*Tratamento de erros*/
    if(file == NULL)
    {
        printf("Erro na Abertura do Arquivo!\n");   
        exit(1);             
    }

    return file;

}


/*
        Descrição:
            Fecha um arquivo anteriormente aberto

        Parâmetros:
            file: descritor do arquivo

        Retorno:

*/
void fecharArquivo(FILE *file)
{
    fclose(file);
}


/*
        Descrição:
            Imprime um arquivo na saída padrão

        Parâmetros:  
            path: caminho do arquivo

        Retorno:

*/
void imprimirArquivo(char *path)
{
   FILE *file;  /*Descritor do arquivo*/
   int c;       /*Caractere lido do arquivo*/

   /*Abertura do arquivo*/
   file = initArquivo(path,"r");
   
   /*Leitura e impressão do arquivo*/
   while(1) 
   {
      
      c = fgetc(file);
      if(feof(file)) break;
      printf("%c", c);
   
   }
   
   printf("\n");

   /*Fechamento do arquivo*/
   fecharArquivo(file);

}


/*
        Descrição:
            Destruição e liberação de memória de um Par de Buffers com Sentinelas sobre um arquivo

        Parâmetros:
            buffer: Buffer do Arquivo de Entrada

        Retorno:

*/ 
void destruirBuffer(Buffer *buffer)
{

    if(buffer != NULL)
    {
        fecharArquivo(buffer->arquivo);

        free(buffer->buffer); 
        buffer->buffer = NULL;
                
        free(buffer); 
        buffer = NULL;

    }

}