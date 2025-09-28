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

Foram incluídos outros comando de uso interno para o protocolo:

> `OP_DATA`     Bloco de dados será recebido    
> `OP_ERROR`    Indica erro encontrado na operação    
> `OP_OK `      Operação realizada com sucesso ou confirmação de OK para andamento    

O protocolo também deve incluir o tratamento de erros para o envio e o armazenamento de arquivos com nomes duplicados.

> Foi tomada a decisão de recusar o recebimento do arquivo com o mesmo nome.



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
* [x] Desenvolver o módulo **Servidor** que escute em um IP e porta.
* [x] Implementar a capacidade do servidor de armazenar arquivos em um diretório.
* [x] Implementar concorrência no servidor para atender múltiplos clientes simultaneamente.
* [x] Desenvolver o módulo **Cliente** com interface de linha de comando.
* [x] Implementar o protocolo de aplicação para as operações `LIST`, `PUT` e `QUIT`.
* [x] Garantir o tratamento de erros, como arquivos com nomes duplicados.

> No caso de arquivo suplicado caso está recusando o envio do arquivo, pode ser alterado para renomear no recebimento.

### Etapa 2: Coleta de Métricas
* [x] Adicionar a funcionalidade no cliente para coletar e logar métricas por conexão (timestamps, bytes, duração, taxa).
* [x] Opcional (Linux): Coletar métricas TCP adicionais (`RTT`, `cwnd`, `ssthresh`, retransmissões) usando `getsockopt(TCP_INFO)`.

> - RTT (Round-Trip Time): Tempo que um pacote leva para ir do cliente ao servidor e voltar, medido em microssegundos. Indica a latência da conexão.
> 
> - cwnd (Congestion Window): Tamanho da janela de congestionamento do TCP, ou seja, quantos bytes podem ser enviados antes de aguardar confirmação (ACK). É ajustado dinamicamente para evitar congestionamento na rede.
> 
> - ssthresh (Slow Start Threshold): Limite que separa os modos de crescimento da janela TCP. Até esse valor, o TCP aumenta a janela rapidamente (slow start); acima dele, o crescimento é mais lento (congestion avoidance).
> 
> - retrans (Total Retransmissions): Número total de retransmissões de pacotes feitos pelo TCP. Um valor alto pode indicar problemas de rede, como perda de pacotes.

* [x] incluído calculo aproximado de taxa de bits no client 

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



## Manual de Compilação

### Compilando na pasta T1/src

1. Abra o terminal e navegue até a pasta do projeto:
	```bash
	cd ~/Git/labredesTP/T1/src
	```
2. Compile todos os binários (cliente e servidor):
	```bash
	make all
	```
	Ou para compilar com debug:
	```bash
	make debug
	```
3. Para limpar os arquivos gerados:
	```bash
	make clean
	```

Após a compilação, os binários `client` e `server` são gerados na pasta `bin/` e automaticamente copiados para a pasta `src/`.

Você pode executar os binários tanto de `bin/` quanto de `src/`.

## Manual de Execução

### Executando o Servidor

1. Execute o servidor informando a porta desejada:

```bash
./server <porta>
```
Exemplo:
```bash
./server 15000
```

> Ao inciar o servidor exibe o ip que está sendo executado.

### Executando o Cliente

1. Execute o cliente informando o IP do servidor, a porta e o comando desejado:

```bash
./client <ip_servidor> <porta>
```
Exemplo:
```bash
./client 127.0.0.1 15000
```
2. Na interface do cliente, utilize os comandos:
	- `put <arquivo>`: Envia arquivo ao servidor
	- `list`: Lista arquivos disponíveis no servidor
	- `quit`: Encerra a conexão

> Para o comando `put` pode-se incluir tanto somente o nome de um arquivo que deverá estar na mesma pasta do client ou informando o caminho completo junto ao nome.
> ex: `put arquivo.png`
> ex2: `/home/user/downloads/enunciado.pdf`
> No segundo caso o client informará para o servidor somente o nome do arquivo sem o caminho informado

### Observações
- Os logs de execução são gerados na pasta `logs/`.
- Os arquivos recebidos pelo servidor ficam em `storage/`.
- Para múltiplos clientes, execute várias instâncias do cliente em terminais/containers diferentes.