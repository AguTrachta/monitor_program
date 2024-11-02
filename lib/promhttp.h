/**
 * @file promhttp.h
 * @brief Provides a HTTP endpoint for metric exposition.
 *
 * References:
 *   * MHD_FLAG: https://www.gnu.org/software/libmicrohttpd/manual/libmicrohttpd.html#microhttpd_002dconst
 *   * MHD_AcceptPolicyCallback:
 *     https://www.gnu.org/software/libmicrohttpd/manual/libmicrohttpd.html#index-_002aMHD_005fAcceptPolicyCallback
 */

#include <string.h>

#include "microhttpd.h"
#include "prom_collector_registry.h"

/**
 * @brief Sets the active registry for metric scraping.
 *
 * @param active_registry The target prom_collector_registry_t*. If NULL is passed, the default registry is used.
 *                        The registry MUST be initialized.
 */
void promhttp_set_active_collector_registry(prom_collector_registry_t* active_registry);

/**
 * @brief Starts a daemon in the background and returns a pointer to a MHD_Daemon.
 *
 * This function starts a background HTTP server daemon to expose Prometheus metrics.
 *
 * References:
 *   * https://www.gnu.org/software/libmicrohttpd/manual/libmicrohttpd.html#microhttpd_002dinit
 *
 * @param flags The flags for the daemon (e.g., MHD_USE_SELECT_INTERNALLY).
 * @param port The port on which the daemon will listen.
 * @param apc The accept policy callback for incoming connections.
 * @param apc_cls The closure argument passed to the accept policy callback.
 * @return A pointer to the initialized MHD_Daemon structure, or NULL on failure.
 */
struct MHD_Daemon* promhttp_start_daemon(unsigned int flags, unsigned short port, MHD_AcceptPolicyCallback apc,
                                         void* apc_cls);

