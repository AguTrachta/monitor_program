# Nombre del ejecutable
TARGET = metrics

# Directorios de código fuente y encabezados
SRC_DIR = src
INCLUDE_DIR = include

# Archivos fuente
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/expose_metrics.c $(SRC_DIR)/metrics.c

# Librerías
LIBS = -lprom -pthread -lpromhttp
LDFLAGS = -L/usr/local/lib
CFLAGS = -I$(INCLUDE_DIR) -I/usr/local/include/

# Exportar la variable de entorno LD_LIBRARY_PATH
export LD_LIBRARY_PATH := /usr/local/lib:$(LD_LIBRARY_PATH)

# Regla por defecto (se ejecuta al correr 'make')
all: $(TARGET)

# Regla para compilar el programa
$(TARGET): $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $(TARGET)

# Regla para limpiar los archivos generados
clean:
	rm -f $(TARGET)

