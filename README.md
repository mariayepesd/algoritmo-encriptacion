# Algoritmo de Encriptación – Cliente/Servidor con Docker

## Elaborado por:

Jacobo Osorio
Vannesa Alfaro
Maria José Yepes

Este proyecto implementa un **sistema de validación de tokens para ejecutar transacciones** usando una arquitectura **Cliente–Servidor** desarrollada en **C++** y ejecutada mediante **Docker**. El objetivo es simular un entorno donde un cliente solicita un token, el servidor lo genera y valida, y luego el cliente usa ese token para realizar una transacción.

---

## 🚀 Objetivo del Proyecto

El propósito de esta aplicación es mostrar cómo funcionan:

* La comunicación Cliente–Servidor usando **sockets TCP**.
* La generación y validación de un **token temporal** para autorizar operaciones o transacciones.
* El uso de **Docker** para empaquetar entorno de ejecución.
* Un flujo simple de autenticación tipo **Clave dinámica / token temporal**.

---

## 🧩 Estructura del Proyecto

```
algoritmo-encriptacion/
├── cliente/
│   ├── cliente.cpp          // Código del cliente
│   ├── Dockerfile
│   └── entrypoint.sh
├── servidor/
│   ├── servidor.cpp         // Código del servidor
│   ├── Dockerfile
│   └── entrypoint.sh
├── docker-compose.yml       // Orquesta cliente y servidor
└── README.md                // (Este archivo)
```

---

## ⚙️ Requisitos Previos

Antes de ejecutar el proyecto es necesario tener:

* **Docker**
* **Docker Compose**

Para verificar:

```bash
docker --version
docker compose version
```

---

## ▶️ ¿Cómo Ejecutarlo?

El proyecto está diseñado para ejecutarse **automáticamente** usando Docker Compose.

### 1. Clonar el repositorio

```bash
git clone https://github.com/mariayepesd/algoritmo-encriptacion.git
cd algoritmo-encriptacion
```

### 2. Construir y ejecutar

```bash
docker compose up --build
```

Esto hará lo siguiente:

* Construirá la imagen del **servidor**.
* Construirá la imagen del **cliente**.
* Iniciará el servidor.
* Luego iniciará el cliente.
* El cliente solicitará un token y luego intentará ejecutar la transacción.

Se verá en la consola mensajes como:

* Servidor escuchando en el puerto 5000.
* Cliente conectado al servidor.
* Token generado.
* Validación correcta.
* Transacción ejecutada.

### 3. Para detener todo

```bash
docker compose down
```

---

## 🖥️ Flujo de Funcionamiento

1. El **cliente** se conecta al servidor mediante sockets.
2. Envía la solicitud: `TOKEN`.
3. El **servidor genera un token** y se lo envía.
4. El cliente recibe el token y envía una transacción con ese token.
5. El **servidor valida**:

   * Si el token es correcto → "Transacción autorizada".
   * Si es incorrecto → "Token inválido".

Todo se ejecuta automáticamente gracias al script `entrypoint.sh` en cada servicio.

---

## 🧪 Ejecución Individual (Opcional)

Si se desea correr solo un servicio:

### Servidor

```bash
docker build -t servidor ./servidor
```

```bash
docker run -p 5000:5000 servidor
```

### Cliente

```bash
docker build -t cliente ./cliente
```

```bash
docker run cliente
```

---

## 🛠 Tecnologías Usadas

* **C++**
* **Sockets TCP**
* **Docker**
* **Docker Compose**
* **Bash (entrypoints)**

---

## 📌 Notas Importantes

* El cliente espera que el servidor esté activo antes de conectarse.
* Los servicios están configurados en la misma red interna de Docker.
* Todo el flujo ocurre automáticamente.
