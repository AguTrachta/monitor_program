#include "../include/metrics.h"

// Definir constantes simbólicas para evitar magic numbers
#define MEMINFO_BUFFER_SIZE 256
#define STAT_BUFFER_SIZE 1024
#define DISKSTATS_BUFFER_SIZE 512
#define NETDEV_BUFFER_SIZE 512
#define LOADAVG_BUFFER_SIZE 128
#define PROC_MEMINFO "/proc/meminfo"
#define PROC_STAT "/proc/stat"
#define PROC_DISKSTATS "/proc/diskstats"
#define PROC_NET_DEV "/proc/net/dev"
#define PROC_LOADAVG "/proc/loadavg"
#define INTERFACE_NAME_SIZE 32
#define SDA_DISK "sda"

// Función para obtener el uso de memoria
double get_memory_usage()
{
    FILE* fp;
    char buffer[MEMINFO_BUFFER_SIZE];
    unsigned long long total_mem = 0, free_mem = 0;

    // Abrir el archivo /proc/meminfo
    fp = fopen(PROC_MEMINFO, "r");
    if (fp == NULL)
    {
        perror("Error al abrir " PROC_MEMINFO);
        return -1.0;
    }

    // Leer los valores de memoria total y disponible
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "MemTotal: %llu kB", &total_mem) == 1)
        {
            continue; // MemTotal encontrado
        }
        if (sscanf(buffer, "MemAvailable: %llu kB", &free_mem) == 1)
        {
            break; // MemAvailable encontrado, podemos dejar de leer
        }
    }

    fclose(fp);

    // Verificar si se encontraron ambos valores
    if (total_mem == 0 || free_mem == 0)
    {
        fprintf(stderr, "Error al leer la información de memoria desde " PROC_MEMINFO "\n");
        return -1.0;
    }

    // Calcular el porcentaje de uso de memoria
    double used_mem = total_mem - free_mem;
    double mem_usage_percent = (used_mem / total_mem) * 100.0;

    return mem_usage_percent;
}

double get_cpu_usage()
{
    static unsigned long long prev_user = 0, prev_nice = 0, prev_system = 0, prev_idle = 0, prev_iowait = 0,
                              prev_irq = 0, prev_softirq = 0, prev_steal = 0;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    unsigned long long totald, idled;
    static double cpu_usage_percent = 0.0; // Valor inicial para mantener el último valor válido

    // Abrir el archivo /proc/stat
    FILE* fp = fopen(PROC_STAT, "r");
    if (fp == NULL)
    {
        perror("Error al abrir " PROC_STAT);
        return cpu_usage_percent;
    }

    char buffer[STAT_BUFFER_SIZE];
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        perror("Error al leer " PROC_STAT);
        fclose(fp);
        return cpu_usage_percent;
    }
    fclose(fp);

    // Analizar los valores de tiempo de CPU
    int ret = sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait,
                     &irq, &softirq, &steal);
    if (ret < 8)
    {
        fprintf(stderr, "Error al parsear " PROC_STAT "\n");
        return cpu_usage_percent;
    }

    // Calcular las diferencias entre las lecturas actuales y anteriores
    unsigned long long prev_idle_total = prev_idle + prev_iowait;
    unsigned long long idle_total = idle + iowait;

    unsigned long long prev_non_idle = prev_user + prev_nice + prev_system + prev_irq + prev_softirq + prev_steal;
    unsigned long long non_idle = user + nice + system + irq + softirq + steal;

    unsigned long long prev_total = prev_idle_total + prev_non_idle;
    unsigned long long total = idle_total + non_idle;

    totald = total - prev_total;
    idled = idle_total - prev_idle_total;

    if (totald == 0)
    {
        fprintf(stderr, "Totald es cero, manteniendo el último valor de uso de CPU\n");
        return cpu_usage_percent; // Retorna el último valor válido en lugar de -1
    }

    // Calcular el porcentaje de uso de CPU
    cpu_usage_percent = ((double)(totald - idled) / totald) * 100.0;

    // Actualizar los valores anteriores para la siguiente lectura
    prev_user = user;
    prev_nice = nice;
    prev_system = system;
    prev_idle = idle;
    prev_iowait = iowait;
    prev_irq = irq;
    prev_softirq = softirq;
    prev_steal = steal;

    return cpu_usage_percent;
}

// Función para obtener estadísticas de disco
DiskStats get_disk_stats()
{
    FILE* fp;
    char buffer[DISKSTATS_BUFFER_SIZE];
    DiskStats stats = {0, 0, 0, 0}; // Inicializar a 0

    // Abrir el archivo /proc/diskstats
    fp = fopen(PROC_DISKSTATS, "r");
    if (fp == NULL)
    {
        perror("Error al abrir " PROC_DISKSTATS);
        return stats;
    }

    // Leer estadísticas de disco para el disco 'sda'
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        unsigned int major, minor;
        char device_name[INTERFACE_NAME_SIZE];
        unsigned long long rd_ios, rd_merges, rd_sectors, rd_ticks, wr_ios, wr_merges, wr_sectors, wr_ticks;

        // leer las columnas relevantes de /proc/diskstats
        int ret = sscanf(buffer, "%u %u %s %llu %llu %llu %llu %llu %llu %llu %llu", &major, &minor, device_name,
                         &rd_ios, &rd_merges, &rd_sectors, &rd_ticks, &wr_ios, &wr_merges, &wr_sectors, &wr_ticks);

        // Verifica si sscanf logró capturar todas las columnas esperadas
        if (ret >= 11)
        {

            if (strcmp(device_name, SDA_DISK) == 0)
            {

                stats.reads = rd_ios;
                stats.writes = wr_ios;
                stats.read_time = rd_ticks;
                stats.write_time = wr_ticks;
                break; // Encontrado el disco, salir del bucle
            }
        }
        else
        {
            printf("sscanf no pudo leer las 11 columnas. Retorno: %d\n", ret); // Depuración para ver por qué falla
        }
    }

    fclose(fp);
    return stats;
}

// Función para obtener estadísticas de red
NetStats get_network_stats(const char* interface_name)
{
    FILE* fp;
    char buffer[NETDEV_BUFFER_SIZE];
    NetStats stats = {0, 0, 0, 0};        // Inicializar las métricas a 0
    char iface_name[INTERFACE_NAME_SIZE]; // Variable para almacenar temporalmente el nombre de la interfaz

    // Abrir el archivo /proc/net/dev
    fp = fopen(PROC_NET_DEV, "r");
    if (fp == NULL)
    {
        perror("Error al abrir " PROC_NET_DEV);
        return stats;
    }

    // Leer el archivo línea por línea
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        // Leer el nombre de la interfaz y saltar el resto de los datos para verificar si es la interfaz correcta
        sscanf(buffer, "%31[^:]:", iface_name);

        // Comprobamos si la interfaz coincide con la que estamos buscando
        if (strcmp(iface_name, interface_name) == 0)
        {
            // Si es la interfaz correcta, ahora leemos los valores de bytes y paquetes
            sscanf(buffer, "%*[^:]: %llu %llu %*u %*u %*u %*u %*u %*u %llu %llu", &stats.bytes_received,
                   &stats.packets_received, &stats.bytes_transmitted, &stats.packets_transmitted);
            break; // Salir del bucle una vez encontrada la interfaz
        }
    }

    fclose(fp);
    return stats;
}

// Función para obtener el número de procesos en ejecución
int get_running_processes()
{
    FILE* fp;
    char buffer[LOADAVG_BUFFER_SIZE];
    int running_processes = 0, total_processes = 0;

    // Abrir el archivo /proc/loadavg
    fp = fopen(PROC_LOADAVG, "r");
    if (fp == NULL)
    {
        perror("Error al abrir " PROC_LOADAVG);
        return -1;
    }

    // Leer el contenido del archivo
    if (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        // Extraer el número de procesos en ejecución y el total de procesos
        sscanf(buffer, "%*f %*f %*f %d/%d", &running_processes, &total_processes);
    }

    fclose(fp);
    return running_processes; // Devolver el número de procesos en ejecución
}

// Función para obtener el número de cambios de contexto
unsigned long long get_context_switches()
{
    FILE* fp;
    char buffer[STAT_BUFFER_SIZE];
    unsigned long long context_switches = 0;

    // Abrir el archivo /proc/stat
    fp = fopen(PROC_STAT, "r");
    if (fp == NULL)
    {
        perror("Error al abrir " PROC_STAT);
        return 0;
    }

    // Leer el archivo línea por línea
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        // Buscamos la línea que comienza con "ctxt"
        if (sscanf(buffer, "ctxt %llu", &context_switches) == 1)
        {
            break; // Hemos encontrado la línea de cambios de contexto
        }
    }

    fclose(fp);
    return context_switches; // número total de cambios de contexto
}
