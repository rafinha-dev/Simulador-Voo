#ifndef DESPACHO_PORAO_H
#define DESPACHO_PORAO_H
#define MAX_ITENS 32 // Limite didatico de itens por voo.

// Estado de cada bagagem ou conteiner no manifesto.
typedef enum {
    CADASTRADO, EMBARCADO, DESCARREGADO, ISOLADO
} EstadoItem;
// Registro individual usado na base digital e na pilha fisica.
typedef struct {
    char codigo[32];
    // Identificador unico do item.
    char dono[64];
    // Passageiro ou responsavel.
    double peso;
    // Peso declarado em quilogramas.
    EstadoItem estado;
    // Situacao registrada do item.
} Item;
/*
 * Manifesto: cadastro digital consultavel por codigo.
 * Porao: pilha LIFO; o ultimo elemento e o primeiro acessivel.
 */
typedef struct {
    Item itens[MAX_ITENS];
    int quantidade;
} Manifesto;
typedef struct {
    Item itens[MAX_ITENS];
    int quantidade;
} Porao;

// Prepara cadastro e pilha vazios.
void inicializar_carga(Manifesto *m, Porao *p);
// Inclui um item valido e ainda nao embarcado no manifesto.
int cadastrar_item(Manifesto *m, const char *codigo, const char *dono, double peso);
// Consulta um registro por codigo; NULL quando ausente.
const Item *consultar_item(const Manifesto *m, const char *codigo);
// Remove registro somente antes do embarque.
int remover_cadastro(Manifesto *m, const char *codigo);
// Coloca no topo da pilha um item previamente cadastrado.
int embarcar_item(Manifesto *m, Porao *p, const char *codigo);
/*
 * Retira o topo e compara codigo, dono, peso e estado com o manifesto.
 * Retorno: 1 conforme; -1 inconforme; 0 porao vazio ou argumento invalido.
 */
int descarregar_item(Manifesto *m, Porao *p, Item *retirado);
// Isola um item alvo e restaura os itens acima na ordem original.
int inspecionar_item(Manifesto *m, Porao *p, const char *codigo);
#endif
