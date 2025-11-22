# Dockerfile - Imagen única con Server y Client
# Ejecuta ambos programas en el mismo contenedor

# ============================================
# ETAPA 1: Compilación
# ============================================
FROM alpine:3.18 AS builder

# Instalar herramientas de compilación
RUN apk add --no-cache g++ make

# Crear directorio de trabajo
WORKDIR /build

# Copiar código fuente del servidor
COPY server/server.cpp ./server.cpp

# Copiar código fuente del cliente
COPY client/client.cpp ./client.cpp

# Compilar servidor
RUN g++ -O3 -static -std=c++17 -o server server.cpp && \
    strip server && \
    chmod +x server

# Compilar cliente
RUN g++ -O3 -static -std=c++17 -o client client.cpp && \
    strip client && \
    chmod +x client

# Verificar compilación
RUN ls -lh server client && \
    echo "Servidor compilado: $(stat -c%s server) bytes" && \
    echo "Cliente compilado: $(stat -c%s client) bytes"

# ============================================
# ETAPA 2: Imagen Final con BusyBox
# ============================================
FROM busybox:latest

# Crear directorio para los binarios
WORKDIR /app

# Copiar ambos ejecutables desde la etapa de compilación
COPY --from=builder /build/server /app/server
COPY --from=builder /build/client /app/client

# Copiar script de inicio
COPY entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh

# Exponer puerto del servidor
EXPOSE 8080

# Script que maneja qué ejecutar
ENTRYPOINT ["/app/entrypoint.sh"]