# pojekt-na-sieci


to compile a server you can do
gcc server.c -o server.out
now you run it by executing file with choose of 3 options 0 - normal checkers, 1 where white wins by capturing all black pieces in his first turn, 2 is where there are only queens on the last line for both colors it connects to server on port 1100
./server.out 1



to compile klient you can do 
gcc klient.c -o klient.out
now you run it by executing file and choosing server to connect it to on port 1100 and any ip that is on the computer server has
./klient.out 1100 127.0.0.1



technologie
Twój serwer to klasyczny przykład C + TCP socket + multithreading (pthreads) + gry planszowe.
Nie używa żadnych wyższych frameworków ani bibliotek do sieci (np. Boost.Asio w C++ czy Node.js) – wszystko jest “czystym C”.
