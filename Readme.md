To run the client code:
command (compile)-> g++ client.cpp -lssl -lcrypto
command(run)->./a.out  127.0.0.1:8000 tracker_info.txt


To run the server code:
command(compile)->  g++ tracker.cpp
command(run)->  ./a.out tracker_info.txt 1

tracker_info.txt file is included in both  client and server folder.
