# ROMFS v2

Requisitos: Python 3.8+

## Requisitos:

Você precisa executar primeiro este comando na raiz do repositório Recalbox para gerar o arquivo `.config` em `./output/target`:

`./scripts/linux/recaldocker.sh`

## Criar novo sistema

Use este recurso para criar um novo sistema no romfs.

Uso: `python3 romfs.py -systems <pasta raiz dos sistemas> -create <nome curto do novo sistema>`

Exemplo:
- `python3 romfs.py -systems ../systems -create snes`
- `python3 ~/recalbox/packages/recalbox-romfs2/scripts/romfs.py -systems ~/recalbox/packages/recalbox-romfs2/systems -create snes`

Isso criará uma nova pasta em `recalbox-romfs2/systems` com uma subpasta `roms` e um arquivo `system.ini`.
A subpasta `roms` contém arquivos readme e jogos pré-instalados opcionais.
O `system.ini` é o descritor do sistema: edite este arquivo adequadamente.

Quando jogos pré-instalados são copiados para o alvo, o conteúdo de `systems/<sistema>/roms` é copiado para `/recalbox/share_init/roms/<sistema>`

## Gerar descritor do sistema

Gere o descritor de sistema final `systemlist.xml` no alvo `/recalbox/share_init/system/.emulationstation/` a partir de todos os `system.ini`

Uso: `python3 romfs.py -systems <pasta raiz dos sistemas> -buildsystems <caminho do arquivo de destino>`

Exemplo:
- `python3 romfs.py -systems ../systems -buildsystems ../output.xml`
- `python3 ~/recalbox/packages/recalbox-romfs2/scripts/romfs.py -systems ~/recalbox/packages/recalbox-romfs2/systems -buildsystems ~/recalbox/output/target/recalbox/share_init/system/.emulationstation/systemlist.xml`

## Copiar para o alvo

Popule a pasta final `share_init/roms` com todos os jogos pré-instalados e todos os arquivos readme.

Uso: `python3 romfs.py -systems <pasta raiz dos sistemas> -installroms <caminho de destino das roms>`

Exemplo:
- `python3 romfs.py -systems ../systems -installroms ~/recalbox/output/target/recalbox/share_init/roms`
- `python3 ~/recalbox/packages/recalbox-romfs2/scripts/romfs.py -systems ~/recalbox/packages/recalbox-romfs2/systems -installroms ~/recalbox/output/target/recalbox/share_init/roms`

| Tabelas   |      São      |  Legal |
|----------|:-------------:|------:|
| col 1 é  |  alinhada à esquerda | $1600 |
| col 2 é  |    centralizada   |   $12 |
| col 3 é  | alinhada à direita |    $1 |
