// BrNN experiment

#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>


#define FILE "data.txt"
#define INPUT_NODE_INDEX -1
constexpr const int is_positive[] = {-1, 1};


class Sample {
    public:
    std::vector<float> input;
    std::vector<float> output;
};



class intFloat {
    public:
    int i;
    float floatValue;
};


class Memory {
   public:
   float accumulator = 0;
   // bool activate;
   // float value;
   // float loss;
   // any other info
};


class NodeV3 { // NodeV3 is basically identical to NodeV2, except for newValue
    public:
    int index = 0;


    // What should be recorded in memory
    float threshold = 0;
    float floor = 0;
    float multiplier = 1;


    float value = 0;
    float newValue = 0;


    float resourceLimit = 0; // Innovation
    float resourcesUsed = 0;


    float avgLoss = 0;
    float adjLoss = 0;

    float use = 0;

   // Somehow encode expectations of predeceding nodes' values
   // which would collectively show what it expects what following nodes want

   std::vector<intFloat> connected_from;

   std::vector<Memory> memory;
};


std::string extract(std::string filename);
std::vector<Sample> parseData(const std::string& data);
void printModel(const std::vector<NodeV3>& nodes);
std::vector<std::array<float, 1>> runModel(
    std::vector<NodeV3>& nodes,
    Sample sample
);


int main(int argc, char *argv[]) {
    srand(time(NULL));

    std::vector<NodeV3> nodes;
    // std::vector<intFloat> activeNodes; // Try 1
    std::string data = extract(FILE);
    std::cout << data << "\n";

    std::vector<Sample> samples;
    //samples = parseData(data);
    //std::cout << (float)3.14 << std::endl;

    for (int i = 0; i < 20; i++) {
        Sample sample;
        float randomN = fmod((float)random() / 2987746, 187.3);
        sample.input.push_back(randomN);
        sample.output.push_back(randomN * 2 + 17);

        samples.push_back(sample);
    }


    // Nodes initialization
    for (int i = 0; i < 8; i++) {
        NodeV3 node;
        node.index = i;
        node.floor = (float)25 - fmod((float)random() / 2987746, 50);
        node.threshold = (float)10 - fmod((float)random() / 2987746, 20);
        node.multiplier = 1;

        if ((i + 2) % 3 == 0) {
            intFloat connect;
            connect.i = -1;
            connect.floatValue = (float)10 - fmod((float)random() / 2987746, 20);
            node.connected_from.push_back(connect);
        }

        int k = random() % 5;
        for (int j = 0; j < k; j++) {
            intFloat connect;
            connect.i = random() % 8;
            connect.floatValue = (float)10 - fmod((float)random() / 2987746, 20);
            node.connected_from.push_back(connect);
        }

        nodes.push_back(node);
    }


    for (auto sample: samples) {
        std::cout << "Here: " << sample.input.front() << "\t" << sample.output.front() <<"\n";
        runModel(nodes, sample);
        std::cout << "There\n";
    }

    return 0;
}


void printModel(const std::vector<NodeV3>& nodes) {
    std::cout << "index\tthreshold\tfloor\n";
    for (auto node: nodes) {
        std::cout << node.index << std::fixed << std::setw(15) << std::setprecision(5) << node.threshold << std::setw(12)  << node.floor << "| ";
        for (auto connections: node.connected_from) {
            std::cout << "(" << std::setw(2) << connections.i << ", " << std::fixed << std::setprecision(6) << std::setw(9) <<  connections.floatValue << "), ";
        }
        std::cout << "\n";
    }
}


void updateNodeValues(std::vector<NodeV3>& nodes) {
    for (auto& node: nodes) {
        node.value = node.newValue;
        node.use = 0;
        node.avgLoss = 0;
        node.adjLoss = 0;
        node.newValue = 0;
    }
}

void addMemory(std::vector<NodeV3>& nodes) {
	for (auto& node: nodes) {
		node.memory.push_back(Memory{});
	}
}

void setValue(std::vector<NodeV3>& nodes) {
	for (auto& node: nodes) {
		node.newValue = node.multiplier * (std::max((float)0.0, node.threshold) - node.threshold) + node.floor;
	}
}

std::vector<std::array<float, 1>> runModel(
    std::vector<NodeV3>& nodes,
    Sample sample
) { // Runs and updates the model

    int timestep = 0;
    std::vector<std::array<float, 1>> output; // generalize

    addMemory(nodes);
    setValue(nodes);
    updateNodeValues(nodes);

    // implement analytical mode (Most strength per computing resource)
    while (1) {
        if (timestep > 7) break; //  primitive handling of the braking system
        
        addMemory(nodes);

        for (auto &node: nodes) { // forward calculation
            float accumulator = 0;

            for (auto inputtingNodeBi: node.connected_from) {
                const float value = inputtingNodeBi.i >= 0 ? nodes[inputtingNodeBi.i].value: sample.input[-inputtingNodeBi.i - 1];
                accumulator += inputtingNodeBi.floatValue * value;
            }

            // std::cout << accumulator << std::endl;
            if (accumulator > 500) std::cout << "Hit Ceiling" << std::endl;
            accumulator = std::clamp(accumulator, (float)-500, (float)-500);
            
            node.newValue = node.multiplier * (std::max(accumulator, node.threshold) - node.threshold) + node.floor;
            node.memory[node.memory.size() - 1].accumulator = accumulator;
          /* threshold, floor, multiplier
           * node.memory.value = node.newValue; 				    // ? Memory has no use yet
           * node.memory.activate = accumulator > node.threshold; 	// ? Not needed in current state adjustments as node.newValue or accumulator > node.threshold gives it
           */


            for (auto inputtingNodeBi: node.connected_from) { // Computing loss from average
                const float loss = (accumulator > node.threshold || nodes[inputtingNodeBi.i].value > 0) ?
                    node.multiplier * (nodes[inputtingNodeBi.i].value - std::max(accumulator, node.threshold)):
                    0;

                // Pressures for each part of the node, f, t, m
                nodes[inputtingNodeBi.i].avgLoss += std::abs(inputtingNodeBi.floatValue) * std::abs(loss) * loss;
		        nodes[inputtingNodeBi.i].use += std::abs(inputtingNodeBi.floatValue);
            } // Use an array to hold the loss of each node ?
            // Try to store everything in arrays in accordance with FP
            // Missing node connections corrections, new connnections and new nodes
        }


        // Should node[0] be connected to input (somehow)?
        const float loss = nodes[0].newValue - sample.output[0];
        nodes[0].avgLoss = std::abs(loss) * loss / (std::exp(-1.4 * ((double)timestep - 3.2)));
        nodes[0].use = 1.0;


        for (auto node: nodes) { // Adjusted correction from received loss
            if (node.use == 0) continue;
            const float adjAcc = node.memory[node.memory.size() - 2].accumulator - (double)is_positive[node.avgLoss > 0] * std::sqrt(std::abs(node.avgLoss))/node.use;
            
            for (auto inputtingNodeBi: node.connected_from) {
                const float loss = (adjAcc > node.threshold || nodes[inputtingNodeBi.i].value > 0) ?
                    node.multiplier * (nodes[inputtingNodeBi.i].value - std::max(adjAcc, node.threshold)):
                    0;

             	// Pressures for each part of the node, f, t, m
	       	    nodes[inputtingNodeBi.i].adjLoss += std::abs(inputtingNodeBi.floatValue) * std::abs(loss) * loss;
            }
        }

        nodes[0].adjLoss = nodes[0].avgLoss;
        
        for (auto& node: nodes) { // Correction of node states from loss
            if (node.use == 0) continue;
            float scaledLoss = (double)is_positive[node.avgLoss > 0] * std::sqrt(std::abs(node.avgLoss))/node.use;

            node.floor -= scaledLoss;
            // Most radical case should have loss swinging back and forth

            node.multiplier = std::clamp(node.multiplier, (float)-300, (float)300);
            node.threshold  = std::clamp(node.threshold,  (float)-500, (float)500);
            node.floor      = std::clamp(node.floor,      (float)-500, (float)500);
		    // Are these clamps necessary? Because they will 'eventually' significantly limit the freedom of the BbNN parameter space
        }


        // Forming output
        output.push_back(std::array<float, 1>{ nodes[0].newValue });

        updateNodeValues(nodes);
        timestep++;
    }


    for (auto outputSlice: output){
        std::cout << outputSlice[0] << std::endl;
    }

    printModel(nodes);

    return output;
}

// Utilities

std::string extract(std::string filename) {
    std::ifstream file;
    std::ostringstream content;
    file.open(filename);
    content << file.rdbuf();
    file.close();


    return content.str();
}


std::vector<Sample> parseData(const std::string& data) {
    std::vector<Sample> samples;


    // Split the input string into lines
    std::istringstream iss(data);
    std::string line;


    while (std::getline(iss, line)) {
        Sample sample;


        // Find the position of 'I:(' and 'O:('
        size_t inputStart = line.find("I:(") + 3;
        size_t inputEnd = line.find("), O:(");
        size_t outputStart = inputEnd + 7;
        size_t outputEnd = line.find(")");


        // Parse input and output substrings
        std::string inputStr = line.substr(inputStart, inputEnd - inputStart);
        std::string outputStr = line.substr(outputStart, outputEnd - outputStart);


        // Parse individual integers from the input string and populate the input vector
        std::istringstream inputStream(inputStr);
        int num;
        while (inputStream >> num) {
            sample.input.push_back(num);


            // Check for the comma separator
            if (inputStream.peek() == ',') {
                inputStream.ignore();
            }
        }


        // Parse individual integers from the output string and populate the output vector
        std::istringstream outputStream(outputStr);
        while (outputStream >> num) {
            sample.output.push_back(num);


            // Check for the comma separator
            if (outputStream.peek() == ',') {
                outputStream.ignore();
            }
        }


        // Add the populated sample to the vector
        samples.push_back(sample);
    }


    return samples;
}

