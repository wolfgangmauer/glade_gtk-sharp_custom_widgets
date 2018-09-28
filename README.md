# glade_gtk-sharp_custom_widgets
Make gtk-sharp cusom-widgets work with glade-designer

## What do you need:

### 1.: runtime-assembly with the custom-widgets

### 2.: static gtype-register class/method for custom-widgets

### 3.: optional editor class/assembly used inside glade-designer

### 4.: glade catalog xml

## see example

```sh
gcc -m64 -o main.o -c main.c -fPIC -Wall `pkg-config --libs --cflags gtk+-3.0 gmodule-export-2.0 gladeui-2.0 mono-2 glib-2.0 gconf-2.0`

gcc -m64 -shared -ldl -o libglade_gtk-sharp_custom_widgets.so  main.o -fPIC -Wall `pkg-config --libs --cflags gtk+-3.0 gmodule-export-2.0 gladeui-2.0 mono-2 glib-2.0 gconf-2.0`
```
