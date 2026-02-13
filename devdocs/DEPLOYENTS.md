# Implantações

## Processo

### Processo de compilação e lançamento
* O **estágio de build** criará arquivos de distribuição (`tar.gz` e `.img`) e os definirá como `artefatos de build`
* Os artefatos de build são salvos pelo GitLab por um mês
* O **estágio de release** do CI baixará todos os artefatos de build e preparará pastas e arquivos HTML para disponibilização nos servidores de implantação (armazenamento de objetos Scaleway para releases e Netlify para a página de índice de archive.recalbox.com)
* O `index.html` é processado para exibir as variáveis de build

#### Especificidade de revisão
* Os builds são manuais
* O **estágio de release** da branch preparará e implantará a nova versão no bucket de revisões https://recalbox-reviews.s3.nl-ams.scw.cloud/review-XXXX)
* Nenhum `.img` é implantado, apenas `.tar.xz`


#### Especificidade de produção
* Os builds de `master` e `tags` são automáticos
* Uma implantação manual da master é recomendada para testar a versão
* Uma tag pode ser criada na `master` e levará a um novo build
* O **estágio de release** da tag preparará e implantará a nova versão no bucket de releases https://recalbox-releases.s3.nl-ams.scw.cloud/stable), e alterará o arquivo https://archive.recalbox.com no Netlify.
