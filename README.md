# NQL Project

NQL (NoKado Query Language) es una implementación simplificada de una base de datos relacional en memoria con una interfaz de línea de comandos similar a SQL. Este proyecto es desarrollado con fines educativos para comprender los fundamentos de bases de datos y lenguajes de consulta.

## Características

* Interfaz de línea de comandos interactiva con autocompletado
* Sintaxis inspirada en SQL
* Soporte para múltiples tipos de datos (INT, FLOAT, STRING, BOOL)
* Operaciones básicas de CRUD:
  * CREATE TABLE - Creación de tablas
  * ALTER TABLE - Modificación de tablas
  * INSERT INTO - Inserción de datos
  * SELECT - Consulta de datos
  * DELETE FROM - Eliminación de datos
  * DESCRIBE - Visualización de estructura de tabla

## Compilación e instalación

### Requisitos previos

* Compilador GCC
* Entorno Linux
* Biblioteca readline (para la interfaz de línea de comandos)
* Make
* Valgrind 

### Compilación

```bash
make
```

### Ejecución

```bash
make run
```

o directamente:

```bash
./bin/nql_cli
```

## Uso básico

```
# Crear una tabla
NQL> CREATE TABLE usuarios

# Añadir columnas
NQL> ALTER TABLE usuarios ADD COLUMN id INT PRIMARY KEY NOT NULL
NQL> ALTER TABLE usuarios ADD COLUMN nombre STRING(50) NOT NULL
NQL> ALTER TABLE usuarios ADD COLUMN edad INT
NQL> ALTER TABLE usuarios ADD COLUMN genero STRING(1)

# Insertar datos
NQL> INSERT INTO usuarios VALUES (1, "Juan", 25, "M")
NQL> INSERT INTO usuarios VALUES (2, "Ana", 30, "F")

# Consultar datos
NQL> SELECT * FROM usuarios

# Ver estructura de la tabla
NQL> DESCRIBE usuarios

# Eliminar datos
NQL> DELETE FROM usuarios WHERE rowid = 0
```

## Estructura del proyecto

```
NQL_Project/
├── src/                          # Código fuente principal
│   ├── main.c                    # Punto de entrada principal
│   ├── cli/                      # Todo lo relacionado con la interfaz de comandos
│   │   ├── cli.c/h               # Procesamiento de comandos y entradas
│   │   ├── input_handler.c/h     # Manejo de entrada y readline
│   │   └── commands/             # Comandos específicos
│   ├── db/                       # Motor de base de datos
│   │   ├── database.c/h          # API de la base de datos
│   │   ├── table.c/h             # Operaciones sobre tablas
│   │   ├── column.c/h            # Operaciones con columnas
│   │   ├── row.c/h               # Operaciones con filas
│   │   └── value.c/h             # Tipos de datos y valores
│   └── utils/                    # Utilidades generales
├── include/                      # Cabeceras públicas
│   └── nql.h                     # API pública
├── Makefile                      # Script de compilación
└── README.md                     # Documentación
```

## Limitaciones actuales

* No hay persistencia de datos (la base de datos existe solo en memoria)
* No hay soporte para consultas complejas como JOIN o GROUP BY
* No hay validación de integridad referencial
* No hay transacciones
* Sin soporte para subconsultas

## Contribuciones

Las contribuciones son bienvenidas. Por favor, asegúrate de seguir las convenciones de código existentes y añadir pruebas para las nuevas funcionalidades.