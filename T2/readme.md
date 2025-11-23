```markdown
Com base no documento fornecido, aqui está uma estrutura de `README.md` para o projeto:

-----

# Laboratório de Redes de Computadores - Trabalho Final: Monitor de Tráfego de Rede em Tempo Real

## 🎯 Objetivo

O objetivo geral deste projeto é desenvolver uma ferramenta para monitoramento de tráfego de rede em tempo real utilizando **raw sockets**.

A ferramenta deve ser capaz de capturar, interpretar e classificar pacotes de rede, além de fornecer uma interface de usuário simples (modo texto) para visualizar estatísticas e contadores de tráfego. Simultaneamente, deve escrever um histórico detalhado dos pacotes recebidos em arquivos de log.

### Objetivos Específicos

  * Desenvolver uma aplicação usando sockets raw.
  * Estudar o funcionamento dos protocolos de rede e a relação entre as camadas.
  * Entender a estruturação e interpretação de pacotes de dados.
  * Analisar o tráfego de uma rede local e seus protocolos.

## 📋 Descrição do Cenário

A aplicação será desenvolvida para analisar o tráfego em uma rede onde clientes acessam a internet através de um servidor proxy, ambos localizados na mesma LAN. O tráfego dos clientes é encapsulado e enviado ao proxy, que desencapsula e encaminha os pacotes.

O monitor de tráfego deverá rodar na máquina que atua como servidor proxy. A interface a ser monitorada é a interface virtual `tuno`, onde o tráfego de todos os clientes pode ser interceptado.

## ✨ Funcionalidades Principais

O monitor de tráfego deve:

1.  **Identificar Protocolos:** Classificar diferentes tipos de pacotes, como IP, TCP, UDP, ICMP, DHCP, etc.
2.  **Extrair Informações:** Identificar origem, destino e tamanho dos pacotes.
3.  **Apresentar Interface:** Possuir uma interface em modo texto que exibe contadores para cada tipo de pacote recebido.
4.  **Monitorar por Cliente:** Apresentar informações detalhadas para cada cliente (identificado pelo IP da rede túnel), incluindo máquinas remotas acessadas, portas, protocolos, conexões e volume de tráfego.
5.  **Gerar Logs:** Manter arquivos de log (`.csv`) atualizados em tempo real.

## 📁 Requisitos de Log

Os logs devem ser gerados em tempo real e separados por camada.

### 1. Camada de Rede (camada_internet.csv)

  * **Protocolos:** IPv4, IPv6, ICMP.
  * **Colunas:**
      * Data e hora da captura
      * Nome do protocolo (IPv4, IPv6, ICMP, outro)
      * Endereço IP de origem
      * Endereço IP de destino
      * Número identificador do protocolo carregado
      * Outras informações do protocolo (para ICMP)
      * Tamanho total do pacote (em bytes)

### 2. Camada de Transporte (camada_transporte.csv)

  * **Protocolos:** TCP, UDP.
  * **Colunas:**
      * Data e hora da captura
      * Nome do protocolo (TCP, UDP, outro)
      * Endereço IP de origem
      * Porta de origem
      * Endereço IP de destino
      * Porta de destino
      * Tamanho total do pacote (em bytes)

### 3. Camada de Aplicação (camada_aplicacao.csv)

  * **Protocolos:** HTTP, DHCP, DNS, NTP.
  * **Colunas:**
      * Data e hora da captura
      * Nome do protocolo (HTTP, DHCP, DNS, NTP, outro)
      * Informações do protocolo (obtidas de seu cabeçalho)

## 🏗️ Arquitetura e Configuração do Túnel

O tráfego das aplicações do cliente passa pela interface virtual (`tuno`) para um programa túnel. Este programa encapsula e mascara o tráfego, injetando-o na interface física (LAN). O servidor proxy recebe o tráfego, desencapsula-o e o injeta em sua própria interface virtual `tuno`. A partir daí, o proxy usa NAT (iptables com masquerading) para rotear o tráfego para a internet.

### Pré-requisitos (Ambiente Linux)

  * `build-essentials`
  * `iptables`
  * (Não são necessárias dependências extras se usar o container da disciplina).

### Compilação

Após descompactar os fontes do túnel, execute:

```bash
make
```

### Execução do Túnel

**1. Modo Servidor Proxy:**
(Use `ifconfig` ou `ip addr` para saber o nome da interface)

```bash
sudo ./traffic_tunnel <interface do servidor> -s
```

**2. Modo Cliente:**
(Use um script diferente para cada cliente, ex: `client1.sh`)

```bash
sudo ./traffic_tunnel <interface do cliente> -c client1.sh
```

## deliverables

  * **Grupo:** 3 ou 4 integrantes.
  * **Apresentação:** 24/11.
  * **Linguagem:** Qualquer linguagem de programação é permitida.
  * **Entrega:** Um único arquivo `.tar.gz` ou `.zip` enviado pelo Moodle até a data especificada.
  * **Conteúdo do Pacote:**
    1.  Nomes dos integrantes.
    2.  Código fonte completo do projeto.
    3.  Relatório descrevendo a implementação, incluindo screenshots que demonstrem o funcionamento.

### ⚠️ Restrições Importantes

  * Não serão aceitos trabalhos entregues fora do prazo.
  * Trabalhos que não compilam ou não executam não serão avaliados.
  * Trabalhos que **não utilizam sockets raw** serão desconsiderados.
  
  ---

Aqui está a lista de tarefas (To-Do List) formatada como um arquivo `README.md`, ideal para acompanhar o progresso do projeto:

---

# Trabalho Final - Monitor de Tráfego de Rede

## 📋 Lista de Tarefas (To-Do List)

### 1. Funcionalidades Principais (Core)
- [ ] Desenvolver a ferramenta de monitoramento utilizando **raw sockets**
- [ ] Conexão com socket RAW
- [ ] Capatura e enfileiramento dos pacotes
- [ ] Decodificação e classificação dos pacotes
  - [ ] Camada de rede
    - [ ] timestamp
    - [ ] Nome do protocolo
    - [ ] IP origem
    - [ ] IP destino
    - [ ] Identificador do protocolo que esta sendo carregado no pacote
    - [ ] extras
    - [ ] tamanho em bytes
  - [ ] Camada de transporte
    - [ ] timestamp
    - [ ] nome do protocolo
    - [ ] IP de origem 
    - [ ] porta de origem
    - [ ] IP de destino 
    - [ ] porta de destino
    - [ ] tamano do pacote em bytes
  - [ ] camada de aplicação
    - [ ] timestamp
    - [ ] nome do protocolo
    - [ ] informações do protocolo (do cabeçalho)
- [ ] Gravação dos pacotes capturados.

### 2. Geração de Logs (.csv)
- [x] Implementar log em tempo real: `camada_internet.csv` (IPv4, IPv6, ICMP)
- [x] Implementar log em tempo real: `camada_transporte.csv` (TCP, UDP)
- [x] Implementar log em tempo real: `camada_aplicacao.csv` (HTTP, DHCP, DNS, NTP)
- [x] Garantir que os arquivos de log sejam atualizados em tempo real e possam ser lidos a qualquer momento (ex: via `cat`).

### 3. Análise e Exibição
- [ ] Desenvolver a funcionalidade de monitoramento por cliente (IP da rede `tuno`).
- [ ] Exibir para cada cliente: máquinas remotas acessadas, portas, protocolos, conexões e volume de tráfego.

### 4. Entrega e Relatório
- [ ] Preparar o relatório final descrevendo a implementação do projeto.
- [ ] Incluir screenshots no relatório que demonstrem o funcionamento da ferramenta no ambiente de rede.
- [ ] Verificar se o projeto compila e executa corretamente (requisito obrigatório).
- [ ] Organizar o código fonte completo e o relatório em um arquivo `.tar.gz` ou `.zip`.
- [ ] Incluir os nomes dos integrantes no pacote de entrega.
- [ ] Enviar o arquivo final pelo Moodle até a data estipulada (24/11).