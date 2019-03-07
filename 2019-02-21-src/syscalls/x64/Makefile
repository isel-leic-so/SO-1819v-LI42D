all:	syscall.exe  

syscall.exe:   syscall.c ntquery.obj
    cl /Zi syscall.c ntquery.obj
   
	
ntquery.obj: ntquery.asm
    ml64 /c ntquery.asm

clean:
	del *.pdb *.obj *.exe *.ilk *.sln *.suo *.sdf