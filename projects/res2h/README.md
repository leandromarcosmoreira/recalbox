# res2h - Um compilador de recursos flexível semelhante ao bin2h e qrc

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) ![Build](https://github.com/HorstBaerbel/res2h/workflows/Build/badge.svg) ![Tests](https://github.com/HorstBaerbel/res2h/workflows/Tests/badge.svg) ![Clang-format](https://github.com/HorstBaerbel/res2h/workflows/Clang-format/badge.svg) ![Clang-tidy](https://github.com/HorstBaerbel/res2h/workflows/Clang-tidy/badge.svg)

**res2h** pode:

* Converter dados binários de arquivos arbitrários para arrays hex C/C++ brutos para incluí-los em seu software (semelhante ao [bin2h](http://code.google.com/p/bin2h/) com funcionalidades adicionais).
* Compilar os dados de todos os arquivos em um arquivo de archive binário (como [tar](https://en.wikipedia.org/wiki/Tar_(computing)) ou [qrc](https://doc.qt.io/qt-5/resources.html)) que você pode então carregar do disco ou anexar ao executável do aplicativo e acessar em tempo de execução. Nenhuma biblioteca externa é necessária para isso.
* Atualmente é testado apenas no Linux, mas Windows e em alguma medida MacOS estão planejados.

As principais ferramentas são [res2h](#res2h) que pode converter arquivos para arquivos .h/.cpp ou empacotá-los em archives binários e [res2hdump](#res2hdump) que permite visualizar ou desempacotar esses archives binários.

Se você encontrar um bug ou fizer uma melhoria, seus pull requests são apreciados.

# Licença

Tudo isso está sob a [Licença MIT](LICENSE).

# Pré-requisitos

* Um compilador compatível com C++14 com suporte para std::filesystem. Para instalar uma nova versão do g++ veja [isso](https://lektiondestages.art.blog/2013/05/23/installing-and-switching-gcc-g-versions-in-debian/).
* CMake 3.1 ou superior para construção e testes de unidade via CTest.

# Construção

* Clone o repositório usando ```git clone https://github.com/HorstBaerbel/res2h```
* Navegue até a pasta res2h, então execute cmake: ```cmake .```
* Então construa usando: ```make```
* Você pode executar o teste de unidade usando: ```make tests```

# Uso

## res2h

res2h tem diferentes modos. Ele pode converter dados binários de arquivos para arrays hex brutos em arquivos de origem .c/.cpp que você pode incluir em seu projeto e compilá-los no executável. Ele também pode criar um cabeçalho comum que permite acessar todos os arrays convertidos com um include. Se você não quiser que seus dados sejam carregados na memória, o res2h também oferece a possibilidade de criar um archive binário contendo todos os arquivos que você pode acessar via a classe "Res2h" fornecida em cabeçalhos separados. Você também pode incorporar este archive em seu executável, então você tem apenas um arquivo, e acessa como faria com qualquer outro archive em disco.
Deveria compilar e funcionar pelo menos no Windows, Ubuntu e Raspbian.

```sh
res2h ARQUIVO_ENTRADA/PASTA_ENTRADA ARQUIVO_SAIDA/PASTA_SAIDA [OPÇÕES]
```

### Opções válidas

**-r**: Recursar em subdiretórios abaixo do diretório de entrada.  
**-c**: Usar arquivos .c e arrays C para armazenar as definições de dados, caso contrário usa arquivos .cpp e std::vector / std::map.  
**-h ARQUIVO_CABEÇALHO**: Coloca todas as declarações no arquivo "ARQUIVO_CABEÇALHO" usando "extern" e inclui esse arquivo de cabeçalho nos arquivos de origem.  
**-u ARQUIVO_FONTE**: Criar funções utilitárias e arrays em um arquivo .c/.cpp. Faz sentido apenas em combinação com **-h**.  
**-1**: Combinar todos os arquivos convertidos em um grande arquivo .c/.cpp (use junto com **-u**).  
**-b**: Compilar archive binário ARQUIVO_SAIDA contendo todos os arquivos de entrada. Para ler em seu software inclua res2hinterface.h/.c/.cpp (dependendo de **-c**) e consulte a documentação.  
**-a**: Anexar ARQUIVO_ENTRADA a ARQUIVO_SAIDA. Pode ser usado para anexar um archive a um executável (apenas um archive incorporado possível).  
**-v**: Ser verboso.

### Exemplos

* Converter um único arquivo: ```res2h ./lenna.png ./resources/lenna_png.cpp```
* Converter todos os arquivos em um diretório, criar um cabeçalho comum e utilitários: ```res2h ./data ./resources -r -h resources.h -u resources.cpp```
* Converter todos os arquivos em um diretório, criar um cabeçalho comum e utilitários, combinar todos os dados em resources.cpp: ```res2h ./data ./resources -r -1 -h resources.h -u resources.cpp```
* Converter dados para um archive binário: ```res2h ./data ./resources/data.bin -b```
* Anexar um archive a um executável: ```res2h ./resources/data.bin ./program.exe -a```

### Gerando arquivos compiláveis/incluíveis

#### O comando ```res2h a.x b_x.cpp -h bla.h``` criaria esses arquivos

**a_x.cpp:**

```c++
// este arquivo foi auto-gerado de a.x por res2h

#include "bla.h"

const uint32_t a_x_size = 123;
const uint8_t a_x_data[a_x_size] = {
    0x11,0x22,...
};
```

**bla.h:**

```c++
// este arquivo foi auto-gerado por res2h

extern const uint32_t a_x_size;
extern const uint8_t a_x_data[];
```

#### O comando ```res2h a.x b_x.cpp *-c* -h bla.h -u bla.cpp``` também criaria a_x.cpp, e

**bla.h:**

```c++
// este arquivo foi auto-gerado por res2h

extern const uint32_t a_x_size;
extern const uint8_t a_x_data[];

struct Res2hEntry {
    const char * relativeFileName;
    const uint32_t size;
    const uint8_t * data;
};

// isto contém todos os recursos com seus nomes e dados
extern const uint32_t res2hNrOfFiles;
extern const Res2hEntry res2hFiles[];
```

**bla.cpp:**

```c++
// este arquivo foi auto-gerado por res2h

#include "bla.h"

const uint32_t res2hNrOfFiles = 4;
const Res2hEntry res2hFiles[res2hNrOfFiles] = {
 {":/a.x", a_x_size, a_x_data}
};
```

#### O comando ```res2h a.x b_x.cpp -h bla.h *-u bla.cpp``` também criaria a_x.cpp, e

**bla.h:**

```c++
// este arquivo foi auto-gerado por res2h

extern const uint32_t a_x_size;
extern const uint8_t a_x_data[];

struct Res2hEntry {
    const std::string relativeFileName;
    const uint32_t size;
    const uint8_t * data;
};

// isto contém todos os recursos com seus nomes e dados
extern const uint32_t res2hNrOfFiles;
extern const Res2hEntry res2hFiles[];

// isso mapeia o nome do arquivo relativo do recurso para seus dados
// uso ex: Res2hEntry resource = res2hMap["a.x"];
typedef const std::map<const std::string, const Res2hEntry> res2hMapType;
extern res2hMapType res2hMap;
```

**bla.cpp:**

```c++
// este arquivo foi auto-gerado por res2h

#include "bla.h"

const uint32_t res2hNrOfFiles = 4;
const Res2hEntry res2hFiles[res2hNrOfFiles] = {
 {":/a.x", a_x_size, a_x_data}
};

res2hMapType::value_type mapTemp[] = {
    std::make_pair(":/a.x", res2hFiles[0]),
};

res2hMapType res2hMap(mapTemp, mapTemp + sizeof mapTemp / sizeof mapTemp[0]);
```

### Gerando archives binários

#### O comando ```res2h ./data archive.bin -r -b``` 

Encontraria todos os arquivos no diretório ./data e os empacotaria no archive binário test.bin. Para ler arquivos de archive ou archives incorporados em seu aplicativo, inclua os arquivos "res2hinterface.h/.cpp" resp. a classe "Res2h". Eles fornecem todas as funções necessárias para ler recursos de archives ou do disco. Você pode encontrar um exemplo de como usar as funções em "res2hdump.cpp" / dumpArchive().

## res2hdump

res2hdump é uma ferramenta que permite despejar informações e/ou arquivos de um archive res2h binário ou um archive incorporado em outro arquivo, por exemplo executável. Também serve como exemplo de como usar a classe "Res2h" contida nos arquivos "res2hinterface".

```sh
res2hdump ARQUIVO [PASTA_SAIDA] [OPÇÕES]
```

### Opções válidas

**-f**: Recriar estrutura deパス, criando diretórios conforme necessário.  
**-i**: Exibir informações sobre o archive e arquivos, mas não extrair nada.  
**-v**: Ser verboso.

### Exemplos

* Exibir informações sobre o archive: ```res2hdump ./resources/data.bin -i```
* Extrair todos os arquivos de um archive com subdiretórios: ```res2hdump ./resources/data.bin ./resources -f```
* Extrair arquivos de archive incorporado: ```res2hdump ./resources/program.exe ./resources```

## Formato do archive binário

<table>
    <tr>
        <th>Offset (decimal)</th><th>Tipo</th><th>Descrição</th>
    </tr>
    <tr>
        <td>Início</td><td>char[8]</td><td>string do número mágico "res2hbin"</td>
    </tr>
    <tr>
        <td>08</td><td>uint32_t</td><td>número da versão do formato do arquivo (atualmente 2)</td>
    </tr>
    <tr>
        <td>12</td><td>uint32_t</td><td>flags de formato (profundidade de 32/64 bits do archive)</td>
    </tr>
    <tr>
        <td>16</td><td>uint32_t/uint64_t</td><td>tamanho de todo o archive em bytes</td>
    </tr>
    <tr>
        <td>20/24</td><td>uint32_t</td><td>número de entradas de diretório e arquivo a seguir</td>
    </tr>
    <tr>
        <td colspan="3">Então segue o diretório:</td>
    </tr>
    <tr>
        <td>24/28 + 00</td><td>uint16_t</td><td>entrada de arquivo #0, tamanho do nome interno SEM caractere nulo</td>
    </tr>
    <tr>
        <td>24/28 + 02</td><td>char[]</td><td>entrada de arquivo #0, nome interno (NÃO terminado em nulo)</td>
    </tr>
    <tr>
        <td>24/28 + 02 + nome</td><td>uint32_t</td><td>entrada de arquivo #0, flags de formato para entrada (atualmente 0)</td>
    </tr>
    <tr>
        <td>24/28 + 06 + nome</td><td>uint32_t/uint64_t</td><td>entrada de arquivo #0, tamanho dos dados</td>
    </tr>
    <tr>
        <td>24/28 + 10/14 + nome</td><td>uint32_t/uint64_t</td><td>entrada de arquivo #0, offset absoluto dos dados no arquivo</td>
    </tr>
    <tr>
        <td>24/28 + 14/22 + nome</td><td>uint32_t/uint64_t</td><td>entrada de arquivo #0, soma Fletcher32/64 dos dados</td>
    </tr>
    <tr>
        <td colspan="3">Então seguem as outras entradas do diretório</td>
    </tr>
    <tr>
        <td colspan="3">Imediatamente após o diretório, os blocos de dados começam</td>
    </tr>
    <tr>
        <td>Fim - 04/08</td><td>uint32_t/uint64_t</td><td>Soma Fletcher32/64 de todo o arquivo até este ponto</td>
    </tr>
</table>

Obviamente com um archive de 32bit você está limitado a ~4GB para todo o arquivo binário e ~4GB por entrada de dados. O res2h automaticamente criará um archive de 32bit para economizar espaço se os dados permitirem, ou um archive de 64bit se necessário. Isso tudo é tratado transparentemente pelo res2hinterface, então você realmente não precisa se preocupar com isso.

## A fazer

* Fazer CI construir + testar no Windows + MacOS também.
* Mais testes de unidade.
* Mais limpeza. C++14 mais moderno.
* Ativar mais avisos no compilador e clang-tidy.
* Usar exceções aninhadas.
* Usar [cxxopts](https://github.com/jarro2783/cxxopts) para parsing de argumentos (suporte a opções longas).
* Suporte a Unicode.
* Economizar espaço em arquivos .c / .cpp outputting hex strings de 32bit ou até 64bit.
* Reutilizar resultados de compilação da ação "Build" em "Unit tests" e "Clang-tidy" para economizar tempo.
* Usar CRC em vez de Fletcher.
* Adicionar compressão de recursos opcional.
* Processamento paralelo de arquivos de entrada.
* Suporte a atualização de archives.
* Opção para apenas salvar hash em archives para economizar espaço.
* Formato binário mais compacto.

## Encontrei um bug ou tenho sugestão

A melhor maneira de relatar um bug ou sugerir algo é postar uma issue no GitHub. Tente ser simples, mas descritivo e adicione TODAS as informações necessárias para REPRODUZIR o bug. **"Não funciona" não é suficiente!** Se você não consegue compilar, por favor informe seu sistema, versão do compilador, etc! Você também pode entrar em contato comigo por e-mail se quiser.
