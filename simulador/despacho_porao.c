#include "despacho_porao.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

static int buscar_item(
    const Manifesto *manifesto,
    const char *identificador
)
{
    int i;

    if (manifesto == NULL || identificador == NULL) {
        return -1;
    }

    for (i = 0; i < manifesto->quantidade; i++) {
        if (strcmp(
                manifesto->itens[i].identificador,
                identificador
            ) == 0) {
            return i;
        }
    }

    return -1;
}

void inicializar_manifesto(Manifesto *manifesto)
{
    if (manifesto == NULL) {
        return;
    }

    manifesto->quantidade = 0;
}

void inicializar_porao(Porao *porao)
{
    if (porao == NULL) {
        return;
    }

    porao->quantidade = 0;
}

int cadastrar_item(
    Manifesto *manifesto,
    const char *identificador,
    const char *dono,
    float peso
)
{
    ItemManifesto *item;

    if (manifesto == NULL ||
        identificador == NULL ||
        dono == NULL ||
        identificador[0] == '\0' ||
        dono[0] == '\0' ||
        !isfinite(peso) ||
        peso <= 0.0f) {
        return 0;
    }

    if (manifesto->quantidade >= CAPACIDADE_MANIFESTO) {
        return 0;
    }

    if (strlen(identificador) >= TAMANHO_IDENTIFICADOR ||
        strlen(dono) >= TAMANHO_DONO) {
        return 0;
    }

    if (buscar_item(manifesto, identificador) != -1) {
        return 0;
    }

    item = &manifesto->itens[manifesto->quantidade];

    strcpy(item->identificador, identificador);
    strcpy(item->dono, dono);
    item->peso = peso;
    item->estado = ITEM_CADASTRADO;

    manifesto->quantidade++;

    printf(
        "CADASTRO: item %s registrado para %s, peso %.2f kg.\n",
        item->identificador,
        item->dono,
        item->peso
    );

    return 1;
}

int consultar_item(
    const Manifesto *manifesto,
    const char *identificador
)
{
    int indice;
    const ItemManifesto *item;

    if (manifesto == NULL || identificador == NULL) {
        return 0;
    }

    indice = buscar_item(manifesto, identificador);

    if (indice == -1) {
        printf(
            "CONSULTA: item %s nao encontrado no manifesto.\n",
            identificador
        );
        return 0;
    }

    item = &manifesto->itens[indice];

    printf("Identificador: %s\n", item->identificador);
    printf("Dono: %s\n", item->dono);
    printf("Peso: %.2f kg\n", item->peso);

    switch (item->estado) {
        case ITEM_CADASTRADO:
            printf("Estado: cadastrado, aguardando embarque.\n");
            break;

        case ITEM_EMBARCADO:
            printf("Estado: embarcado no porao.\n");
            break;

        case ITEM_DESCARREGADO:
            printf("Estado: descarregado.\n");
            break;

        case ITEM_ISOLADO:
            printf("Estado: isolado para inspecao.\n");
            break;
    }

    return 1;
}

int remover_item_administrativamente(
    Manifesto *manifesto,
    const char *identificador
)
{
    int indice;
    int i;

    if (manifesto == NULL || identificador == NULL) {
        return 0;
    }

    indice = buscar_item(manifesto, identificador);

    if (indice == -1) {
        printf("REMOCAO: item nao encontrado.\n");
        return 0;
    }

    if (manifesto->itens[indice].estado != ITEM_CADASTRADO) {
        printf(
            "REMOCAO NEGADA: item %s nao esta apenas cadastrado.\n",
            identificador
        );
        return 0;
    }

    for (i = indice; i < manifesto->quantidade - 1; i++) {
        manifesto->itens[i] = manifesto->itens[i + 1];
    }

    manifesto->quantidade--;

    printf(
        "ADMINISTRACAO: item %s removido do manifesto.\n",
        identificador
    );

    return 1;
}

int embarcar_item(
    Manifesto *manifesto,
    Porao *porao,
    const char *identificador
)
{
    int indice;
    ItemManifesto item;

    if (manifesto == NULL ||
        porao == NULL ||
        identificador == NULL) {
        return 0;
    }

    indice = buscar_item(manifesto, identificador);

    if (indice == -1) {
        printf(
            "ALERTA: item %s nao consta no manifesto.\n",
            identificador
        );
        return 0;
    }

    if (manifesto->itens[indice].estado != ITEM_CADASTRADO) {
        printf(
            "EMBARQUE NEGADO: item %s nao esta disponivel.\n",
            identificador
        );
        return 0;
    }

    if (porao->quantidade >= CAPACIDADE_PORAO) {
        printf("ERRO: capacidade do porao atingida.\n");
        return 0;
    }

    item = manifesto->itens[indice];
    item.estado = ITEM_EMBARCADO;

    porao->itens[porao->quantidade] = item;
    porao->quantidade++;

    manifesto->itens[indice].estado = ITEM_EMBARCADO;

    printf("EMBARQUE: item %s colocado no porao.\n", identificador);

    return 1;
}

int descarregar_proximo_item(
    Manifesto *manifesto,
    Porao *porao
)
{
    ItemManifesto fisico;
    int indice;
    int divergencia = 0;

    if (manifesto == NULL || porao == NULL) {
        return 0;
    }

    if (porao->quantidade == 0) {
        printf("DESCARREGAMENTO: porao vazio.\n");
        return 0;
    }

    /* Acesso somente ao item que esta no topo da pilha. */
    fisico = porao->itens[porao->quantidade - 1];
    porao->quantidade--;

    indice = buscar_item(manifesto, fisico.identificador);

    if (indice == -1) {
        printf(
            "ALERTA DE INCONFORMIDADE: item fisico %s "
            "nao consta no manifesto.\n",
            fisico.identificador
        );

        return 1;
    }

    if (strcmp(
            manifesto->itens[indice].dono,
            fisico.dono
        ) != 0 ||
        fabsf(manifesto->itens[indice].peso - fisico.peso) > 0.01f) {
        divergencia = 1;
    }

    if (divergencia) {
        printf(
            "ALERTA DE INCONFORMIDADE: divergencia no item %s.\n",
            fisico.identificador
        );

        printf(
            "Manifesto: dono=%s, peso=%.2f kg.\n",
            manifesto->itens[indice].dono,
            manifesto->itens[indice].peso
        );

        printf(
            "Fisico: dono=%s, peso=%.2f kg.\n",
            fisico.dono,
            fisico.peso
        );

        return 1;
    }

    if (manifesto->itens[indice].estado != ITEM_EMBARCADO) {
        printf(
            "ALERTA DE INCONFORMIDADE: estado inesperado para %s.\n",
            fisico.identificador
        );

        return 1;
    }

    manifesto->itens[indice].estado = ITEM_DESCARREGADO;

    printf(
        "DOUBLE-CHECK OK: item %s, dono %s, peso %.2f kg.\n",
        fisico.identificador,
        fisico.dono,
        fisico.peso
    );

    return 1;
}

int inspecionar_item(
    Manifesto *manifesto,
    Porao *porao,
    const char *identificador
)
{
    ItemManifesto temporarios[CAPACIDADE_PORAO];
    ItemManifesto alvo;
    int quantidade_temporaria = 0;
    int encontrou = 0;
    int indice;
    int i;

    if (manifesto == NULL ||
        porao == NULL ||
        identificador == NULL) {
        return 0;
    }

    indice = buscar_item(manifesto, identificador);

    if (indice == -1) {
        printf(
            "INSPECAO NEGADA: %s nao consta no manifesto.\n",
            identificador
        );
        return 0;
    }

    if (manifesto->itens[indice].estado != ITEM_EMBARCADO) {
        printf(
            "INSPECAO NEGADA: item %s nao esta embarcado.\n",
            identificador
        );
        return 0;
    }

    /*
     * Retira temporariamente os itens que estao acima do alvo.
     * O alvo so pode ser alcançado pelo topo da pilha.
     */
    while (porao->quantidade > 0) {
        ItemManifesto atual =
            porao->itens[porao->quantidade - 1];

        porao->quantidade--;

        if (strcmp(atual.identificador, identificador) == 0) {
            alvo = atual;
            encontrou = 1;
            break;
        }

        temporarios[quantidade_temporaria] = atual;
        quantidade_temporaria++;
    }

    if (!encontrou) {
        /* Restaura a pilha se o alvo nao estiver fisicamente presente. */
        for (i = quantidade_temporaria - 1; i >= 0; i--) {
            porao->itens[porao->quantidade] = temporarios[i];
            porao->quantidade++;
        }

        printf(
            "INSPECAO NEGADA: item %s nao encontrado no porao.\n",
            identificador
        );

        return 0;
    }

    /*
     * Isola o alvo no manifesto.
     * O item suspeito nao volta para a pilha fisica.
     */
    manifesto->itens[indice].estado = ITEM_ISOLADO;

    printf(
        "SEGURANCA: item %s isolado para inspecao.\n",
        alvo.identificador
    );

    /*
     * Restaura os demais itens na ordem original.
     */
    for (i = quantidade_temporaria - 1; i >= 0; i--) {
        porao->itens[porao->quantidade] = temporarios[i];
        porao->quantidade++;
    }

    printf(
        "INSPECAO: %d item(ns) temporariamente removido(s) "
        "e restaurado(s) na ordem original.\n",
        quantidade_temporaria
    );

    return 1;
}

void imprimir_manifesto(const Manifesto *manifesto)
{
    int i;

    if (manifesto == NULL) {
        return;
    }

    printf("\n=== MANIFESTO DIGITAL ===\n");

    for (i = 0; i < manifesto->quantidade; i++) {
        printf(
            "%s | Dono: %s | Peso: %.2f kg | Estado: %d\n",
            manifesto->itens[i].identificador,
            manifesto->itens[i].dono,
            manifesto->itens[i].peso,
            manifesto->itens[i].estado
        );
    }
}

void imprimir_porao(const Porao *porao)
{
    int i;

    if (porao == NULL) {
        return;
    }

    printf("\n=== PORAO: BASE PARA TOPO ===\n");

    for (i = 0; i < porao->quantidade; i++) {
        printf(
            "%s | Dono: %s | Peso: %.2f kg\n",
            porao->itens[i].identificador,
            porao->itens[i].dono,
            porao->itens[i].peso
        );
    }
}
