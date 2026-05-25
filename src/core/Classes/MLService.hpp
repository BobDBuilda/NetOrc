#ifndef ML_SERVICE_HPP
#define ML_SERVICE_HPP

#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>

/**
 * MLService: Handles flow classification and distribution decisions.
 * This is where you would integrate libraries like LibTorch, ONNX Runtime, or TensorFlow Lite.
 */
class MLService {
public:
    struct FlowFeatures {
        uint32_t src_ip;
        uint32_t dst_ip;
        uint16_t src_port;
        uint16_t dst_port;
        uint8_t protocol;
        size_t packet_length;
    };

    MLService() {
        // Initialize your ML model here (e.g., load weights)
        std::cout << "[MLService] Initializing ML Model..." << std::endl;
    }

    /**
     * Predict the optimal output port or action for a given flow.
     * In a real-world scenario, this would run inference using a loaded model.
     */
    uint16_t predictOutputPort(const FlowFeatures& features) {
        // Placeholder for ML Inference logic.
        // E.g., model->forward(tensor_from_features)
        
        std::cout << "[MLService] Predicting for Flow: IP " 
                  << (features.src_ip & 0xFF) << "." << ((features.src_ip >> 8) & 0xFF) << " -> "
                  << (features.dst_ip & 0xFF) << "." << ((features.dst_ip >> 8) & 0xFF) 
                  << " [Size: " << features.packet_length << "]" << std::endl;

        // Mock decision logic:
        // High-volume flows could be routed to high-capacity links.
        if (features.packet_length > 1200) {
            return 2; // Route to Port 2
        }
        return 1;     // Route to Port 1
    }
};

#endif // ML_SERVICE_HPP
