## volumed

Controle de volume pela roda.

Este programa escuta eventos ABS_VOLUME em um dispositivo de entrada.
Ele alterará o volume padrão do sink pelo valor do evento.

## Requisitos

Você precisará do PulseAudio para compilar este software.

## Compilação

```
autoreconf -i
./configure && make && make install
```

## Uso

Execute:

```
volumed /dev/input/inputX # adapte X para o dispositivo de entrada desejado
```

Use a roda de volume para alterar o volume da saída de áudio.

