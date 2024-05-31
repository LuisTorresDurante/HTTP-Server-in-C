# Simple HTTP Server in C

This is a simple HTTP server using only C. The server can handle HTTP GET requests and return HTML files. It also implements proper handling of 404 (Not Found) and 200 (OK) status codes.


## Compilation and Execution

### Running on windows
1. Clone the repository:
   ```git clone https://github.com/LuisTorresDurante/HTTP-Server-in-C```
2. Navigate to the project directory:
   ```cd HTTP-Server-in-C/```
3. Compile the server:
   ```gcc WinServer.c -o WinServer.exe -lws2_32```
4. Run the server, to do so double click the file.
   ```WinServer.exe```  
5. Access it through localhost:8080

### Running on Linux/Unix(?)
1. Clone the repository:
   ```git clone https://github.com/LuisTorresDurante/HTTP-Server-in-C```
2. Navigate to the project directory:
   ```cd HTTP-Server-in-C/```
3. Compile the server:
   ```gcc .\Server.c -o Server```
4. Run the server
   ```./Server```  
5. Access it through localhost:8080


To see your own files just add .html that do not have css files on the same directory as the server and access to them through your browser. For example:
``` localhost:8080/index.html``` 
