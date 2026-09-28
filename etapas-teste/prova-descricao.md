# Sistema de Controle de Tráfego Aéreo e Pistas de Pouso

Você foi contratado pelo DCEA (Departamento de Controle do Espaçoo aereo) para implementar o protótipo de um Controlador Aéreo Nacional.Um protótipo deve ser proposto com aplicaçãoo em um aeroporto de
Minas Gerais, mas baseado na eficiẽncia do sistema, haveria um potencial de implementaçãoo tambémm em outros aeroportos nacionais.

Neste sistema considera-se o gerenciamento de 

i) tráfego aéreo, 
ii) manobra em pista de táxi e 
iii) despacho de bagagens.

## Requisitos Operacionais do Sistema

Seu sistema deverá solucionar os trêss componentes principais:

### Componente 1: Aproximaçãoo e Autorizaçãoo de Pouso

A autorizaçãoo de pouso das aeronaves que chegam ao espaçoo aéreo do aeroporto deve seguir uma política de atendimento diferenciada baseada nos seguintes níveis:

- nivel 1: Emergência: Nível de combustível crítico (< 10%) ou emergencia médica a bordo.

-  Nível 2: Voos Comerciais: Voos regulares de linhas aéreas.

-  Nível 3: Carga ou Particular: Voos exclusivos de frete ou jatos particulares.

**Regra Operacional:** Uma aeronave de nível inferior deve sempre ser autorizada a pousar sempre antes de uma de nível superior.
