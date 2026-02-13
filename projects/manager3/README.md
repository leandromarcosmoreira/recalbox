![](https://s3-eu-west-1.amazonaws.com/forums.recalbox.com/8d81e556-aefb-4729-ad2f-9d7386eff4cd.png)

<a href="https://vuejs.org/"><img src="https://img.shields.io/badge/vue-3.x-brightgreen.svg?style=flat-square" alt=""/></a>
<a href="https://quasar.dev/"><img src="https://img.shields.io/npm/v/quasar.svg?label=quasar" alt=""></a>
<a href="https://quasar.dev/"><img src="https://img.shields.io/npm/v/%40quasar/app.svg?label=@quasar/app" alt=""></a>
<a href="https://quasar.dev/"><img src="https://img.shields.io/npm/v/%40quasar/cli.svg?label=@quasar/cli" alt=""></a>
<a href="https://quasar.dev/"><img src="https://img.shields.io/npm/v/%40quasar/app-webpack?label=@quasar/app-webpack" alt=""></a>
<a href="https://quasar.dev/"><img src="https://img.shields.io/npm/v/%40quasar/extras.svg?label=@quasar/extras" alt=""></a>

[![Join the chat at https://discord.com/invite/NbQFbGM](https://img.shields.io/badge/chat-on%20discord-7289da.svg)](https://discord.com/invite/NbQFbGM)
<a href="https://forum.recalbox.com/" target="_blank"><img src="https://img.shields.io/badge/community-forum-brightgreen.svg"></a>

# Recalbox Manager v3

## Instalar Quasar CLI
```bash
yarn global add @quasar/cli
# ou
npm i -g @quasar/cli
```


## Instalar as dependências
```bash
yarn
# ou
npm install

Para compilar: defina a variável de ambiente 'API_IP' na configuração do seu servidor
```

## Criar arquivo .env na raiz do projeto com seu IP de API local
```bash
API_IP=recalbox.local
```

## Iniciar o aplicativo em modo de desenvolvimento (hot-code reloading, relatório de erros, etc.)
```bash
quasar dev
```


## Verificar os arquivos
```bash
yarn lint
# ou
npm run lint
```



## Compilar o aplicativo para produção
```bash
quasar build
```

## Personalizar a configuração
Veja [Configuring quasar.config.mjs](https://v2.quasar.dev/quasar-cli-webpack/quasar-config-js).
