Recalbox EmulationStation
=========================

Este frontend está incluído como parte do monorepo do Recalbox e foi
adaptado especificamente para o Recalbox.

Abaixo desta linha está o readme original do EmulationStation.

EmulationStation
================

Uma interface gráfica multiplataforma para emuladores com navegação por controle.

Website do projeto: http://emulationstation.org

**Usuários de Raspberry Pi:**
Um cara legal chamado petrockblog fez um script que instala automaticamente muitos emuladores e o ES. Ele também inclui opções para configurar seu RPi e configurá-lo para iniciar diretamente no ES. Você pode encontrar aqui: https://github.com/petrockblog/RetroPie-Setup

Download
========

Baixe uma versão pré-compilada em [emulationstation.org](http://emulationstation.org#download).

Encontrei um bug! Tenho um problema!
====================================

- Primeiro, tente verificar a [lista de issues](https://github.com/Aloshi/EmulationStation/issues?state=open) para algumas entradas que possam corresponder ao seu problema. Certifique-se de verificar issues fechadas também!

- Se você estiver executando o EmulationStation em um Raspberry Pi e tiver problemas com alterações no arquivo de configuração que não têm efeito, conteúdo ausente após editar, etc., verifique se seu cartão SD está corrompido (veja as issues [#78](https://github.com/Aloshi/EmulationStation/issues/78) e [#107](https://github.com/Aloshi/EmulationStation/issues/107)). Você pode fazer isso com ferramentas gratuitas como [h2testw](http://www.heise.de/download/h2testw.html) ou [F3](http://oss.digirati.com.br/f3/).

- Tente atualizar para a versão mais recente do EmulationStation usando git (você pode precisar excluir seu `es_input.cfg` e `es_settings.cfg` depois para redefini-los para os valores padrão):
```bash
cd SeuDiretorioDoEmulationStation
git pull
cmake .
make
```

- Se seu problema ainda não foi resolvido, a melhor maneira de relatar um bug é postar uma issue no GitHub. Tente postar os passos mais simples possíveis para reproduzir o bug. Inclua arquivos que você acha que podem estar relacionados (exceto ROMs, claro). Se você não executou o ES desde o crash, o arquivo de log `~/logs/es_log.txt` também é útil. Caso contrário, `~/logs/es_log.txt.backup` contém logs da sessão anterior do es.

Compilação
==========

O EmulationStation usa algum código C++, o que significa que você precisará usar pelo menos g++-4.7 no Linux, ou VS2010 no Windows, para compilar.

O EmulationStation tem algumas dependências. Para compilar, você precisará do CMake, SDL2, Boost (System, Filesystem, DateTime, Locale), FreeImage, FreeType, Eigen3, cURL e Liblzma. Você também provavelmente deve instalar o pacote `fonts-droid` que contém fontes de fallback para caracteres chineses/japoneses/coreanos, mas o ES funcionará bem sem ele (este pacote é usado apenas em tempo de execução).

**No Debian/Ubuntu:**
Tudo isso pode ser facilmente instalado com apt-get:
```bash
sudo apt-get install libavfilter-dev libavdevice-dev libprocps-dev libsdl2-dev libsdl2-mixer-dev libfreeimage-dev libfreetype6-dev libcurl4-openssl-dev libasound2-dev libgl1-mesa-dev libzip-dev build-essential cmake fonts-droid liblzma-dev
```

Então, gere e compile o Makefile com CMake:
```bash
cd SeuDiretorioDoEmulationStation
cmake .
make
```

**No Raspberry Pi:**

Instruções completas de compilação para Raspberry Pi em [emulationstation.org](http://emulationstation.org/gettingstarted.html#install_rpi_standalone).

**No Windows:**

[FreeImage](http://downloads.sourceforge.net/freeimage/FreeImage3154Win32.zip)

[FreeType2](http://download.savannah.gnu.org/releases/freetype/freetype-2.4.9.tar.bz2) (você precisará compilar)

[SDL2](http://www.libsdl.org/release/SDL2-devel-2.0.3-VC.zip)

[cURL](http://curl.haxx.se/download.html) (você precisará compilar ou obter a versão DLL pré-compilada)

(Lembre-se de copiar os arquivos .DLL necessários para a mesma pasta do executável: provavelmente FreeImage.dll, freetype6.dll, SDL2.dll, libcurl.dll e zlib1.dll. A lista exata depende de você ter compilado suas bibliotecas em modo "estático" ou não.)

[CMake](http://www.cmake.org/cmake/resources/software.html) (isso é usado para gerar o projeto do Visual Studio)

(Se você não sabe como usar o CMake, aqui estão algumas dicas: execute o cmake-gui e aponte-o para sua pasta do EmulationStation. Aponte o diretório "build" para algum lugar - eu uso EmulationStation/build. Clique em configure, escolha "Visual Studio [ano] Project", preencha os campos vermelhos conforme aparecem e continue clicando em Configure (você pode precisar marcar "Advanced"), então clique em Generate.)

Configuração
============

**~/.emulationstation/es_systems.cfg:**
Na primeira execução, um arquivo de exemplo de configuração de sistemas será criado em `~/.emulationstation/es_systems.cfg`. `~` é `$HOME` no Linux, e `%HOMEPATH%` no Windows. Este exemplo tem alguns comentários explicando como escrever o arquivo de configuração. Veja a seção "Escrevendo um es_systems.cfg" para mais informações.

**Lembre-se de que você terá que configurar seu emulador separadamente do EmulationStation!**

**~/.emulationstation/es_input.cfg:**
Quando você iniciar o EmulationStation pela primeira vez, será solicitado a configurar um dispositivo de entrada. O processo é o seguinte:

1. Segure um botão no dispositivo que deseja configurar. Isso inclui o teclado.

2. Pressione os botões conforme aparecem na lista. Algumas entradas podem ser ignoradas segurando qualquer botão por alguns segundos (por exemplo, page up/page down).

3. Você pode revisar seus mapeamentos pressionando para cima e para baixo, fazendo alterações pressionando A.

4. Escolha "SALVAR" para salvar este dispositivo e fechar a tela de configuração de entrada.

A nova configuração será adicionada ao arquivo `~/.emulationstation/es_input.cfg`.

**Ambos os dispositivos novos e antigos podem ser (re)configurados a qualquer momento pressionando o botão Iniciar e escolhendo "CONFIGURAR ENTRADA".** A partir daqui, você pode desconectar o dispositivo que usou para abrir o menu e conectar um novo, se necessário. Novos dispositivos serão anexados ao arquivo de configuração de entrada existente, então seus dispositivos antigos permanecerão configurados.

**Se seu controle parar de funcionar, você pode excluir o arquivo `~/.emulationstation/es_input.cfg` para fazer a tela de configuração de entrada reaparecer na próxima execução.**

Você pode usar `--help` ou `-h` para ver uma lista de opções de linha de comando. Resumidamente descritas aqui:
```
--resolution [largura] [altura]	- tentar forçar uma resolução específica
--gamelist-only		- exibir apenas jogos definidos em um arquivo gamelist.xml
--ignore-gamelist	- não analisar nenhum arquivo gamelist.xml
--draw-framerate	- desenhar a taxa de quadros
--no-exit		- não exibir 'sair' no menu do ES
--debug			- mostrar a janela do console no Windows, fazer um pouco mais de日志
--windowed	- executar o ES em uma janela, funciona melhor em conjunto com --resolution [l] [a]
--vsync [1/on ou 0/off]	- ativar ou desativar o vsync (o padrão é ativado)
--scrape	- executar o interativo scraper de metadados de linha de comando
```

Desde que o ES não tenha travado, você sempre pode pressionar F4 para fechar o aplicativo.

Escrevendo um es_systems.cfg
=============================

Instruções completas de configuração em [emulationstation.org](http://emulationstation.org/gettingstarted.html#config).

O arquivo `es_systems.cfg` contém os dados de configuração do sistema para o EmulationStation, escritos em XML. Isso informa ao EmulationStation quais sistemas você tem, a qual plataforma eles correspondem (para scraping) e onde os jogos estão localizados.

O ES verificará dois lugares para um arquivo es_systems.cfg, na seguinte ordem, parando após encontrar um que funcione:
* `~/.emulationstation/es_systems.cfg`
* `/etc/emulationstation/es_systems.cfg`

A ordem em que o EmulationStation exibe os sistemas reflete a ordem em que você os define.

**NOTA:** Um sistema *deve* ter pelo menos um jogo presente em seu diretório "path", ou o ES o ignorará! Se nenhum sistema válido for encontrado, o ES relatará um erro e sairá!

Aqui está um exemplo de es_systems.cfg:

```xml
<!-- Este é o arquivo de configuração de sistemas do EmulationStation.
Todos os sistemas devem estar contidos dentro da tag <systemList>. -->

<systemList>
	<!-- Aqui está um exemplo de sistema para você começar. -->
	<system>
		<!-- Um nome curto, usado internamente. -->
		<name>snes</name>

		<!-- Um nome "bonito", exibido nos menus e tal. Este é opcional. -->
		<fullname>Super Nintendo Entertainment System</fullname>

		<!-- O caminho para começar a procurar ROMs. '~' será expandido para $HOME ou %HOMEPATH%, dependendo da plataforma.
		Todos os subdiretórios (e links não recursivos) serão incluídos. -->
		<path>~/roms/snes</path>

		<!-- Uma lista de extensões para procurar, delimitada por qualquer dos caracteres de espaço em branco (", \r\n\t").
		Você DEVE incluir o ponto no início da extensão! Também diferencia maiúsculas de minúsculas. -->
		<extension>.smc .sfc .SMC .SFC</extension>

		<!-- O comando shell executado quando um jogo é selecionado. Algumas tags especiais são substituídas se encontradas em um comando, como %ROM% (veja abaixo). -->
		<command>snesemulator %ROM%</command>
		<!-- Este exemplo executaria o comando bash "snesemulator /home/user/roms/snes/Super\ Mario\ World.sfc". -->

		<!-- A(s) plataforma(s) a ser usada(m) ao fazer scraping. Você pode ver a lista completa de plataformas aceitas em src/PlatformIds.cpp.
		É sensível a maiúsculas/minúsculas, mas tudo é minúsculo. Esta tag é opcional.
		Você também pode usar múltiplas plataformas, delimitadas com qualquer dos caracteres de espaço em branco (", \r\n\t"), por exemplo: "genesis, megadrive" -->
		<platform>snes</platform>

		<!-- O tema a ser carregado do conjunto de temas atual. Veja THEMES.md para mais informações.
		Esta tag é opcional; se não for definida, usará o valor de <name>. -->
		<theme>snes</theme>
	</system>
</systemList>
```

As seguintes "tags" são substituídas pelo ES nos comandos de inicialização:

`%ROM%` - Substituído pelo caminho absoluto para a ROM selecionada, com a maioria dos caracteres especiais do Bash escapados com uma barra invertida.

`%BASENAME%` - Substituído pelo nome "base" do caminho para a ROM selecionada. Por exemplo, um caminho de "/foo/bar.rom", esta tag seria "bar". Esta tag é útil para configurar o AdvanceMAME.

`%ROM_RAW%` - Substituído pelo caminho absoluto, sem escape, para a ROM selecionada. Se o seu emulador for exigente sobre caminhos, você pode querer usar isso em vez de %ROM%, mas entre aspas.

Veja [SYSTEMS.md](SYSTEMS.md) para alguns exemplos reais no EmulationStation.

gamelist.xml
============

O arquivo gamelist.xml para um sistema define metadados para jogos, como nome, imagem (como uma captura de tela ou arte da caixa), descrição, data de lançamento e classificação.

Se pelo menos um jogo em um sistema tiver uma imagem especificada, o ES usará a visualização detalhada para esse sistema (que exibe metadados junto com a lista de jogos).

*Você pode usar as [ferramentas de scraping](http://en.wikipedia.org/wiki/Web_scraping) do ES para evitar criar um gamelist.xml manualmente.* Há duas maneiras de executar o scraper:

* **Se você quiser fazer scraping de vários jogos:** pressione start para abrir o menu e escolha a opção "SCRAPER". Ajuste suas configurações e pressione "SCRAPEAR AGORA".
* **Se você quiser apenas fazer scraping de um jogo:** encontre o jogo na lista de jogos no ES e pressione select. Escolha "EDITAR METADADOS DESTE JOGO" e então pressione o botão "SCRAPEAR" na parte inferior do editor de metadados.

Você também pode editar metadados dentro do ES usando o editor de metadados - basta encontrar o jogo que deseja editar na lista de jogos, pressione Select e escolha "EDITAR METADADOS DESTE JOGO".

Uma versão de linha de comando do scraper também é fornecida - basta executar o emulationstation com `--scrape` *(atualmente quebrado)*.

O switch `--ignore-gamelist` pode ser usado para ignorar a gamelist e forçar o ES a usar a visualização não detalhada.

Se você estiver escrevendo uma ferramenta para gerar ou analisar arquivos gamelist.xml, você deve verificar [GAMELISTS.md](GAMELISTS.md) para documentação mais detalhada.

Temas
=====

Por padrão, o EmulationStation parece bem feio. Você pode consertar isso. Se quiser saber mais sobre criar seus próprios temas (ou editar os existentes), leia [THEMES.md](THEMES.md)!

Coloquei alguns temas para download na minha página do EmulationStation: http://aloshi.com/emulationstation#themes

Se você estiver usando o RetroPie, você já deve ter um bom conjunto de temas instalados automaticamente!

-Alec "Aloshi" Lofquist
http://www.aloshi.com
http://www.emulationstation.org
