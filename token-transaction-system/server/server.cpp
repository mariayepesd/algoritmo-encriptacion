// server.cpp - Servidor Multiplataforma (Windows + Linux)
#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <random>
#include <ctime>
#include <sstream>
#include <iomanip>

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
    #include <unistd.h>
    typedef int socket_t;
    #define close_socket close
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

using namespace std;

// ==================== ESTRUCTURAS ====================

struct Token {
    string value;
    time_t expiration;
    bool used;
};

map<string, Token> activeTokens;

// ==================== FUNCIONES ====================

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

string generateToken() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(100000, 999999);
    return to_string(dis(gen));
}

string createToken() {
    string tokenValue = generateToken();
    Token token;
    token.value = tokenValue;
    token.expiration = time(nullptr) + 60;
    token.used = false;
    activeTokens[tokenValue] = token;
    
    cout << "[" << getCurrentTimestamp() << "] Token generado: " << tokenValue << endl;
    return tokenValue;
}

bool validateToken(const string& tokenValue) {
    auto it = activeTokens.find(tokenValue);
    
    if (it == activeTokens.end()) {
        cout << "[" << getCurrentTimestamp() << "] Token NO encontrado" << endl;
        return false;
    }
    
    Token& token = it->second;
    time_t now = time(nullptr);
    
    if (token.used) {
        cout << "[" << getCurrentTimestamp() << "] Token YA usado" << endl;
        return false;
    }
    
    if (now > token.expiration) {
        cout << "[" << getCurrentTimestamp() << "] Token EXPIRADO" << endl;
        activeTokens.erase(it);
        return false;
    }
    
    token.used = true;
    cout << "[" << getCurrentTimestamp() << "] Token VALIDO" << endl;
    return true;
}

string processMessage(const string& message) {
    stringstream ss(message);
    string command;
    ss >> command;
    
    if (command == "REQUEST_TOKEN") {
        string token = createToken();
        return "TOKEN:" + token;
    }
    else if (command == "TRANSACTION") {
        string transactionId, amount, token;
        ss >> transactionId >> amount >> token;
        
        cout << "\n========== NUEVA TRANSACCION ==========" << endl;
        cout << "ID: " << transactionId << endl;
        cout << "Monto: $" << amount << endl;
        cout << "Token: " << token << endl;
        cout << "Timestamp: " << getCurrentTimestamp() << endl;
        
        if (validateToken(token)) {
            cout << "Estado: APROBADA" << endl;
            cout << "======================================\n" << endl;
            return "SUCCESS:Transaccion aprobada - ID:" + transactionId;
        } else {
            cout << "Estado: RECHAZADA" << endl;
            cout << "======================================\n" << endl;
            return "ERROR:Token invalido o expirado";
        }
    }
    
    return "ERROR:Comando desconocido";
}

// ==================== MAIN ====================

int main() {
    // Inicializar Winsock solo en Windows
    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            cerr << "ERROR: No se pudo inicializar Winsock" << endl;
            return 1;
        }
    #endif

    cout << "\n========================================" << endl;
    cout << "  SERVIDOR DE AUTENTICACION" << endl;
    cout << "========================================\n" << endl;

    socket_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "ERROR: No se pudo crear socket" << endl;
        #ifdef _WIN32
            WSACleanup();
        #endif
        return 1;
    }
    cout << "[OK] Socket creado" << endl;
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "ERROR: No se pudo hacer bind" << endl;
        close_socket(serverSocket);
        #ifdef _WIN32
            WSACleanup();
        #endif
        return 1;
    }
    cout << "[OK] Bind exitoso en puerto 8080" << endl;
    
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cerr << "ERROR: No se pudo hacer listen" << endl;
        close_socket(serverSocket);
        #ifdef _WIN32
            WSACleanup();
        #endif
        return 1;
    }
    cout << "[OK] Servidor escuchando en puerto 8080" << endl;
    cout << "\n>>> Esperando conexiones de clientes...\n" << endl;
    
    while (true) {
        sockaddr_in clientAddr;
        #ifdef _WIN32
            int clientSize = sizeof(clientAddr);
        #else
            socklen_t clientSize = sizeof(clientAddr);
        #endif
        
        socket_t clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "[ERROR] No se pudo aceptar la conexion" << endl;
            continue;
        }
        
        cout << "[" << getCurrentTimestamp() << "] Cliente conectado" << endl;
        
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived > 0) {
            string message(buffer);
            cout << "[" << getCurrentTimestamp() << "] Mensaje recibido: " << message << endl;
            
            string response = processMessage(message);
            send(clientSocket, response.c_str(), (int)response.length(), 0);
            cout << "[" << getCurrentTimestamp() << "] Respuesta enviada: " << response << endl;
        }
        
        close_socket(clientSocket);
    }
    
    close_socket(serverSocket);
    #ifdef _WIN32
        WSACleanup();
    #endif
    return 0;
}