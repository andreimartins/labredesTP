# Laboratório de Redes: Trabalho 1

Este projeto é um mini-serviço de transferência de arquivos cliente-servidor que utiliza **sockets TCP** para comunicação. O objetivo principal é gerar tráfego TCP realista para analisar o comportamento dos mecanismos do protocolo, como o controle de congestionamento.

---

## Especificações

### Objetivo Geral
O trabalho visa o desenvolvimento de uma aplicação cliente-servidor sobre TCP/IPv4, a criação de um protocolo de aplicação, o tratamento de erros, a implementação de concorrência no servidor, a coleta de métricas TCP e a configuração de um ambiente de testes com modelagem de link.

### Módulos

* **Cliente**: Este módulo é responsável por interpretar os comandos do usuário, gerenciar a comunicação com o servidor e exibir informações para o usuário.
* **Servidor**: Este módulo gerencia a aplicação, recebe e armazena arquivos de clientes de forma concorrente, atendendo múltiplos clientes ao mesmo tempo. O servidor deve escutar em um IP e porta específicos e armazenar os arquivos em um diretório.

### Protocolo de Aplicação
O protocolo deve ser baseado em um quadro (frame) que contém informações de controle e os dados da aplicação. Cada mensagem deve incluir um campo para a operação, um para o tamanho do payload de dados e o payload de dados em si.

As operações suportadas são:
* `LIST`: O servidor retorna uma lista de arquivos.
* `PUT <nome_arquivo>`: Realiza o upload de um arquivo do cliente para o servidor.
* `QUIT`: Encerra a sessão.

O protocolo também deve incluir o tratamento de erros para o envio e o armazenamento de arquivos com nomes duplicados.

### Interface do Cliente
A interface do cliente deve ser de linha de comando e suportar os seguintes comandos:
* `put <arquivo>`
* `list`
* `quit`

Além disso, a interface deve permitir que o usuário especifique o host (IP do servidor) e a porta da aplicação.

### Instrumentação e Análise
Para analisar o desempenho das transmissões, os seguintes dados devem ser coletados no cliente e registrados em um log por conexão:
* Timestamps de início e fim da conexão.
* Bytes enviados/recebidos.
* Duração da conexão.
* Taxa aproximada de bytes por segundo.

É necessário utilizar o **Wireshark** para capturar e analisar o tráfego. Para ambientes Linux, informações adicionais como **RTT**, `cwnd`, `ssthresh` e retransmissões podem ser coletadas utilizando `getsockopt(TCP_INFO)`.

### Ambiente de Experimentos
O ambiente mínimo para o trabalho é composto por um cliente e um servidor (máquinas físicas, VMs, WSL2 ou containers Docker). Para simular gargalos na rede, é obrigatório usar ferramentas como `tc/netem` no Linux para limitar a banda, a latência e a perda de pacotes.

### Cenários de Teste
Os seguintes cenários devem ser executados para a coleta e comparação de resultados:

1.  **Cenário 1**: Executar uma instância do cliente para enviar um arquivo de 200 MB, sem alterações na interface de rede.
2.  **Cenário 2**: Executar de duas a quatro instâncias de clientes concorrentes para enviar arquivos de 200 MB, sem alterações na interface de rede.
3.  **Cenário 3**: Repetir o Cenário 1, mas com alterações na interface de rede do cliente, uma de cada vez, para incluir perda de pacotes e latência variável.
4.  **Cenário 4**: Repetir o Cenário 2, mas com as mesmas alterações de rede do Cenário 3.

---

## To-Do List

### Etapa 1: Implementação
* [ ] Desenvolver o módulo **Servidor** que escute em um IP e porta.
* [ ] Implementar a capacidade do servidor de armazenar arquivos em um diretório.
* [ ] Implementar concorrência no servidor para atender múltiplos clientes simultaneamente.
* [ ] Desenvolver o módulo **Cliente** com interface de linha de comando.
* [ ] Implementar o protocolo de aplicação para as operações `LIST`, `PUT` e `QUIT`.
* [ ] Garantir o tratamento de erros, como arquivos com nomes duplicados.

### Etapa 2: Coleta de Métricas
* [ ] Adicionar a funcionalidade no cliente para coletar e logar métricas por conexão (timestamps, bytes, duração, taxa).
* [ ] Opcional (Linux): Coletar métricas TCP adicionais (`RTT`, `cwnd`, `ssthresh`, retransmissões) usando `getsockopt(TCP_INFO)`.

### Etapa 3: Configuração e Execução
* [ ] Configurar um ambiente de testes com cliente e servidor.
* [ ] Instalar o **Wireshark** e, se estiver no Linux, as ferramentas `tc/netem`.
* [ ] Executar os **Cenários 1 e 2** sem alterações de rede e coletar todos os dados e capturas de tráfego.
* [ ] Executar os **Cenários 3 e 4** com perda de pacotes e latência variável na interface de rede, e coletar os dados e capturas para cada subcenário.

### Etapa 4: Análise e Relatório
* [ ] Utilizar o Wireshark para analisar os dados capturados, aplicando filtros para identificar retransmissões, `ACKs` duplicados e segmentos perdidos.
* [ ] Gerar os gráficos **Time-Sequence Graph** e **I/O Graph** no Wireshark para visualizar o comportamento da janela de congestionamento.
* [ ] Adicionar as colunas "Delta time displayed" e "TCP Bytes in Flight" no Wireshark para facilitar a análise.
* [ ] Elaborar um relatório comparando o desempenho nos quatro cenários, incluindo dados, capturas de tela e gráficos.
* [ ] No relatório, explicar o tipo de perda (fast retransmit ou timeout), a reação do TCP (`cwnd`/`ssthresh` reduction) e o processo de recuperação.
* [ ] Juntar o relatório, o código-fonte comentado e entregar o trabalho até a data final de **29 de setembro**.
