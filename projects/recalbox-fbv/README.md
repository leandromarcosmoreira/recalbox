1. VISÃO GERAL
    O fbv (FrameBuffer Viewer) é um programa simples para visualizar imagens em
  um dispositivo framebuffer do Linux. Em 2000, quando o fbv foi criado, não
  havia outros programas adequados para realizar a mesma tarefa, então os
  autores decidiram seguir a regra: 'Se você precisa de uma ferramenta - escreva
  você mesmo!' :-)

2. REQUISITOS
  - Linux, configurado para fornecer a interface do dispositivo framebuffer
    (/dev/fb0 ou /dev/fb/0)
  - libungif para suporte a GIF
  - libjpeg para suporte a JPEG
  - libpng para suporte a PNG

3. INSTALAÇÃO
  - descompacte o arquivo (você provavelmente já fez isso)
  - execute ./configure
  - digite: make
  - digite: make install
  - aproveite...

4. USO
    Basta executar o fbv sem nenhum argumento e uma breve mensagem de ajuda
  será exibida...

5. AUTORES
  Tomasz 'smoku' Sterna <tomek@smoczy.net>
  Mateusz 'mteg' Golicz <mtg@elsat.net.pl>
  
  Sinta-se à vontade para enviar comentários, patches, correções de bugs, sugestões, etc. Os
  autores não são falantes nativos de inglês e estão cientes de que seu inglês
  está longe de ser perfeito. Por isso, relatos sobre erros de gramática e
  vocabulário neste arquivo também são bem-vindos.

6. BUGS E A FAZER
  - o código está realmente mal formatado e precisa de algumas correções...
  - o inglês nas mensagens não é o melhor de todos :-)

7. AGRADECIMENTOS
  - os autores do fbset: parte do código em fb_display.c é baseado nele...
  - Nat Ersoz - por suas sugestões e correções de bugs
  - Mauro Meneghin - pelo suporte a GIF transparente
  - Marcin 'Piaskowy' Zieba - por suas pequenas correções de bugs
  - Mariusz 'Ma-rYu-sH' Witkowski - por suas sugestões sobre suporte a canal
        alfa e testes

8. LICENÇA
  O pacote está licenciado sob a licença GNU GPL, versão 2.
  Obtenha sua cópia em http://www.gnu.org.
