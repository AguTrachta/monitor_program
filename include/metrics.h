/**
 * @file metrics.h
 * @brief Funciones para obtener estadísticas del sistema (CPU, memoria, disco, red, procesos y cambios de contexto)
 * desde el sistema de archivos /proc.
 */

#ifndef METRICS_H
#define METRICS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Tamaño del búfer utilizado para las operaciones de lectura.
 */
#define BUFFER_SIZE 256

/**
 * @brief Estructura para almacenar estadísticas de disco.
 */
typedef struct
{
    unsigned long long reads;      ///< Número de lecturas realizadas en el disco
    unsigned long long writes;     ///< Número de escrituras realizadas en el disco
    unsigned long long read_time;  ///< Tiempo dedicado a operaciones de lectura
    unsigned long long write_time; ///< Tiempo dedicado a operaciones de escritura
} DiskStats;

/**
 * @brief Estructura para almacenar estadísticas de red.
 */
typedef struct
{
    unsigned long long bytes_received;      ///< Bytes recibidos a través de la interfaz de red
    unsigned long long bytes_transmitted;   ///< Bytes transmitidos a través de la interfaz de red
    unsigned long long packets_received;    ///< Paquetes recibidos a través de la interfaz de red
    unsigned long long packets_transmitted; ///< Paquetes transmitidos a través de la interfaz de red
} NetStats;

/**
 * @brief Obtiene estadísticas de red (bytes y paquetes transmitidos y recibidos).
 *
 * Lee los valores de /proc/net/dev para una interfaz de red específica y devuelve las estadísticas de tráfico.
 *
 * @param interface_name Nombre de la interfaz de red a monitorear (e.g., "wlp2s0").
 * @return Estructura NetStats con los valores recolectados de la interfaz especificada.
 */
NetStats get_network_stats(const char* interface_name);

/**
 * @brief Obtiene el número de procesos en ejecución.
 *
 * Lee el número de procesos en ejecución desde /proc/loadavg.
 *
 * @return Número de procesos en ejecución, o -1 en caso de error.
 */
int get_running_processes();

/**
 * @brief Obtiene el número total de cambios de contexto del sistema.
 *
 * Lee el número de cambios de contexto desde /proc/stat.
 *
 * @return Número total de cambios de contexto, o 0 en caso de error.
 */
unsigned long long get_context_switches();

/**
 * @brief Obtiene estadísticas de disco (lecturas, escrituras y tiempos).
 *
 * Lee los valores de /proc/diskstats para un disco específico y devuelve las
 * lecturas, escrituras y tiempos de operación.
 *
 * @return Estructura DiskStats con los valores recolectados.
 */
DiskStats get_disk_stats();

/**
 * @brief Obtiene el porcentaje de uso de memoria desde /proc/meminfo.
 *
 * Lee los valores de memoria total y disponible desde /proc/meminfo y calcula
 * el porcentaje de uso de memoria.
 *
 * @return Uso de memoria como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_memory_usage();

/**
 * @brief Obtiene el porcentaje de uso de CPU desde /proc/stat.
 *
 * Lee los tiempos de CPU desde /proc/stat y calcula el porcentaje de uso de CPU
 * en un intervalo de tiempo.
 *
 * @return Uso de CPU como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_cpu_usage();

#endif // METRICS_H

