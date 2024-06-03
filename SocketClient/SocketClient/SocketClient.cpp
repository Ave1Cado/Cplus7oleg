#define WIN32_LEAN_AND_MEAN //указывает что из заголовка windows.h нужно брать минимум кода, за счет этого уменьшается размер нашего файла и ускоряется компиляция

#include <Windows.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

int main() {
    WSADATA wsaData;  // Переменная для хранения информации о реализации Windows Sockets
    ADDRINFO hints;   // Структура для хранения параметров соединения
    ADDRINFO* addrResult = nullptr;  // Указатель на результат адресации
    SOCKET ConnectSocket = INVALID_SOCKET;  // Переменная для хранения сокета
    char recvBuffer[512];  // Буфер для получения данных

    const char* sendBuffer1 = "Client first message";  // Сообщение 1
    const char* sendBuffer2 = "Client second message";  // Сообщение 2

    // Инициализация библиотеки Winsock (API для создания сетевых приложений)
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed with result: " << result << endl;
        return 1;
    }

    // Заполнение структуры hints нулями и установка параметров соединения
    //hints это экземпляр структуры ADDRINFO, которая используется в функциях Winsock для указания параметров, которые следует использовать при разрешении адресов и установке сетевых соединений)
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;       // Использовать IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP сокет
    hints.ai_protocol = IPPROTO_TCP; // Протокол TCP

    // Получение адреса сервера
    result = getaddrinfo("localhost", "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Создание сокета
    ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Подключение к серверу
    result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Unable to connect to server" << endl;
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Отправка первого сообщения на сервер
    result = send(ConnectSocket, sendBuffer1, (int)strlen(sendBuffer1), 0);
    if (result == SOCKET_ERROR) {
        cout << "Send failed, error: " << result << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }
    cout << "Sent: " << result << " bytes" << endl;

    // Отправка второго сообщения на сервер
    result = send(ConnectSocket, sendBuffer2, (int)strlen(sendBuffer2), 0);
    if (result == SOCKET_ERROR) {
        cout << "Send failed, error: " << result << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }
    cout << "Sent: " << result << " bytes" << endl;

    // Завершение отправки
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Получение ответа от сервера
    do {
        ZeroMemory(recvBuffer, 512);  // Очистка буфера
        result = recv(ConnectSocket, recvBuffer, 512, 0);  // Получение данных
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;
        }
        else if (result == 0) {
            cout << "Connection closed" << endl;
        }
        else {
            cout << "Recv failed, error: " << WSAGetLastError() << endl;
        }
    } while (result > 0);

    // Закрытие сокета и освобождение ресурсов
    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);
    WSACleanup();
    return 0;
}
