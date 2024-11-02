/**
 * @file expose_metrics.h
 * @brief Encabezado para la gestión y exposición de métricas del sistema.
 *
 * Este archivo define las funciones necesarias para leer el uso de recursos del sistema (CPU, memoria, disco, red,
 * procesos y cambios de contexto) y exponer dichas métricas en formato Prometheus a través de un servidor HTTP.
 */

#ifndef EXPOSE_METRICS_H
#define EXPOSE_METRICS_H

#include "metrics.h"
#include <prom.h>
#include <promhttp.h>
#include <pthread.h>

#define BUFFER_SIZE 256 ///< Tamaño del búfer utilizado en las operaciones de lectura.

/**
 * @brief Actualiza las métricas de la red.
 *
 * Lee las estadísticas de la interfaz de red y actualiza las métricas relacionadas con el ancho de banda
 * y el número de paquetes transmitidos y recibidos.
 */
void update_network_metrics();

/**
 * @brief Actualiza la métrica de procesos en ejecución.
 *
 * Lee el número de procesos en ejecución desde el sistema y actualiza la métrica correspondiente.
 */
void update_count_processes();

/**
 * @brief Actualiza la métrica de cambios de contexto.
 *
 * Lee el número de cambios de contexto desde el sistema y actualiza la métrica correspondiente.
 */
void update_context_switches_metric();

/**
 * @brief Actualiza las métricas del disco.
 *
 * Actualiza las métricas relacionadas con las operaciones de lectura/escritura en los discos y los tiempos asociados.
 */
void update_disk_metrics();

/**
 * @brief Actualiza la métrica de uso de CPU.
 *
 * Calcula y actualiza el porcentaje de uso de CPU basado en los tiempos de ejecución de la CPU.
 */
void update_cpu_gauge();

/**
 * @brief Actualiza la métrica de uso de memoria.
 *
 * Calcula y actualiza el porcentaje de uso de memoria basado en los valores leídos desde el sistema.
 */
void update_memory_gauge();

/**
 * @brief Función del hilo para exponer las métricas vía HTTP en el puerto 8000.
 *
 * Esta función crea un servidor HTTP en el puerto 8000, donde se exponen todas las métricas registradas
 * para que Prometheus las recoja.
 *
 * @param arg Argumento no utilizado.
 * @return Siempre retorna `NULL`.
 */
void* expose_metrics(void* arg);

/**
 * @brief Inicializa las métricas del sistema.
 *
 * Inicializa las métricas relacionadas con CPU, memoria, disco y red, además de configurar los
 * mutex para la sincronización de acceso en los hilos.
 */
void init_metrics();

/**
 * @brief Inicializa las métricas de disco.
 *
 * Configura las métricas relacionadas con las operaciones de lectura/escritura en los discos.
 */
void init_disk_metrics();

/**
 * @brief Inicializa las métricas de red.
 *
 * Configura las métricas relacionadas con el tráfico de red, incluyendo ancho de banda y paquetes
 * transmitidos/recibidos.
 */
void init_network_metrics();

/**
 * @brief Inicializa las métricas de procesos en ejecución.
 *
 * Configura la métrica que cuenta el número de procesos en ejecución en el sistema.
 */
void init_count_processes();

/**
 * @brief Inicializa las métricas de cambios de contexto.
 *
 * Configura la métrica que cuenta los cambios de contexto que han ocurrido en el sistema.
 */
void init_context_switches_metric();

/**
 * @brief Destruye los mutex utilizados en la protección de las métricas.
 *
 * Libera los recursos asociados con los mutex utilizados para proteger el acceso a las métricas en los hilos.
 */
void destroy_mutex();

#endif // EXPOSE_METRICS_H

