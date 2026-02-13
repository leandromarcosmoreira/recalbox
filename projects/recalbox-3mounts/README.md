# 3mounts

Anteriormente USBmount.

O pacote 3mounts monta automaticamente dispositivos de armazenamento em massa USB, MCC e HDD (por exemplo, pendrives USB ou HDs em enclosures USB, cartão SD, drives nvme e sata internos) quando são conectados ou in-place.
Os pontos de montagem (`/recalbox/share/externals/[usb|mmc|hdd][0-x]` por padrão), tipos de filesystem a serem considerados e opções de montagem são configuráveis. Quando vários dispositivos são conectados, o primeiro ponto de montagem disponível é automaticamente selecionado.

Se o dispositivo conectado fornecer um nome de modelo, um link simbólico em `/var/run/3mounts/MODELNAME` apontando para o ponto de montagem é automaticamente criado. Quando o dispositivo não está mais presente no sistema (por exemplo, após ter sido desconectado), o 3mounts exclui os links simbólicos que foram criados.

O script que faz o (des)montagem é chamado pelo daemon `udev`. Portanto, o 3mounts requer um kernel 2.6 (ou mais recente).

O 3mounts é destinado como uma solução leve que é independente de um ambiente de desktop. Os usuários que gostariam de ver um ícone quando um dispositivo USB é conectado devem usar outras alternativas.

Os comentários no arquivo de configuração `/etc/3mounts/3mounts.conf` descrevem como configurar o pacote.

## Comentários Gerais sobre Pendrives

Os usuários devem estar cientes de que, independentemente do sistema de arquivos usado pelo dispositivo de armazenamento, *QUALQUER* sistema de arquivos que reside em memória flash se tornará ilegível após algum tempo. Esta situação unfortunate é intrínseca ao meio de armazenamento e pendrives de melhor qualidade executam uma operação de "wear levelling", distribuindo a carga de operações por todo o dispositivo. [*]

Sistemas de arquivos usando memória flash e montados com a opção `sync` podem degradar mais cedo devido ao fato de que a opção de montagem sync força o sistema operacional a gravar dados com mais frequência no dispositivo do que se fosse montado sem a opção `sync`.

Então, por que montar sistemas de arquivos com a opção `sync`? A razão é manter os dados gravados no drive refletindo o que o usuário pensa que está no pendrive, e, mais importante, evitar o problema do usuário desconectar o dispositivo antes de terminar de receber dados que o kernel tem na memória do computador e que deve ser gravada no dispositivo.

Se você não gosta da opção `sync` com seus sistemas de arquivos, então você pode removê-la do arquivo de configuração do 3mounts e usar seus dispositivos com melhor desempenho e maior vida útil. **MAS** você deve sempre garantir que usa o comando `sync` (em um shell) para garantir que não haja gravações pendentes para o dispositivo em questão, para que você não perca nenhum dado quando desconectar o dispositivo do computador.

[*] Você pode ver se seus pendrives suportam wear levelling vendo as especificações técnicas de seus drives específicos no site do fabricante (por exemplo, o fabricante Kingston fornece essas informações sobre seus drives e outros provavelmente também fazem).

Claro que o 3mounts não funciona apenas com pendrives. Discos rígidos comuns colocados em enclosures são perfeitamente usados com o 3mounts e o 3mounts, apesar de seu nome, pode montar drives conectados via portas Firewire, desde que o kernel tenha suporte para isso (a maioria dos kernels de distribuição, incluindo os enviados com Debian e Ubuntu, tem).

## Considerações Técnicas

### Controle de Sistemas de Arquivos Montados pelo 3mounts

Você pode escolher quais sistemas de arquivos você quer que o 3mounts manipule automaticamente listando os tipos de sistemas de arquivos fornecidos pelo sistema operacional na variável de configuração `FILESYSTEMS`.

### Recomendações para Sistemas de Arquivos vfat

O sistema de arquivos vfat é um dos sistemas de arquivos mais comumente usados em pendrives. Infelizmente, devido à sua idade, é muito pobre em recursos e, em particular, não possui o controle de acesso mais básico presente em sistemas Unix, namely: permissões em arquivos.

O Linux contorna isso criando permissões e restrições "virtuais" baseadas em quem montou o sistema de arquivos. Como o 3mounts é usado, o usuário atribuído ao sistema de arquivos vfat é, por padrão, root.

Para uma configuração mais flexível, algumas opções úteis para sistemas de arquivos vfat são especificar explicitamente quem é o usuário e as permissões. Por favor, leia a página de manual do comando mount para obter detalhes.

Um exemplo é especificar `-fstype=vfat,gid=floppy,dmask=0007,fmask=0117` na variável `FS_MOUNTOPTIONS` do arquivo de configuração. As opções específicas no exemplo significam que membros do grupo floppy podem ler e escrever no meio, mas ninguém mais pode acessá-lo.

## Solução de Problemas do 3mounts

Nenhum software está livre de problemas e a situação não é diferente com o 3mounts. Para facilitar a solução de problemas, você pode tentar verificar o seguinte:

* Você tem HAL rodando? Qualquer daemon GNOME ou KDE automontando dispositivos?

* Vamos supor que a partição contendo o sistema de arquivos que você quer que o 3mounts manipule automaticamente é `/dev/sda1` (seu caso pode, muito possivelmente, variar). Então, verifique o resultado do seguinte comando:

        udevadm test --action=add /sys/class/block/sda1

  O comando acima apenas dá diagnósticos do que o 3mounts faria com o dispositivo, mas ele na verdade não monta ou interfere com o dispositivo. É destinado para fins de depuração. Cuidado que ele gera *muita* saída. Muitas telas, dependendo do dispositivo.

* Sob as mesmas suposições acima, outra boa ferramenta de diagnóstico é a seguinte:

        udevadm info -a -p $(udevadm info -q path -n /dev/sdb1)

## Desmontando Sistemas de Arquivos com Segurança

Como não é possível para o sistema detectar quando o dispositivo deve ser desmontado (tal informação só está presente quando o dispositivo já foi desconectado, o que é tarde demais para algumas limpezas, como descargar buffers não gravados no disco e marcar o sistema de arquivos como limpo), o usuário deve manualmente desmontar o dispositivo que foi automaticamente montado.

Esta situação é semelhante às em ambientes de desktop gráficos onde o usuário tem que clicar em um ícone e informar ao sistema que deseja remover o dispositivo do computador.

Uma solução recomendada para este problema é usar o comando `pummount` (fornecido pelo pacote `pmount`), que atua como um wrapper em torno do comando mount regular e permite que usuários normais (ou seja, não root) desmontem os sistemas de arquivos convenientemente.

**Aviso:** remover descuidadamente o dispositivo/sistema de arquivos sem primeiro desmontá-lo pode (e leva) a corrupção massiva do sistema de arquivos e deve ser executado apenas se você souber exatamente o que está fazendo.

## O Caso Especial de Sistemas de Arquivos FUSE

Muitos usuários usam drives removíveis com sistemas de arquivos NTFS e o sistema de arquivos de espaço de usuário NTFS-3g, pois ele fornece mais flexibilidade do que o módulo nativo presente no kernel do Linux.

Tais usuários têm dificuldade ao desmontar os sistemas de arquivos, pois eles estão presentes no sistema `/etc/mtab` com um tipo de sistema de arquivos de `fuseblk`, não com `ntfs` (ou `ntfs-3g`) como se poderia esperar.

Para tais sistemas de arquivos, pode ser conveniente:

* adicionar `ntfs-3g` à variável `FILESYSTEMS` de `/etc/3mounts/3mounts.conf` (para propósitos de montagem)
* adicionar `fuseblk` à variável `FILESYSTEMS` de `/etc/usb/3mounts.conf` (para propósitos de desmontagem).

Comentários semelhantes podem se aplicar a outros sistemas de arquivos gerenciados por FUSE. Em geral, se você precisar de um sistema de arquivos FUSE, pode ser uma boa ideia adicionar o nome desse sistema de arquivos à variável `FILESYSTEMS`, bem como garantir que o sistema de arquivos especial `fuseblk` esteja contido nessa lista.

Esta subseção é uma adaptação de descrições feitas por Thomas Jancar e Jan Schulz.

## Remontando Sistemas de Arquivos sem Remoção Física

O 3mounts opera (leia: "monta ou desmonta sistemas de arquivos") com base em eventos emitidos pelo kernel Linux/udev. Consequentemente, se você desmontar um sistema de arquivos e quiser montá-lo novamente, você tem básicamente duas escolhas:

1. desconectar e conectar o dispositivo, o que pode não ser desejado, por várias razões.
2. fazer o kernel/udev gerar outro evento para que o 3mounts saiba que tem algum trabalho a fazer.

Isso último pode ser accomplished pelo uso do comando

	udevadm trigger --action=add /dev/sdd2

onde `/dev/sdd2` deve ser substituído pela partição correta. Este comando provavelmente precisa ser executado com privilégios de superusuário. Acionar eventos de kernel também é uma forma de obter um sistema de arquivos específico montado após uma inicialização a frio.
