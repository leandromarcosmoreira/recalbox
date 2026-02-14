recalbox
=======
![](https://s3-eu-west-1.amazonaws.com/forums.recalbox.com/8d81e556-aefb-4729-ad2f-9d7386eff4cd.png)

> **Nota**: Este repositório é um fork de [https://github.com/recalbox/recalbox-os](https://github.com/recalbox/recalbox-os).

Bem-vindo ao repositório principal do Recalbox.

Este repositório contém o código-fonte que compila o Recalbox para diferentes placas.

O Recalbox é um projeto open source. Esperamos que você contribua e nos ajude a melhorar este sistema operacional.
Porém, se você estiver trabalhando em um fork, por respeito ao nosso trabalho, pedimos que não integre nosso trabalho em andamento localizado em branches diferentes da master.
Obrigado por aguardar um merge na branch master.

NOVO: integramos projetos externos neste repositório. O Recalbox agora é um monorepo. Isso significa:
- você pode fazer alterações em todas as partes do Recalbox (buildroot, frontend, configgen...) em uma única alteração
- o pipeline agora testa cada projeto antes da compilação
- referências de commits não são mais necessárias em `packages.mk`

Por favor, use o rastreador de issues deste repositório para reportar bugs ou solicitar recursos.

## Projetos Recalbox
- recalbox: este repositório contém o código-fonte para compilar o Recalbox.
- [emulationstation](./projects/frontend/): o frontend EmulationStation para Recalbox.
- [configgen](./projects/frontend/): a ferramenta para configuração automática de joystick.
- [recalbox-hardware](./projects/recalbox-hardware/): gerenciamento de case e diversos hardwares para Recalbox.
- [recalbox-rgb-dual](./projects/recalbox-rgb-dual/): Projeto Recalbox RGB Dual com DT e módulo do kernel.
- [wpaf](./projects/wpaf/): gerenciador de ventilador.
- [piboy](./projects/piboy/): detector PiBoy DMG.


## Links úteis
- [www.recalbox.com](https://www.recalbox.com): o site oficial do Recalbox.
- [Wiki do Recalbox](https://wiki.recalbox.com): a wiki do Recalbox.
- [forum.recalbox.com](https://forum.recalbox.com): fórum do Recalbox. Você encontrará suporte lá.


# Como compilar (apenas Linux)

### Passos gerais

Instale o Docker: [docs.docker.com/install/](https://docs.docker.com/install/)

Certifique-se de que seu usuário pertence ao grupo docker -> `sudo usermod -a -G docker $USER` e então faça logout/login

Clone o repositório (substitua `<sua-url-do-repo>` adequadamente):

```bash
ARCH="rpi4_64" && git clone --recurse-submodules <sua-url-do-repo> recalbox-${ARCH}
```

**Compile** o Recalbox:

```bash
export ARCH="rpi4_64"
cd recalbox-${ARCH}
./scripts/linux/recaldocker.sh
```

O tempo de compilação depende do poder do processador do seu computador e produzirá uma imagem do Recalbox para a arquitetura especificada na variável `ARCH`.

Ao final da compilação, a imagem estará disponível em `output/images/recalbox/`.

### Personalizar a compilação

Você pode definir as seguintes variáveis de ambiente para personalizar a compilação:
* `ARCH`: para forçar a arquitetura alvo (veja as arquiteturas disponíveis no [diretório `configs`](configs), o padrão é inferir pelo nome do diretório atual)
* `RECALBOX_VERSION`: para definir a versão da compilação do Recalbox (sem impacto na compilação, apenas a versão exibida)
* `PACKAGE`: se você quiser compilar um único pacote

No diretório recalbox, você encontrará alguns diretórios criados pela compilação:
* pasta `host` que contém a saída compilada para seu host
* pasta `dl` que contém todos os pacotes baixados
* pasta `output` que contém os arquivos compilados

### Comando personalizado e menuconfig

Usando os argumentos da linha de comando, você pode passar um comando personalizado para executar:
```bash
ARCH=rpi4_64 && ./scripts/linux/recaldocker.sh make menuconfig
```

Dessa forma, você pode executar o menuconfig para configurar o sistema. Se você nunca compilou o sistema, use o seguinte comando para criar a configuração padrão para sua placa:
```bash
ARCH=rpi4_64 && ./scripts/linux/recaldocker.sh make "recalbox-${ARCH}_defconfig" && make menuconfig
```

Seu comando substituirá o comando de compilação padrão da imagem Docker, então você pode precisar copiar e colar algumas variáveis dela.

### Problemas conhecidos

O processo de compilação Docker funciona apenas em `linux`.

Durante a compilação da imagem, se você encontrar erros como os seguintes:

```text
Reading package lists...
W: Failed to fetch http://archive.ubuntu.com/ubuntu/dists/xenial/InRelease  Temporary failure resolving 'archive.ubuntu.com'
W: Failed to fetch http://archive.ubuntu.com/ubuntu/dists/xenial-updates/InRelease  Temporary failure resolving 'archive.ubuntu.com'
W: Failed to fetch http://archive.ubuntu.com/ubuntu/dists/xenial-backports/InRelease  Temporary failure resolving 'archive.ubuntu.com'
W: Failed to fetch http://security.ubuntu.com/ubuntu/dists/xenial-security/InRelease  Temporary failure resolving 'security.ubuntu.com'
W: Some index files failed to download. They have been ignored, or old ones used instead.
```
O Docker não consegue acessar a internet para fazer atualizações. Mais precisamente, é um problema de DNS (veja https://odino.org/cannot-connect-to-the-internet-from-your-docker-containers/). Se seu `/etc/resolv.conf` estiver vazio (isso acontece quando tudo é gerenciado pelo `network-manager`, por exemplo), o Docker não sabe qual DNS usar. Você precisará informar através do arquivo `/etc/default/docker` adicionando esta linha:

```text
DOCKER_OPTS="--dns IP.DO.SEU.DNS"
```
Reinicie o Docker e tente novamente. Deve funcionar.

```bash
sudo service docker restart
docker build -t "recalbox-dev" .
```
test
