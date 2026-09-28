#ifndef DESPACHO_PORAO_H
#define DESPACHO_PORAO_H

#define CAPACIDADE_MANIFESTO 500
#define CAPACIDADE_PORAO 500
#define TAMANHO_DONO 80
#define TAMANHO_IDENTIFICADOR 30

typedef enum {
    ITEM_CADASTRADO,
    ITEM_EMBARCADO,
    ITEM_DESCARREGADO,
    ITEM_ISOLADO
} EstadoItem;

typedef struct {
    char identificador[TAMANHO_IDENTIFICADOR];
    char dono[TAMANHO_DONO];
    float peso;
    EstadoItem estado;
} ItemManifesto;

typedef struct {
    ItemManifesto itens[CAPACIDADE_MANIFESTO];
    int quantidade;
} Manifesto;

typedef struct {
    ItemManifesto itens[CAPACIDADE_PORAO];
    int quantidade;
} Porao;

void inicializar_manifesto(Manifesto *manifesto);
void inicializar_porao(Porao *porao);

int cadastrar_item(
    Manifesto *manifesto,
    const char *identificador,
    const char *dono,
    float peso
);

int consultar_item(
    const Manifesto *manifesto,
    const char *identificador
);

int remover_item_administrativamente(
    Manifesto *manifesto,
    const char *identificador
);

int embarcar_item(
    Manifesto *manifesto,
    Porao *porao,
    const char *identificador
);

int descarregar_proximo_item(
    Manifesto *manifesto,
    Porao *porao
);

int inspecionar_item(
    Manifesto *manifesto,
    Porao *porao,
    const char *identificador
);

void imprimir_manifesto(const Manifesto *manifesto);
void imprimir_porao(const Porao *porao);

#endif
