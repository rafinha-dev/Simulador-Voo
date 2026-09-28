/*
 * Sistema de Controle de Trafego Aereo
 *
 * A fila de prioridade e implementada utilizando
 * um Heap Binario armazenado em um array dinamico.
 *
 * Regras de prioridade:
 *
 * Prioridade 1:
 *   - emergencia medica
 *   - combustivel abaixo de 10%
 *
 * Prioridade 2:
 *   - voo comercial ou regular
 *
 * Prioridade 3:
 *   - voo de carga
 *   - voo particular
 *
 * Quanto menor o numero da prioridade, maior a prioridade.
 *
 * Em caso de empate, a aeronave que chegou primeiro
 * possui prioridade sobre a que chegou depois.
 *
 * A chegada das aeronaves e simulada automaticamente
 * por meio de valores aleatorios.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


/*
 * Representa os tipos de voo aceitos pelo sistema.
 */
typedef enum {

    COMERCIAL,
    CARGA,
    PARTICULAR

} TipoVoo;


/*
 * Representa uma aeronave.
 *
 * identificacao:
 *     Identificacao do voo.
 *
 * combustivel:
 *     Percentual de combustivel restante.
 *
 * emergencia_medica:
 *     1 = existe emergencia medica.
 *     0 = nao existe emergencia medica.
 *
 * tipo:
 *     Tipo do voo.
 *
 * prioridade:
 *     Nivel calculado automaticamente pelo sistema.
 *
 * ordem_chegada:
 *     Ordem em que a aeronave chegou ao sistema.
 */
typedef struct {

    char identificacao[20];

    float combustivel;

    int emergencia_medica;

    TipoVoo tipo;

    int prioridade;

    unsigned long ordem_chegada;

} Aviao;


/*
 * Representa a fila de prioridade.
 *
 * dados:
 *     Array dinamico utilizado para armazenar
 *     as aeronaves do Heap.
 *
 * tamanho:
 *     Quantidade atual de aeronaves.
 *
 * capacidade:
 *     Quantidade de aeronaves que podem ser
 *     armazenadas antes de uma realocacao.
 */
typedef struct {

    Aviao *dados;

    int tamanho;

    int capacidade;

} FilaPrioridade;


/*
 * Calcula automaticamente a prioridade de uma aeronave.
 *
 * Prioridade 1:
 *     emergencia medica ou combustivel < 10%.
 *
 * Prioridade 2:
 *     voo comercial.
 *
 * Prioridade 3:
 *     carga ou particular.
 */
int calcular_prioridade(const Aviao *aviao)
{
    /*
     * Teste de integridade do ponteiro.
     */
    if (aviao == NULL) {
        fprintf(stderr, "Erro: ponteiro de aviao invalido.\n");
        exit(EXIT_FAILURE);
    }

    if (aviao->emergencia_medica ||
        aviao->combustivel < 10.0f) {

        return 1;
    }

    if (aviao->tipo == COMERCIAL) {

        return 2;
    }

    return 3;
}


/*
 * Retorna o nome correspondente ao tipo de voo.
 */
const char *nome_tipo_voo(TipoVoo tipo)
{
    switch (tipo) {

        case COMERCIAL:
            return "Comercial";

        case CARGA:
            return "Carga";

        case PARTICULAR:
            return "Particular";

        default:
            return "Desconhecido";
    }
}


/*
 * Verifica qual das duas aeronaves deve ser atendida primeiro.
 *
 * O primeiro criterio e o nivel de prioridade.
 *
 * O segundo criterio e a ordem de chegada.
 */
int tem_prioridade(Aviao a, Aviao b)
{
    if (a.prioridade != b.prioridade) {

        return a.prioridade < b.prioridade;
    }

    return a.ordem_chegada < b.ordem_chegada;
}


/*
 * Troca duas aeronaves de posicao no array.
 */
void trocar(Aviao *a, Aviao *b)
{
    /*
     * Teste de integridade dos dois ponteiros.
     */
    if (a == NULL || b == NULL) {

        fprintf(
            stderr,
            "Erro: ponteiro invalido durante troca.\n"
        );

        exit(EXIT_FAILURE);
    }

    Aviao temporaria = *a;

    *a = *b;

    *b = temporaria;
}


/*
 * Move uma aeronave para cima no Heap.
 *
 * Essa operacao e utilizada depois da insercao
 * de uma nova aeronave.
 */
void subir_heap(FilaPrioridade *fila, int indice)
{
    /*
     * Verifica se a fila existe.
     */
    if (fila == NULL) {

        fprintf(
            stderr,
            "Erro: ponteiro da fila invalido.\n"
        );

        exit(EXIT_FAILURE);
    }


    /*
     * Verifica se o array da fila existe.
     */
    if (fila->dados == NULL) {

        fprintf(
            stderr,
            "Erro: array da fila invalido.\n"
        );

        exit(EXIT_FAILURE);
    }


    while (indice > 0) {

        /*
         * Calcula a posicao do pai.
         */
        int pai = (indice - 1) / 2;


        /*
         * Se o filho nao tiver maior prioridade
         * que o pai, o Heap esta correto.
         */
        if (!tem_prioridade(
                fila->dados[indice],
                fila->dados[pai])) {

            break;
        }


        trocar(
            &fila->dados[indice],
            &fila->dados[pai]
        );


        indice = pai;
    }
}


/*
 * Move uma aeronave para baixo no Heap.
 *
 * Essa operacao e utilizada depois da remocao
 * da aeronave que estava na raiz.
 */
void descer_heap(FilaPrioridade *fila, int indice)
{
    /*
     * Verifica a integridade da fila.
     */
    if (fila == NULL) {

        fprintf(
            stderr,
            "Erro: ponteiro da fila invalido.\n"
        );

        exit(EXIT_FAILURE);
    }


    /*
     * Verifica a integridade do array.
     */
    if (fila->dados == NULL) {

        fprintf(
            stderr,
            "Erro: array da fila invalido.\n"
        );

        exit(EXIT_FAILURE);
    }


    while (1) {

        /*
         * Calcula as posicoes dos filhos.
         */
        int esquerda = 2 * indice + 1;

        int direita = 2 * indice + 2;


        /*
         * Inicialmente considera o elemento atual
         * como o de maior prioridade.
         */
        int melhor = indice;


        /*
         * Verifica o filho esquerdo.
         */
        if (esquerda < fila->tamanho &&
            tem_prioridade(
                fila->dados[esquerda],
                fila->dados[melhor])) {

            melhor = esquerda;
        }


        /*
         * Verifica o filho direito.
         */
        if (direita < fila->tamanho &&
            tem_prioridade(
                fila->dados[direita],
                fila->dados[melhor])) {

            melhor = direita;
        }


        /*
         * Se o elemento atual ja for o melhor,
         * o Heap esta organizado.
         */
        if (melhor == indice) {

            break;
        }


        trocar(
            &fila->dados[indice],
            &fila->dados[melhor]
        );


        indice = melhor;
    }
}


/*
 * Inicializa a fila de prioridade.
 */
void inicializar_fila(
    FilaPrioridade *fila,
    int capacidade
)
{
    /*
     * Verifica o ponteiro da fila.
     */
    if (fila == NULL) {

        fprintf(
            stderr,
            "Erro: ponteiro da fila invalido.\n"
        );

        exit(EXIT_FAILURE);
    }


    /*
     * A capacidade precisa ser positiva.
     */
    if (capacidade <= 0) {

        fprintf(
            stderr,
            "Erro: capacidade invalida.\n"
        );

        exit(EXIT_FAILURE);
    }


    /*
     * Aloca memoria para o array de aeronaves.
     */
    fila->dados =
        malloc(capacidade * sizeof(Aviao));


    /*
     * Teste de integridade do ponteiro retornado
     * pelo malloc.
     */
    if (fila->dados == NULL) {

        fprintf(
            stderr,
            "Erro: malloc nao conseguiu alocar memoria.\n"
        );

        exit(EXIT_FAILURE);
    }


    fila->tamanho = 0;

    fila->capacidade = capacidade;
}


/*
 * Insere uma aeronave na fila de prioridade.
 *
 * A prioridade e determinada automaticamente
 * pelo sistema.
 */
void inserir(
    FilaPrioridade *fila,
    Aviao aviao
)
{
    /*
     * Verifica a integridade da fila.
     */
    if (fila == NULL) {

        fprintf(
            stderr,
            "Erro: ponteiro da fila invalido.\n"
        );

        exit(EXIT_FAILURE);
    }


    /*
     * Verifica se o array ja foi inicializado.
     */
    if (fila->dados == NULL) {

        fprintf(
            stderr,
            "Erro: array da fila nao inicializado.\n"
        );

        exit(EXIT_FAILURE);
    }


    /*
     * Calcula a prioridade automaticamente.
     */
    aviao.prioridade =
        calcular_prioridade(&aviao);


    /*
     * Verifica se o array esta cheio.
     */
    if (fila->tamanho >= fila->capacidade) {

        /*
         * Dobra a capacidade.
         */
        int nova_capacidade =
            fila->capacidade * 2;


        /*
         * Realoca o array.
         */
        Aviao *novo =
            realloc(
                fila->dados,
                nova_capacidade * sizeof(Aviao)
            );


        /*
         * Teste de integridade do ponteiro retornado
         * por realloc.
         */
        if (novo == NULL) {

            fprintf(
                stderr,
                "Erro: realloc nao conseguiu realocar memoria.\n"
            );

            /*
             * O ponteiro original continua valido
             * quando realloc falha.
             */
            exit(EXIT_FAILURE);
        }


        /*
         * Atualiza o ponteiro somente depois
         * de confirmar que realloc funcionou.
         */
        fila->dados = novo;

        fila->capacidade =
            nova_capacidade;
    }


    /*
     * Insere a aeronave no final do Heap.
     */
    int indice = fila->tamanho;

    fila->dados[indice] = aviao;

    fila->tamanho++;


    /*
     * Reorganiza o Heap.
     */
    subir_heap(
        fila,
        indice
    );
}


/*
 * Remove a aeronave de maior prioridade.
 */
Aviao remover_proximo(FilaPrioridade *fila)
{
    /*
     * Verifica o ponteiro da fila.
     */
    if (fila == NULL) {

        fprintf(
            stderr,
            "Erro: ponteiro da fila invalido.\n"
        );

        exit(EXIT_FAILURE);
    }


    /*
     * Verifica o ponteiro do array.
     */
    if (fila->dados == NULL) {

        fprintf(
            stderr,
            "Erro: array da fila invalido.\n"
        );

        exit(EXIT_FAILURE);
    }


    /*
     * Verifica se existem aeronaves.
     */
    if (fila->tamanho == 0) {

        fprintf(
            stderr,
            "Erro: tentativa de remover de uma fila vazia.\n"
        );

        exit(EXIT_FAILURE);
    }


    /*
     * A raiz do Heap possui a maior prioridade.
     */
    Aviao proximo =
        fila->dados[0];


    /*
     * Diminui a quantidade de elementos.
     */
    fila->tamanho--;


    /*
     * Se ainda houver elementos,
     * coloca o ultimo elemento na raiz.
     */
    if (fila->tamanho > 0) {

        fila->dados[0] =
            fila->dados[fila->tamanho];


        /*
         * Reorganiza o Heap.
         */
        descer_heap(
            fila,
            0
        );
    }


    return proximo;
}


/*
 * Consulta a aeronave de maior prioridade
 * sem remove-la da fila.
 */
Aviao consultar_proximo(
    const FilaPrioridade *fila
)
{
    /*
     * Verifica o ponteiro da fila.
     */
    if (fila == NULL) {

        fprintf(
            stderr,
            "Erro: ponteiro da fila invalido.\n"
        );

        exit(EXIT_FAILURE);
    }


    /*
     * Verifica o array.
     */
    if (fila->dados == NULL) {

        fprintf(
            stderr,
            "Erro: array da fila invalido.\n"
        );

        exit(EXIT_FAILURE);
    }


    /*
     * Verifica se a fila esta vazia.
     */
    if (fila->tamanho == 0) {

        fprintf(
            stderr,
            "Erro: fila vazia.\n"
        );

        exit(EXIT_FAILURE);
    }


    return fila->dados[0];
}


/*
 * Verifica se a fila esta vazia.
 */
int fila_vazia(
    const FilaPrioridade *fila
)
{
    /*
     * Verifica a integridade do ponteiro.
     */
    if (fila == NULL) {

        fprintf(
            stderr,
            "Erro: ponteiro da fila invalido.\n"
        );

        exit(EXIT_FAILURE);
    }


    return fila->tamanho == 0;
}


/*
 * Gera uma identificacao para a aeronave.
 */
void gerar_identificacao(
    char identificacao[],
    int numero
)
{
    /*
     * Verifica se o array recebido existe.
     */
    if (identificacao == NULL) {

        fprintf(
            stderr,
            "Erro: ponteiro de identificacao invalido.\n"
        );

        exit(EXIT_FAILURE);
    }


    sprintf(
        identificacao,
        "VOO%03d",
        numero
    );
}


/*
 * Gera uma aeronave aleatoriamente.
 */
Aviao gerar_aviao(int numero)
{
    Aviao aviao;


    /*
     * Gera a identificacao do voo.
     */
    gerar_identificacao(
        aviao.identificacao,
        numero
    );


    /*
     * Gera combustivel entre 0 e 100%.
     */
    aviao.combustivel =
        (float)(rand() % 101);


    /*
     * Gera aleatoriamente uma emergencia.
     */
    aviao.emergencia_medica =
        (rand() % 10 == 0);


    /*
     * Gera aleatoriamente o tipo do voo.
     */
    aviao.tipo =
        (TipoVoo)(rand() % 3);


    /*
     * A prioridade sera calculada pela funcao inserir().
     */
    aviao.prioridade = 0;


    /*
     * Registra a ordem de chegada.
     */
    aviao.ordem_chegada =
        (unsigned long)numero;


    return aviao;
}


/*
 * Imprime os dados de uma aeronave.
 */
void imprimir_aviao(
    const Aviao *aviao
)
{
    /*
     * Teste de integridade do ponteiro.
     */
    if (aviao == NULL) {

        fprintf(
            stderr,
            "Erro: ponteiro de aviao invalido.\n"
        );

        return;
    }


    printf(
        "%s | Tipo: %-10s | Combustivel: %5.1f%% | "
        "Emergencia: %-3s | Prioridade: %d\n",

        aviao->identificacao,

        nome_tipo_voo(aviao->tipo),

        aviao->combustivel,

        aviao->emergencia_medica
            ? "SIM"
            : "NAO",

        aviao->prioridade
    );
}


/*
 * Libera a memoria utilizada pela fila.
 */
void destruir_fila(
    FilaPrioridade *fila
)
{
    /*
     * Se o ponteiro for invalido, nao existe
     * estrutura que possa ser liberada.
     */
    if (fila == NULL) {

        fprintf(
            stderr,
            "Aviso: tentativa de destruir uma fila invalida.\n"
        );

        return;
    }


    /*
     * free(NULL) e seguro em C, mas mantemos
     * a verificacao explicita para deixar a
     * integridade do ponteiro clara.
     */
    if (fila->dados != NULL) {

        free(fila->dados);

        fila->dados = NULL;
    }


    /*
     * Zera os demais campos.
     */
    fila->tamanho = 0;

    fila->capacidade = 0;
}


/*
 * Funcao principal.
 */
int main(void)
{
    /*
     * Inicializa o gerador de numeros aleatorios.
     *
     * O horario atual faz com que cada execucao
     * normalmente produza uma sequencia diferente.
     */
    srand(
        (unsigned int)time(NULL)
    );


    /*
     * Cria a fila de prioridade.
     */
    FilaPrioridade fila;


    /*
     * Inicializa a fila com capacidade inicial
     * para cinco aeronaves.
     */
    inicializar_fila(
        &fila,
        5
    );


    /*
     * Quantidade de aeronaves que sera simulada.
     */
    const int quantidade_avioes = 15;


    printf(
        "\nSistema de Controle de Trafego Aereo\n\n"
    );


    /*
     * Simula a chegada das aeronaves.
     */
    for (
        int i = 1;
        i <= quantidade_avioes;
        i++
    ) {

        /*
         * Cria uma nova aeronave.
         */
        Aviao aviao =
            gerar_aviao(i);


        /*
         * Insere a aeronave na fila.
         *
         * A prioridade e calculada automaticamente.
         */
        inserir(
            &fila,
            aviao
        );


        /*
         * Consulta a aeronave que atualmente
         * possui maior prioridade.
         */
        Aviao proximo =
            consultar_proximo(&fila);


        printf(
            "Chegada: "
        );

        imprimir_aviao(
            &aviao
        );


        printf(
            "Proximo para pouso: %s\n\n",
            proximo.identificacao
        );
    }


    /*
     * Autoriza os pousos seguindo a ordem
     * determinada pela fila de prioridade.
     */
    printf(
        "\nOrdem de autorizacao para pouso:\n\n"
    );


    while (!fila_vazia(&fila)) {

        /*
         * Remove a aeronave mais prioritária.
         */
        Aviao aviao =
            remover_proximo(&fila);


        printf(
            "Pouso autorizado: "
        );

        imprimir_aviao(
            &aviao
        );
    }


    /*
     * Libera a memoria utilizada pelo Heap.
     */
    destruir_fila(
        &fila
    );


    return 0;
}
