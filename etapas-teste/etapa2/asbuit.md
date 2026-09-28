A lista duplamente encadeada tem uma vantagem interessante

Ela permite representar os dois sentidos:

[N1] ⇄ [N2] ⇄ [N3] ⇄ [N4]

Se a aeronave estiver no N3:

horário:
N3 → N4

anti-horário:
N3 → N2

Então o controlador pode decidir:

```

if (sentido == HORARIO)
    proximo = no->proximo;
else
    proximo = no->anterior;
```

## Os portões 

                 AEROPORTO
                     │
       ┌─────────────┼─────────────┐
       ↓             ↓             ↓
    Pistas        Taxiways       Portões
                     │
                     ↓
                   Lista
                     │
       [N1] ⇄ [N2] ⇄ [N3] ⇄ [N4]


## Estrutura 

Etapa 1 — Configuração

main()
  ↓
escolher aeroporto
  ↓
carregar arquivo .cfg
  ↓
criar infraestrutura

Etapa 2 — Estrutura

Lista duplamente encadeada
        ↓
       Nós
        ↓
     Taxiways

Etapa 3 — Aeronaves

Aeronave
   ↓
posição atual
   ↓
destino
   ↓
sentido

Etapa 4 — Regras

horário → comportamento padrão

anti-horário → somente se:
                ├─ portão estiver logo atrás
                └─ caminho estiver livre

Etapa 5 — Conflitos

duas aeronaves
      ↓
mesmo trecho
      ↓
sentidos opostos
      ↓
detectar impasse
      ↓
reorganizar circulação
Etapa 1 — Configuração

main()
  ↓
escolher aeroporto
  ↓
carregar arquivo .cfg
  ↓
criar infraestrutura

Etapa 2 — Estrutura

Lista duplamente encadeada
        ↓
       Nós
        ↓
     Taxiways

Etapa 3 — Aeronaves

Aeronave
   ↓
posição atual
   ↓
destino
   ↓
sentido

Etapa 4 — Regras

horário → comportamento padrão

anti-horário → somente se:
                ├─ portão estiver logo atrás
                └─ caminho estiver livre

Etapa 5 — Conflitos

duas aeronaves
      ↓
mesmo trecho
      ↓
sentidos opostos
      ↓
detectar impasse
      ↓
reorganizar circulação
