# comp-wrapper

O objetivo é poder compilar projetos com um único arquivo Makefile.

O comp-wrapper é uma adaptação [desse script](http://stackoverflow.com/questions/13510618/is-there-a-simple-way-to-redirect-object-file-creation-into-a-specified-dire).

### Instalação
```sh
$ git clone https://github.com/xalalau/Xalateca.git
$ sudo cp './Xalateca/Bash/comp-wrapper/src/comp-wrapper' /usr/bin
$ chmod +x /usr/bin/comp-wrapper
```

### Uso
Por enquanto eu só uso com o gcc, mingw-gcc e windres. Costuma ser assim:

Chamada com -c:

```sh
comp-wrapper compilador diretorio_destino COISAS -c COISAS
```

Chamada com -o: 

```sh
comp-wrapper compilador COISAS -o nome_programa COISAS
```

onde "COISAS" costumam ser arquivos.

### Notas
- Em chamadas do tipo "-c" podemos definir o "diretorio_destino" como "./" para usarmos a pasta corrente;
- O comp-wrapper ainda é experimental pois eu não conheço todas as possibilidades de flags. Ele nunca me deu problemas, mas vale a pena ficar de olho nos outputs e nas possibilidades.
