/* Modulo controle_aereo.c: fila de prioridade e aeronaves. */
#include "controle_aereo.h"
#include <stdlib.h>
#include <stdio.h>
/* Compara prioridade e, em empate, a ordem de chegada. */
static int antes(const Aviao *a, const Aviao *b) {
    if (a->prioridade != b->prioridade) {
        return a->prioridade < b->prioridade;
    }
    return a->ordem_chegada < b->ordem_chegada;
}
/* Troca dois ponteiros sem copiar as estruturas Aviao. */
static void troca(Aviao **a, Aviao **b) {
    Aviao *t = *a;
    *a = *b;
    *b = t;
}
/* Aloca o vetor que armazena o heap de pouso. */
int inicializar_fila(FilaPrioridade *f, int capacidade) {
    if (!f || capacidade <= 0) {
        return 0;
    }
    f->dados = calloc((size_t) capacidade, sizeof(* f->dados));
    f->tamanho = 0;
    f->capacidade = f->dados ? capacidade : 0;
    return f->dados != NULL;
}
/* Insere e sobe no heap ate respeitar a prioridade. */
int inserir_aviao(FilaPrioridade *f, Aviao *a) {
    int i, p;
    if (!f || !f->dados || !a || f->tamanho == f->capacidade) {
        return 0;
    }
    calcular_prioridade(a);
    i = f->tamanho++;
    f->dados[i] = a;
    while (i > 0) {
        p = (i -1) / 2;
        if (!antes(f->dados[i], f->dados[p])) {
            break;
        }
        troca( &f->dados[i], &f->dados[p]);
        i = p;
    }
    return 1;
}
/* Retorna o topo do heap sem remover. */
Aviao *consultar_proximo_aviao(const FilaPrioridade *f) {
    return f && f->tamanho ? f->dados[0] : NULL;
}
/* Remove o topo e desce o novo primeiro elemento no heap. */
Aviao *remover_proximo_aviao(FilaPrioridade *f) {
    int i = 0, e, d, m;
    Aviao *r;
    if (!f || !f->tamanho) {
        return NULL;
    }
    r = f->dados[0];
    f->tamanho--;
    f->dados[0] = f->dados[f->tamanho];
    f->dados[f->tamanho] = NULL;
    while (1) {
        e = 2 * i + 1;
        d = e + 1;
        m = i;
        if (e < f->tamanho && antes(f->dados[e], f->dados[m])) {
            m = e;
        }
        if (d < f->tamanho && antes(f->dados[d], f->dados[m])) {
            m = d;
        }
        if (m == i) {
            break;
        }
        troca( &f->dados[i], &f->dados[m]);
        i = m;
    }
    return r;
}
/* Verifica se ha aeronaves aguardando. */
int fila_vazia(const FilaPrioridade *f) {
    return !f || f->tamanho == 0;
}
/* Aplica as tres regras de prioridade do enunciado. */
int calcular_prioridade(Aviao *a) {
    if (!a) {
        return 0;
    }
    a->prioridade = (a->combustivel < 10 || a->emergencia_medica) ? 1 : (a->tipo == VOO_COMERCIAL ? 2 : 3);
    return a->prioridade;
}
/* Gera um voo ficticio com combustivel, tipo e emergencia. */
Aviao gerar_aviao(unsigned long ordem) {
    Aviao a = {
        0
    };
    int r = rand() % 100;
    snprintf(a.identificacao, sizeof(a.identificacao), "VOO%04lu", ordem);
    a.combustivel = 5 + rand() % 96;
    a.emergencia_medica = rand() % 10 == 0;
    a.tipo = r < 60 ? VOO_COMERCIAL : (r < 85 ? VOO_CARGA : VOO_PRIVADO);
    a.ordem_chegada = ordem;
    a.sentido = rand() % 2 ? HORARIO : ANTI_HORARIO;
    a.estado = AGUARDANDO_POUSO;
    a.posicao_taxiway = a.portao = a.pista = -1;
    calcular_prioridade( &a);
    return a;
}
/* Libera o vetor, nao os objetos Aviao. */
void destruir_fila(FilaPrioridade *f) {
    if (!f) {
        return;
    }
    free(f->dados);
    f->dados = NULL;
    f->tamanho = f->capacidade = 0;
}
