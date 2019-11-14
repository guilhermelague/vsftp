all: vs.exe vc.exe 

vs.exe: VSFTPServidor.c
	gcc -pthread VSFTPServidor.c -o vs.exe

vc.exe: VSFTPCliente.c
	gcc VSFTPCliente.c -o vc.exe

clean:
	rm *.exe