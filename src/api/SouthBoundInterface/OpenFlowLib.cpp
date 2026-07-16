#include "OpenFlowLib.h"
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace oflib {

Server::Server(size_t worker_threads) 
    : pool_(std::make_unique<ThreadEnvironment>(worker_threads)),
      event_queue_(std::make_unique<TaskQueue<Event>>()) {
}

Server::~Server() {
    stop();
}

bool Server::listen(const std::string& host, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(sock);
        return false;
    }

    if (::listen(sock, SOMAXCONN) < 0) {
        ::close(sock);
        return false;
    }

    running_ = true;
    acceptor_thread_ = std::thread(&Server::accept_loop, this, sock);
    return true;
}

void Server::stop() {
    running_ = false;
    if (acceptor_thread_.joinable()) {
        // In a simple refactor, we just detach. 
        // A better approach would be to shutdown the listen socket first.
        acceptor_thread_.detach(); 
    }

    std::lock_guard<std::mutex> lock(connections_mutex_);
    for (auto& pair : connections_) {
        close_connection(pair.second);
    }
    connections_.clear();
}

void Server::wait_for_event(Connection& out_conn, Frame& out_frame) {
    Event ev;
    event_queue_->wait_and_pop(ev);
    out_conn = ev.conn;
    out_frame = std::move(ev.frame);
}

void Server::accept_loop(int listen_sock) {
    while (running_) {
        int client_fd = accept(listen_sock, nullptr, nullptr);
        if (client_fd >= 0) {
            {
                std::lock_guard<std::mutex> lock(connections_mutex_);
                connections_[client_fd] = Connection{client_fd};
            }
            
            pool_->enqueue([this, client_fd]() {
                handle_client(client_fd);
            });
        }
    }
    ::close(listen_sock);
}

void Server::handle_client(int client_fd) {
    std::vector<uint8_t> scratch(8192);
    
    while (running_) {
        Connection* conn = nullptr;
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            auto it = connections_.find(client_fd);
            if (it == connections_.end()) break;
            conn = &it->second;
        }

        ssize_t n = 0;
        if (!recv_into_buffer(*conn, scratch, n)) {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            close_connection(*conn);
            connections_.erase(client_fd);
            break;
        }

        if (!ingest(*conn, scratch.data(), static_cast<size_t>(n))) {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            close_connection(*conn);
            connections_.erase(client_fd);
            break;
        }
    }
}

void Server::dispatch(Connection& conn, PacketType type, const Frame& frame) const {
    OpenFlowEndpoint::dispatch(conn, type, frame);
    event_queue_->push(Event{conn, frame});
}

} // namespace oflib
