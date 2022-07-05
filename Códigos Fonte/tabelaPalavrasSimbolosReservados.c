#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/*Constantes para manipular a Tabela de Palavras e Símbolos reservados*/
#define TAMANHO_HASH 51
#define N_PALAVRAS_SIMBOLOS_RESERVADOS 34


/*Par cadeia-token*/
typedef struct
{
    char cadeia[10];
    char token[30];

}palavraSimboloReservado;


palavraSimboloReservado** initTabelaPalavrasSimbolosReservados();
char* buscaTabelaPalavrasSimbolosReservados(palavraSimboloReservado** tabelaPalavrasSimbolosReservados, char* string);
int hash(unsigned char *str);
void destruirTabelaPalavrasSimbolosReservados(palavraSimboloReservado** tabelaPalavrasSimbolosReservados);


/*
        Descrição:  
            Criação e Inicialização da Tabela de Palavras e Símbolos Reservados

        Parâmetros:

        Retorno:
            Tabela de Palavras e Símbolos Reservados que contém um conjunto de pares cadeia-token ou NULL se houver falha na alocação de memória

*/
palavraSimboloReservado** initTabelaPalavrasSimbolosReservados()
{
    int i; /*Iterador*/

    /*Conjunto de Palavras e Símbolos Reservados*/
    char palavrasReservadas[N_PALAVRAS_SIMBOLOS_RESERVADOS][10] = { "program", 
                                                                    "begin", 
                                                                    "end", 
                                                                    "const", 
                                                                    "var", 
                                                                    "real", 
                                                                    "integer", 
                                                                    "procedure", 
                                                                    "if", 
                                                                    "then", 
                                                                    "else", 
                                                                    "read", 
                                                                    "write", 
                                                                    "while", 
                                                                    "do", 
                                                                    "for", 
                                                                    "to", 
                                                                    ":", 
                                                                    ":=", 
                                                                    "=", 
                                                                    "<>", 
                                                                    ">", 
                                                                    ">=", 
                                                                    "<", 
                                                                    "<=", 
                                                                    "+", 
                                                                    "-", 
                                                                    "*", 
                                                                    "/", 
                                                                    ".", 
                                                                    ";", 
                                                                    ",",
                                                                    "(",
                                                                    ")"};

    /*Conjunto de tokens associados a cada palavra ou símbolos reservado*/
    char tokens[N_PALAVRAS_SIMBOLOS_RESERVADOS][30] = {             "program", 
                                                                    "begin", 
                                                                    "end", 
                                                                    "const", 
                                                                    "var", 
                                                                    "real", 
                                                                    "integer", 
                                                                    "procedure", 
                                                                    "if", 
                                                                    "then", 
                                                                    "else", 
                                                                    "read", 
                                                                    "write", 
                                                                    "while", 
                                                                    "do", 
                                                                    "for", 
                                                                    "to", 
                                                                    "simbolo dois pontos", 
                                                                    "simbolo atribuição", 
                                                                    "simbolo igual", 
                                                                    "simbolo diferente", 
                                                                    "simbolo maior", 
                                                                    "simbolo maior igual", 
                                                                    "simbolo menor", 
                                                                    "simbolo menor igual", 
                                                                    "simbolo soma", 
                                                                    "simbolo subtração", 
                                                                    "simbolo multiplicação", 
                                                                    "simbolo divisão", 
                                                                    "simbolo ponto", 
                                                                    "simbolo ponto e vírgula", 
                                                                    "simbolo virgula", 
                                                                    "simbolo abre parêntese", 
                                                                    "simbolo fecha parêntese"};

    /*Criação da Tabela de Palavras e Símbolos Reservados*/
    palavraSimboloReservado **tabelaPalavrasSimbolosReservados = (palavraSimboloReservado**)malloc(sizeof(palavraSimboloReservado*) * TAMANHO_HASH);
        
    if(tabelaPalavrasSimbolosReservados != NULL)
    {

            for(i = 0; i < TAMANHO_HASH; i++)
            {

                tabelaPalavrasSimbolosReservados[i] = (palavraSimboloReservado*)malloc(sizeof(palavraSimboloReservado));

                /*Inicialização de valores na Tabela de Palavras e Símbolos Reservados*/
                if(tabelaPalavrasSimbolosReservados[i] != NULL)
                    strcpy(tabelaPalavrasSimbolosReservados[i]->cadeia,"");
            
            }

            /*Inserção do conjunto de pares palavra-token na Tabela de Palavras e Símbolos Reservados, utilizando a Função Hash definida*/
            for(i=0; i < N_PALAVRAS_SIMBOLOS_RESERVADOS; i++) 
            {
                strcpy(tabelaPalavrasSimbolosReservados[hash(palavrasReservadas[i])]->cadeia, palavrasReservadas[i]);
                strcpy(tabelaPalavrasSimbolosReservados[hash(palavrasReservadas[i])]->token, tokens[i]);
            }

        
    }

    return tabelaPalavrasSimbolosReservados;

}


/*
        Descrição:
            Busca por uma cadeia na Tabela de Palavras e Símbolos Reservados

        Parâmetros:
            tabelaPalavrasSimbolosReservados: Tabela de Palavras e Símbolos Reservados contendo um conjunto de cadeias e seus respectivos tokens
            string: cadeia a ser buscada na Tabela de Palavras e Símbolos Reservados

        Retorno:
            Se a cadeia está presente na Tabela de Palavras e Símbolos Reservados retorna seu token associado. Caso Contrário, retorna "Identificador"

*/
char* buscaTabelaPalavrasSimbolosReservados(palavraSimboloReservado** tabelaPalavrasSimbolosReservados, char* string)
{
    /*Mapeamento da cadeia de entrada na Tabela de Palavras e Símbolos Reservados*/
    int indice = hash(string);
    
    /*
        Como não há colisões somente para o conjunto de palavras e símbolos reservados, temos que verificar se a cadeia de entrada é igual
        a encontrada na Tabela de Palavras e Símbolos Reservados.
    */
    if(strcmp(tabelaPalavrasSimbolosReservados[indice]->cadeia, string) == 0)
        return tabelaPalavrasSimbolosReservados[indice]->token;
    
    else return "Identificador";

}


/*
        Descrição: 
            Função Hash que mapeia todas as Palavras e Simbolos Reservados em um intervalo, definido por TAMANHO_HASH, sem colisões para
            este conjunto de cadeias.

        Parâmetros:
            str: cadeia de entrada da Função Hash

        Retorno:
            Valor Hash da cadeia de entrada mapeado no intervalo definido po TAMANHO_HASH 

*/
int hash(unsigned char *str)
{
        int c;          /*Caractere de str*/
        int hash = 0;   /*Valor Hash*/

        /*Parâmetros obtidos empiricamente para o Conjunto de Palavras e Símbolos Reservados*/
        int A = 39;
        int B = 11;
        int P = 47;

        /*Computação do Valor Hash*/
        while (c = *str++) hash +=((A*c+ B)%P); 

        return hash%TAMANHO_HASH;

}



/*
        Descrição:  
            Destruição e liberação de memória da Tabela de Palavras e Símbolos Reservados

        Parâmetros:
            tabelaPalavrasSimbolosReservados: Tabela de Palavras e Símbolos Reservados contendo um conjunto de cadeias e seus respectivos tokens

        Retorno:

*/
void destruirTabelaPalavrasSimbolosReservados(palavraSimboloReservado** tabelaPalavrasSimbolosReservados)
{
    int i; /*Iterador*/
    
    if(tabelaPalavrasSimbolosReservados != NULL)
    {

            for(i = 0; i < TAMANHO_HASH; i++)
            {

                free(tabelaPalavrasSimbolosReservados[i]);
                tabelaPalavrasSimbolosReservados[i] = NULL;
            
            }

            free(tabelaPalavrasSimbolosReservados);
            tabelaPalavrasSimbolosReservados = NULL;
   
    }

}