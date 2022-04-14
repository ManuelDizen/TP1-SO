# TP1 - SO		     

## Para compilar el trabajo:

```
make all
```

## Para ejecutar el trabajo:
```
./solve (path a directorio/archivo)
```

Aclaración: Se puede colocar mas de un argumento para procesar

Para conectar el proceso vista, debemos conectar una terminal en paralelo
y (desde el directorio donde se compilo) ejecutar el comando:

```
./view shm
```

Aclaración: shm fue el nombre asignado a la memoria compartida por el grupo.

También se pueden ejecutar ambos procesos mediante un pipe sin necesidad de aclarar el nombre de la memoria compartida:
```
./solve (path a directorio/archivo) | ./view
```

#### Nota: El makefile contempla la instalación del programa "minisat", necesario para procesar los archivos .cnf . Al estar pensado para ser compilado en docker, no utiliza la keyword 'sudo'. En caso de querer instalar minisat localmente, es posible que el sistema requiera la instalación del minisat por separado. Para hacer esto, simplemente ejecute el comando, desde su consola local:
```
sudo apt-get install minisat
```

## Integrantes del grupo:
Nombre | Legajo
-------|--------
De Simone, Franco | 61100
Dizenhaus, Manuel | 61101
Cornidez, Milagros | 61432
