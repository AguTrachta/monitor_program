#include "../include/json_metrics.h"
#include "../include/metrics.h"
#include <cjson/cJSON.h>
#include <stdio.h>

void send_metrics_as_json() {
  cJSON *root = cJSON_CreateObject();

  // Recolectar las métricas de CPU, memoria, disco, red, procesos y cambios de
  // contexto
  double cpu_usage = get_cpu_usage();
  double memory_usage = get_memory_usage();
  DiskStats disk_stats = get_disk_stats();
  NetStats network_stats = get_network_stats("wlp2s0");
  int running_processes = get_running_processes();
  unsigned long long context_switches = get_context_switches();

  // Añadir métricas al objeto JSON
  cJSON_AddNumberToObject(root, "cpu_usage_percentage", cpu_usage);
  cJSON_AddNumberToObject(root, "memory_usage_percentage", memory_usage);
  cJSON_AddNumberToObject(root, "disk_reads", disk_stats.reads);
  cJSON_AddNumberToObject(root, "disk_writes", disk_stats.writes);
  cJSON_AddNumberToObject(root, "disk_read_time_seconds",
                          disk_stats.read_time / 1000.0);
  cJSON_AddNumberToObject(root, "disk_write_time_seconds",
                          disk_stats.write_time / 1000.0);
  cJSON_AddNumberToObject(root, "network_bandwidth_rx",
                          network_stats.bytes_received);
  cJSON_AddNumberToObject(root, "network_bandwidth_tx",
                          network_stats.bytes_transmitted);
  cJSON_AddNumberToObject(root, "network_packet_ratio",
                          (network_stats.packets_received > 0)
                              ? (double)network_stats.packets_transmitted /
                                    network_stats.packets_received
                              : 0.0);
  cJSON_AddNumberToObject(root, "running_processes_count", running_processes);
  cJSON_AddNumberToObject(root, "context_switches_total", context_switches);

  // Convertir el objeto JSON a string y enviarlo al pipe
  char *json_data = cJSON_Print(root);
  if (json_data != NULL) {
    FILE *pipe =
        fopen("/tmp/monitor_pipe",
              "a"); // "a" mantiene el pipe abierto para múltiples escrituras
    if (pipe) {
      fprintf(pipe, "%s\n", json_data);
      fflush(pipe); // Asegura que los datos se escriban inmediatamente
      fclose(pipe);
    } else {
      perror("Error al abrir el pipe para enviar métricas");
    }

    free(json_data);
  }

  // Limpiar el objeto JSON
  cJSON_Delete(root);
}
