/**
 * @file prom_histogram.h
 * @brief Histogram metric for Prometheus.
 *
 * See https://prometheus.io/docs/concepts/metric_types/#histogram for more details on histograms.
 */

#ifndef PROM_HISTOGRAM_INCLUDED
#define PROM_HISTOGRAM_INCLUDED

#include <stdlib.h>

#include "prom_histogram_buckets.h"
#include "prom_metric.h"

/**
 * @brief A Prometheus histogram.
 *
 * References:
 * * See https://prometheus.io/docs/concepts/metric_types/#histogram
 */
typedef prom_metric_t prom_histogram_t;

/**
 * @brief Construct a prom_histogram_t*.
 *
 * @param name The name of the metric.
 * @param help The metric description.
 * @param buckets The prom_histogram_buckets_t*. See prom_histogram_buckets.h.
 * @param label_key_count The number of labels associated with the given metric. Pass 0 if the metric does not
 *                        require labels.
 * @param label_keys A collection of label keys. The number of keys MUST match the value passed as label_key_count. If
 *                   no labels are required, pass NULL. Otherwise, it may be convenient to pass this value as a
 *                   literal.
 * @return The constructed prom_histogram_t*.
 *
 * *Example*
 *
 *     // An example with labels
 *     prom_histogram_buckets_t* buckets = prom_histogram_buckets_linear(5.0, 5.0, 10);
 *     prom_histogram_new("foo", "foo is a counter with labels", buckets, 2, (const char**) { "one", "two" });
 *
 *     // An example without labels
 *     prom_histogram_buckets_t* buckets = prom_histogram_buckets_linear(5.0, 5.0, 10);
 *     prom_histogram_new("foo", "foo is a counter without labels", buckets, 0, NULL);
 */
prom_histogram_t* prom_histogram_new(const char* name, const char* help, prom_histogram_buckets_t* buckets,
                                     size_t label_key_count, const char** label_keys);

/**
 * @brief Destroy a prom_histogram_t*. The pointer self MUST be set to NULL after destruction.
 *
 * @param self The target prom_histogram_t* to destroy.
 * @return Non-zero value upon failure.
 */
int prom_histogram_destroy(prom_histogram_t* self);

/**
 * @brief Observe the prom_histogram_t given the value and labels.
 *
 * @param self The target prom_histogram_t*.
 * @param value The value to observe.
 * @param label_values A collection of label values. The number of values MUST match the label count of the histogram.
 *                     Pass NULL if no labels are used.
 * @return Non-zero value upon failure.
 */
int prom_histogram_observe(prom_histogram_t* self, double value, const char** label_values);

#endif // PROM_HISTOGRAM_INCLUDED
