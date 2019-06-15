#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <string.h>


/***
 **
 * Funcoes a serem alteradas!
 **
 ***/

/*
 * Retorna o tamanho (em bits) de um pacote codificado com base no
 * tamanho do pacote original (em bytes).
 */
int getCodedLength(int packetLength, int totalBitsMatrix) {
    
    /* 
       total de bits na matriz = (nº de colunas * nº de linhas + número de colunas + número de linhas)
       retorna total de bits na matriz * tamanho do pacote em bytes 
     */
     
    return (totalBitsMatrix * packetLength);
}

/*
 * Codifica o pacote de entrada, gerando um pacote
 * de saida com bits redundantes.
 */

void codePacket(unsigned char * codedPacket, unsigned char * originalPacket, int packetLength, int num_colunas, int num_linhas, int totalBitsMatrix) {

    unsigned char matrix[num_linhas][num_colunas];
    int i, j, k, soma;
    
    /* percorre cada byte do pacote */
    for(i = 0; i< packetLength; i++) {
        
        /* insere o pacote original na matriz */
        for(j = 0; j < num_linhas; j ++) {
            for(k = 0; k< num_colunas; k++) {
                matrix[j][k] = originalPacket[i * 8 + num_colunas * j + k];
            }
        }
        
        /* insere os bits do pacote original no pacote codificado */
        for(j = 0; j < 8; j++) {
            codedPacket[i * totalBitsMatrix + j] = originalPacket[i * 8 + j];
        }
        
        /* calcula bits de paridade das colunas da matriz */
        for(j = 0; j < num_colunas; j++) {
            /* ao iniciar em uma nova coluna, o valor de 'soma' é resetado*/
            soma = 0;
            for(k = 0; k < num_linhas; k++) {
                /* irá incrementar a variável 'soma' com a soma dos bits de cada linha. */
                soma += matrix[k][j];
    
            }
            
            /*  
                verifica se a variável 'soma' possui resto 0 ao dividir seu valor por 2;
                adiciona o bit de paridade referente na primeira posicição disponível, que será a seguinte dos bits de replicação do pack original 
            */
            if(soma % 2 == 0) {
                codedPacket[i * totalBitsMatrix + 8 + j] = 0;
            }
            else {
                codedPacket[i * totalBitsMatrix + 8 + j] = 1;
            }
        }
        
        /* calcula bits de paridade das linhas da matriz */
            for(j = 0; j < num_linhas; j++) {
                /* ao iniciar em uma nova linha, o valor de 'soma' é resetado*/
                soma = 0;
                for(k = 0; k < num_colunas; k++) {
                     /* irá incrementar a variável 'soma' com a soma dos bits de cada coluna. */
                    soma += matrix[j][k];
                }
                
                /*  
                    verifica se a variável soma possui resto 0 ao dividir seu valor por 2;
                    adiciona o bit de paridade referente na primeira posição disponível, que será a seguinte dos bits de replicação do pack original + bits de paridade das colunas 
                */
                if(soma % 2 == 0) {
                    codedPacket[i * totalBitsMatrix + 8 + num_colunas + j] = 0;
                }
                else {
                    codedPacket[i * totalBitsMatrix + 8 + num_colunas + j] = 1;
                }
            }
      }
}

/*
 * Executa decodificacao do pacote transmittedPacket, gerando
 * novo pacote decodedPacket. Nota: codedLength eh em bits.
 */
void decodePacket(unsigned char * decodedPacket, unsigned char * transmittedPacket, int codedLength, int num_colunas, int num_linhas, int totalBitsMatrix) {

    unsigned char parityMatrix[num_linhas][num_colunas];
    unsigned char parityColumns[num_colunas];
    unsigned char parityRows[num_linhas];
    int errorInColumn, errorInRow;
    int i, j, k, soma;
    int n = 0; // Contador de bytes no pacote decodificado.
    
    /* Itera por cada sequencia do total de bits em uma matriz (totalBitsMatrix) .*/
    for (i = 0; i < codedLength; i += totalBitsMatrix) {
        
        /*
         * dispõe bits do i-esimo conjunto na matriz.
         */
        for (j = 0; j < num_linhas; j++) {
            for (k = 0; k < num_colunas; k++) {
                parityMatrix[j][k] = transmittedPacket[i + num_colunas * j + k];
            }
        }
        
        /*
         * dispõe bits de paridade das colunas no vetor parityColumns.
         */
        for (j = 0; j < num_colunas; j++) {
            parityColumns[j] = transmittedPacket[i + 8 + j];
        }

        /*
         * dispõe bits de paridade das linhas no vetor parityRows.
         */
        for (j = 0; j < num_linhas; j++) {
            parityRows[j] = transmittedPacket[i + 8 + num_colunas + j];
        }

        /*
         * Verificacao dos bits de paridade: colunas.
         * Note que paramos no primeiro erro, ja que se houver mais
         * erros, o metodo eh incapaz de corrigi-los de qualquer
         * forma.
         */
        errorInColumn = -1;
        for (j = 0; j < num_colunas; j++) {
            /* Ao iniciar uma nova coluna, a variável 'soma' é resetada */
            soma = 0;
            for(k = 0; k < num_linhas; k++) {
                /* irá incrementar a variável 'soma' com a soma dos bits de cada linha. */
                soma += parityMatrix[k][j];
            }
            
            /* caso encontre algum erro, a execução é encerrada */
            if (soma % 2 != parityColumns[j]) {
                errorInColumn = j;
                break ;
            }
        }

        /*
         * Verificacao dos bits de paridade: linhas.
         * Note que paramos no primeiro erro, ja que se houver mais
         * erros, o metodo eh incapaz de corrigi-los de qualquer
         * forma.
         */
        errorInRow = -1;
        for (j = 0; j < num_linhas; j++) {
            /* Ao iniciar uma nova linha, a variável 'soma' é resetada */
            soma = 0;
            
            for (k = 0; k < num_colunas; k ++) {
                /* irá incrementar a variável 'soma' com a soma dos bits de cada coluna. */
                soma+= parityMatrix[j][k];
            }

/* caso encontre algum erro, a execução é encerrada */
            if (soma % 2 != parityRows[j]) {
                errorInRow = j;
                break ;
            }
        }

        /*
         * Se algum erro foi encontrado, corrigir.
         */
        if (errorInRow > -1 && errorInColumn > -1) {

            if (parityMatrix[errorInRow][errorInColumn] == 1)
                parityMatrix[errorInRow][errorInColumn] = 0;
            else
                parityMatrix[errorInRow][errorInColumn] = 1;
        }

        /*
         * Colocar bits (possivelmente corrigidos) na saida.
         */
         for (j = 0; j < num_linhas; j++) {
             for (k = 0; k < num_colunas; k++) {
                 decodedPacket[8 * n + num_colunas * j + k] = parityMatrix[j][k];
             }
         }

         /*
          * Incrementar numero de bytes na saida.
          */
         n++;
    }

}

/***
 **
 * Outras funcoes.
 **
 ***/

/*
 * Gera conteudo aleatorio no pacote passado como
 * parÃ¢metro. Pacote eh representado por um vetor de
 * bytes, em que cada posicao representa um bit.
 * Comprimento do pacote (em bytes) tambÃ©m deve ser
 * especificado.
 */
void generateRandomPacket(unsigned char * packet, int length) {

    int i;

    for (i = 0; i < length * 8; i++) {

        packet[i] = rand() % 2;
    }
}

/*
 * Gera um numero pseudo-aleatorio com distribuicao geometrica.
 */
int geomRand(double p) {

    double uRand = ((double) rand() + 1) / ((double) RAND_MAX + 1);

    return((int) floor(log(uRand) / log(1 - p)));
}

/*
 * Insere erros aleatorios no pacote, gerando uma nova versao.
 * Cada bit tem seu erro alterado com probabilidade errorProb,
 * e de forma independente dos demais bits.
 * Retorna o numero de erros inseridos no pacote.
 */
int insertErrors(unsigned char * transmittedPacket, unsigned char * codedPacket, int codedLength, double errorProb) {

    int i = -1;
    int n = 0; // Numero de erros inseridos no pacote.
    int r;

    /*
     * Copia o conteÃºdo do pacote codificado para o novo pacote.
     */
    memcpy(transmittedPacket, codedPacket, codedLength);

    while (1) {

        /*
         * Sorteia a proxima posicao em que um erro sera inserido.
         */
        r = geomRand(errorProb);
        i = i + 1 + r;

        if (i >= codedLength) break ;

        /*
         * Altera o valor do bit.
         */
        if (transmittedPacket[i] == 1)
            transmittedPacket[i] = 0;
        else
            transmittedPacket[i] = 1;

        n++;
    }

    return(n);
}

/*
 * Conta o numero de bits errados no pacote
 * decodificado usando como referencia
 * o pacote original. O parametro packetLength especifica o
 * tamanho dos dois pacotes em bytes.
 */
int countErrors(unsigned char * originalPacket, unsigned char * decodedPacket, int packetLength) {

    int i;
    int errors = 0;

    for (i = 0; i < packetLength * 8; i++) {

        if (originalPacket[i] != decodedPacket[i])
            errors++;
    }

    return(errors);
}

/*
 * Exibe modo de uso e aborta execucao.
 */
void help(char * self) {

    fprintf(stderr, "Simulador de metodos de FEC/codificacao.\n\n");
    fprintf(stderr, "Modo de uso:\n\n");
    fprintf(stderr, "\t%s <tam_pacote> <reps> <prob. erro>\n\n", self);
    fprintf(stderr, "Onde:\n");
    fprintf(stderr, "\t- <tam_pacote>: tamanho do pacote usado nas simulacoes (em bytes).\n");
    fprintf(stderr, "\t- <reps>: numero de repeticoes da simulacao.\n");
    fprintf(stderr, "\t- <prob. erro>: probabilidade de erro de bits (i.e., probabilidade\n"
                    "de que um dado bit tenha seu valor alterado pelo canal.)\n\n");

    exit(1);
}

/*
 * Programa principal:
 *  - le parametros de entrada;
 *  - gera pacote aleatorio;
 *  - gera bits de redundancia do pacote
 *  - executa o numero pedido de simulacoes:
 *      + Introduz erro
 *  - imprime estatisticas.
 */
int main(int argc, char ** argv) {

    
    /*
     * Parametros de entrada.
     */
    int packetLength, reps;
    double errorProb;

    /* Dimensões da matriz que serão inseridas pelo usuário*/ 
    int num_colunas, num_linhas;

    /*
     * Pacotes manipulados.
     */
    unsigned char * originalPacket;
    unsigned char * codedPacket;
    unsigned char * decodedPacket;
    unsigned char * transmittedPacket;

    /*
     * VariÃ¡veis auxiliares.
     */
    int i;
    unsigned long bitErrorCount;
    unsigned long totalBitErrorCount = 0;
    unsigned long totalPacketErrorCount = 0;
    unsigned long totalInsertedErrorCount = 0;
    int codedLength;

    /*
     * Leitura dos argumentos de linha de comando.
     */
    if (argc != 6)
        help(argv[0]);

    packetLength = atoi(argv[1]);
    reps = atoi(argv[2]);
    errorProb = atof(argv[3]);
    /* Definição das dimensões da matriz */
    num_colunas = atoi(argv[4]);
    num_linhas = atoi(argv[5]);

    if (packetLength <= 0 || reps <= 0 || errorProb < 0 || errorProb > 1)
        help(argv[0]);
        
    /* cálculo da quantidade de bits que a matriz irá produzir uma vez que estiver completa */
    int totalBitsMatrix = (num_colunas * num_linhas) + num_colunas + num_linhas;

    /*
     * Inicializacao da semente do gerador de numeros
     * pseudo-aleatorios.
     */
    srand(time(NULL));
    
   

    /*
     * Geracao do pacote original aleatorio.
     */
    codedLength = getCodedLength(packetLength, totalBitsMatrix);
    originalPacket = malloc(packetLength * 8);
    decodedPacket = malloc(packetLength * 8);
    codedPacket = malloc(codedLength);
    transmittedPacket = malloc(codedLength);

    generateRandomPacket(originalPacket, packetLength);
    codePacket(codedPacket, originalPacket, packetLength, num_colunas, num_linhas, totalBitsMatrix);

    /*
     * Loop de repeticoes da simulacao.
     */
    for (i = 0; i < reps; i++) {

        /*
         * Gerar nova versao do pacote com erros aleatorios.
         */
        totalInsertedErrorCount += insertErrors(transmittedPacket, codedPacket, codedLength, errorProb);

        /*
         * Gerar versao decodificada do pacote.
         */
        decodePacket(decodedPacket, transmittedPacket, codedLength, num_colunas, num_linhas, totalBitsMatrix);

        /*
         * Contar erros.
         */
        bitErrorCount = countErrors(originalPacket, decodedPacket, packetLength);

        if (bitErrorCount) {

            totalBitErrorCount += bitErrorCount;
            totalPacketErrorCount++;
        }
    }

    printf("Numero de transmissoes simuladas: %d\n\n", reps);
    printf("Numero de bits transmitidos: %d\n", reps * packetLength * 8);
    printf("Numero de bits errados inseridos: %lu\n", totalInsertedErrorCount);
    printf("Taxa de erro de bits (antes da decodificacao): %.2f%%\n\n", (double) totalInsertedErrorCount / (double) (reps * codedLength) * 100.0);
    printf("Numero de bits corrompidos apos decodificacao: %lu\n", totalBitErrorCount);
    printf("Taxa de erro de bits (apos decodificacao): %.2f%%\n\n", (double) totalBitErrorCount / (double) (reps * packetLength * 8) * 100.0);
    printf("Numero de pacotes corrompidos: %lu\n", totalPacketErrorCount);
    printf("Taxa de erro de pacotes: %.2f%%\n", (double) totalPacketErrorCount / (double) reps * 100.0);

    return(0);
}