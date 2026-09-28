#include "controle_aereo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void trocar(Aviao **a, Aviao **b)
{
    Aviao *temp;

    if (a == NULL || b == NULL) {
        return;
    }

    temp = *a;
    *a = *b;
    *b = temp;
}

static int maior_prioridade(
    const Aviao *a,
    const Aviao *b
)
{
    if (a == NULL || b == NULL) {
        return 0;
    }

    if (a->prioridade != b->prioridade) {
        return a->prioridade < b->prioridade;
    }

    return a->ordem_chegada < b->ordem_chegada;
}

static void subir_heap(
    FilaPrioridade *fila,
    int indice
)
{
    int pai;

    if (fila == NULL || fila->dados == NULL) {
        return;
    }

    while (indice > 0) {

        pai = (indice - 1) / 2;

        if (!maior_prioridade(
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

static void descer_heap(
    FilaPrioridade *fila,
    int indice
)
{
    int esquerda;
    int direita;
    int maior;

    if (fila == NULL || fila->dados == NULL) {
        return;
    }

    while (1) {

        esquerda = 2 * indice + 1;
        direita = 2 * indice + 2;

        maior = indice;

        if (esquerda < fila->tamanho &&
            maior_prioridade(
                fila->dados[esquerda],
                fila->dados[maior])) {

            maior = esquerda;
        }

        if (direita < fila->tamanho &&
            maior_prioridade(
                fila->dados[direita],
                fila->dados[maior])) {

            maior = direita;
        }

        if (maior == indice) {
            break;
        }

        trocar(
            &fila->dados[indice],
            &fila->dados[maior]
        );

        indice = maior;
    }
}

void inicializar_fila(
    FilaPrioridade *fila,
    int capacidade
)
{
    if (fila == NULL) {
        return;
    }

    if (capacidade < 1) {
        capacidade = 10;
    }

    fila->dados = malloc(
        (size_t)capacidade * sizeof(Aviao *)
    );

    if (fila->dados == NULL) {
        fila->tamanho = 0;
        fila->capacidade = 0;
        return;
    }

    fila->tamanho = 0;
    fila->capacidade = capacidade;
}

int inserir_aviao(
    FilaPrioridade *fila,
    Aviao *aviao
)
{
    Aviao **novo;
    int nova_capacidade;

    if (fila == NULL || aviao == NULL) {
        return 0;
    }

    if (fila->dados == NULL ||
        fila->capacidade <= 0) {

        return 0;
    }

    if (fila->tamanho >= fila->capacidade) {

        nova_capacidade = fila->capacidade * 2;

        novo = realloc(
            fila->dados,
            (size_t)nova_capacidade * sizeof(Aviao *)
        );

        if (novo == NULL) {
            return 0;
        }

        fila->dados = novo;
        fila->capacidade = nova_capacidade;
    }

    fila->dados[fila->tamanho] = aviao;

    subir_heap(
        fila,
        fila->tamanho
    );

    fila->tamanho++;

    return 1;
}

Aviao *consultar_proximo_aviao(
    const FilaPrioridade *fila
)
{
    if (fila == NULL ||
        fila->dados == NULL ||
        fila->tamanho <= 0) {

        return NULL;
    }

    return fila->dados[0];
}

Aviao *remover_proximo_aviao(
    FilaPrioridade *fila
)
{
    Aviao *aviao;

    if (fila == NULL ||
        fila->dados == NULL ||
        fila->tamanho <= 0) {

        return NULL;
    }

    aviao = fila->dados[0];

    fila->tamanho--;

    if (fila->tamanho > 0) {

        fila->dados[0] =
            fila->dados[fila->tamanho];

        descer_heap(
            fila,
            0
        );
    }

    return aviao;
}

int fila_vazia(
    const FilaPrioridade *fila
)
{
    return fila == NULL ||
           fila->tamanho == 0;
}

int calcular_prioridade(
    Aviao *aviao
)
{
    if (aviao == NULL) {
        return 3;
    }

    /*
     * Prioridade 1:
     * combustível abaixo de 10%
     * OU emergência médica.
     */
    if (aviao->combustivel < 10 ||
        aviao->emergencia_medica) {

        return 1;
    }

    /*
     * Prioridade 2:
     * voo comercial.
     */
    if (aviao->tipo == VOO_COMERCIAL) {
        return 2;
    }

    /*
     * Prioridade 3:
     * carga ou privado.
     */
    return 3;
}

Aviao gerar_aviao(
    unsigned long ordem
)
{
    Aviao aviao;
    int sorteio_tipo;

    memset(
        &aviao,
        0,
        sizeof(Aviao)
    );

    snprintf(
        aviao.identificacao,
        sizeof(aviao.identificacao),
        "VOO%03lu",
        ordem % 1000
    );

    aviao.combustivel =
        5 + rand() % 96;

    /*
     * Aproximadamente 10% dos voos
     * possuem emergência médica.
     */
    aviao.emergencia_medica =
        (rand() % 10 == 0);

    sorteio_tipo = rand() % 100;

    if (sorteio_tipo < 60) {

        aviao.tipo = VOO_COMERCIAL;

    } else if (sorteio_tipo < 85) {

        aviao.tipo = VOO_CARGA;

    } else {

        aviao.tipo = VOO_PRIVADO;
    }

    aviao.sentido =
        (rand() % 2 == 0)
            ? HORARIO
            : ANTI_HORARIO;

    aviao.ordem_chegada = ordem;

    aviao.prioridade =
        calcular_prioridade(&aviao);

    aviao.estado =
        APROXIMANDO;

    aviao.posicao_taxiway = -1;
    aviao.portao = -1;
    aviao.pista = -1;

    aviao.chegada = time(NULL);

    aviao.alerta_conflito_registrado = 0;

    return aviao;
}

void destruir_fila(
    FilaPrioridade *fila
)
{
    if (fila == NULL) {
        return;
    }

    free(fila->dados);

    fila->dados = NULL;
    fila->tamanho = 0;
    fila->capacidade = 0;
}
