# recalbox-hardware

## Gerenciamento de hardware

O objetivo da biblioteca é detectar e/ou configurar automaticamente os hardwares conectados às diferentes placas.
Os scripts Python principais *devem* ser rápidos. Por favor, escreva código curto e rápido. Use carregamento dinâmico de módulos sempre que for possível/eficiente.

### Cases

Aqui está a parte que detecta e/ou configura automaticamente cases com peças de hardware, começando com o GPi.

Compilar:
```
python -m compilall .
```

Como iniciar:
```
python case/manage.py "<casename>" <install> <machine> <phase>
com:
- casename : Case para instalar/desinstalar. Vazio para autodetectar.
- install  : 1 = instalar, 0 = desinstalar
- machine  : 1 = solicitação da máquina, 0 = solicitação do usuário
- phase    : 0 = instalar hardware, 1 = instalar software
```
