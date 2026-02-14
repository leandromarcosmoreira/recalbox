# Recalbox

![Recalbox Logo](https://s3-eu-west-1.amazonaws.com/forums.recalbox.com/8d81e556-aefb-4729-ad2f-9d7386eff4cd.png)

[![GitHub Releases](https://img.shields.io/github/v/release/leandromarcosmoreira/recalbox?include_prereleases&label=Release)](https://github.com/leandromarcosmoreira/recalbox/releases)
[![GitHub Actions Workflow](https://github.com/leandromarcosmoreira/recalbox/actions/workflows/build-recalbox.yml/badge.svg)](https://github.com/leandromarcosmoreira/recalbox/actions)
[![GitHub Pages](https://img.shields.io/badge/Downloads-GitHub%20Pages-purple)](https://leandromarcosmoreira.github.io/recalbox/)

> **Nota**: Este repositório é um fork de [https://github.com/recalbox/recalbox-os](https://github.com/recalbox/recalbox-os).

Bem-vindo ao repositório principal do Recalbox.

Este repositório contém o código-fonte que compila o Recalbox para diferentes placas.

O Recalbox é um projeto open source. Esperamos que você contribua e nos ajude a melhorar este sistema operacional.
Porém, se você estiver trabalhando em um fork, por respeito ao nosso trabalho, pedimos que não integre nosso trabalho em andamento localizado em branches diferentes da master.
Obrigado por aguardar um merge na branch master.

## Download de Builds Prontos

### Builds Automáticos via GitHub Actions

Este repositório possui pipelines de CI/CD que compilam automaticamente o Recalbox para múltiplas arquiteturas:

| Arquitetura | Hardware | Status |
|-------------|----------|--------|
| `x86_64` | PC (Intel/AMD) | ✅ |
| `rpi5_64` | Raspberry Pi 5 | ✅ |
| `rpi4_64` | Raspberry Pi 4 | ✅ |
| `rpi3` | Raspberry Pi 3B/B+ | ✅ |
| `rpizero2` | Raspberry Pi Zero 2 W | ✅ |
| `rg353x` | Anbernic RG353X/V | ✅ |
| `odroidgo2` | Odroid Go Advance/Black | ✅ |

#### Como Baixar

1. Acesse a página de downloads: **[leandromarcosmoreira.github.io/recalbox/](https://leandromarcosmoreira.github.io/recalbox/)**
2. Selecione seu hardware
3. Baixe a imagem mais recente

#### ou via GitHub Releases

Acesse a página de [Releases](https://github.com/leandromarcosmoreira/recalbox/releases) para baixar imagens diretamente.

## Configurações Especiais

### Recalbox Business (4K)

Este repositório inclui configurações especiais para:

- **Recalbox Business 4K**: Versão otimizada para TVs 4K com resolução superior
- **Configurações Premium**: Inclui pacotes adicionais para uso comercial

As configurações estão disponíveis em `configs/recalbox-*_defconfig`:

| Arquitetura | Defconfig Normal | Defconfig Business |
|-------------|------------------|-------------------|
| x86_64 | recalbox-x86_64_defconfig | recalbox-x86_64_business_defconfig |
| rpi5_64 | recalbox-rpi5_64_defconfig | recalbox-rpi5_64_business_defconfig |
| rpi4_64 | recalbox-rpi4_64_defconfig | recalbox-rpi4_64_business_defconfig |
| rpi3 | recalbox-rpi3_defconfig | recalbox-rpi3_business_defconfig |

### Compilar versão Business

```bash
export ARCH="rpi5_64"
./scripts/linux/recaldocker.sh make "recalbox-${ARCH}_business_defconfig"
./scripts/linux/recaldocker.sh
```

## Arquiteturas Suportadas

- **x86_64**: PCs e notebooks Intel/AMD
- **rpi5_64**: Raspberry Pi 5 (8GB recomendado)
- **rpi4_64**: Raspberry Pi 4 (4GB ou 8GB)
- **rpi3**: Raspberry Pi 3B/B+
- **rpizero2**: Raspberry Pi Zero 2 W
- **rg353x**: Anbernic RG353X/V
- **odroidgo2**: Odroid Go Advance/Black

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
ARCH="rpi5_64" && git clone --recurse-submodules <sua-url-do-repo> recalbox-${ARCH}
```

**Compile** o Recalbox:

```bash
export ARCH="rpi5_64"
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
ARCH=rpi5_64 && ./scripts/linux/recaldocker.sh make menuconfig
```

Dessa forma, você pode executar o menuconfig para configurar o sistema. Se você nunca compilou o sistema, use o seguinte comando para criar a configuração padrão para sua placa:
```bash
ARCH=rpi5_64 && ./scripts/linux/recaldocker.sh make "recalbox-${ARCH}_defconfig" && make menuconfig
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
