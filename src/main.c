/**
 * @file main.c
 * @brief Punto de entrada principal del sistema.
 *
 * Este archivo contiene el código principal que inicializa y actualiza las
 * métricas del sistema y las expone a través de un servidor HTTP utilizando un
 * hilo independiente.
 */

#include "../../../lib/memory/include/memory.h"
#include "../../../lib/memory/include/stats_memory.h"
#include "../include/expose_metrics.h"
#include "../include/json_metrics.h"
#include "../include/metrics.h"
#include <complex.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SLEEP_TIME                                                             \
  1 ///< Intervalo de tiempo en segundos entre cada actualización de métricas.

/**
 * @brief Función principal del programa.
 *
 * Inicializa las métricas del sistema (CPU, memoria, disco, red, procesos y
 * cambios de contexto) y crea un hilo para exponer las métricas a través de un
 * servidor HTTP en el puerto 8000.
 *
 * El programa entra en un bucle infinito, actualizando las métricas
 * periódicamente.
 *
 * @param argc Número de argumentos de línea de comandos (no utilizados).
 * @param argv Lista de argumentos de línea de comandos (no utilizados).
 * @return `EXIT_SUCCESS` si el programa se ejecuta correctamente,
 * `EXIT_FAILURE` si ocurre un error.
 */
volatile sig_atomic_t keep_running = 1;

void simulate_memory_operations();

int main(int argc, char *argv[]) {

  // Inicialización de las métricas del sistema
  init_metrics();
  // init_disk_metrics();
  // init_network_metrics();
  // init_count_processes();
  // init_context_switches_metric();

  // Iniciar el hilo para exponer las métricas
  pthread_t tid;
  if (pthread_create(&tid, NULL, expose_metrics, NULL) != 0) {
    fprintf(stderr, "Error al crear el hilo del servidor HTTP\n");
    return EXIT_FAILURE;
  }

  enable_unmapping = 0;
  // Bucle principal para actualizar las métricas
  while (keep_running) {
    // update_disk_metrics();
    // update_cpu_gauge();
    // update_memory_gauge();
    // update_network_metrics();
    // update_count_processes();
    // update_context_switches_metric();

    simulate_memory_operations();
    update_memory_fragmentation_metric();
    update_allocation_policy_metrics();

    send_metrics_as_json();

    sleep(SLEEP_TIME);
  }

  return EXIT_SUCCESS;
}

void simulate_memory_operations() {
  // Array of allocation methods
  int methods[] = {FIRST_FIT, BEST_FIT, WORST_FIT};
  const char *method_names[] = {"First Fit", "Best Fit", "Worst Fit"};

  // Arrays to keep track of allocations for each method
  static void *allocated_blocks[3][100];
  static int num_allocated[3] = {0, 0, 0};

  // Loop over each allocation method
  for (int m = 0; m < 3; m++) {
    malloc_control(methods[m]);

    // Randomly decide to allocate or free memory
    if (rand() % 2 == 0 && num_allocated[m] < 100) {
      // Allocate a new block of random size
      size_t size = (rand() % 256) + 16; // Sizes between 16 and 271 bytes
      void *ptr = my_malloc(size);
      if (ptr) {
        allocated_blocks[m][num_allocated[m]++] = ptr;
      }
    } else if (num_allocated[m] > 0) {
      // Free a random block
      int index = rand() % num_allocated[m];
      my_free(allocated_blocks[m][index]);
      // Remove the freed block from the list
      allocated_blocks[m][index] = allocated_blocks[m][num_allocated[m] - 1];
      num_allocated[m]--;
    }
  }
}
