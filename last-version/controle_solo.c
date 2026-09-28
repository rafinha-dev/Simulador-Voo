/* Modulo controle_solo.c: circulacao, pistas e portoes. */
#include "controle_solo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Zera o aeroporto antes de carregar configuracoes. */
void inicializar_aeroporto(Aeroporto *a) {
    if (a) {
        memset(a, 0, sizeof(*a));
        a->sentido_ativo = -1;
    }
}
/* Le as quantidades e o nome do aeroporto do arquivo .cfg. */
int carregar_configuracao(Aeroporto *a, const char *arquivo) {
    FILE *f;
    char linha[128];
    int p = 0, t = 0, g = 0;
    char nome[50] = "";
    if (!a || !arquivo) {
        return 0;
    }
    f = fopen(arquivo, "r");
    if (!f) {
        return 0;
    }
    while (fgets(linha, sizeof(linha), f)) {
        if (sscanf(linha, "NOME=%49[^\r\n]", nome) == 1) {
            continue;
        }
        if (sscanf(linha, "PISTAS=%d", &p) == 1) {
            continue;
        }
        if (sscanf(linha, "TAXIWAYS=%d", &t) == 1) {
            continue;
        }
        (void) sscanf(linha, "PORTOES=%d", &g);
    }
    fclose(f);
    if (!nome[0] || p <= 0 || t <= 0 || g <= 0 || p > 100 || t > 1000 || g > 1000) {
        return 0;
    }
    snprintf(a->nome, sizeof(a->nome), "%s", nome);
    a->quantidade_pistas = p;
    a->quantidade_taxiways = t;
    a->quantidade_portoes = g;
    return 1;
}
/* Aloca pistas, portoes e encadeia N1 ate Nn. */
int criar_circuito(Aeroporto *a) {
    int i;
    No *n;
    No *anterior = NULL;
    if (!a || a->inicio || a->quantidade_taxiways <= 0) {
        return 0;
    }
    a->portoes = calloc((size_t) a->quantidade_portoes, sizeof(* a->portoes));
    a->pistas = calloc((size_t) a->quantidade_pistas, sizeof(* a->pistas));
    if (!a->portoes || !a->pistas) {
        destruir_aeroporto(a);
        return 0;
    }
    for (i = 0; i < a->quantidade_portoes; i++) a->portoes[i].id = i + 1;
    for (i = 0; i < a->quantidade_pistas; i++) a->pistas[i].id = i + 1;
    for (i = 1; i <= a->quantidade_taxiways; i++) {
        n = calloc(1, sizeof(*n));
        if (!n) {
            destruir_aeroporto(a);
            return 0;
        }
        n->id = i;
        n->anterior = anterior;
        if (anterior) {
            anterior->proximo = n;
        }
        else a->inicio = n;
        anterior = n;
    }
    a->fim = anterior;
    return 1;
}
/* Percorre a lista ate encontrar o no solicitado. */
No *obter_no(Aeroporto *a, int id) {
    No *n;
    if (!a || id < 1) {
        return NULL;
    }
    for (n = a->inicio; n; n = n->proximo) {
        if (n->id == id) {
            return n;
        }
    }
    return NULL;
}
/* Confere se a entrada apropriada esta vazia e no sentido permitido. */
int entrada_taxiway_livre(Aeroporto *a, const Aviao *aviao) {
    No *n;
    if (!a || !aviao) {
        return 0;
    }
    if (a->sentido_ativo != -1 && a->sentido_ativo != (int) aviao->sentido) {
        return 0;
    }
    n = aviao->sentido == HORARIO ? a->inicio : a->fim;
    return n && n->aviao == NULL;
}
/* Procura um portao sem aeronave. */
int obter_portao_livre(const Aeroporto *a) {
    int i;
    if (!a || !a->portoes) {
        return -1;
    }
    for (i = 0; i < a->quantidade_portoes; i++) {
        if (!a->portoes[i].aviao) {
            return i + 1;
        }
    }
    return -1;
}
/* Compara portoes livres com reservas de aeronaves a caminho. */
int solo_completamente_congestionado(const Aeroporto *a) {
    int i, livres = 0;
    if (!a || !a->portoes) {
        return 1;
    }
    for (i = 0; i < a->quantidade_portoes; i++) if (!a->portoes[i].aviao) livres++;
    return livres <= a->reservas_portao;
}
/* Procura uma pista sem aeronave. */
int obter_pista_livre(const Aeroporto *a) {
    int i;
    if (!a || !a->pistas) {
        return -1;
    }
    for (i = 0; i < a->quantidade_pistas; i++) {
        if (!a->pistas[i].aviao) {
            return i + 1;
        }
    }
    return -1;
}
/* Exige pista, capacidade de portao e entrada de taxiway livres. */
int autorizar_pouso(Aeroporto *a, const Aviao *aviao, int *pista) {
    int p;
    if (!a || !aviao || !pista) {
        return 0;
    }
    p = obter_pista_livre(a);
    if (p < 0 || solo_completamente_congestionado(a) || !entrada_taxiway_livre(a, aviao)) {
        return 0;
    }
    *pista = p;
    return 1;
}
/* Libera o sentido ativo quando nao resta aeronave na lista. */
static void atualizar_sentido(Aeroporto *a) {
    No *n;
    if (!a) {
        return;
    }
    for (n = a->inicio; n; n = n->proximo) {
        if (n->aviao) {
            return;
        }
    }
    a->sentido_ativo = -1;
}
/* Ocupa um no e fixa o sentido de circulacao. */
int ocupar_no(Aeroporto *a, int id, Aviao *aviao) {
    No *n = obter_no(a, id);
    if (!n || !aviao || n->aviao) {
        return 0;
    }
    if (a->sentido_ativo != -1 && a->sentido_ativo != (int) aviao->sentido) {
        return 0;
    }
    n->aviao = aviao;
    a->sentido_ativo = (int) aviao->sentido;
    return 1;
}
/* Valida ocupante, adjacencia, destino livre e sentido antes de mover. */
int mover_no(Aeroporto *a, int origem, int destino, Aviao *aviao) {
    No *o = obter_no(a, origem), *d = obter_no(a, destino);
    if (!o || !d || !aviao || o->aviao != aviao || d->aviao) {
        return 0;
    }
    if (o->proximo != d && o->anterior != d) {
        return 0;
    }
    if (aviao->sentido == HORARIO && o->proximo != d) {
        return 0;
    }
    if (aviao->sentido == ANTI_HORARIO && o->anterior != d) {
        return 0;
    }
    d->aviao = aviao;
    o->aviao = NULL;
    return 1;
}
/* Retira a aeronave do no e reavalia o sentido ativo. */
int liberar_no(Aeroporto *a, int id, Aviao *aviao) {
    No *n = obter_no(a, id);
    if (!n || !aviao || n->aviao != aviao) {
        return 0;
    }
    n->aviao = NULL;
    atualizar_sentido(a);
    return 1;
}
/* Estaciona a aeronave em um portao livre. */
int ocupar_portao(Aeroporto *a, int id, Aviao *aviao) {
    if (!a || !aviao || id < 1 || id > a->quantidade_portoes || a->portoes[id - 1].aviao) {
        return 0;
    }
    a->portoes[id - 1].aviao = aviao;
    return 1;
}
/* Libera somente o portao ocupado pela aeronave indicada. */
int liberar_portao(Aeroporto *a, int id, Aviao *aviao) {
    if (!a || !aviao || id < 1 || id > a->quantidade_portoes || a->portoes[id - 1].aviao != aviao) {
        return 0;
    }
    a->portoes[id - 1].aviao = NULL;
    return 1;
}
/* Registra a ocupacao e o prazo de liberacao da pista. */
int ocupar_pista(Aeroporto *a, int id, Aviao *aviao, time_t ate) {
    if (!a || !aviao || id < 1 || id > a->quantidade_pistas || a->pistas[id - 1].aviao) {
        return 0;
    }
    a->pistas[id - 1].aviao = aviao;
    a->pistas[id - 1].liberacao = ate;
    return 1;
}
/* Libera pistas cujo evento terminou. */
void atualizar_pistas(Aeroporto *a, time_t agora) {
    int i;
    if (!a || !a->pistas) {
        return;
    }
    for (i = 0; i < a->quantidade_pistas; i++) {
        Pista *p = &a->pistas[i];
        if (p->aviao && p->liberacao <= agora) {
            p->aviao = NULL;
            p->liberacao = 0;
        }
    }
}
/* Libera nos da lista e vetores do aeroporto. */
void destruir_aeroporto(Aeroporto *a) {
    No *n, *proximo;
    if (!a) {
        return;
    }
    for (n = a->inicio; n; n = proximo) {
        proximo = n->proximo;
        free(n);
    }
    free(a->portoes);
    free(a->pistas);
    a->inicio = a->fim = NULL;
    a->portoes = NULL;
    a->pistas = NULL;
    a->sentido_ativo = -1;
    a->reservas_portao = 0;
}
