// client.cpp - Cliente Multiplataforma (Windows + Linux)
#include <iostream>
#include <string>
#include <sstream>
#include <random>
#include <ctime>
#include <iomanip>
#include <cstring>

// Includes específicos de plataforma
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define close_socket closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define close_socket close
    #define INVALID_SOCKET -1
#endif

using namespace std;

// ==================== FUNCIONES ====================

string generateTransactionId() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1000000, 9999999);
    return "TXN-" + to_string(dis(gen));
}

string getCurrentTimestamp() {
    time_t now = time(nullptr);
    char buffer[80];
    
    #ifdef _WIN32
        tm timeinfo;
        localtime_s(&timeinfo, &now);
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &timeinfo);
    #else
        tm *timeinfo = localtime(&now);
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
    #endif
    
    return string(buffer);
}

socket_t connectToServer(const string& serverIP, int port) {
    // Inicializar Winsock solo en Windows
    #ifdef _WIN32
        static bool wsaInitialized = false;
        if (!wsaInitialized) {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
                return INVALID_SOCKET;
            }
            wsaInitialized = true;
        }
    #endif
    
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
    
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close_socket(sock);
        return INVALID_SOCKET;
    }
    
    return sock;
}

string sendMessage(const string& serverIP, int port, const string& message) {
    cout << "[DEBUG] Intentando conectar a " << serverIP << ":" << port << endl;

    socket_t sock = connectToServer(serverIP, port);
    if (sock == INVALID_SOCKET) {
        cerr << "[DEBUG] No se pudo conectar al servidor " << serverIP << ":" << port << endl;
        return "ERROR:No se pudo conectar";
    }
    
    cout << "[DEBUG] Conexion exitosa, enviando mensaje..." << endl;

    send(sock, message.c_str(), (int)message.length(), 0);
    
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
    
    close_socket(sock);
    
    if (bytesReceived > 0) {
        cout << "[DEBUG] Respuesta recibida: " << buffer << endl;
        return string(buffer);
    }

    cerr << "[DEBUG] No se recibio respuesta del servidor" << endl;
    return "ERROR:Sin respuesta";
}

string requestToken(const string& serverIP, int port) {
    cout << "\n[INFO] Solicitando token..." << endl;
    string response = sendMessage(serverIP, port, "REQUEST_TOKEN");
    
    if (response.substr(0, 6) == "TOKEN:") {
        string token = response.substr(6);
        cout << "\n+--------------------------------+" << endl;
        cout << "  TOKEN: " << token << endl;
        cout << "  Valido por 60 segundos" << endl;
        cout << "+--------------------------------+\n" << endl;
        return token;
    }
    
    cout << "ERROR al obtener token" << endl;
    return "";
}

void performTransaction(const string& serverIP, int port, 
                       const string& amount, const string& token) {
    string txId = generateTransactionId();
    
    cout << "\n============================" << endl;
    cout << "  PROCESANDO TRANSACCION" << endl;
    cout << "============================" << endl;
    cout << "ID: " << txId << endl;
    cout << "Monto: $" << amount << endl;
    cout << "Timestamp: " << getCurrentTimestamp() << endl;
    cout << "============================\n" << endl;
    
    string message = "TRANSACTION " + txId + " " + amount + " " + token;
    string response = sendMessage(serverIP, port, message);
    
    if (response.substr(0, 7) == "SUCCESS") {
        cout << "\n[EXITO] TRANSACCION APROBADA" << endl;
        cout << response.substr(8) << endl;
    } else {
        cout << "\n[ERROR] TRANSACCION RECHAZADA" << endl;
        cout << "Razon: " << response.substr(6) << endl;
    }
}

void showMenu() {
    cout << "\n================================" << endl;
    cout << "  MENU DE TRANSACCIONES" << endl;
    cout << "================================" << endl;
    cout << "1. Solicitar token" << endl;
    cout << "2. Realizar transaccion" << endl;
    cout << "3. Flujo completo" << endl;
    cout << "4. Salir" << endl;
    cout << "\nOpcion: ";
}

// ==================== MAIN ====================

int main(int argc, char* argv[]) {
    cout << "\n================================" << endl;
    cout << "  CLIENTE DE TRANSACCIONES" << endl;
    cout << "================================\n" << endl;
    
    string serverIP = "127.0.0.1";  // localhost por defecto (Windows)
    int port = 8080;
    
    // En Docker, usar nombre del servicio
    if (argc > 1) {
        serverIP = argv[1];  // Será "server" en Docker
    }
    
    cout << "Servidor: " << serverIP << ":" << port << endl;
    
    string currentToken = "";
    
    while (true) {
        showMenu();
        
        int option;
        cin >> option;
        cin.ignore();
        
        switch (option) {
        case 1:
            currentToken = requestToken(serverIP, port);
            break;
            
        case 2:
            if (currentToken.empty()) {
                cout << "\n[ADVERTENCIA] No hay token disponible" << endl;
                break;
            }
            {
                cout << "\nMonto: $";
                string amount;
                getline(cin, amount);
                performTransaction(serverIP, port, amount, currentToken);
                currentToken = "";
            }
            break;
            
        case 3:
            currentToken = requestToken(serverIP, port);
            if (!currentToken.empty()) {
                cout << "\nMonto: $";
                string amount;
                getline(cin, amount);
                performTransaction(serverIP, port, amount, currentToken);
                currentToken = "";
            }
            break;
            
        case 4:
            cout << "\nSaliendo..." << endl;
            #ifdef _WIN32
                WSACleanup();
            #endif
            return 0;
            
        default:
            cout << "\nOpcion invalida" << endl;
        }
    }
    
    return 0;
}