# NetOrc

This project is a custom **Software Defined Networking (SDN) Controller** built in C++. It acts as the "brain" of a network, separating the control logic from the underlying hardware.

The project is currently in its **initial development phase**, with the primary focus on building the **Southbound Interface (SBI)**.

---

### ## Project Structure

The directory is organized to keep the network logic separate from the communication protocols:

* **`src/api/`**: The communication layers.
    * **`sbi/`**: **(Current Focus)** The "Southbound" layer that talks directly to network switches.
    * **`nbi/`**: The "Northbound" layer for future external apps/dashboards.
* **`src/core/`**: The main logic (pathfinding, topology, and switch management).
* **`src/utils/`**: General helper tools and logging.

---

### ## Current Status: Southbound Development

We are currently building the bridge between the controller and the switches. This involves:
* **Switch Handshaking:** Establishing a connection when a switch joins the network.
* **Packet Handling:** Reading and "understanding" data packets sent from the hardware.
* **Flow Control:** Telling the switches where to send traffic.

---

### ## How to Build
1.  Ensure you have a **C++ compiler** and **CMake** installed.
2.  Run the following:
    ```bash
    mkdir build && cd build
    cmake ..
    make
    ```

### ## Observability

NetOrc now exposes telemetry-friendly interfaces for standard tooling:

- Structured logs: JSON on stdout (`ts`, `service`, `level`, `message`, and context fields).
- Metrics endpoint: `GET /metrics` in Prometheus exposition format.
- Health endpoint: `GET /health`.
- Correlation ID: `X-Request-Id` accepted/returned on `POST /events`.

Configuration:

- `NETORC_SERVICE_NAME` (default: `netorc`)
- `NETORC_NBI_PORT` (default: `8192`)
- CLI flags:
  - `--debug`
  - `--verbose`
  - `--service-name=<name>`
  - `--nbi-port=<port>`

Prometheus example scrape target:

```yaml
scrape_configs:
  - job_name: netorc
    metrics_path: /metrics
    static_configs:
      - targets: ["localhost:8192"]
```

Datadog/OpenTelemetry:

- Use log collection from stdout (container logs or systemd journal).
- Scrape `http://<host>:<port>/metrics` with Datadog OpenMetrics integration or OTel collector.

Local stack (Prometheus + Grafana + optional Datadog Agent):

```bash
docker compose -f docker-compose.observability.yml up -d --build
```

Access:
- NetOrc: `http://localhost:8192/health`
- Prometheus: `http://localhost:9090`
- Grafana: `http://localhost:3000` (admin/admin)

To include Datadog Agent:

```bash
DD_API_KEY=<your_key> docker compose -f docker-compose.observability.yml --profile datadog up -d --build
```

### ## Roadmap
* [x] Basic Directory Setup
* [ ] **In Progress:** Reliable Switch Connection (SBI)
* [ ] Network Topology Mapping
* [ ] REST API for Northbound Control

---

Would you like me to add a **"Core Technologies"** section listing the specific C++ libraries you're using for this?
