#include <stdio.h>
#include <ctype.h>

#include "buffer.c"
#include "tabelaPalavrasSimbolosReservados.c"


/*Constantes de manipulação de arquivos*/
#define TAMANHO_BUFFER 4096
#define TAMANHO_NOME_ARQUIVO 30
#define ARQUIVO_SAIDA "Analise Lexica.txt"

/*Constantes para tratamento de transições do Autômato que representa o Analisador Léxico*/
#define ESTADO_FINAL -5
#define TRANSICAO_INDEFINIDA -10

/*Constantes para tratamento de erros na Análise Léxica*/
#define ERRO 1
#define IDENTIFICADO 0


void analisadorLexico(Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados);
int transicao(int estado, char simbolo, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados);
int falhar(Buffer *buffer);
void sinalizarErro(Buffer* buffer, char *msg);


/* Conjunto de Variáveis Globais*/

/*Par cadeia-token. Atualizados a cada chamada do Analisador Léxico*/
char cadeia[30]; 
char valorLexico[30];

/*Tratamento de Erros*/
int erro=IDENTIFICADO;  /*Sinalizador de Erros durante a Análise Léxica, inicialmente não há erros.*/
int linhaErro;          /*Linha em que o erro ocorreu*/
char msgErro[50];       /*Mensagem de Erro*/

int partida;            /*Seleção de Autômato no conjunto de Autômatos do Analizador Léxico*/


int main()
{   
    char path[TAMANHO_NOME_ARQUIVO];    /*Caminho do Arquivo de Entrada*/
    char resposta;                      /*Resposta da decisão de impressão do Arquivo de Saída*/

    printf("Caminho do Programa Fonte: ");
    scanf("%s", path);

    /*Inicialização de um Buffer para a Leitura do Arquivo de Entrada*/
    Buffer *buffer = initBuffer(path, TAMANHO_BUFFER);

    /*Inicialização da Tabela de Palavras e Símbolos Reservados*/
    palavraSimboloReservado **tabelaPalavrasSimbolosReservados = initTabelaPalavrasSimbolosReservados();

    /*Inicialização do Arquivo de Saída da Análise Léxica*/
    FILE *saidaAnaliseLexica = initArquivo(ARQUIVO_SAIDA, "w");
    

    /*Leitura do Arquivo de Entrada*/
    while(!leituraFinalizada(buffer)) 
    {
            /*Analisador Léxico*/
            analisadorLexico(buffer, tabelaPalavrasSimbolosReservados);
            
            /*Ao consumir comentários ou chegar ao fim de arquivo o Analisador Léxico retorna uma cadeia vazia, então devemos ignorá-la*/
            
            if(strcmp(cadeia,"")) 
                    if(fprintf(saidaAnaliseLexica, "\n%s", cadeia) < 0) printf("Erro na escrita do Arquivo de Saída!");;
            
            if(strcmp(valorLexico, ""))
                    if(fprintf(saidaAnaliseLexica, ", %s", valorLexico) < 0) printf("Erro na escrita do Arquivo de Saída!");
            
            /*Mensagens de Erro*/
            if(erro) 
            {
                    if(fprintf(saidaAnaliseLexica, " %s na Linha %d", msgErro, linhaErro) < 0) printf("Erro na escrita do Arquivo de Saída!");;
                    erro = IDENTIFICADO;
            }

    }

    /*Liberação de memória das estruturas utilizadas*/
    destruirBuffer(buffer);
    destruirTabelaPalavrasSimbolosReservados(tabelaPalavrasSimbolosReservados);
    fecharArquivo(saidaAnaliseLexica);

    /*Impressão do Arquivo de Saída*/
    printf("Análise Léxica concluída. \nDeseja imprimir o Arquivo de saída da Análise Léxica? (S/N) ");
    scanf(" %c", &resposta);

    if(toupper(resposta) == 'S') imprimirArquivo(ARQUIVO_SAIDA);
    
    
    return 0;

}


/*
        Descrição:
                Analisador Léxico com Função de Transição explicitada

        Parâmetros:

                buffer: Buffer do Arquivo de Entrada
                tabelaPalavrasSimbolosReservados: Tabela de Palavras e Símbolos Reservados

        Retorno:

*/
void analisadorLexico(Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados)
{
 
    char simbolo;       /*Símbolo lido do Buffer*/
    int estado = 0;     /*Estado atual*/
    partida = 0;        /*Seleção do Autômato do conjunto de Autômatos do Analisador Léxico*/

    /*Leitura do Buffer*/
    simbolo = lerBuffer(buffer);

     while(1) 
     {

        /*Transição de Estado*/    
        estado = transicao(estado, simbolo, buffer, tabelaPalavrasSimbolosReservados);

        if(estado == ESTADO_FINAL) break;

        if(estado == TRANSICAO_INDEFINIDA) break;

        /*Próximo símbolo da cadeia*/
        simbolo = lerBuffer(buffer);
     
     } 

}


/*
        Descrição:
                Função de Transição do Conjunto de Autômatos do Analisador Léxico. Embora os Autômatos estejam separados, eles funcionam
                como apenas um. Cada estado tem, além de seu conjunto de transições, ações associadas.

        Parâmetros:
                
                estado: estado atual
                simbolo: simbolo lido do Buffer
                buffer: Buffer do Arquivo de Entrada
                tabelaPalavrasSimbolosReservados: Tabela de Palavras e Símbolos Reservados

        Retorno:
                Novo estado considerando o estado atual e símbolo lido

*/
int transicao(int estado, char simbolo, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados)
{
        /*Cada estado é um caso do switch, estados finais e de erro estão distinguidos*/
        switch (estado)
        {
            
            /*Autômato de Comenetários*/

            case 0:
                    if(simbolo == '{') estado = 1;
                    
                    else if(simbolo == ' ' || simbolo == '\t' || simbolo == '\n') 
                    {
                        estado = 0; 
                        avancarInicioBuffer(buffer);
                    }        
                    
                    else estado = falhar(buffer); /*Cadeia atual não é um comentário ou espaço em branco, tabulação ou quebra de linha*/
                    
                    break;

            case 1:
                    if(simbolo == '}') estado = 2;
                    else if(leituraFinalizada(buffer)) estado = 3;
                    else estado = 1;
                    
                    avancarInicioBuffer(buffer); /*Consumir caractere de comentario*/
                   
                    break;

            case 2: /*Estado Final*/
                    
                    retrocederBuffer(buffer);

                    /*Não há cadeia e portanto não há token*/
                    strcpy(cadeia, "");
                    strcpy(valorLexico, "");
                    
                    estado = ESTADO_FINAL;
                    
                    break;
            
            
            case 3: /*Erro*/

                    retrocederBuffer(buffer);
                    retrocederBuffer(buffer);
                    
                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico,"");

                    /*Sinalização de Erro*/
                    sinalizarErro(buffer, "\nErro: fim de arquivo inesperado");
                   
                    estado = ESTADO_FINAL;
                   
                    break;


            /*Autômato de Operadores*/

            case 4:

                    if
                    (   
                        simbolo == '=' || 
                        simbolo == '+' ||      
                        simbolo == '-' || 
                        simbolo == '*' || 
                        simbolo == '/' || 
                        simbolo == '.' || 
                        simbolo == ';' || 
                        simbolo == ',' || 
                        simbolo == '(' || 
                        simbolo == ')'
                    
                    ) estado = 9;
                    
                    else if(simbolo == '<') estado = 5; 
                    else if(simbolo == '>') estado = 10;
                    else if(simbolo == ':') estado = 13; 

                    else estado = falhar(buffer); /*Cadeia atual não é um operador*/
    
                    break;

            case 5: 
                    if(simbolo == '=') estado = 6;
                    else if(simbolo == '>') estado = 7;
                    else estado = 8;
    
                    break;

            case 6: /*Estado Final*/

                    retrocederBuffer(buffer);
            
                    /*Par cadeia-token*/
                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico,buscaTabelaPalavrasSimbolosReservados(tabelaPalavrasSimbolosReservados, cadeia));
            
                    estado = ESTADO_FINAL;
            
                    break;

            case 7: /*Estado Final*/
                    
                    retrocederBuffer(buffer);
                    
                    /*Par cadeia-token*/
                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico,buscaTabelaPalavrasSimbolosReservados(tabelaPalavrasSimbolosReservados, cadeia));
                    
                    estado = ESTADO_FINAL;
                    
                    break;

            case 8: /*Estado Final*/

                    retrocederBuffer(buffer);
                    retrocederBuffer(buffer);
            
                    /*Par cadeia-token*/
                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico,buscaTabelaPalavrasSimbolosReservados(tabelaPalavrasSimbolosReservados, cadeia));
            
                    estado = ESTADO_FINAL;
            
                    break;

            case 9: /*Estado Final*/

                    retrocederBuffer(buffer);
            
                    /*Par cadeia-token*/
                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico,buscaTabelaPalavrasSimbolosReservados(tabelaPalavrasSimbolosReservados, cadeia));
            
                    estado = ESTADO_FINAL;

                    break;

            case 10:
                    if(simbolo == '=') estado = 11;
                    else estado = 12;
    
                    break;

            case 11: /*Estado Final*/

                    retrocederBuffer(buffer);
            
                    /*Par cadeia-token*/
                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico,buscaTabelaPalavrasSimbolosReservados(tabelaPalavrasSimbolosReservados, cadeia));
            
                    estado = ESTADO_FINAL;
            
                    break;

            case 12: /*Estado Final*/

                    retrocederBuffer(buffer); 
                    retrocederBuffer(buffer);
            
                    /*Par cadeia-token*/
                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico,buscaTabelaPalavrasSimbolosReservados(tabelaPalavrasSimbolosReservados, cadeia));
            
                    estado = ESTADO_FINAL;
            
                    break;
        
            case 13: 
                    if(simbolo == '=') estado = 14;
                    else estado = 15;
    
                    break;
            
            case 14: /*Estado Final*/

                    retrocederBuffer(buffer);
                   
                    /*Par cadeia-token*/
                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico,buscaTabelaPalavrasSimbolosReservados(tabelaPalavrasSimbolosReservados, cadeia));
                   
                    estado = ESTADO_FINAL;
                   
                    break;

            case 15: /*Estado Final*/

                    retrocederBuffer(buffer);
                    retrocederBuffer(buffer); 
            
                    /*Par cadeia-token*/
                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico,buscaTabelaPalavrasSimbolosReservados(tabelaPalavrasSimbolosReservados, cadeia));
                    
                    estado = ESTADO_FINAL;
                   
                    break;


            /*Autômato Identificador*/

            case 16: 
                    if(isalpha(simbolo)) estado = 17;
                    else estado = falhar(buffer); /*Cadeia atual não é um identificador*/

                    break;

            case 17:
                    if(isalpha(simbolo) || isdigit(simbolo)) estado = 17;
                    
                    else if 
                    (
                        simbolo == ':'  ||
                        simbolo == '='  ||
                        simbolo == '<'  ||
                        simbolo == '>'  ||
                        simbolo == '+'  || 
                        simbolo == '-'  ||
                        simbolo == '*'  ||
                        simbolo == '/'  ||
                        simbolo == '.'  ||
                        simbolo == ';'  ||
                        simbolo == ','  ||
                        simbolo == '('  ||
                        simbolo == ')'  ||
                        simbolo == ' '  ||
                        simbolo == '\t' ||
                        simbolo == '\n' ||
                        simbolo == '{'  ||
                        simbolo == EOF

                    )estado = 18;


                    else estado = 19;
    
                    break;

            case 18: /*Estado Final*/

                    retrocederBuffer(buffer);
                    retrocederBuffer(buffer);
            
                    /*Par cadeia-token com busca por palavras reservadas*/
                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico,buscaTabelaPalavrasSimbolosReservados(tabelaPalavrasSimbolosReservados, cadeia));
            
                    estado = ESTADO_FINAL;
            
                    break;

            
            case 19: /*Erro*/

                    retrocederBuffer(buffer);
                    retrocederBuffer(buffer);
                    
                    /*Par cadeia-token sem o caractere de erro*/
                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico, "Identificador");

                    /*Sinalização de erro*/
                    sinalizarErro(buffer, "Erro: identificador mal formado");
                    
                    estado = ESTADO_FINAL;
                    
                    break;


            /*Autômato Número Real e Número Inteiro*/

            case 20:
                    if(isdigit(simbolo)) estado = 22;
                    else estado = falhar(buffer); /*Cadeia atual não é um número*/
    
                    break;

            case 22:
                    if(isdigit(simbolo)) estado = 22;
                    else if(simbolo == '.') estado = 24;
                    else estado = 23;
    
                    break;

            case 23: /*Estado Final*/

                    retrocederBuffer(buffer);
                    retrocederBuffer(buffer);
            
                    /*Par cadeia-token*/
                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico, "Número Inteiro");
            
                    estado = ESTADO_FINAL;
            
                    break;

            case 24:
                    if(isdigit(simbolo)) estado = 25;
                    else estado = 27;
    
                    break;

            case 25:
                    if(isdigit(simbolo)) estado = 25;
                    else estado = 26;
    
                    break;

            case 26: /*Estado Final*/

                    retrocederBuffer(buffer);
                    retrocederBuffer(buffer);

                    /*Par cadeia-token*/
                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico, "Número Real");
            
                    estado = ESTADO_FINAL;
            
                    break;

            
            case 27: /*Erro*/
                    
                    retrocederBuffer(buffer);
                    retrocederBuffer(buffer);

                    strcpy(cadeia,getToken(buffer));
                    strcpy(valorLexico,"");
                    
                    /*Sinalização de Erro*/
                    sinalizarErro(buffer, "Erro: número real mal formado");
                    
                    estado = ESTADO_FINAL;
                    
                    break;


            default:  /*Símbolo inesperado, como um caractere desconhecido ou um "}" sem um "{" precedente*/

                    if(simbolo != EOF)
                    {
                        /*Sinalização de erro*/
                        sinalizarErro(buffer, "Erro: caractere inesperado");
                        
                        strcpy(cadeia,getToken(buffer));
                        strcpy(valorLexico, "");
                        
                    }

                    /* Se EOF, análise terminada e não temos uma cadeia ou token*/
                    else 
                    {
                            strcpy(cadeia,"");
                            strcpy(valorLexico, "");
                            retrocederBuffer(buffer);      
                    }

                    estado = TRANSICAO_INDEFINIDA;
                    break;
        }


        return estado;

}


/*
        Descrição:
                Determina o próximo Autômato do conjunto de Autômatos do Analisador Léxico depois de uma falha
                no reconhecimento da cadeia atual por um dos Autômatos

        Parâmetros:
                buffer: Buffer do Arquivo de Entrada

        Retorno:
                Estado inicial do Próximo Autômato

*/
int falhar(Buffer *buffer)
{ 

    /*Quando há uma falha no reconhecimento da cadeia atual, o Buffer precisa ser restaurado para tentar reconher a mesma cadeia novamente*/
    restaurarBuffer(buffer);

    switch (partida)
    {
        case 0:
                partida = 4; /*Operadores*/
                break;
        
        case 4:
                partida = 16; /*Identificadores*/
                break;
        
        case 16:
                partida = 20; /*Números*/
                break;

        case 20:
                partida=-1; /*Cadeia Rejeitada*/
                break;
            
    }

    return partida;

}


/*
        Descrição:
                Sinalização e manipulação de erros durante a Análise Léxica

        Parâmetros:
                buffer: Buffer do Arquivo de Entrada
                msg: mensagem de erro a ser exibida

        Retorno:                

*/
void sinalizarErro(Buffer* buffer, char *msg)
{
    erro = ERRO;                        /*Sinaliza a flag de erro*/
    strcpy(msgErro, msg);               /*Registra a mensagem de erro*/
    linhaErro = buffer->contadorLinhas; /*Registra a linha em que o erro ocorreu*/
}