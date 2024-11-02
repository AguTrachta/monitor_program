/**
 * @file main.c
 * @brief Punto de entrada principal del sistema.
 *
 * Este archivo contiene el código principal que inicializa y actualiza las métricas del sistema
 * y las expone a través de un servidor HTTP utilizando un hilo independiente.
 */

#include "../include/expose_metrics.h"
#include "../include/json_metrics.h"
#include "../include/metrics.h"
#include <complex.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#define SLEEP_TIME 1 ///< Intervalo de tiempo en segundos entre cada actualización de métricas.

/**
 * @brief Función principal del programa.
 *
 * Inicializa las métricas del sistema (CPU, memoria, disco, red, procesos y cambios de contexto)
 * y crea un hilo para exponer las métricas a través de un servidor HTTP en el puerto 8000.
 *
 * El programa entra en un bucle infinito, actualizando las métricas periódicamente.
 *
 * @param argc Número de argumentos de línea de comandos (no utilizados).
 * @param argv Lista de argumentos de línea de comandos (no utilizados).
 * @return `EXIT_SUCCESS` si el programa se ejecuta correctamente, `EXIT_FAILURE` si ocurre un error.
 */
volatile sig_atomic_t keep_running = 1;

int main(int argc, char* argv[])
{

    // Inicialización de las métricas del sistema
    init_metrics();
    init_disk_metrics();
    init_network_metrics();
    init_count_processes();
    init_context_switches_metric();

    // Iniciar el hilo para exponer las métricas
    pthread_t tid;
    if (pthread_create(&tid, NULL, expose_metrics, NULL) != 0)
    {
        fprintf(stderr, "Error al crear el hilo del servidor HTTP\n");
        return EXIT_FAILURE;
    }

    // Bucle principal para actualizar las métricas
    while (keep_running)
    {
        update_disk_metrics();
        update_cpu_gauge();
        update_memory_gauge();
        update_network_metrics();
        update_count_processes();
        update_context_switches_metric();

        send_metrics_as_json();

        sleep(SLEEP_TIME);
    }

    return EXIT_SUCCESS;
}
