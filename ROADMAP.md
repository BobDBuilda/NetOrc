# NetOrc Roadmap — 10k Lines Goal

Current: ~2,800 lines of real code (excluding vendored `httplib.h`).
Target: **10,000 lines** — a production-capable SDN controller.

## Priority 1: Critical (~3,200 lines)

| Feature | Est. Lines | Why |
|---|---|---|
| **OF 1.3 support** (version negotiation, multi-table, IPv6/MPLS matches, group tables, meters) | 1,500 | Modern switches speak OF 1.3+. OF 1.0 alone is obsolete. |
| **Proactive LLDP discovery** (periodic `PACKET_OUT` on all ports, timer-driven) | 500 | Controller must actively discover topology, not just react. |
| **FLOW_MOD generation** (install, delete, modify rules — not mock data) | 400 | Core purpose of an SDN controller. |
| **REST API: flows/switches/topology** (CRUD for flow tables, switch inventory, topology graph, port stats) | 800 | Northbound is how every consumer talks to the controller. |

## Priority 2: High (~1,500 lines)

| Feature | Est. Lines | Why |
|---|---|---|
| **Echo request/reply keepalive** | 200 | Detect dead switches instead of waiting for TCP timeout. |
| **FEATURES_REPLY parsing** (capabilities, ports, DPID, buffers) | 300 | Currently the controller ignores switch capabilities. |
| **Path computation** (Dijkstra, ECMP, loop-free) | 400 | Without this you can't route traffic across the topology. |
| **Southbound abstraction layer** (interface + plugin registry) | 600 | SBI must not be hardcoded to OF 1.0 forever. |

## Priority 3: Nice-to-Have (~1,500 lines)

| Feature | Est. Lines | Why |
|---|---|---|
| **Basic auth / API keys on NBI** | 400 | Anyone on the network can call every endpoint today. |
| **Web UI with topology graph** | 600 | Visual > `printTopology()` to stdout. |
| **Port/flow stats collection** (periodic `OFPT_STATS_REQUEST`) | 400 | Visibility into switch state. |
| **Graceful shutdown + signal handling** | 100 | `SIGTERM` should clean up, not kill -9. |

## Won't Fit in 10k

Clustering/HA, TLS for SBI (DTLS), P4Runtime, NETCONF/YANG, full OF 1.5,
distributed tracing, role-based access control.
