#include <stdio.h>
#include <ctype.h>

#include "buffer.c"
#include "tabelaPalavrasSimbolosReservados.c"


/*Constantes de manipulação de arquivos*/
#define TAMANHO_BUFFER 4096
#define TAMANHO_NOME_ARQUIVO 30
#define ARQUIVO_SAIDA "Analise Sintática.txt"

/*Constantes para tratamento de transições do Autômato que representa o Analisador Léxico*/
#define ESTADO_FINAL -5
#define TRANSICAO_INDEFINIDA -10

/*Constantes para tratamento de erros na Análise Léxica*/
#define ERRO 1
#define IDENTIFICADO 0

/*Constantes para tratamento de erros na Análise Léxica*/
#define SEGUIDOR_LOCAL 0
#define SEGUIDOR_PAI 1

void analisadorLexico(Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
int transicao(int estado, char simbolo, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados);
int falhar(Buffer *buffer);
void sinalizarErro(Buffer* buffer, char *msg);

/*Procedimentos do Analisador Sintático Descendente Preditivo Recursivo*/
void asd(Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void programa(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void corpo(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void dc_c(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void dc_v(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void dc_p(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void parametros(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void lista_par(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void corpo_p(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void lista_arg(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void pfalsa(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void comandos(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void cmd(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void x(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void condicao(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void expressao(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void termo(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
void fator(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);

int erroSintatico(char* msg, char **conjuntoSincronizacaoLocal, int tamanhoConjuntoLocal, char **conjuntoSincronizacaoPai, int tamanhoConjuntoPai, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida);
int buscaToken(char **conjuntoSincronizacao, int tamanhoConjunto, char *token);
char** criarSeguidor(char **seguidor, int tamanhoSeguidor, char** conjuntoAdicional, int tamanhoAdicional);
void delSeguidor(char**seguidor, int tamanhoSeguidor);



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
    FILE *saida = initArquivo(ARQUIVO_SAIDA, "w");
    
    /*Analisador Sintático*/
    asd(buffer, tabelaPalavrasSimbolosReservados, saida);

    /*Liberação de memória das estruturas utilizadas*/
    destruirBuffer(buffer);
    destruirTabelaPalavrasSimbolosReservados(tabelaPalavrasSimbolosReservados);
    fecharArquivo(saida);

    /*Impressão do Arquivo de Saída*/
    printf("\nDeseja imprimir o relatório de erros detectados na Análise Sintática? (S/N) ");
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
                saida: arquivo de saida com identificação e mensagens de erro

        Retorno:

*/
void analisadorLexico(Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
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

      /*Ao consumir comentários ou chegar ao fim de arquivo o Analisador Léxico retorna uma cadeia vazia, então devemos ignorá-la*/
      /*      
      if(strcmp(cadeia,"")) 
                if(fprintf(saida, "\n%s", cadeia) < 0) printf("Erro na escrita do Arquivo de Saída!");;
            
      if(strcmp(valorLexico, ""))
                if(fprintf(saida, ", %s", valorLexico) < 0) printf("Erro na escrita do Arquivo de Saída!");
            
      */

      /*Mensagens de Erro do Analisador Léxico*/
      if(erro) 
      {
                if(fprintf(saida, "\n%s na Linha %d", msgErro, linhaErro) < 0) printf("\nErro na escrita do Arquivo de Saída!");;
                erro = IDENTIFICADO;
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
                    sinalizarErro(buffer, "\nErro Léxico: fim de arquivo inesperado");
                   
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
                    if(isalpha(simbolo) || isdigit(simbolo) || simbolo == '_') estado = 17;
                    
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
                    sinalizarErro(buffer, "Erro Léxico: identificador mal formado");
                    
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
                    sinalizarErro(buffer, "Erro Léxico: número real mal formado");
                    
                    estado = ESTADO_FINAL;
                    
                    break;


            default:  /*Símbolo inesperado, como um caractere desconhecido ou um "}" sem um "{" precedente*/

                    if(simbolo != EOF)
                    {
                        /*Sinalização de erro*/
                        sinalizarErro(buffer, "Erro Léxico: caractere inesperado");
                        
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



void asd(Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
    analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo Inicial
	
    programa(NULL, 0, buffer, tabelaPalavrasSimbolosReservados, saida); //Primeira Regra da Gramática
	
    if(leituraFinalizada(buffer))
	printf("\nAnálise Sintática concluída com sucesso!"); 
    else 
        printf("\nAnálise Sintática não concluída!");

}



void programa(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{

    if (strcmp(cadeia, "program") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {
        /*Erro Sintático*/

        int tamanhoSeguidoreLocais = 1;
        char *seguidoresLocais[1] = {"Identificador"};

        if( erroSintatico("\nErro Sintático: program esperado", seguidoresLocais, tamanhoSeguidoreLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                return;
    }
	

    if (strcmp(valorLexico, "Identificador") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {
        /*Erro Sintático*/

        int tamanhoSeguidoreLocais = 1;
        char *seguidoresLocais[1] = {";"};

        if( erroSintatico("\nErro Sintático: identificador de programa esperado", seguidoresLocais, tamanhoSeguidoreLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                return;
    }
	

    if (strcmp(cadeia, ";") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {
        /*Erro Sintático*/

        int tamanhoSeguidoresLocais = 4;
        char *seguidoresLocais[4] = {"const", "var", "procedure", "begin"}; // Primeiro(corpo)

        if( erroSintatico("\nErro Sintático: ponto e vírgula esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                return;
    }

	
    //<corpo>
    
    int tamanhoSeguidoresLocais = 1;
    char *seguidoresLocais[1] = {"."};

    char **s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais, tamanhoSeguidoresLocais);
    int tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais;

    corpo(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
	
    delSeguidor(s, tamanhoS);


    // Ponto

    if (strcmp(cadeia, ".") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {

        /*Erro Sintático*/

        /*Tratamento de Erro Sintático*/
        if( erroSintatico("\nErro Sintático: ponto esperado", NULL, 0, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                return;

     }

}



void corpo(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{

        //<dc_c>
    
        /*Seguidores Locais*/
        int tamanhoSeguidoresLocais = 1;
        char *seguidoresLocais[1] = {"var"}; //Primeiro(dc_v)

        /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
        char **s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais, tamanhoSeguidoresLocais);
        int tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais;

        /*Chamada de Procedimento*/
        dc_c(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                
        delSeguidor(s, tamanhoS);

        
        //<dc_v>
        
        /*Seguidores Locais*/
        int tamanhoSeguidoresLocais2 = 1;
        char *seguidoresLocais2[1] = {"procedure"};

        /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
        s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais2, tamanhoSeguidoresLocais2);
        tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais2;

        /*Chamada de Procedimento*/
        dc_v(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                
        delSeguidor(s, tamanhoS);
        
        
        //<dc_p>
        
        /*Seguidores Locais*/
        int tamanhoSeguidoresLocais3 = 1;
        char *seguidoresLocais3[1] = {"begin"};

        /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
        s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais3, tamanhoSeguidoresLocais3);
        tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais3;

        /*Chamada de Procedimento*/
        dc_p(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                
        delSeguidor(s, tamanhoS);
        

        if(strcmp(cadeia, "begin") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 7;
                char *seguidoresLocais[7] = {"read", "write", "while", "for", "if", "Identificador", "begin"}; // Primeiro(comandos)

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: begin esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }


        //<comandos>
        
        /*Seguidores Locais*/
        int tamanhoSeguidoresLocais4 = 1;
        char *seguidoresLocais4[1] = {"end"};

        /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
        s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais4, tamanhoSeguidoresLocais4);
        tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais4;

        /*Chamada de Procedimento*/
        comandos(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                
        delSeguidor(s, tamanhoS);


        if(strcmp(cadeia, "end") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {

                /*Erro Sintático*/

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: end esperado", NULL, 0, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;

        }

}



void dc_c(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
    if(strcmp(cadeia, "const") != 0) return;
    else 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
	

    if(strcmp(valorLexico, "Identificador") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo        
    else
    {
    
	/*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 1;
	char *seguidoresLocais[1] = {"="}; 

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: identificador esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
    }


    if(strcmp(cadeia, "=") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {
    
	/*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 2;
	char *seguidoresLocais[2] = {"Número Inteiro", "Número Real"}; 

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: sinal de igual esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
    }


    if( (strcmp(valorLexico, "Número Inteiro") == 0) || (strcmp(valorLexico, "Número Real") == 0) )
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {
    
	/*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 1;
	char *seguidoresLocais[1] = {";"};

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: número inteiro ou real esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
    }
	
    if (strcmp(cadeia, ";") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {

        /*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 1;
	char *seguidoresLocais[1] = {"const"}; //Primeiro(dc_c)

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: ponto e vírgula esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;

    }
	
    dc_c(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);

}



void dc_v(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
    
    if (strcmp(cadeia, "var") != 0) return;
	
    else 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
	
    if (strcmp(valorLexico, "Identificador") == 0) 
    {
	analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo

	while(strcmp(cadeia, ",") == 0)
        {
	
            analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
			
            if (strcmp(valorLexico, "Identificador") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
            else
            {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 2;
                char *seguidoresLocais[2] = {",", ":"};

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: identificador esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
            }

        }

    }        
    else
    {
    
	/*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 1;
	char *seguidoresLocais[1] = {":"};

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: identificador esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
    }
	

    if (strcmp(cadeia, ":") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {
    
	/*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 2;
	char *seguidoresLocais[2] = {"real", "integer"}; 

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: dois pontos esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
    }
	


    if( (strcmp(cadeia, "real") == 0) || (strcmp(cadeia, "integer") == 0) )
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {
    
	/*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 1;
	char *seguidoresLocais[1] = {";"}; 

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: real ou integer esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
    }
	

    if (strcmp(cadeia, ";") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {
    
	/*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 1;
	char *seguidoresLocais[1] = {"var"}; // Primeiro(dc_v)

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: ponto e vírgula esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
    }
	

    dc_v(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);

}



void dc_p(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
    if(strcmp(cadeia, "procedure") != 0) return;
    else 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
	
    if(strcmp(valorLexico, "Identificador") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo        
    else
    {
    
	/*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 1;
	char *seguidoresLocais[1] = {"("}; // Primeiro(parametros)

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: identificador esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
    }
	

    //<parametros>
    
    /*Seguidores Locais*/
    int tamanhoSeguidoresLocais = 1;
    char *seguidoresLocais[1] = {";"};

    /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
    char **s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais, tamanhoSeguidoresLocais);
    int tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais;

    /*Chamada de Procedimento*/
    parametros(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
	
    delSeguidor(s, tamanhoS);



    if (strcmp(cadeia, ";") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {
    
	/*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 2;
	char *seguidoresLocais[2] = {"var", "begin"}; // Primeiro(corpo_p)

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: ponto e vírgula esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
     }



    //<corpo_p>
    
    /*Seguidores Locais*/
    int tamanhoSeguidoresLocais2 = 1;
    char *seguidoresLocais2[1] = {"procedure"}; //Primeiro(dc_p)

    /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
    s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais2, tamanhoSeguidoresLocais2);
    tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais2;

    /*Chamada de Procedimento*/
    corpo_p(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
	
    delSeguidor(s, tamanhoS);

   
    dc_p(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);

}



void parametros(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
    if(strcmp(cadeia, "(") != 0) return;
    else 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
	

    //<lista_par>
    
    /*Seguidores Locais*/
    int tamanhoSeguidoresLocais = 1;
    char *seguidoresLocais[1] = {")"};

    /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
    char **s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais, tamanhoSeguidoresLocais);
    int tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais;

    /*Chamada de Procedimento*/
    lista_par(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
	
    delSeguidor(s, tamanhoS);



    if (strcmp(cadeia, ")") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    { 

        /*Erro Sintático*/

        /*Tratamento de Erro Sintático*/
        if( erroSintatico("\nErro Sintático: fecha parentese esperado", NULL, 0, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                return;

     }


}



void lista_par(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
    if (strcmp(valorLexico, "Identificador") == 0) 
    {
	analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo

	while(strcmp(cadeia, ",") == 0)
        {
	
            analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
			
            if (strcmp(valorLexico, "Identificador") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
            else
            {
    
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 2;
                char *seguidoresLocais[2] = {",", ":"};

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: identificador esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
    
            }

        }

    }        
    else
    {
    
	/*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 1;
	char *seguidoresLocais[1] = {":"}; 

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: identificador esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
    }
	

    if (strcmp(cadeia, ":") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {
    
	/*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 2;
	char *seguidoresLocais[2] = {"real", "integer"}; 

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: dois pontos esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
    }
	

    if( (strcmp(cadeia, "real") == 0) || (strcmp(cadeia, "integer") == 0) )
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {
    
	/*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 1;
	char *seguidoresLocais[1] = {";"}; 

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: real ou integer esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
    }
	
    if (strcmp(cadeia, ";") == 0) 
    {
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        lista_par(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);

    }
    
}



void corpo_p(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{

        //<dc_v>
        
        /*Seguidores Locais*/
        int tamanhoSeguidoresLocais = 1;
        char *seguidoresLocais[1] = {"begin"};

        /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
        char **s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais, tamanhoSeguidoresLocais);
        int tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais;

        /*Chamada de Procedimento*/
        dc_v(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                
        delSeguidor(s, tamanhoS);



        if (strcmp(cadeia, "begin") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {
        
                /*Erro Sintático*/

               /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 7;
                char *seguidoresLocais[7] = {"read", "write", "while", "for", "if", "Identificador", "begin"}; // Primeiro(comandos)

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: begin esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }       


        //<comandos>
        
        /*Seguidores Locais*/
        int tamanhoSeguidoresLocais2 = 1;
        char *seguidoresLocais2[1] = {"end"};

        /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
        s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais2, tamanhoSeguidoresLocais2);
        tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais2;

        /*Chamada de Procedimento*/
        comandos(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                
        delSeguidor(s, tamanhoS);


        if(strcmp(cadeia, "end") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 1;
                char *seguidoresLocais[1] = {";"}; 

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: end esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }

        if(strcmp(cadeia, ";") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {
        
                /*Erro Sintático*/

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: ponto e vírgula esperado", NULL, 0, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }

}



void lista_arg(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
    
    if(strcmp(cadeia, "(") != 0) return;
    else 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo

    if (strcmp(valorLexico, "Identificador") == 0) 
    {
	analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo

	while(strcmp(cadeia, ";") == 0)
        {
	
            analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
			
            if (strcmp(valorLexico, "Identificador") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
            else
            {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 2;
                char *seguidoresLocais[2] = {";", ")"};

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: identificador esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
            }

        }

    }  

    else
    {
    
	/*Erro Sintático*/

	/*Seguidores Locais*/
	int tamanhoSeguidoresLocais = 1;
	char *seguidoresLocais[1] = {")"}; 

	/*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: identificador esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
    }


    // Fecha Parentese
    if (strcmp(cadeia, ")") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {
    
	/*Erro Sintático*/

        /*Tratamento de Erro Sintático*/
	if( erroSintatico("\nErro Sintático: fecha parentese esperado", NULL, 0, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
		return;
    
    } 
    
}



void pfalsa(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
    if(strcmp(cadeia, "else") != 0) return;
    else 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo

    cmd(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);

}



void comandos(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
    
    /*Verifica se a cadeia fornecida pelo Analisador Léxico está no conjunto primeiro de <cmd>*/
    if
    (
        strcmp(cadeia, "read")                  != 0 &&
        strcmp(cadeia, "write")                 != 0 &&
        strcmp(cadeia, "while")                 != 0 &&
        strcmp(cadeia, "for")                   != 0 &&
        strcmp(cadeia, "if")                    != 0 &&
        strcmp(cadeia, "begin")                 != 0 &&
        strcmp(valorLexico, "Identificador")    != 0
    
    ) return;


    //<cmd>
    
    /*Seguidores Locais*/
    int tamanhoSeguidoresLocais = 1;
    char *seguidoresLocais[1] = {";"};

    /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
    char **s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais, tamanhoSeguidoresLocais);
    int tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais;

    /*Chamada de Procedimento*/
    cmd(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
	
    delSeguidor(s, tamanhoS);


    // Ponto e vírgula

    if(strcmp(cadeia, ";") == 0) 
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
    else
    {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 7;
                char *seguidoresLocais[7] = {"read", "write", "while", "for", "if", "Identificador", "begin"}; // Primeiro(comandos)

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: ponto e vírgula esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
     }


    comandos(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);

}



void cmd(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
    
     /*Comandos read e write*/
     if( strcmp(cadeia, "read") == 0 || strcmp(cadeia, "write") == 0 )
     {
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo

        if(strcmp(cadeia, "(") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 1;
                char *seguidoresLocais[1] = {"Identificador"}; // Primeiro(variaveis)

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: abre parentese esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        } 


        /*<variaveis>*/
        if (strcmp(valorLexico, "Identificador") == 0) 
        {
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo

                while(strcmp(cadeia, ",") == 0)
                {
                
                        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
                                        
                        if (strcmp(valorLexico, "Identificador") == 0) 
                                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
                        else
                        {
                
                                /*Erro Sintático*/

                                /*Seguidores Locais*/
                                int tamanhoSeguidoresLocais = 2;
                                char *seguidoresLocais[2] = {",", ")"};

                                /*Tratamento de Erro Sintático*/
                                if( erroSintatico("\nErro Sintático: identificador esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                                        return;
                
                        }

                }

        }        
        else
        {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 1;
                char *seguidoresLocais[1] = {")"}; 

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: identificador esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }



        if(strcmp(cadeia, ")") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {

                /*Erro Sintático*/

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: fecha parentese esperado", NULL, 0, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;

        }
        
     }


     /*Comando while*/
     else if(strcmp(cadeia, "while") == 0)
     {
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo 

        if(strcmp(cadeia, "(") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 6;
                char *seguidoresLocais[6] = {"+", "-", "Identificador", "(", "Número Inteiro", "Número Real"}; // Primeiro(condicao)

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: abre parentese esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        } 

        
        //<condicao>
    
        /*Seguidores Locais*/
        int tamanhoSeguidoresLocais = 1;
        char *seguidoresLocais[1] = {")"};

        /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
        char **s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais, tamanhoSeguidoresLocais);
        int tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais;

        /*Chamada de Procedimento*/
        condicao(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                
        delSeguidor(s, tamanhoS);


        if(strcmp(cadeia, ")") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 1;
                char *seguidoresLocais[1]= {"do"};

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: fecha parentese esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }


        if(strcmp(cadeia, "do") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 7;
                char *seguidoresLocais[7] = {"read", "write", "while", "for", "if", "Identificador", "begin"}; // Primeiro(cmd)

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: do esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }

        cmd(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);
        
     }

     
     /*Comando for*/
     else if(strcmp(cadeia, "for") == 0)
     {
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo 

        if(strcmp(valorLexico, "Identificador") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 1;
                char *seguidoresLocais[1] = {":="}; 

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: identificador esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }


        if(strcmp(cadeia, ":=") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 6;
                char *seguidoresLocais[6] = {"+", "-", "Identificador", "(", "Número Inteiro", "Número Real"}; // Primeiro(expressao)

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: sinal de atribuição esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }

        //<expressao>
        
        /*Seguidores Locais*/
        int tamanhoSeguidoresLocais = 1;
        char *seguidoresLocais[1] = {"to"};

        /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
        char **s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais, tamanhoSeguidoresLocais);
        int tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais;

        /*Chamada de Procedimento*/
        expressao(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                
        delSeguidor(s, tamanhoS);

        
        if(strcmp(cadeia, "to") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 6;
                char *seguidoresLocais[6] = {"+", "-", "Identificador", "(", "Número Inteiro", "Número Real"}; // Primeiro(expressao)

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: to esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }

        
        //<expressao>
        
        /*Seguidores Locais*/
        int tamanhoSeguidoresLocais2 = 1;
        char *seguidoresLocais2[1] = {"do"};

        /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
        s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais2, tamanhoSeguidoresLocais2);
        tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais2;

        /*Chamada de Procedimento*/
        expressao(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                
        delSeguidor(s, tamanhoS);


        if(strcmp(cadeia, "do") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 7;
                char *seguidoresLocais[7] = {"read", "write", "while", "for", "if", "Identificador", "begin"}; // Primeiro(cmd)

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: do esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }


        cmd(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);
        
     }


     /*Comando if*/
     else if(strcmp(cadeia, "if") == 0)
     {
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo 

        //<condicao>
        
        /*Seguidores Locais*/
        int tamanhoSeguidoresLocais = 1;
        char *seguidoresLocais[1] = {"then"};

        /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
        char **s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais, tamanhoSeguidoresLocais);
        int tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais;

        /*Chamada de Procedimento*/
        condicao(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                
        delSeguidor(s, tamanhoS);
    


        if(strcmp(cadeia, "then") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 7;
                char *seguidoresLocais[7] = {"read", "write", "while", "for", "if", "Identificador", "begin"}; // Primeiro(cmd)

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: then esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }

        //<cmd>
    
        /*Seguidores Locais*/
        int tamanhoSeguidoresLocais2 = 1;
        char *seguidoresLocais2[1] = {"else"};

        /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
        s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais2, tamanhoSeguidoresLocais2);
        tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais2;

        /*Chamada de Procedimento*/
        cmd(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                
        delSeguidor(s, tamanhoS);


        pfalsa(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);
        
     }


     //Identificador
     else if(strcmp(valorLexico, "Identificador") == 0)
     {
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo 

        x(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);
        
     }


     //begin
     else if(strcmp(cadeia, "begin") == 0)
     {
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo 

        //<comandos>
        
        /*Seguidores Locais*/
        int tamanhoSeguidoresLocais = 1;
        char *seguidoresLocais[1] = {"end"};

        /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
        char **s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais, tamanhoSeguidoresLocais);
        int tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais;

        /*Chamada de Procedimento*/
        comandos(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                
        delSeguidor(s, tamanhoS);


        if(strcmp(cadeia, "end") == 0) 
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        else
        {

                /*Erro Sintático*/

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: end esperado", NULL, 0, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;

        }
        
     }


     else
     {

        /*Erro Sintático*/

        /*Tratamento de Erro Sintático*/
        if( erroSintatico("\nErro Sintático: read, write, while, for, if, identificador ou begin esperado", NULL, 0, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                return;

     }

}



void x(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
     if(strcmp(cadeia, ":=") == 0)
     {
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo 
        expressao(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);
     }
     else lista_arg(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);
}



void condicao(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{

        //<expressao>
    
        /*Seguidores Locais*/
        int tamanhoSeguidoresLocais = 6;
        char *seguidoresLocais[6] = {"=", "<>", ">=", "<=", ">", "<"};

        /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
        char **s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais, tamanhoSeguidoresLocais);
        int tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais;

        /*Chamada de Procedimento*/
        expressao(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                
        delSeguidor(s, tamanhoS);

        /*Operador Relacional*/
        if
        (
                strcmp(cadeia, "=")  == 0  ||
                strcmp(cadeia, "<>") == 0  ||
                strcmp(cadeia, ">=") == 0  ||
                strcmp(cadeia, "<=") == 0  ||
                strcmp(cadeia, ">")  == 0  ||
                strcmp(cadeia, "<")  == 0 
        
        ) analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo
        
        else
        {

                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 10;
                char *seguidoresLocais[10] = {";", "else", "=", "<>", ">=",  "<=", ">", "<", ")", "then"}; // Primeiro(expressao)

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: operador relacional esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }

        expressao(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);

}



void expressao(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
    //<termo>
    
    /*Seguidores Locais*/
    int tamanhoSeguidoresLocais = 2;
    char *seguidoresLocais[2] = {"+", "-"};

    /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
    char **s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais, tamanhoSeguidoresLocais);
    int tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais;

    /*Chamada de Procedimento*/
    termo(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
	
    delSeguidor(s, tamanhoS);

     if( strcmp(cadeia, "+") == 0 || strcmp(cadeia, "-") == 0 )
     {
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo 
        expressao(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);
     }

}



void termo(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
     if( strcmp(cadeia, "+") == 0 || strcmp(cadeia, "-") == 0 )
        analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo 
        
     fator(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);    

}



void fator(char **seguidor, int tamanhoSeguidor, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
        if
        (
                strcmp(valorLexico, "Identificador")  == 0  ||
                strcmp(valorLexico, "Número Inteiro") == 0  ||
                strcmp(valorLexico, "Número Real") == 0  
        
        ) analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo

        else if(strcmp(cadeia, "(") == 0)
        {
               
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo 

                //<expressao>
                
                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 1;
                char *seguidoresLocais[1] = {")"};

                /*Novo Conjunto de Seguidores que contém os Seguidores do Procedimento e os Seguidores Locais*/
                char **s = criarSeguidor(seguidor, tamanhoSeguidor, seguidoresLocais, tamanhoSeguidoresLocais);
                int tamanhoS = tamanhoSeguidor+tamanhoSeguidoresLocais;

                /*Chamada de Procedimento*/
                expressao(s, tamanhoS, buffer, tabelaPalavrasSimbolosReservados, saida);
                        
                delSeguidor(s, tamanhoS);


                if( strcmp(cadeia, ")") == 0)
                                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo 
                else
                {
                
                        /*Erro Sintático*/

                        /*Seguidores Locais*/
                        int tamanhoSeguidoresLocais = 2;
                        char *seguidoresLocais[2] = {"*", "/"};

                        /*Tratamento de Erro Sintático*/
                        if( erroSintatico("\nErro Sintático: fecha parentese esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                                return;
                
                }

        }
        else
        {
        
                /*Erro Sintático*/

                /*Seguidores Locais*/
                int tamanhoSeguidoresLocais = 2;
                char *seguidoresLocais[2] = {"*", "/"};

                /*Tratamento de Erro Sintático*/
                if( erroSintatico("\nErro Sintático: identificador, número inteiro, número real ou abre parentese esperado", seguidoresLocais, tamanhoSeguidoresLocais, seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida) )
                        return;
        
        }
        
        
        if( strcmp(cadeia, "*") == 0 || strcmp(cadeia, "/") == 0 )
        {
                analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo 
                fator(seguidor, tamanhoSeguidor, buffer, tabelaPalavrasSimbolosReservados, saida);
        }
                        
}



int erroSintatico(char* msg, char **conjuntoSincronizacaoLocal, int tamanhoConjuntoLocal, char **conjuntoSincronizacaoPai, int tamanhoConjuntoPai, Buffer *buffer, palavraSimboloReservado **tabelaPalavrasSimbolosReservados, FILE *saida)
{
        int localizacao;
        char token[30];

        if(fprintf(saida, " %s na Linha %d", msg, buffer->contadorLinhas) < 0) printf("\nErro na escrita do Arquivo de Saída!");

        if
        (       
                strcmp(valorLexico, "Identificador")    == 0    ||
                strcmp(valorLexico, "Número Inteiro")   == 0    ||
                strcmp(valorLexico, "Número Real")      == 0
        ) 
                strcpy(token, valorLexico);

        else    
                strcpy(token, cadeia);     
        

        while(1)
        {
            if(buscaToken(conjuntoSincronizacaoLocal, tamanhoConjuntoLocal, token))
            {
                localizacao = SEGUIDOR_LOCAL;
                break;
            }
            
            if(buscaToken(conjuntoSincronizacaoPai, tamanhoConjuntoPai, token))
            {
                localizacao = SEGUIDOR_PAI;
                break;
            }

            if(leituraFinalizada(buffer)) break;
            
            analisadorLexico(buffer, tabelaPalavrasSimbolosReservados, saida); //Obter Símbolo  

            if
            (       
                strcmp(valorLexico, "Identificador")    == 0    ||
                strcmp(valorLexico, "Número Inteiro")   == 0    ||
                strcmp(valorLexico, "Número Real")      == 0
            ) 
                strcpy(token, valorLexico);

            else    
                strcpy(token, cadeia);   
                   
        }

}



int buscaToken(char **conjuntoSincronizacao, int tamanhoConjunto, char *token)
{
        int i; /*Iterador*/
        int encontrou = 0; /*Flag que sinaliza a presença do token no Conjunto de Sincronização*/

        for(i = 0; i < tamanhoConjunto; i++)
        {
                if(strcmp(conjuntoSincronizacao[i], token) == 0) 
                {
                        encontrou = 1;
                        break;
                }
        }

        return encontrou;
}



char** criarSeguidor(char **seguidor, int tamanhoSeguidor, char** conjuntoAdicional, int tamanhoAdicional)
{
   int i; /*Iterador*/
   char **novoSeguidor; /*novo conjunto de seguidores resultante da união de seguidor com conjunto Adicional*/

   /* Alocação de memória */
   novoSeguidor = (char **)malloc(sizeof(char*)*(tamanhoSeguidor+tamanhoAdicional));

   if(novoSeguidor != NULL)
   {
        for(i=0; i < tamanhoSeguidor; i++)
        {
                novoSeguidor[i] = (char *)malloc(sizeof(char)*(strlen(seguidor[i])+1));
                strcpy(novoSeguidor[i], seguidor[i]);
        }
        
        for(i; i < tamanhoSeguidor+tamanhoAdicional; i++)
        {
                novoSeguidor[i] = (char *)malloc( sizeof(char)* (strlen(conjuntoAdicional[i-tamanhoSeguidor])+1) );
                strcpy(novoSeguidor[i], conjuntoAdicional[i-tamanhoSeguidor]);
        }

   }
   
   return novoSeguidor;
}



void delSeguidor(char**seguidor, int tamanhoSeguidor)
{
        int i; /*Iterador*/

        if(seguidor != NULL)
        {
                for(i=0; i < tamanhoSeguidor; i++)
                        free(seguidor[i]);

                free(seguidor);

        }

}


