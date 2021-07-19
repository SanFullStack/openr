/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <gflags/gflags.h>

#include <openr/common/Constants.h>

// openr config file
DECLARE_string(config);

// drain workflow flags
DECLARE_bool(override_drain_state);

// migration workflow flags
DECLARE_bool(enable_bgp_route_programming);

// bgp stateful ha
DECLARE_string(spr_ha_state_file);
DECLARE_bool(bgp_enable_stateful_ha);

// security related flags
DECLARE_bool(enable_secure_thrift_server);
DECLARE_string(x509_cert_path);
DECLARE_string(x509_key_path);
DECLARE_string(x509_ca_path);
DECLARE_string(tls_ticket_seed_path);
DECLARE_string(tls_ecc_curve_name);
DECLARE_string(tls_acceptable_peers);

// TODO: Deprecate when port reference is gone for ZMQ
DECLARE_int32(openr_ctrl_port);
DECLARE_int32(kvstore_rep_port);

//
// TODO: All following flags will be removed once GflagsConfig.h is gone.
//

DECLARE_string(domain);
DECLARE_string(listen_addr);
DECLARE_string(node_name);
DECLARE_bool(dryrun);
DECLARE_string(loopback_iface);
DECLARE_string(seed_prefix);

DECLARE_bool(enable_prefix_alloc);
DECLARE_int32(alloc_prefix_len);
DECLARE_bool(static_prefix_alloc);

DECLARE_bool(set_loopback_address);
DECLARE_bool(override_loopback_addr);

DECLARE_string(iface_regex_include);
DECLARE_string(iface_regex_exclude);
DECLARE_string(redistribute_ifaces);

DECLARE_bool(enable_rtt_metric);
DECLARE_bool(enable_v4);

DECLARE_int32(decision_graceful_restart_window_s);

DECLARE_uint64(step_detector_fast_window_size);
DECLARE_uint64(step_detector_slow_window_size);
DECLARE_uint32(step_detector_lower_threshold);
DECLARE_uint32(step_detector_upper_threshold);
DECLARE_uint64(step_detector_ads_threshold);

DECLARE_bool(enable_netlink_fib_handler);

DECLARE_int32(link_flap_initial_backoff_ms);
DECLARE_int32(link_flap_max_backoff_ms);

DECLARE_bool(enable_rib_policy);

DECLARE_bool(enable_segment_routing);
DECLARE_bool(set_leaf_node);

DECLARE_string(key_prefix_filters);
DECLARE_string(key_originator_id_filters);

DECLARE_int32(kvstore_flood_msg_per_sec);
DECLARE_int32(kvstore_flood_msg_burst_size);
DECLARE_int32(kvstore_key_ttl_ms);
DECLARE_int32(kvstore_sync_interval_s);
DECLARE_int32(kvstore_ttl_decrement_ms);

DECLARE_bool(enable_flood_optimization);
DECLARE_bool(is_flood_root);

DECLARE_int32(spark_mcast_port);
DECLARE_int32(spark_hold_time_s);
DECLARE_int32(spark2_heartbeat_hold_time_s);
DECLARE_int32(spark2_hello_time_s);
DECLARE_int32(spark2_hello_fastinit_time_ms);
DECLARE_int32(spark2_heartbeat_time_s);

DECLARE_bool(prefix_fwd_type_mpls);
DECLARE_bool(prefix_algo_type_ksp2_ed_ecmp);

DECLARE_int32(bgp_local_as);
DECLARE_string(bgp_router_id);
DECLARE_int32(bgp_hold_time_s);
DECLARE_int32(bgp_gr_time_s);
DECLARE_string(bgp_peer_addr);
DECLARE_int32(bgp_confed_as);
DECLARE_int32(bgp_remote_as);
DECLARE_bool(bgp_is_confed);
DECLARE_bool(bgp_is_rr_client);
DECLARE_int32(bgp_thrift_port);
DECLARE_string(bgp_nexthop4);
DECLARE_string(bgp_nexthop6);
DECLARE_bool(bgp_nexthop_self);
DECLARE_bool(bgp_override_auto_config);
DECLARE_uint32(bgp_min_nexthop);

DECLARE_uint32(monitor_max_event_log);
DECLARE_bool(enable_event_log_submission);
