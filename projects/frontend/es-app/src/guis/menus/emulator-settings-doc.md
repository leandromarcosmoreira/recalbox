# Introdução

Este documento explica o funcionamento dos novos "configurações de emuladores" acessíveis diretamente no ES.

O objetivo dessas configurações é permitir aos usuários atuar nas opções que impactam os emuladores. Não se trata de colocar todas as opções de um core RetroArch no ES.
Por outro lado, permitir alterar o idioma no Dolphin, por exemplo, ou permitir alterar a máquina emulada no emulador Atari 16/32 bits trará um grande benefício e permitirá que aqueles que não se sentem à vontade com o RetroArch ou as opções nos emuladores standalone possam fazê-lo com facilidade.
Funciona em praticamente todos os emuladores Libretro ou standalone. Apenas os raros emuladores com arquivos de configuração em formato incomum não poderão ser endereçados por enquanto.
Essas configurações permitem modificar diretamente os arquivos de configuração dos emuladores. Portanto, é necessário tomar cuidado para que as opções que você propuser não sejam suportadas pelo configgen, caso contrário, o que o usuário modificar será sobrescrito pelo configgen.

Essas configurações são ditas "declarativas", ou seja, não estão codificadas no ES. Elas são declaradas e definidas em um XML e interpretadas pelo ES.

***Todos*** podem adicioná-las (pelo menos nas betas), e conto com vocês para nos ajudar ;)

# Funcionamento interno e edição

Essas configurações são declaradas no XML dos menus, em um menu especial. Para o ES, são entradas de menu padrão sobre as quais ele deve executar operações especiais quando o menu é criado e quando uma entrada é alterada.

O XML de declaração dos menus está incorporado no ES e não pode ser modificado, mas... preparamos atalhos de teclado (você precisará de um) para gerar um arquivo real que será modificável e carregado prioritariamente pelo ES:
- **ALT+F8**: Modifica o arquivo XML no local `/recalbox/share/system/.emulationstation/menu.xml` (ou `\\RECALBOX\system\.emulationstation\menu.xml`). Se o arquivo já existir, pedirá confirmação para sobrescrever, portanto não há risco de perder suas configurações inadvertidamente.
- **ALT+F7**: Recarrega os menus. Não deve haver nenhum menu ou janela (bios/Scrape, ...) na tela. Se houver, será solicitado que você feche tudo antes de recomeçar.

Esses atalhos funcionam **apenas nas alphas**.

O arquivo menu.xml é rigorosamente controlado pelo ES e qualquer malformação resultará em log + parada imediata do ES. Portanto, é aconselhável executá-lo:
- No PC: `killall -9 emulationstation` e depois `emulationstation --debug`. Você pode usar CTRL+C para pará-lo se necessário.
- No Pi: `killall -9 emulationstation` ou `es stop`, e depois o mesmo que no PC.

Executando-o no console e em modo debug, você terá os logs de recarregamento de menus, erros e avisos, se houver.

***Disponível a partir da 10 alpha 18!***

# Edição e sintaxe

A área que nos interessa está no início do arquivo:
```xml
	<!--
		CONFIGURAÇÕES ESPECÍFICAS DO EMULADOR
	-->
	<menu id="EmulatorSpecificSettings">
		<menu id="auto" caption="Configurações Libretro HatariB" include="true">
			<setting caption="Tipo de Máquina" core="libretro:hatari" file="$configs/retroarch/cores/retroarch-core-options.cfg" fileType="ini" key="hatari_machine" type="list" props="d" values="0:ST|1:Mega ST|2:STE|3:Mega STE|4:TT|5:Falcon" default="0" help="Escolha o modelo de máquina a emular. STE é uma boa escolha para a maioria dos jogos." />
			...
		</menu>
	</menu>
```

## Modificar opções ou adicionar

Se você quiser adicionar uma opção ou modificar uma opção existente, localize o menu que interessa pelo nome do emulador no `caption`.
Se você quiser adicionar um conjunto de opções inexistentes para um core/emulador, comece adicionando uma entrada de menu assim:
```xml
	<menu id="auto" caption="Emulador PS12 Standalone" include="true">
	</menu>
```
O id é sempre `auto` e a propriedade `include` é necessária para que o menu possa ser incluído no menu avançado de um emulador, caso contrário gerará um submenu deselegante. O nome em `caption` será usado para criar um cabeçalho de separação.

Em seguida, adicione uma ou mais entradas `<setting ... />`, uma entrada por opção.

## Propriedades

- `caption`: [obrigatório] texto da entrada que aparecerá no menu. Deve ser curto, conciso e em inglês (poderá ser traduzido como os outros textos dos menus).
- `help`: [opcional] texto de ajuda quando o usuário pressionar Y na entrada do menu.
- `core`: [obrigatório] Indica qual emulador ou core é o alvo.
- `file`: [obrigatório] Especifica o arquivo de configuração a modificar.
- `fileType`: [obrigatório] Especifica o tipo de arquivo (ini, sini).
- `key`: [obrigatório] Chave a ser modificada.
- `type`: [obrigatório] Tipo de valores (list, nbool, ybool, tbool, cbool, range).
- `values`: Obrigatório para listas e ranges.
- `default`: Valor padrão.

### Atalhos na propriedade `file`

- `$home` = `/recalbox/share/system`
- `$bios` = `/recalbox/share/bios`
- `$saves` = `/recalbox/share/saves`
- `$configs` = `/recalbox/share/system/configs`
- `$.config` = `/recalbox/share/system/.config`

### Solução de problemas

P: O que fazer se minha configuração não aparecer?
R: Comece verificando o conteúdo de `core`. Se estiver correto, verifique os logs - você provavelmente cometeu um erro e a configuração é inválida.
