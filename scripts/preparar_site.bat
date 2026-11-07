@echo off
cd /d "%~dp0.."
if not exist "src\esp32\esp32_server\data" mkdir "src\esp32\esp32_server\data"
copy /Y "src\client\index.html" "src\esp32\esp32_server\data\" >nul
copy /Y "src\client\style.css"  "src\esp32\esp32_server\data\" >nul
copy /Y "src\client\app.js"     "src\esp32\esp32_server\data\" >nul
echo Site copiado para src\esp32\esp32_server\data
pause
