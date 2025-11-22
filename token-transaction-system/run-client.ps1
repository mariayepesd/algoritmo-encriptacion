#!/usr/bin/env pwsh
# run-client.ps1 - Ejecuta cliente con IP correcta automáticamente

Write-Host "`n==================================" -ForegroundColor Cyan
Write-Host "  INICIANDO CLIENTE" -ForegroundColor Cyan
Write-Host "==================================`n" -ForegroundColor Cyan

# Obtener IP del servidor
Write-Host "[INFO] Obteniendo IP del servidor..." -ForegroundColor Yellow
$SERVER_IP = docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' token_server

if ([string]::IsNullOrEmpty($SERVER_IP)) {
    Write-Host "[ERROR] No se pudo obtener la IP del servidor" -ForegroundColor Red
    Write-Host "        Asegurate de que token_server este corriendo" -ForegroundColor Red
    exit 1
}

Write-Host "[OK] Servidor encontrado en: $SERVER_IP" -ForegroundColor Green
Write-Host "`nConectando al servidor...`n" -ForegroundColor White

# Ejecutar cliente con la IP
docker exec -it token_client /app/client $SERVER_IP