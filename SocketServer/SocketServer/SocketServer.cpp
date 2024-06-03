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
    SOCKET ListenSocket = INVALID_SOCKET;  // Переменная для хранения слушающего сокета
    SOCKET ConnectSocket = INVALID_SOCKET;  // Переменная для хранения сокета подключения
    char recvBuffer[512];  // Буфер для получения данных

    const char* sendBuffer = "Server message";  // Сообщение сервера

    // Инициализация библиотеки Winsock (API для создания сетевых приложений)
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed with result: " << result << endl;
        return 1;
    }

    // Заполнение структуры hints нулями и установка параметров соединения
    // hints это экземпляр структуры ADDRINFO, которая используется в функциях Winsock для указания параметров, которые следует использовать при разрешении адресов и установке сетевых соединений)
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;       // Использовать IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP сокет
    hints.ai_protocol = IPPROTO_TCP; // Протокол TCP
    hints.ai_flags = AI_PASSIVE;     // Нужен для использования с bind()

    // Получение адреса для прослушивания
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Создание сокета для прослушивания
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Привязка сокета к адресу
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Bind failed, error: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Прослушивание входящих соединений
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cout << "Listen failed, error: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Ожидание входящего соединения
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    closesocket(ListenSocket);  // Закрытие сокета прослушивания

    // Обработка входящих данных
    do {
        ZeroMemory(recvBuffer, 512);  // Очистка буфера
        result = recv(ConnectSocket, recvBuffer, 512, 0);  // Получение данных
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;

            // Отправка данных обратно клиенту
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                cout << "Send failed, error: " << result << endl;
                closesocket(ConnectSocket);
                freeaddrinfo(addrResult);
                WSACleanup();
                return 1;
            }
        }
        else if (result == 0) {
            cout << "Connection closing" << endl;
        }
        else {
            cout << "Recv failed, error: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);
            freeaddrinfo(addrResult);
            WSACleanup();
            return 1;
        }
    } while (result > 0);

    // Завершение соединения
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Закрытие сокета и освобождение ресурсов
    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);
    WSACleanup();
    return 0;
}
