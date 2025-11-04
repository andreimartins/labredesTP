Com base no documento fornecido, aqui está uma estrutura de `README.md` para o projeto:

-----

# Laboratório de Redes de Computadores - Trabalho Final: Monitor de Tráfego de Rede em Tempo Real

## 🎯 Objetivo

O objetivo geral deste projeto é desenvolver uma ferramenta para monitoramento de tráfego de rede em tempo real utilizando **raw sockets**[cite: 4].

A ferramenta deve ser capaz de capturar, interpretar e classificar pacotes de rede, além de fornecer uma interface de usuário simples (modo texto) para visualizar estatísticas e contadores de tráfego[cite: 5]. Simultaneamente, deve escrever um histórico detalhado dos pacotes recebidos em arquivos de log[cite: 5].

### Objetivos Específicos

  * Desenvolver uma aplicação usando sockets raw[cite: 7].
  * Estudar o funcionamento dos protocolos de rede e a relação entre as camadas[cite: 8].
  * Entender a estruturação e interpretação de pacotes de dados[cite: 9].
  * Analisar o tráfego de uma rede local e seus protocolos[cite: 10].

## 📋 Descrição do Cenário

A aplicação será desenvolvida para analisar o tráfego em uma rede onde clientes acessam a internet através de um servidor proxy, ambos localizados na mesma LAN[cite: 13, 14]. O tráfego dos clientes é encapsulado e enviado ao proxy [cite: 16], que desencapsula e encaminha os pacotes[cite: 17].

O monitor de tráfego deverá rodar na máquina que atua como servidor proxy[cite: 18]. A interface a ser monitorada é a interface virtual `tuno`, onde o tráfego de todos os clientes pode ser interceptado[cite: 23].

## ✨ Funcionalidades Principais

O monitor de tráfego deve:

1.  **Identificar Protocolos:** Classificar diferentes tipos de pacotes, como IP, TCP, UDP, ICMP, DHCP, etc.[cite: 19, 20].
2.  **Extrair Informações:** Identificar origem, destino e tamanho dos pacotes[cite: 20].
3.  **Apresentar Interface:** Possuir uma interface em modo texto que exibe contadores para cada tipo de pacote recebido[cite: 21].
4.  **Monitorar por Cliente:** Apresentar informações detalhadas para cada cliente (identificado pelo IP da rede túnel), incluindo máquinas remotas acessadas, portas, protocolos, conexões e volume de tráfego[cite: 47].
5.  **Gerar Logs:** Manter arquivos de log (`.csv`) atualizados em tempo real[cite: 22, 46].

## 📁 Requisitos de Log

Os logs devem ser gerados em tempo real e separados por camada[cite: 22, 46].

### 1\. Camada de Rede (camada\_internet.csv)

  * **Protocolos:** IPv4, IPv6, ICMP[cite: 25].
  * **Colunas:**
      * Data e hora da captura [cite: 26]
      * Nome do protocolo (IPv4, IPv6, ICMP, outro) [cite: 27]
      * Endereço IP de origem [cite: 28]
      * Endereço IP de destino [cite: 29]
      * Número identificador do protocolo carregado [cite: 30]
      * Outras informações do protocolo (para ICMP) [cite: 30]
      * Tamanho total do pacote (em bytes) [cite: 31]

### 2\. Camada de Transporte (camada\_transporte.csv)

  * **Protocolos:** TCP, UDP[cite: 32].
  * **Colunas:**
      * Data e hora da captura [cite: 33]
      * Nome do protocolo (TCP, UDP, outro) [cite: 36]
      * Endereço IP de origem [cite: 37]
      * Porta de origem [cite: 38]
      * Endereço IP de destino [cite: 39]
      * Porta de destino [cite: 40]
      * Tamanho total do pacote (em bytes) [cite: 41]

### 3\. Camada de Aplicação (camada\_aplicacao.csv)

  * **Protocolos:** HTTP, DHCP, DNS, NTP[cite: 42].
  * **Colunas:**
      * Data e hora da captura [cite: 43]
      * Nome do protocolo (HTTP, DHCP, DNS, NTP, outro) [cite: 44]
      * Informações do protocolo (obtidas de seu cabeçalho) [cite: 45]

## 🏗️ Arquitetura e Configuração do Túnel

O tráfego das aplicações do cliente passa pela interface virtual (`tuno`) para um programa túnel[cite: 90]. Este programa encapsula e mascara o tráfego, injetando-o na interface física (LAN)[cite: 91]. O servidor proxy recebe o tráfego, desencapsula-o e o injeta em sua própria interface virtual `tuno`[cite: 92]. A partir daí, o proxy usa NAT (iptables com masquerading) para rotear o tráfego para a internet[cite: 93].

### Pré-requisitos (Ambiente Linux)

  * `build-essentials` [cite: 95]
  * `iptables` [cite: 95]
  * (Não são necessárias dependências extras se usar o container da disciplina) [cite: 96].

### Compilação

Após descompactar os fontes do túnel, execute:

```bash
make
```

[cite: 96]

### Execução do Túnel

**1. Modo Servidor Proxy:**
(Use `ifconfig` ou `ip addr` para saber o nome da interface) [cite: 102]

```bash
sudo ./traffic_tunnel <interface do servidor> -s
```

[cite: 103]

**2. Modo Cliente:**
(Use um script diferente para cada cliente, ex: `client1.sh`) [cite: 106]

```bash
sudo ./traffic_tunnel <interface do cliente> -c client1.sh
```

[cite: 108]

## deliverables

  * **Grupo:** 3 ou 4 integrantes[cite: 116].
  * **Apresentação:** 24/11[cite: 116].
  * **Linguagem:** Qualquer linguagem de programação é permitida[cite: 118].
  * **Entrega:** Um único arquivo `.tar.gz` ou `.zip` enviado pelo Moodle até a data especificada[cite: 119].
  * **Conteúdo do Pacote:**
    1.  Nomes dos integrantes[cite: 119].
    2.  Código fonte completo do projeto[cite: 119].
    3.  Relatório descrevendo a implementação, incluindo screenshots que demonstrem o funcionamento[cite: 119, 120].

### ⚠️ Restrições Importantes

  * Não serão aceitos trabalhos entregues fora do prazo[cite: 121].
  * Trabalhos que não compilam ou não executam não serão avaliados[cite: 121].
  * Trabalhos que **não utilizam sockets raw** serão desconsiderados[cite: 122].
  
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
- [x] Garantir que os arquivos de log sejam atualizados em tempo real e possam ser lidos a qualquer momento (ex: via `cat`)[cite: 46].

### 3. Análise e Exibição
- [ ] Desenvolver a funcionalidade de monitoramento por cliente (IP da rede `tuno`)[cite: 47].
- [ ] Exibir para cada cliente: máquinas remotas acessadas, portas, protocolos, conexões e volume de tráfego[cite: 47].

### 4. Entrega e Relatório
- [ ] Preparar o relatório final descrevendo a implementação do projeto[cite: 119].
- [ ] Incluir screenshots no relatório que demonstrem o funcionamento da ferramenta no ambiente de rede[cite: 120].
- [ ] Verificar se o projeto compila e executa corretamente (requisito obrigatório)[cite: 121].
- [ ] Organizar o código fonte completo e o relatório em um arquivo `.tar.gz` ou `.zip`[cite: 119].
- [ ] Incluir os nomes dos integrantes no pacote de entrega[cite: 119].
- [ ] Enviar o arquivo final pelo Moodle até a data estipulada (24/11)[cite: 116, 119].