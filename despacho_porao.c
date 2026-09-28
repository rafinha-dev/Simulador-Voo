/* Modulo despacho_porao.c: manifesto digital e pilha do porao. */
#include "despacho_porao.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
/* Busca um item no manifesto pelo codigo. */
static int indice(const Manifesto *m, const char *c) {
    int i;
    if (!m || !c) {
        return -1;
    }
    for (i = 0; i < m->quantidade; i++) {
        if (strcmp(m->itens[i].codigo, c) == 0) {
            return i;
        }
    }
    return -1;
}
/* Inicia o manifesto e a pilha vazios. */
void inicializar_carga(Manifesto *m, Porao *p) {
    if (m) {
        m->quantidade = 0;
    }
    if (p) {
        p->quantidade = 0;
    }
}
/* Valida e adiciona um registro digital ao manifesto. */
int cadastrar_item(Manifesto *m, const char *c, const char *d, double peso) {
    Item *x;
    if (!m || !c || !d || !c[0] || !d[0] || strlen(c) >= 32 || strlen(d) >= 64 || !isfinite(peso) || peso <= 0 || m->quantidade >= MAX_ITENS || indice(m, c) >= 0) {
        return 0;
    }
    x = &m->itens[m->quantidade++];
    snprintf(x->codigo, sizeof(x->codigo), "%s", c);
    snprintf(x->dono, sizeof(x->dono), "%s", d);
    x->peso = peso;
    x->estado = CADASTRADO;
    return 1;
}
/* Busca o item sem alterar o cadastro. */
const Item *consultar_item(const Manifesto *m, const char *c) {
    int i = indice(m, c);
    return i < 0 ? NULL :&m->itens[i];
}
/* Remove um item que ainda nao foi embarcado. */
int remover_cadastro(Manifesto *m, const char *c) {
    int i = indice(m, c), j;
    if (i < 0 || m->itens[i].estado != CADASTRADO) {
        return 0;
    }
    for (j = i; j < m->quantidade -1; j++) m->itens[j] = m->itens[j + 1];
    m->quantidade--;
    return 1;
}
/* Copia o item cadastrado para o topo da pilha LIFO. */
int embarcar_item(Manifesto *m, Porao *p, const char *c) {
    int i = indice(m, c);
    if (i < 0 || !p || p->quantidade >= MAX_ITENS || m->itens[i].estado != CADASTRADO) {
        return 0;
    }
    m->itens[i].estado = EMBARCADO;
    p->itens[p->quantidade++] = m->itens[i];
    return 1;
}
/* Retira o topo e confere seus dados com o manifesto. */
int descarregar_item(Manifesto *m, Porao *p, Item *retirado) {
    int i;
    Item x;
    if (!m || !p || !retirado || !p->quantidade) {
        return 0;
    }
    x = p->itens[--p->quantidade];
    *retirado = x;
    i = indice(m, x.codigo);
    if (i < 0) {
        return -1;
    }
    if (strcmp(m->itens[i].dono, x.dono) != 0 || fabs(m->itens[i].peso - x.peso) > 0.01 || m->itens[i].estado != EMBARCADO) {
        m->itens[i].estado = ISOLADO;
        return -1;
    }
    m->itens[i].estado = DESCARREGADO;
    return 1;
}
/* Retira temporariamente os itens acima do alvo, isola e restaura. */
int inspecionar_item(Manifesto *m, Porao *p, const char *codigo) {
    Item temp[MAX_ITENS], x;
    int n = 0, i, j, achou = 0;
    if (!m || !p || !codigo) {
        return 0;
    }
    i = indice(m, codigo);
    if (i < 0 || m->itens[i].estado != EMBARCADO) {
        return 0;
    }
    while (p->quantidade) {
        x = p->itens[--p->quantidade];
        if (strcmp(x.codigo, codigo) == 0) {
            achou = 1;
            break;
        }
        temp[n++] = x;
    }
    for (j = n -1; j >= 0; j--) p->itens[p->quantidade++] = temp[j];
    if (!achou) {
        return 0;
    }
    m->itens[i].estado = ISOLADO;
    return 1;
}
