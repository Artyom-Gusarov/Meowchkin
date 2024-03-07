# Meowchkin

## Compilation and Running Examples

```bash
cd build
cmake ..
make
./server_example <count-of-clients>
example: ./server_example 2
```
 
```bash
cd build
./client_example <server-ip>:<server-port> <client-name>
example: ./client_example 0.0.0.0:<что вывел сервер> lupa
```

```bash
cd build
./client_example 0.0.0.0:<что вывел сервер> pupa
```

И так столько клиентов, сколько указано в аргументе запуска сервера.
Читаем хедеры и кайфуем.
