# Manual de Utilização do Mercurial

## Introdução

O Mercurial é um sistema de controle de versão distribuído (DVCS) que permite aos desenvolvedores rastrear e gerenciar alterações em seus projetos de software de forma eficiente. Assim como o Git, ele oferece um modelo de desenvolvimento descentralizado, onde cada desenvolvedor possui uma cópia completa do repositório, incluindo todo o histórico de revisões. Isso proporciona maior flexibilidade, resiliência e a capacidade de trabalhar offline.

Desenvolvido em Python, o Mercurial é conhecido por sua simplicidade, facilidade de uso e desempenho robusto, especialmente em grandes bases de código. Ele foi projetado com uma curva de aprendizado suave, tornando-o uma excelente escolha para equipes que buscam uma alternativa ao Git ou outros sistemas de controle de versão centralizados. Este manual tem como objetivo fornecer um guia abrangente para a utilização do Mercurial, desde a instalação até os comandos mais avançados, permitindo que você aproveite ao máximo essa poderosa ferramenta de versionamento.





## Instalação do Mercurial

A instalação do Mercurial é um processo relativamente simples e varia de acordo com o sistema operacional. Abaixo, descrevemos os passos para os sistemas mais comuns:

### Windows

1.  **Baixar o instalador:** Acesse o site oficial do Mercurial (https://www.mercurial-scm.org/downloads/) e baixe o instalador para Windows (geralmente um arquivo `.exe`).
2.  **Executar o instalador:** Execute o arquivo baixado e siga as instruções do assistente de instalação. Certifique-se de marcar a opção para adicionar o Mercurial ao PATH do sistema, o que permitirá que você execute os comandos do Mercurial a partir de qualquer diretório no prompt de comando.
3.  **Verificar a instalação:** Abra o prompt de comando (CMD) ou PowerShell e digite `hg version`. Se a instalação foi bem-sucedida, você verá a versão do Mercurial instalada.

### macOS

1.  **Usando Homebrew (recomendado):** Se você tiver o Homebrew instalado, pode instalar o Mercurial com um único comando no Terminal:
    ```bash
    brew install mercurial
    ```
2.  **Baixar o instalador:** Alternativamente, você pode baixar o instalador `.pkg` do site oficial do Mercurial (https://www.mercurial-scm.org/downloads/) e seguir as instruções.
3.  **Verificar a instalação:** Abra o Terminal e digite `hg version`. Você deverá ver a versão do Mercurial.

### Linux (Ubuntu/Debian)

1.  **Usando APT:** No Ubuntu ou Debian, você pode instalar o Mercurial usando o gerenciador de pacotes `apt`:
    ```bash
    sudo apt update
    sudo apt install mercurial
    ```
2.  **Verificar a instalação:** Abra o Terminal e digite `hg version`. A versão do Mercurial será exibida.

### Outras distribuições Linux

Para outras distribuições Linux, você pode usar o gerenciador de pacotes correspondente (por exemplo, `yum install mercurial` para Fedora/CentOS, `pacman -S mercurial` para Arch Linux) ou compilar a partir do código-fonte, seguindo as instruções no site oficial.

Após a instalação, é recomendável configurar seu nome de usuário e e-mail para que suas contribuições sejam corretamente identificadas nos commits. Isso pode ser feito editando o arquivo de configuração global do Mercurial (`~/.hgrc` no Linux/macOS ou `Mercurial.ini` no Windows, localizado na pasta do usuário):

```ini
[ui]
username = Seu Nome <seu.email@example.com>
```

Substitua `Seu Nome` e `seu.email@example.com` pelas suas informações. Esta configuração é crucial para o rastreamento de autoria em seus repositórios.





## Comandos Básicos

Esta seção aborda os comandos fundamentais do Mercurial que você usará diariamente para gerenciar seus projetos. Compreender esses comandos é essencial para um fluxo de trabalho eficiente.

### `hg init` - Inicializar um novo repositório

Para começar a usar o Mercurial em um novo projeto, o primeiro passo é inicializar um repositório. Isso cria um diretório `.hg` na raiz do seu projeto, que contém todos os metadados e o histórico de revisões do Mercurial.

```bash
cd meu_projeto
hg init
```

Após a execução, o diretório `meu_projeto` se torna um repositório Mercurial local.

### `hg add` - Adicionar arquivos ao controle de versão

Depois de criar ou modificar arquivos em seu projeto, você precisa informar ao Mercurial que deseja rastreá-los. O comando `hg add` marca os arquivos para serem incluídos no próximo commit.

```bash
hg add arquivo.txt
hg add pasta/
hg add .
```

- `hg add arquivo.txt`: Adiciona um arquivo específico.
- `hg add pasta/`: Adiciona todos os arquivos dentro de uma pasta.
- `hg add .`: Adiciona todos os arquivos novos e modificados no diretório atual e subdiretórios (exceto os ignorados).

### `hg commit` - Registrar alterações

O comando `hg commit` registra as alterações marcadas (adicionadas) no histórico do repositório. Cada commit representa um ponto no tempo no histórico do seu projeto e deve incluir uma mensagem descritiva.

```bash
hg commit -m "Mensagem descritiva do commit"
```

É uma boa prática escrever mensagens de commit claras e concisas que expliquem o propósito das alterações.

### `hg status` - Verificar o status do repositório

Para ver quais arquivos foram modificados, adicionados, removidos ou estão sem rastreamento, use o comando `hg status`. Ele fornece uma visão geral do estado atual do seu diretório de trabalho em relação ao último commit.

```bash
hg status
```

As saídas comuns incluem:
- `A`: Adicionado (added)
- `M`: Modificado (modified)
- `R`: Removido (removed)
- `?`: Não rastreado (untracked)
- `C`: Conflito (conflicted)

### `hg log` - Visualizar o histórico de commits

O comando `hg log` exibe o histórico de commits do repositório, mostrando informações como o autor, a data, a mensagem do commit e o identificador de revisão (changeset ID).

```bash
hg log
```

Você pode usar opções para filtrar ou formatar a saída:
- `hg log -l 5`: Mostra os últimos 5 commits.
- `hg log -u Seu Nome`: Mostra commits de um autor específico.
- `hg log -p`: Mostra as diferenças (patches) de cada commit.

### `hg diff` - Ver diferenças entre versões

Para inspecionar as alterações feitas em arquivos antes de um commit, ou para comparar diferentes revisões, use `hg diff`.

```bash
hg diff
```

- `hg diff`: Mostra as diferenças entre o diretório de trabalho e o último commit.
- `hg diff arquivo.txt`: Mostra as diferenças para um arquivo específico.
- `hg diff -r <revisao1> -r <revisao2>`: Compara duas revisões específicas.

### `hg revert` - Desfazer alterações locais

Se você fez alterações em um arquivo e deseja descartá-las, retornando ao estado do último commit, use `hg revert`.

```bash
hg revert arquivo.txt
```

**Cuidado:** Este comando descarta as alterações não comitadas e não pode ser desfeito facilmente.

### `hg remove` - Remover arquivos do controle de versão

Para remover um arquivo do controle de versão do Mercurial (e opcionalmente do sistema de arquivos), use `hg remove`.

```bash
hg remove arquivo_antigo.txt
hg commit -m "Removido arquivo_antigo.txt"
```

O arquivo será marcado para remoção no próximo commit.





## Fluxo de Trabalho

O Mercurial, sendo um sistema de controle de versão distribuído, possui um fluxo de trabalho que envolve a interação entre repositórios locais e remotos. Esta seção detalha os comandos essenciais para colaborar em projetos.

### `hg clone` - Clonar um repositório existente

Para começar a trabalhar em um projeto existente que já está sob controle de versão Mercurial, você precisa clonar o repositório remoto para o seu ambiente local. Isso cria uma cópia completa do repositório, incluindo todo o histórico de revisões.

```bash
hg clone <URL_do_repositorio> [nome_do_diretorio]
```

- `<URL_do_repositorio>`: O endereço do repositório remoto (pode ser HTTP, SSH, ou um caminho de arquivo local).
- `[nome_do_diretorio]`: (Opcional) O nome do diretório onde o repositório será clonado. Se omitido, o nome do diretório será derivado da URL.

Exemplo:
```bash
hg clone https://example.com/meu_projeto
```

### `hg pull` - Obter alterações de um repositório remoto

O comando `hg pull` é usado para baixar as alterações mais recentes de um repositório remoto para o seu repositório local. Ele busca as novas revisões, mas não as mescla automaticamente com o seu diretório de trabalho.

```bash
hg pull [URL_do_repositorio]
```

Se a URL for omitida, o Mercurial tentará buscar do repositório de origem configurado (geralmente o repositório de onde você clonou).

### `hg update` - Atualizar o diretório de trabalho

Após um `hg pull`, as novas revisões estão no seu repositório local, mas seu diretório de trabalho ainda não foi atualizado para refletir essas mudanças. O comando `hg update` aplica as revisões baixadas ao seu diretório de trabalho.

```bash
hg update
```

Se houver conflitos entre suas alterações locais e as alterações puxadas, o Mercurial tentará mesclá-las automaticamente. Se não for possível, ele marcará os arquivos com conflito para resolução manual.

É comum usar `hg pull` seguido de `hg update` para sincronizar seu repositório local e diretório de trabalho com as últimas alterações do remoto.

### `hg push` - Enviar alterações para um repositório remoto

Depois de fazer commits em seu repositório local, você pode enviar essas alterações para um repositório remoto, tornando-as disponíveis para outros colaboradores. O comando `hg push` envia todas as revisões que estão no seu repositório local e não estão no remoto.

```bash
hg push [URL_do_repositorio]
```

Assim como no `pull`, se a URL for omitida, o Mercurial enviará para o repositório de destino configurado.

Exemplo:
```bash
hg push ssh://usuario@servidor/caminho/para/repositorio
```

### Sincronizando seu trabalho: `pull`, `update` e `push`

Um fluxo de trabalho comum para manter seu repositório sincronizado e colaborar com outros é:

1.  **Puxar as últimas alterações:** Antes de começar a trabalhar ou antes de enviar suas próprias alterações, sempre puxe as últimas mudanças do repositório remoto.
    ```bash
    hg pull
    ```
2.  **Atualizar seu diretório de trabalho:** Aplique as alterações puxadas ao seu diretório de trabalho.
    ```bash
    hg update
    ```
3.  **Resolver conflitos (se houver):** Se o `update` resultar em conflitos, resolva-os manualmente nos arquivos afetados.
4.  **Fazer suas alterações e commits:** Trabalhe em seu código, adicione novos arquivos e faça commits locais.
    ```bash
    # ... faça suas edições ...
    hg add novos_arquivos.txt
    hg commit -m "Minhas novas funcionalidades"
    ```
5.  **Enviar suas alterações:** Depois de testar suas alterações localmente, envie-as para o repositório remoto.
    ```bash
    hg push
    ```

Este ciclo de `pull`, `update`, `commit` e `push` é fundamental para um desenvolvimento colaborativo eficaz com Mercurial.





## Ramificação e Fusão

Ramificação (branching) e fusão (merging) são conceitos cruciais em sistemas de controle de versão, permitindo que os desenvolvedores trabalhem em diferentes linhas de desenvolvimento simultaneamente e, posteriormente, combinem essas alterações. O Mercurial oferece um modelo flexível para gerenciar ramificações.

### `hg branch` - Criar e gerenciar ramificações

No Mercurial, as ramificações são leves e fazem parte do histórico do repositório. Você pode criar uma nova ramificação para desenvolver uma nova funcionalidade ou corrigir um bug sem afetar a linha principal de desenvolvimento.

```bash
hg branch nome_da_ramificacao
```

Este comando cria uma nova ramificação no seu repositório local. Para que essa ramificação seja visível para outros, você precisará fazer um commit e, em seguida, um push.

Para ver a ramificação atual:
```bash
hg branch
```

Para listar todas as ramificações existentes no repositório:
```bash
hg branches
```

### `hg merge` - Fundir ramificações

Quando o trabalho em uma ramificação é concluído, você pode fundir (merge) as alterações de volta para a ramificação principal (ou qualquer outra ramificação). O comando `hg merge` combina o histórico de duas ramificações.

Primeiro, certifique-se de estar na ramificação para a qual deseja fundir as alterações (a ramificação de destino):
```bash
hg update master # ou a ramificação principal
```

Em seguida, execute o comando merge, especificando a ramificação de origem:
```bash
hg merge nome_da_ramificacao_a_fundir
```

O Mercurial tentará fundir as alterações automaticamente. Se houver conflitos (alterações nas mesmas linhas de código em ambas as ramificações), o Mercurial marcará os arquivos conflitantes. Você precisará resolver esses conflitos manualmente, editando os arquivos e, em seguida, marcando-os como resolvidos.

Após resolver os conflitos, faça um commit para finalizar a fusão:
```bash
hg commit -m "Fundindo nome_da_ramificacao na master"
```

### `hg backout` - Desfazer um commit

Se você precisar desfazer um commit que já foi enviado, o comando `hg backout` pode ser útil. Ele cria um novo commit que reverte as alterações de um commit anterior, mantendo o histórico intacto.

```bash
hg backout -r <revisao_do_commit_a_desfazer>
```

Este comando cria um novo commit que desfaz as alterações do commit especificado. É uma forma segura de reverter alterações sem reescrever o histórico.

### `hg strip` - Remover revisões (com cautela)

O comando `hg strip` remove revisões do histórico do repositório. **Use este comando com extrema cautela**, pois ele reescreve o histórico e pode causar problemas se as revisões já tiverem sido compartilhadas com outros repositórios.

```bash
hg strip <revisao_a_remover>
```

Este comando é geralmente usado para limpar o histórico local antes de compartilhar alterações, ou para remover commits que contêm informações sensíveis. No entanto, é preferível evitar reescrever o histórico em repositórios compartilhados.





## Mercurial vs. Git

Mercurial e Git são ambos sistemas de controle de versão distribuídos (DVCS) populares, mas possuem filosofias e abordagens ligeiramente diferentes. Embora ambos permitam o desenvolvimento colaborativo e o rastreamento de alterações, as distinções podem influenciar a escolha da ferramenta para um projeto específico.

### Filosofia e Design

-   **Mercurial:** É frequentemente elogiado por sua simplicidade e design mais consistente. A curva de aprendizado do Mercurial é geralmente considerada mais suave, com comandos que são mais intuitivos e previsíveis. Ele foi projetado para ser fácil de usar e entender, com uma ênfase na estabilidade e na prevenção de reescrita de histórico.

-   **Git:** É conhecido por sua flexibilidade e poder. O Git oferece um conjunto mais amplo de ferramentas para manipular o histórico do repositório, o que pode ser extremamente poderoso para usuários avançados, mas também pode tornar a curva de aprendizado mais íngreme. A filosofia do Git é que o histórico pode ser reescrito para criar um histórico de projeto mais limpo e linear.

### Ramificação (Branching)

-   **Mercurial:** As ramificações no Mercurial são tratadas como parte do histórico do repositório. Uma vez que uma ramificação é criada, ela se torna parte permanente do histórico. Isso torna o histórico mais linear e fácil de seguir, mas pode ser menos flexível para fluxos de trabalho que exigem muitas ramificações de curta duração que são descartadas após a fusão.

-   **Git:** O Git usa um modelo de ramificação mais leve e flexível, onde as ramificações são essencialmente apenas ponteiros para commits. Isso permite a criação e exclusão rápida de ramificações, facilitando fluxos de trabalho como o Git Flow, onde ramificações de funcionalidade e de correção de bugs são criadas e mescladas frequentemente. A reescrita de histórico é mais comum no Git para manter um histórico de ramificações limpo.

### Performance

-   **Mercurial:** Historicamente, o Mercurial tem sido considerado mais rápido em repositórios muito grandes, especialmente em termos de operações como `clone` e `pull`. Empresas como o Facebook usam o Mercurial para gerenciar seus monorepositórios massivos, o que demonstra sua escalabilidade para grandes bases de código [1].

-   **Git:** O Git é otimizado para velocidade em muitas operações, especialmente em repositórios de tamanho médio. No entanto, em repositórios extremamente grandes, o desempenho pode ser um desafio, embora melhorias contínuas estejam sendo feitas.

### Comunidade e Ecossistema

-   **Mercurial:** Possui uma comunidade ativa, mas menor em comparação com o Git. Existem menos ferramentas e integrações de terceiros disponíveis para o Mercurial em comparação com o Git.

-   **Git:** Desfruta de uma comunidade massiva e um ecossistema vasto de ferramentas, integrações e recursos de aprendizado. É o sistema de controle de versão mais amplamente utilizado atualmente, o que significa que há mais suporte e recursos disponíveis.

### Tabela Comparativa

| Característica         | Mercurial                                     | Git                                              |
| :--------------------- | :-------------------------------------------- | :----------------------------------------------- |
| **Filosofia**          | Simplicidade, consistência, histórico imutável | Flexibilidade, poder, histórico mutável          |
| **Curva de Aprendizado** | Mais suave, intuitiva                         | Mais íngreme, poderosa                           |
| **Ramificação**        | Ramificações permanentes no histórico         | Ramificações leves, fáceis de criar/excluir      |
| **Reescrita de Histórico** | Desencorajada (`backout` para reverter)     | Encorajada (`rebase`, `amend` para limpar)       |
| **Performance**        | Bom para grandes repositórios                 | Excelente para repositórios médios, desafiador para muito grandes |
| **Comunidade/Ecossistema** | Menor, mas ativa                              | Grande, vasto ecossistema                        |

Em resumo, a escolha entre Mercurial e Git muitas vezes se resume às preferências da equipe e aos requisitos específicos do projeto. Se a simplicidade, a consistência e um histórico de projeto mais linear são prioridades, o Mercurial pode ser uma excelente escolha. Se a flexibilidade máxima, a capacidade de reescrever o histórico e um ecossistema robusto são mais importantes, o Git pode ser mais adequado.





## Exemplos de Código

Para ilustrar o uso dos comandos do Mercurial, apresentamos alguns cenários comuns com seus respectivos exemplos de código.

### Cenário 1: Iniciando um novo projeto

Você acabou de criar um novo diretório para o seu projeto e deseja começar a versioná-lo com Mercurial.

```bash
mkdir meu_novo_projeto
cd meu_novo_projeto
hg init
echo "# Meu Novo Projeto" > README.md
echo "/venv" > .hgignore
hg add README.md .hgignore
hg commit -m "Primeiro commit: inicialização do projeto e adição de README e .hgignore"
```

Neste exemplo:
1.  Criamos um novo diretório `meu_novo_projeto`.
2.  Entramos no diretório.
3.  Inicializamos o repositório Mercurial com `hg init`.
4.  Criamos um arquivo `README.md`.
5.  Criamos um arquivo `.hgignore` para ignorar o diretório `venv` (ambiente virtual Python, por exemplo).
6.  Adicionamos os arquivos `README.md` e `.hgignore` ao controle de versão com `hg add`.
7.  Registramos as alterações com `hg commit`.

### Cenário 2: Clonando um repositório e fazendo alterações

Você precisa trabalhar em um projeto existente que está hospedado em um repositório remoto.

```bash
hg clone https://example.com/projeto_existente
cd projeto_existente
hg pull
hg update

# Faça suas alterações no código, por exemplo, editando um arquivo
echo "Nova linha adicionada." >> src/main.py

hg status
hg add src/main.py # Se for um arquivo novo
hg commit -m "Adicionada nova funcionalidade ao main.py"
hg push
```

Neste exemplo:
1.  Clonamos o repositório remoto `projeto_existente`.
2.  Entramos no diretório clonado.
3.  Puxamos as últimas alterações (embora em um clone recém-feito, isso pode não ser estritamente necessário, é uma boa prática).
4.  Atualizamos o diretório de trabalho com `hg update`.
5.  Modificamos um arquivo existente (`src/main.py`).
6.  Verificamos o status para ver as alterações.
7.  Adicionamos o arquivo modificado (ou novo) ao controle de versão.
8.  Comitamos as alterações.
9.  Enviamos as alterações para o repositório remoto.

### Cenário 3: Criando e fundindo uma ramificação de funcionalidade

Você precisa desenvolver uma nova funcionalidade sem interferir na linha principal de desenvolvimento.

```bash
hg update default # Certifique-se de estar na ramificação principal
hg pull
hg update

hg branch feature/nova_funcionalidade

# Faça o desenvolvimento da nova funcionalidade
echo "def nova_funcao(): pass" > src/feature.py
hg add src/feature.py
hg commit -m "Implementada nova_funcao na ramificação feature/nova_funcionalidade"

# Volte para a ramificação principal para fundir
hg update default
hg merge feature/nova_funcionalidade

# Resolva conflitos se houver

hg commit -m "Fundida feature/nova_funcionalidade na default"
hg push
```

Neste exemplo:
1.  Garantimos que estamos na ramificação `default` (principal) e atualizados.
2.  Criamos uma nova ramificação chamada `feature/nova_funcionalidade`.
3.  Desenvolvemos a funcionalidade, criando um novo arquivo `src/feature.py`.
4.  Adicionamos e comitamos as alterações na ramificação da funcionalidade.
5.  Voltamos para a ramificação `default`.
6.  Fundimos a ramificação `feature/nova_funcionalidade` na `default`.
7.  Comitamos o resultado da fusão.
8.  Enviamos as alterações para o repositório remoto.

Estes exemplos demonstram o fluxo básico de trabalho com Mercurial, cobrindo desde a inicialização de um projeto até a colaboração e o gerenciamento de ramificações.





## Referências

[1] Perforce Software. *Mercurial vs. Git: How Are They Different?*. Disponível em: [https://www.perforce.com/blog/vcs/mercurial-vs-git-how-are-they-different](https://www.perforce.com/blog/vcs/mercurial-vs-git-how-are-they-different)

- Mercurial SCM. *Tutorial*. Disponível em: [https://www.mercurial-scm.org/wiki/Tutorial](https://www.mercurial-scm.org/wiki/Tutorial)
- Mercurial SCM. *Learn Mercurial*. Disponível em: [https://mercurial-scm.org/learn.html](https://mercurial-scm.org/learn.html)
- Joel Spolsky. *HgInit: a Mercurial tutorial*. Disponível em: [https://hginit.github.io/](https://hginit.github.io/)
- GeeksforGeeks. *Introduction to Mercurial*. Disponível em: [https://www.geeksforgeeks.org/software-engineering/introduction-to-mercurial/](https://www.geeksforgeeks.org/software-engineering/introduction-to-mercurial/)
- Mark Gates. *Mercurial Tutorial*. Disponível em: [https://icl.utk.edu/~mgates3/docs/mercurial-tutorial.pdf](https://icl.utk.edu/~mgates3/docs/mercurial-tutorial.pdf)



