# 🖼️ Wachar

Wachar es un visor de imágenes ultraminimalista para Wayland escrito completamente en C++.

### Sistema

* Wayland
* wayland-protocols
* EGL
* OpenGL ES 2.0

### Librerías incluidas

* stb_image

## Compilación

Ejemplo genérico:

```bash
make clean
make
```


Dependiendo de la distribución pueden ser necesarias rutas adicionales para los protocolos XDG Shell generados.

## Uso

```bash
wachar imagen.png

wachar fondo.jpg
```

## Controles

| Tecla | Acción |
| ----- | ------ |
| ESC   | Salir  |
| Q     | Salir  |

## Filosofía

* Abrir una imagen.
* Mostrarla.
* Salir.
