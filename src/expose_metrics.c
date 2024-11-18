#include "../include/expose_metrics.h"
#include "../../../lib/memory/include/memory.h"
#include "../../../lib/memory/include/stats_memory.h"
#include <prom_collector_registry.h>
#include <prom_gauge.h>
#include <pthread.h>

/** Mutex para sincronización de hilos */
pthread_mutex_t lock;

/** Métrica de Prometheus para el uso de CPU */
static prom_gauge_t *cpu_usage_metric;

/** Métrica de Prometheus para el uso de memoria */
static prom_gauge_t *memory_usage_metric;

/* Metricas de Prometheus para esrituras y lecturas de disco */
static prom_gauge_t *disk_reads_metric;
static prom_gauge_t *disk_writes_metric;
static prom_gauge_t *disk_read_time_metric;
static prom_gauge_t *disk_write_time_metric;

/* Metricas de Prometheus para ancho de banda y promedio de paquetes */
static prom_gauge_t *network_bandwidth_tx_metric;
static prom_gauge_t *network_bandwidth_rx_metric;
static prom_gauge_t *network_packet_ratio_metric;

/* Metrica de prometheus para el conteo de procesos */
static prom_gauge_t *count_processes_metric;

/* Metrica de prometheus para los cambios de contexto */
static prom_gauge_t *context_switches_metric;

/* Metrics for memory fragmentation rate per allocation strategy */
static prom_gauge_t *memory_fragmentation_first_fit_metric;
static prom_gauge_t *memory_fragmentation_best_fit_metric;
static prom_gauge_t *memory_fragmentation_worst_fit_metric;

/* Metrics for allocation counts per strategy */
static prom_gauge_t *first_fit_allocations_metric;
static prom_gauge_t *best_fit_allocations_metric;
static prom_gauge_t *worst_fit_allocations_metric;

/* Metrics for average allocation time per strategy */
static prom_gauge_t *first_fit_avg_allocation_time_metric;
static prom_gauge_t *best_fit_avg_allocation_time_metric;
static prom_gauge_t *worst_fit_avg_allocation_time_metric;

void update_cpu_gauge() {
  double usage = get_cpu_usage();
  if (usage >= 0) {
    pthread_mutex_lock(&lock);
    prom_gauge_set(cpu_usage_metric, usage, NULL);
    pthread_mutex_unlock(&lock);
  } else {
    fprintf(stderr, "Error al obtener el uso de CPU\n");
  }
}

void update_memory_gauge() {
  double usage = get_memory_usage();
  if (usage >= 0) {
    pthread_mutex_lock(&lock);
    prom_gauge_set(memory_usage_metric, usage, NULL);
    pthread_mutex_unlock(&lock);
  } else {
    fprintf(stderr, "Error al obtener el uso de memoria\n");
  }
}

void update_disk_metrics() {
  DiskStats stats = get_disk_stats();

  // Convertir las operaciones de lectura y escritura a miles (dividir entre
  // 1000)
  double reads_in_thousands = stats.reads / 1000.0;
  double writes_in_thousands = stats.writes / 1000.0;
  // Actualizar las métricas de lectura, escritura y tiempo dedicado en
  // Prometheus
  pthread_mutex_lock(&lock);
  prom_gauge_set(disk_reads_metric, (double)stats.reads, NULL);
  prom_gauge_set(disk_writes_metric, (double)stats.writes, NULL);
  prom_gauge_set(disk_read_time_metric, reads_in_thousands, NULL);
  prom_gauge_set(disk_write_time_metric, writes_in_thousands, NULL);
  pthread_mutex_unlock(&lock);
}

void update_network_metrics() {
  static NetStats prev_stats = {0, 0, 0,
                                0}; // Para almacenar los valores previos
  NetStats current_stats = get_network_stats("wlp2s0");

  // Calcular el ancho de banda (en bytes por segundo)
  double bandwidth_rx =
      (double)(current_stats.bytes_received - prev_stats.bytes_received);
  double bandwidth_tx =
      (double)(current_stats.bytes_transmitted - prev_stats.bytes_transmitted);

  // Calcular la relación de paquetes
  double packet_ratio = 0;
  if (current_stats.packets_received > 0) {
    packet_ratio = (double)current_stats.packets_transmitted /
                   (double)current_stats.packets_received;
  }

  pthread_mutex_lock(&lock);
  prom_gauge_set(network_bandwidth_rx_metric, bandwidth_rx,
                 NULL); // Ancho de banda en recepción
  prom_gauge_set(network_bandwidth_tx_metric, bandwidth_tx,
                 NULL); // Ancho de banda en transmisión
  prom_gauge_set(network_packet_ratio_metric, packet_ratio,
                 NULL); // Relación de paquetes
  pthread_mutex_unlock(&lock);

  prev_stats = current_stats;
}

void update_count_processes() {
  int running_processes = get_running_processes();
  if (running_processes >= 0) {
    pthread_mutex_lock(&lock);
    prom_gauge_set(count_processes_metric, (double)running_processes, NULL);
    pthread_mutex_unlock(&lock);
  } else {
    fprintf(stderr, "Error al obtener el numero de procesos en ejecucion\n");
  }
}

unsigned long long prev_context_switches = 0;

void update_context_switches_metric() {
  unsigned long long current_context_switches = get_context_switches();

  if (current_context_switches > 0) {
    unsigned long long diff = current_context_switches - prev_context_switches;

    if (diff > 0) {
      pthread_mutex_lock(&lock);
      prom_gauge_set(context_switches_metric, (double)diff, NULL);
      pthread_mutex_unlock(&lock);
    }

    prev_context_switches = current_context_switches;
  } else {
    fprintf(stderr, "Error al obtener los cambios de contexto\n");
  }
}

void *expose_metrics(void *arg) {
  (void)arg; // Argumento no utilizado

  // Aseguramos que el manejador HTTP esté adjunto al registro por defecto
  promhttp_set_active_collector_registry(NULL);

  // Iniciamos el servidor HTTP en el puerto 8000
  struct MHD_Daemon *daemon =
      promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000, NULL, NULL);
  if (daemon == NULL) {
    fprintf(stderr, "Error al iniciar el servidor HTTP\n");
    return NULL;
  }

  // Mantenemos el servidor en ejecución
  while (1) {
    sleep(1);
  }

  // Nunca debería llegar aquí
  MHD_stop_daemon(daemon);
  return NULL;
}

void init_metrics() {
  // Inicializamos el mutex
  if (pthread_mutex_init(&lock, NULL) != 0) {
    fprintf(stderr, "Error al inicializar el mutex\n");
  }

  // Inicializamos el registro de coleccionistas de Prometheus
  if (prom_collector_registry_default_init() != 0) {
    fprintf(stderr, "Error al inicializar el registro de Prometheus\n");
  }

  // Creamos la métrica para el uso de CPU
  cpu_usage_metric = prom_gauge_new("cpu_usage_percentage",
                                    "Porcentaje de uso de CPU", 0, NULL);
  if (cpu_usage_metric == NULL) {
    fprintf(stderr, "Error al crear la métrica de uso de CPU\n");
  }

  // Registramos la métrica de CPU
  if (prom_collector_registry_must_register_metric(cpu_usage_metric) != 0) {
    fprintf(stderr, "Error al registrar las métricas de CPU\n");
  }

  // Creamos la métrica para el uso de memoria
  memory_usage_metric = prom_gauge_new("memory_usage_percentage",
                                       "Porcentaje de uso de memoria", 0, NULL);
  if (memory_usage_metric == NULL) {
    fprintf(stderr, "Error al crear la métrica de uso de memoria\n");
  }

  // Registramos la métrica de memoria
  if (prom_collector_registry_must_register_metric(memory_usage_metric) != 0) {
    fprintf(stderr, "Error al registrar las métricas de memoria\n");
  }

  // Initialize fragmentation rate metrics for each allocation strategy
  memory_fragmentation_first_fit_metric =
      prom_gauge_new("memory_fragmentation_rate_first_fit",
                     "Memory fragmentation rate (%) for First Fit", 0, NULL);
  prom_collector_registry_must_register_metric(
      memory_fragmentation_first_fit_metric);

  memory_fragmentation_best_fit_metric =
      prom_gauge_new("memory_fragmentation_rate_best_fit",
                     "Memory fragmentation rate (%) for Best Fit", 0, NULL);
  prom_collector_registry_must_register_metric(
      memory_fragmentation_best_fit_metric);

  memory_fragmentation_worst_fit_metric =
      prom_gauge_new("memory_fragmentation_rate_worst_fit",
                     "Memory fragmentation rate (%) for Worst Fit", 0, NULL);
  prom_collector_registry_must_register_metric(
      memory_fragmentation_worst_fit_metric);

  // Initialize allocation counts metrics
  first_fit_allocations_metric =
      prom_gauge_new("first_fit_allocations_total",
                     "Total allocations using First Fit", 0, NULL);
  prom_collector_registry_must_register_metric(first_fit_allocations_metric);

  best_fit_allocations_metric =
      prom_gauge_new("best_fit_allocations_total",
                     "Total allocations using Best Fit", 0, NULL);
  prom_collector_registry_must_register_metric(best_fit_allocations_metric);

  worst_fit_allocations_metric =
      prom_gauge_new("worst_fit_allocations_total",
                     "Total allocations using Worst Fit", 0, NULL);
  prom_collector_registry_must_register_metric(worst_fit_allocations_metric);

  // Initialize average allocation time metrics
  first_fit_avg_allocation_time_metric = prom_gauge_new(
      "first_fit_avg_allocation_time",
      "Average allocation time for First Fit (seconds)", 0, NULL);
  prom_collector_registry_must_register_metric(
      first_fit_avg_allocation_time_metric);

  best_fit_avg_allocation_time_metric =
      prom_gauge_new("best_fit_avg_allocation_time",
                     "Average allocation time for Best Fit (seconds)", 0, NULL);
  prom_collector_registry_must_register_metric(
      best_fit_avg_allocation_time_metric);

  worst_fit_avg_allocation_time_metric = prom_gauge_new(
      "worst_fit_avg_allocation_time",
      "Average allocation time for Worst Fit (seconds)", 0, NULL);
  prom_collector_registry_must_register_metric(
      worst_fit_avg_allocation_time_metric);
}

void init_disk_metrics() {
  // Creamos la métrica para operaciones de lectura en disco
  disk_reads_metric = prom_gauge_new(
      "disk_reads_operations",
      "Número de operaciones de lectura en el disco (en miles)", 0, NULL);
  if (disk_reads_metric == NULL) {
    fprintf(stderr,
            "Error al crear la métrica de operaciones de lectura de disco\n");
  }

  // Registramos la métrica de lectura de disco
  if (prom_collector_registry_must_register_metric(disk_reads_metric) != 0) {
    fprintf(
        stderr,
        "Error al registrar las métricas de operaciones de lectura de disco\n");
  }

  // Creamos la métrica para operaciones de escritura en disco
  disk_writes_metric = prom_gauge_new(
      "disk_writes_operations",
      "Número de operaciones de escritura en el disco (en miles)", 0, NULL);
  if (disk_writes_metric == NULL) {
    fprintf(stderr,
            "Error al crear la métrica de operaciones de escritura de disco\n");
  }

  // Registramos la métrica de escritura de disco
  if (prom_collector_registry_must_register_metric(disk_writes_metric) != 0) {
    fprintf(stderr, "Error al registrar las métricas de operaciones de "
                    "escritura de disco\n");
  }

  // Creamos la métrica para el tiempo dedicado a operaciones de lectura
  disk_read_time_metric = prom_gauge_new(
      "disk_read_time", "Tiempo dedicado a operaciones de lectura (segundos)",
      0, NULL);
  if (disk_read_time_metric == NULL) {
    fprintf(stderr,
            "Error al crear la métrica de tiempo de lectura de disco\n");
  }

  // Registramos la métrica de tiempo de lectura de disco
  if (prom_collector_registry_must_register_metric(disk_read_time_metric) !=
      0) {
    fprintf(stderr,
            "Error al registrar las métricas de tiempo de lectura de disco\n");
  }

  // Creamos la métrica para el tiempo dedicado a operaciones de escritura
  disk_write_time_metric = prom_gauge_new(
      "disk_write_time",
      "Tiempo dedicado a operaciones de escritura (segundos)", 0, NULL);
  if (disk_write_time_metric == NULL) {
    fprintf(stderr,
            "Error al crear la métrica de tiempo de escritura de disco\n");
  }

  // Registramos la métrica de tiempo de escritura de disco
  if (prom_collector_registry_must_register_metric(disk_write_time_metric) !=
      0) {
    fprintf(
        stderr,
        "Error al registrar las métricas de tiempo de escritura de disco\n");
  }
}

void init_network_metrics() {
  // Creamos las métricas para el ancho de banda
  network_bandwidth_rx_metric =
      prom_gauge_new("network_bandwidth_receive",
                     "Ancho de banda de recepción (bytes/segundo)", 0, NULL);
  network_bandwidth_tx_metric =
      prom_gauge_new("network_bandwidth_transmit",
                     "Ancho de banda de transmisión (bytes/segundo)", 0, NULL);
  network_packet_ratio_metric =
      prom_gauge_new("network_packet_ratio",
                     "Relación de paquetes transmitidos/recibidos", 0, NULL);

  // Registramos las métricas en el registro por defecto
  prom_collector_registry_must_register_metric(network_bandwidth_rx_metric);
  prom_collector_registry_must_register_metric(network_bandwidth_tx_metric);
  prom_collector_registry_must_register_metric(network_packet_ratio_metric);
}

void init_count_processes() {
  // Creamos la métrica para el número de procesos en ejecución
  count_processes_metric = prom_gauge_new(
      "running_processes_count", "Número de procesos en ejecución", 0, NULL);
  if (count_processes_metric == NULL) {
    fprintf(stderr, "Error al crear la métrica de procesos en ejecución\n");
  }

  // Registramos la métrica en el registro por defecto
  if (prom_collector_registry_must_register_metric(count_processes_metric) !=
      0) {
    fprintf(stderr, "Error al registrar la métrica de procesos en ejecución\n");
  }
}

void init_context_switches_metric() {
  // Creamos la métrica para los cambios de contexto
  context_switches_metric = prom_gauge_new(
      "context_switches_total", "Número total de cambios de contexto", 0, NULL);
  if (context_switches_metric == NULL) {
    fprintf(stderr, "Error al crear la métrica de cambios de contexto\n");
  }

  // Registramos la métrica en el registro por defecto
  if (prom_collector_registry_must_register_metric(context_switches_metric) !=
      0) {
    fprintf(stderr, "Error al registrar la métrica de cambios de contexto\n");
  }
}

void destroy_mutex() { pthread_mutex_destroy(&lock); }

void update_memory_fragmentation_metric() {
  double fragmentation_rates[3] = {0.0, 0.0, 0.0};
  calculate_fragmentation_per_method(fragmentation_rates);

  pthread_mutex_lock(&lock);
  prom_gauge_set(memory_fragmentation_first_fit_metric,
                 fragmentation_rates[FIRST_FIT], NULL);
  prom_gauge_set(memory_fragmentation_best_fit_metric,
                 fragmentation_rates[BEST_FIT], NULL);
  prom_gauge_set(memory_fragmentation_worst_fit_metric,
                 fragmentation_rates[WORST_FIT], NULL);
  pthread_mutex_unlock(&lock);
}

void update_allocation_policy_metrics() {
  pthread_mutex_lock(&lock);

  // Update allocation counts
  prom_gauge_set(first_fit_allocations_metric, (double)first_fit_count, NULL);
  prom_gauge_set(best_fit_allocations_metric, (double)best_fit_count, NULL);
  prom_gauge_set(worst_fit_allocations_metric, (double)worst_fit_count, NULL);

  // Calculate average allocation times
  double first_fit_avg_time =
      first_fit_allocation_count > 0
          ? first_fit_allocation_time / first_fit_allocation_count
          : 0.0;
  double best_fit_avg_time =
      best_fit_allocation_count > 0
          ? best_fit_allocation_time / best_fit_allocation_count
          : 0.0;
  double worst_fit_avg_time =
      worst_fit_allocation_count > 0
          ? worst_fit_allocation_time / worst_fit_allocation_count
          : 0.0;

  // Update average allocation time metrics
  prom_gauge_set(first_fit_avg_allocation_time_metric, first_fit_avg_time,
                 NULL);
  prom_gauge_set(best_fit_avg_allocation_time_metric, best_fit_avg_time, NULL);
  prom_gauge_set(worst_fit_avg_allocation_time_metric, worst_fit_avg_time,
                 NULL);

  pthread_mutex_unlock(&lock);
}
