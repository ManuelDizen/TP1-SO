-----------------------------------------------------|
			TP1 - SO		     |
-----------------------------------------------------|

Para compilar el trabajo:

	make all

Para ejecutar el trabajo:

	./solve (path a directorio/archivo)

Aclaración: Se puede colocar mas de un argumento para procesar

Para conectar el proceso vista, debemos conectar una terminal en paralelo
y (desde el directorio donde se compilo) ejecutar el comando:

	./view shm

Aclaración: shm fue el nombre asignado a la memoria compartida por el grupo.

También se pueden ejecutar ambos procesos mediante un pipe sin necesidad de aclarar el nombre de la memoria compartida:

 	./solve (path a directorio/archivo) | ./view

Integrantes del grupo:
	- De Simone, Franco - 61100
	- Dizenhaus, Manuel - 61101
	- Cornidez, Milagros - 61432
