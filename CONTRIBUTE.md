# Como contribuir com o projeto Recalbox

1. Crie uma Merge Request com suas alterações.
   Cada correção/recurso é desenvolvido em sua própria branch e associado a uma Merge Request.
2. Verifique se os **linters** do pipeline passam com sucesso.
   Os linters podem ser executados primeiro localmente executando `./run_linters` na raiz do projeto.
   O processo de linting consiste em verificar arquivos Buildroot para erros de sintaxe, recursos obsoletos, ...
   Também verifica arquivos de configuração.
   E finalmente, testa suas mensagens de commit que devem seguir os Conventional Commits (https://www.conventionalcommits.org/pt-br/v1.0.0/)
3. Envie sua branch e faça rebase contra master o mais frequentemente possível.
4. Proponha uma merge request
   A merge request é rotulada com ~Testing::Beta pela equipe Recalbox. Este rótulo indica incorporação na próxima beta.
5. Uma vez testada pela equipe, a merge request pode ser incorporada (apenas fast-forward, então fazer rebase frequentemente é importante).
   Não esqueça de inserir uma entrada no `RELEASE-NOTES.md`.
