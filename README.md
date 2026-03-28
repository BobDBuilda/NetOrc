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

### ## Roadmap
* [x] Basic Directory Setup
* [ ] **In Progress:** Reliable Switch Connection (SBI)
* [ ] Network Topology Mapping
* [ ] REST API for Northbound Control

---

Would you like me to add a **"Core Technologies"** section listing the specific C++ libraries you're using for this?
