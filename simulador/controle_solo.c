#include "controle_solo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static No *criar_no(int id)
{
    No *novo = malloc(sizeof(No));

    if (novo == NULL) {
        return NULL;
    }

    novo->id = id;
    novo->ocupado = 0;
    novo->aviao = NULL;

    novo->proximo = NULL;
    novo->anterior = NULL;

    return novo;
}

static int adicionar_no(
    Aeroporto *aeroporto,
    int id
)
{
    No *novo;

    if (aeroporto == NULL) {
        return 0;
    }

    novo = criar_no(id);

    if (novo == NULL) {
        return 0;
    }

    if (aeroporto->inicio == NULL) {

        aeroporto->inicio = novo;
        aeroporto->fim = novo;

    } else {

        novo->anterior =
            aeroporto->fim;

        aeroporto->fim->proximo =
            novo;

        aeroporto->fim =
            novo;
    }

    return 1;
}

void inicializar_aeroporto(
    Aeroporto *aeroporto
)
{
    if (aeroporto == NULL) {
        return;
    }

    memset(
        aeroporto,
        0,
        sizeof(Aeroporto)
    );
}

int carregar_configuracao(
    Aeroporto *aeroporto,
    const char *arquivo
)
{
    FILE *fp;
    char linha[128];

    if (aeroporto == NULL ||
        arquivo == NULL) {

        return 0;
    }

    fp = fopen(
        arquivo,
        "r"
    );

    if (fp == NULL) {
        return 0;
    }

    while (fgets(
        linha,
        sizeof(linha),
        fp
    ) != NULL) {

        linha[
            strcspn(
                linha,
                "\r\n"
            )
        ] = '\0';

        if (strncmp(
                linha,
                "NOME=",
                5
            ) == 0) {

            snprintf(
                aeroporto->nome,
                sizeof(aeroporto->nome),
                "%s",
                linha + 5
            );

        } else if (strncmp(
                       linha,
                       "PISTAS=",
                       7
                   ) == 0) {

            aeroporto->pistas =
                atoi(linha + 7);

        } else if (strncmp(
                       linha,
                       "TAXIWAYS=",
                       9
                   ) == 0) {

            aeroporto->quantidade_taxiways =
                atoi(linha + 9);

        } else if (strncmp(
                       linha,
                       "PORTOES=",
                       8
                   ) == 0) {

            aeroporto->quantidade_portoes =
                atoi(linha + 8);
        }
    }

    fclose(fp);

    if (aeroporto->nome[0] == '\0' ||
        aeroporto->pistas <= 0 ||
        aeroporto->quantidade_taxiways <= 0 ||
        aeroporto->quantidade_portoes <= 0) {

        return 0;
    }

    return 1;
}

int criar_circuito(
    Aeroporto *aeroporto
)
{
    int i;

    if (aeroporto == NULL ||
        aeroporto->quantidade_taxiways <= 0) {

        return 0;
    }

    for (
        i = 1;
        i <= aeroporto->quantidade_taxiways;
        i++
    ) {

        if (!adicionar_no(
                aeroporto,
                i
            )) {

            return 0;
        }
    }

    return aeroporto->inicio != NULL &&
           aeroporto->fim != NULL;
}

int inicializar_portoes(
    Aeroporto *aeroporto
)
{
    int i;

    if (aeroporto == NULL ||
        aeroporto->quantidade_portoes <= 0) {

        return 0;
    }

    aeroporto->portoes =
        calloc(
            (size_t)aeroporto->quantidade_portoes,
            sizeof(Portao)
        );

    if (aeroporto->portoes == NULL) {
        return 0;
    }

    for (
        i = 0;
        i < aeroporto->quantidade_portoes;
        i++
    ) {

        aeroporto->portoes[i].id = i + 1;
        aeroporto->portoes[i].ocupado = 0;
        aeroporto->portoes[i].aviao = NULL;
    }

    return 1;
}

int inicializar_pistas(
    Aeroporto *aeroporto
)
{
    int i;

    if (aeroporto == NULL ||
        aeroporto->pistas <= 0) {

        return 0;
    }

    aeroporto->pistas_operacao =
        calloc(
            (size_t)aeroporto->pistas,
            sizeof(Pista)
        );

    if (aeroporto->pistas_operacao == NULL) {
        return 0;
    }

    for (
        i = 0;
        i < aeroporto->pistas;
        i++
    ) {

        aeroporto->pistas_operacao[i].id =
            i + 1;

        aeroporto->pistas_operacao[i].ocupada =
            0;

        aeroporto->pistas_operacao[i].aviao =
            NULL;

        aeroporto->pistas_operacao[i].liberacao =
            0;
    }

    return 1;
}

No *obter_no(
    Aeroporto *aeroporto,
    int id
)
{
    No *atual;

    if (aeroporto == NULL ||
        id <= 0) {

        return NULL;
    }

    atual = aeroporto->inicio;

    while (atual != NULL) {

        if (atual->id == id) {
            return atual;
        }

        atual = atual->proximo;
    }

    return NULL;
}

int taxiway_livre(
    Aeroporto *aeroporto,
    int id
)
{
    No *no =
        obter_no(
            aeroporto,
            id
        );

    return no != NULL &&
           !no->ocupado;
}

int ocupar_taxiway(
    Aeroporto *aeroporto,
    int id,
    Aviao *aviao
)
{
    No *no;

    if (aeroporto == NULL ||
        aviao == NULL) {

        return 0;
    }

    no =
        obter_no(
            aeroporto,
            id
        );

    if (no == NULL ||
        no->ocupado) {

        return 0;
    }

    no->ocupado = 1;
    no->aviao = aviao;

    return 1;
}

int liberar_taxiway(
    Aeroporto *aeroporto,
    int id
)
{
    No *no;

    if (aeroporto == NULL) {
        return 0;
    }

    no =
        obter_no(
            aeroporto,
            id
        );

    if (no == NULL) {
        return 0;
    }

    no->ocupado = 0;
    no->aviao = NULL;

    return 1;
}

int obter_portao_livre(
    Aeroporto *aeroporto
)
{
    int i;

    if (aeroporto == NULL ||
        aeroporto->portoes == NULL) {

        return -1;
    }

    for (
        i = 0;
        i < aeroporto->quantidade_portoes;
        i++
    ) {

        if (!aeroporto->portoes[i].ocupado) {
            return aeroporto->portoes[i].id;
        }
    }

    return -1;
}

int ocupar_portao(
    Aeroporto *aeroporto,
    int id,
    Aviao *aviao
)
{
    Portao *portao;

    if (aeroporto == NULL ||
        aeroporto->portoes == NULL ||
        aviao == NULL ||
        id <= 0 ||
        id > aeroporto->quantidade_portoes) {

        return 0;
    }

    portao =
        &aeroporto->portoes[id - 1];

    if (portao->ocupado) {
        return 0;
    }

    portao->ocupado = 1;
    portao->aviao = aviao;

    return 1;
}

int liberar_portao(
    Aeroporto *aeroporto,
    int id
)
{
    Portao *portao;

    if (aeroporto == NULL ||
        aeroporto->portoes == NULL ||
        id <= 0 ||
        id > aeroporto->quantidade_portoes) {

        return 0;
    }

    portao =
        &aeroporto->portoes[id - 1];

    portao->ocupado = 0;
    portao->aviao = NULL;

    return 1;
}

int obter_pista_livre(
    Aeroporto *aeroporto
)
{
    int i;

    if (aeroporto == NULL ||
        aeroporto->pistas_operacao == NULL) {

        return -1;
    }

    for (
        i = 0;
        i < aeroporto->pistas;
        i++
    ) {

        if (!aeroporto->pistas_operacao[i].ocupada) {
            return aeroporto->pistas_operacao[i].id;
        }
    }

    return -1;
}

int ocupar_pista(
    Aeroporto *aeroporto,
    int id,
    Aviao *aviao,
    time_t liberacao
)
{
    Pista *pista;

    if (aeroporto == NULL ||
        aeroporto->pistas_operacao == NULL ||
        aviao == NULL ||
        id <= 0 ||
        id > aeroporto->pistas) {

        return 0;
    }

    pista =
        &aeroporto->pistas_operacao[id - 1];

    if (pista->ocupada) {
        return 0;
    }

    pista->ocupada = 1;
    pista->aviao = aviao;
    pista->liberacao = liberacao;

    return 1;
}

void atualizar_pistas(
    Aeroporto *aeroporto,
    time_t agora
)
{
    int i;

    if (aeroporto == NULL ||
        aeroporto->pistas_operacao == NULL) {

        return;
    }

    for (
        i = 0;
        i < aeroporto->pistas;
        i++
    ) {

        if (
            aeroporto->pistas_operacao[i].ocupada &&
            aeroporto->pistas_operacao[i].liberacao <= agora
        ) {

            aeroporto->pistas_operacao[i].ocupada = 0;
            aeroporto->pistas_operacao[i].aviao = NULL;
            aeroporto->pistas_operacao[i].liberacao = 0;
        }
    }
}

void imprimir_aeroporto(
    const Aeroporto *aeroporto
)
{
    if (aeroporto == NULL) {
        return;
    }

    printf(
        "\nAeroporto: %s\n",
        aeroporto->nome
    );

    printf(
        "Pistas: %d\n",
        aeroporto->pistas
    );

    printf(
        "Taxiways: %d\n",
        aeroporto->quantidade_taxiways
    );

    printf(
        "Portoes: %d\n",
        aeroporto->quantidade_portoes
    );
}

void imprimir_circuito(
    const Aeroporto *aeroporto
)
{
    const No *atual;

    if (aeroporto == NULL) {
        return;
    }

    printf(
        "Circuito de taxiways: "
    );

    atual = aeroporto->inicio;

    while (atual != NULL) {

        printf(
            "N%d",
            atual->id
        );

        if (atual->proximo != NULL) {
            printf(" <-> ");
        }

        atual = atual->proximo;
    }

    printf("\n");
}

void destruir_aeroporto(
    Aeroporto *aeroporto
)
{
    No *atual;
    No *proximo;

    if (aeroporto == NULL) {
        return;
    }

    atual = aeroporto->inicio;

    while (atual != NULL) {

        proximo = atual->proximo;

        atual->aviao = NULL;

        free(atual);

        atual = proximo;
    }

    aeroporto->inicio = NULL;
    aeroporto->fim = NULL;

    free(aeroporto->portoes);
    aeroporto->portoes = NULL;

    free(aeroporto->pistas_operacao);
    aeroporto->pistas_operacao = NULL;
}
