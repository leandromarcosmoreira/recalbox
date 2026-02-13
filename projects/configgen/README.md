# recalbox-configgen
## Ferramentas de configuração de emuladores para RecalboxOS

*Em desenvolvimento*

O objetivo da biblioteca é configurar automaticamente os emuladores, usando argumentos de linha de comando e variáveis de ambiente disponíveis no Recalbox.

Como testar:
```
python -m runtest discover -p "*test*"
```

Como iniciar:
```
python configgen/emulatorlauncher.py  -system neogeo -rom myrom.zip
```
