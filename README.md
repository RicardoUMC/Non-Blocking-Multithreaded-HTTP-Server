# Servidor Web en C con Alberca de Hilos y Select

Este servidor web en C utiliza una alberca de hilos y la función `select` para gestionar múltiples conexiones de manera eficiente. El servidor es capaz de manejar solicitudes GET, enviando archivos estáticos desde el directorio de recursos.

## Estructura del Directorio

La estructura del proyecto asume la siguiente organización:

```
/
|- web_server.c           // El código fuente del servidor
|- resources/             // Directorio para los recursos servidos
   |- archivo_fuente
   |- archivo_prueba1
```

## Compilación

Para compilar el servidor, necesitas tener instalados GCC y las bibliotecas de desarrollo para `pthread`. Utiliza el siguiente comando para compilar:

```bash
gcc -o web_server web_server.c -pthread
```

Este comando compila el código fuente en el archivo `web_server.c` en un ejecutable llamado `web_server`, enlazando la biblioteca necesaria para la gestión de múltiples hilos.

## Ejecución

Para iniciar el servidor en tu máquina local, ejecuta el comando:

```bash
./web_server
```

Esto lanzará el servidor en el puerto `8080`. Asegúrate de que el puerto `8080` esté libre o modifica el puerto en el código fuente si es necesario.

## Pruebas

Para probar el servidor, puedes utilizar un navegador o una herramienta de línea de comandos como `curl`. Aquí te dejo algunos ejemplos de cómo puedes realizar solicitudes:

### Solicitar un Archivo Estático

```bash
curl http://localhost:8080/archivo_fuente
```

### Ver Detalles de la Comunicación

Para obtener más detalles sobre la solicitud y la respuesta, puedes usar el modo verboso de `curl`:

```bash
curl -v http://localhost:8080/archivo_fuente
```

## Notas Adicionales

- Este servidor es principalmente para propósitos educativos y de prueba. No incluye el manejo completo de todas las características de HTTP.
- Asegúrate de que el directorio `resources` contenga los archivos necesarios para las pruebas.
- Verifica que los permisos de los archivos y directorios sean adecuados para que el servidor pueda acceder a ellos.

