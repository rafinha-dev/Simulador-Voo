#ifndef CONTROLE_SOLO_H
#define CONTROLE_SOLO_H

#include "controle_aereo.h"

typedef struct No {

    int id;
    int ocupado;

    Aviao *aviao;

    struct No *proximo;
    struct No *anterior;

} No;

typedef struct {

    int id;
    int ocupado;

    Aviao *aviao;

} Portao;

typedef struct {

    int id;
    int ocupada;

    Aviao *aviao;

    time_t liberacao;

} Pista;

typedef struct {

    char nome[50];

    int pistas;
    int quantidade_taxiways;
    int quantidade_portoes;

    No *inicio;
    No *fim;

    Portao *portoes;
    Pista *pistas_operacao;

} Aeroporto;

void inicializar_aeroporto(
    Aeroporto *aeroporto
);

int carregar_configuracao(
    Aeroporto *aeroporto,
    const char *arquivo
);

int criar_circuito(
    Aeroporto *aeroporto
);

int inicializar_portoes(
    Aeroporto *aeroporto
);

int inicializar_pistas(
    Aeroporto *aeroporto
);

No *obter_no(
    Aeroporto *aeroporto,
    int id
);

int taxiway_livre(
    Aeroporto *aeroporto,
    int id
);

int ocupar_taxiway(
    Aeroporto *aeroporto,
    int id,
    Aviao *aviao
);

int liberar_taxiway(
    Aeroporto *aeroporto,
    int id
);

int obter_portao_livre(
    Aeroporto *aeroporto
);

int ocupar_portao(
    Aeroporto *aeroporto,
    int id,
    Aviao *aviao
);

int liberar_portao(
    Aeroporto *aeroporto,
    int id
);

int obter_pista_livre(
    Aeroporto *aeroporto
);

int ocupar_pista(
    Aeroporto *aeroporto,
    int id,
    Aviao *aviao,
    time_t liberacao
);

void atualizar_pistas(
    Aeroporto *aeroporto,
    time_t agora
);

void imprimir_aeroporto(
    const Aeroporto *aeroporto
);

void imprimir_circuito(
    const Aeroporto *aeroporto
);

void destruir_aeroporto(
    Aeroporto *aeroporto
);

#endif
