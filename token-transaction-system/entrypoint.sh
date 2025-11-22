#!/bin/sh
# entrypoint.sh - Decide qué ejecutar según el argumento

# Si no hay argumentos o es "server", ejecutar servidor
if [ $# -eq 0 ] || [ "$1" = "server" ]; then
    echo "Iniciando SERVIDOR..."
    exec /app/server
# Si el argumento es "client", ejecutar cliente
elif [ "$1" = "client" ]; then
    # Si hay segundo argumento, es la IP del servidor
    if [ $# -eq 2 ]; then
        echo "Iniciando CLIENTE conectando a $2..."
        exec /app/client "$2"
    else
        echo "Iniciando CLIENTE (localhost)..."
        exec /app/client
    fi
# Si el argumento es "both", ejecutar ambos
elif [ "$1" = "both" ]; then
    echo "Iniciando SERVIDOR en segundo plano..."
    /app/server &
    sleep 2
    echo "Iniciando CLIENTE..."
    exec /app/client localhost
else
    echo "Uso: entrypoint.sh [server|client|both] [server_ip]"
    echo "  server - Ejecuta solo el servidor (por defecto)"
    echo "  client [ip] - Ejecuta solo el cliente"
    echo "  both - Ejecuta servidor y cliente juntos"
    exit 1
fi