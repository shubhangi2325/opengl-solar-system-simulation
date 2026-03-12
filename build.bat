cl.exe /c /EHsc /I C:\OpenGL\glew-2.1.0\include OGL.cpp stack.cpp
rc.exe OGL.rc 
link.exe OGL.obj stack.obj OGL.res /LIBPATH:C:\OpenGL\glew-2.1.0\lib\Release\x64 gdi32.lib user32.lib kernel32.lib /SUBSYSTEM:WINDOWS
