# Laboratório de Redes: Trabalho 1

Este projeto é um mini-serviço de transferência de arquivos cliente-servidor que utiliza **sockets TCP** para comunicação. [cite_start]O objetivo principal é gerar tráfego TCP realista para analisar o comportamento dos mecanismos do protocolo, como o controle de congestionamento[cite: 6].

---

## Especificações

### Objetivo Geral
[cite_start]O trabalho visa o desenvolvimento de uma aplicação cliente-servidor sobre TCP/IPv4, a criação de um protocolo de aplicação, o tratamento de erros, a implementação de concorrência no servidor, a coleta de métricas TCP e a configuração de um ambiente de testes com modelagem de link[cite: 8, 9, 10, 11, 12, 13].

### Módulos

* [cite_start]**Cliente**: Este módulo é responsável por interpretar os comandos do usuário, gerenciar a comunicação com o servidor e exibir informações para o usuário[cite: 17].
* [cite_start]**Servidor**: Este módulo gerencia a aplicação, recebe e armazena arquivos de clientes de forma concorrente, atendendo múltiplos clientes ao mesmo tempo[cite: 18]. [cite_start]O servidor deve escutar em um IP e porta específicos e armazenar os arquivos em um diretório[cite: 29, 30].

### Protocolo de Aplicação
[cite_start]O protocolo deve ser baseado em um quadro (frame) que contém informações de controle e os dados da aplicação[cite: 20]. [cite_start]Cada mensagem deve incluir um campo para a operação, um para o tamanho do payload de dados e o payload de dados em si[cite: 22].

As operações suportadas são:
* [cite_start]`LIST`: O servidor retorna uma lista de arquivos[cite: 25].
* [cite_start]`PUT <nome_arquivo>`: Realiza o upload de um arquivo do cliente para o servidor[cite: 26].
* [cite_start]`QUIT`: Encerra a sessão[cite: 26].

[cite_start]O protocolo também deve incluir o tratamento de erros para o envio e o armazenamento de arquivos com nomes duplicados[cite: 27].

### Interface do Cliente
[cite_start]A interface do cliente deve ser de linha de comando e suportar os seguintes comandos[cite: 32, 33]:
* [cite_start]`put <arquivo>` [cite: 34]
* [cite_start]`list` [cite: 36]
* [cite_start]`quit` [cite: 39]

[cite_start]Além disso, a interface deve permitir que o usuário especifique o host (IP do servidor) e a porta da aplicação[cite: 40, 41, 42].

### Instrumentação e Análise
[cite_start]Para analisar o desempenho das transmissões, os seguintes dados devem ser coletados no cliente e registrados em um log por conexão[cite: 44, 45]:
* Timestamps de início e fim da conexão.
* Bytes enviados/recebidos.
* Duração da conexão.
* Taxa aproximada de bytes por segundo.

[cite_start]É necessário utilizar o **Wireshark** para capturar e analisar o tráfego[cite: 46]. [cite_start]Para ambientes Linux, informações adicionais como **RTT**, `cwnd`, `ssthresh` e retransmissões podem ser coletadas utilizando `getsockopt(TCP_INFO)`[cite: 47, 48].

### Ambiente de Experimentos
[cite_start]O ambiente mínimo para o trabalho é composto por um cliente e um servidor (máquinas físicas, VMs, WSL2 ou containers Docker)[cite: 52]. [cite_start]Para simular gargalos na rede, é obrigatório usar ferramentas como `tc/netem` no Linux para limitar a banda, a latência e a perda de pacotes[cite: 53].

### Cenários de Teste
[cite_start]Os seguintes cenários devem ser executados para a coleta e comparação de resultados[cite: 61]:

1.  [cite_start]**Cenário 1**: Executar uma instância do cliente para enviar um arquivo de 200 MB, sem alterações na interface de rede[cite: 62, 63].
2.  [cite_start]**Cenário 2**: Executar de duas a quatro instâncias de clientes concorrentes para enviar arquivos de 200 MB, sem alterações na interface de rede[cite: 64, 65].
3.  [cite_start]**Cenário 3**: Repetir o Cenário 1, mas com alterações na interface de rede do cliente, uma de cada vez, para incluir perda de pacotes e latência variável[cite: 66, 67, 68, 69].
4.  [cite_start]**Cenário 4**: Repetir o Cenário 2, mas com as mesmas alterações de rede do Cenário 3[cite: 70, 71, 72, 73].

---

## To-Do List

### Etapa 1: Implementação
* [ ] Desenvolver o módulo **Servidor** que escute em um IP e porta.
* [ ] Implementar a capacidade do servidor de armazenar arquivos em um diretório.
* [ ] Implementar concorrência no servidor para atender múltiplos clientes simultaneamente.
* [ ] Desenvolver o módulo **Cliente** com interface de linha de comando.
* [ ] Implementar o protocolo de aplicação para as operações `LIST`, `PUT` e `QUIT`.
* [cite_start][ ] Garantir o tratamento de erros, como arquivos com nomes duplicados[cite: 27].

### Etapa 2: Coleta de Métricas
* [cite_start][ ] Adicionar a funcionalidade no cliente para coletar e logar métricas por conexão (timestamps, bytes, duração, taxa)[cite: 45].
* [cite_start][ ] Opcional (Linux): Coletar métricas TCP adicionais (`RTT`, `cwnd`, `ssthresh`, retransmissões) usando `getsockopt(TCP_INFO)`[cite: 47].

### Etapa 3: Configuração e Execução
* [ ] Configurar um ambiente de testes com cliente e servidor.
* [cite_start][ ] Instalar o **Wireshark** e, se estiver no Linux, as ferramentas `tc/netem`[cite: 46, 53].
* [ ] Executar os **Cenários 1 e 2** sem alterações de rede e coletar todos os dados e capturas de tráfego.
* [cite_start][ ] Executar os **Cenários 3 e 4** com perda de pacotes e latência variável na interface de rede, e coletar os dados e capturas para cada subcenário[cite: 66, 70].

### Etapa 4: Análise e Relatório
* [cite_start][ ] Utilizar o Wireshark para analisar os dados capturados, aplicando filtros para identificar retransmissões, `ACKs` duplicados e segmentos perdidos[cite: 86, 87, 88, 89, 90, 91].
* [cite_start][ ] Gerar os gráficos **Time-Sequence Graph** e **I/O Graph** no Wireshark para visualizar o comportamento da janela de congestionamento[cite: 92, 96].
* [cite_start][ ] Adicionar as colunas "Delta time displayed" e "TCP Bytes in Flight" no Wireshark para facilitar a análise[cite: 106, 108].
* [cite_start][ ] Elaborar um relatório comparando o desempenho nos quatro cenários, incluindo dados, capturas de tela e gráficos[cite: 111, 112, 113, 114].
* [cite_start][ ] No relatório, explicar o tipo de perda (fast retransmit ou timeout), a reação do TCP (`cwnd`/`ssthresh` reduction) e o processo de recuperação[cite: 115, 116, 117].
* [cite_start][ ] Juntar o relatório, o código-fonte comentado e entregar o trabalho até a data final de **29 de setembro**[cite: 120, 123].
