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

#define FILE "data.txt"
#define INPUT_NODE_INDEX -1
class Sample {
  public:
  std::vector<float> input;
  std::vector<float> output;
};


class bi_int {
  public:
  int index;
  float value;
};


class Node {
  public:
  int threshold = 0;
  int value; // ???
  // int timeStepRemoved = 0 ?;
  std::vector<int> lastInput{};
  std::vector<int> lastTarget{};
  // Test one dimensional first
  int lastX = 0;
  int lastY = 0;
  // int threshold = 0;
  int bias = 0;
  int weight = 0;
  std::vector<int> connected_from;
  int index;
  /* This idea is that the node picks the sources of input and tries to produce the most highly valued output. Who grades the output neurons? Going with this idea first. */
  std::vector<int> connected_to;
  /* This idea is that the node tries to decrease the level of chaos by controlling and picking other neurons. End Goal: The model develops a method to control its environment. */
};

class NodeV2 {
   public:
   float threshold = 0;
   float bias = 0;
   // int weight = 1;
   float value = 0; // ?
   
   // float points ?
   int index = 0;
   std::vector<bi_int> connected_from;
};

class Connect {
    public:
    int index;
    float strength;
};

class NodeV3 { // NodeV3 is basically identical to NodeV2, except for newValue
    public:
    int index = 0;

    float threshold = 0;
    float floor = 0;
    float multiplier;

    float value = 0;
    float newValue = 0;

    float resourceLimit = 0; // Innovation
    float resourcesUsed = 0;

    float overLoss;
    float underLoss;

    // Somehow encode expectations
    // Expectations of values of predeceding nodes would show what it expects what following nodes want

    // std::vector<> past A and loss
    std::vector<Connect> connected_from;
};

std::string extract(std::string filename);
std::vector<std::array<int, 1>> runModel(
  std::vector<NodeV2>& nodes,
  std::vector<bi_int> activeNodes,
  Sample sample
);
std::vector<Sample> parseData(const std::string& data);
void printModel(const std::vector<NodeV2>& nodes);
std::vector<std::array<int, 1>> runModel(
    std::vector<NodeV3>& nodes,
    Sample sample
);

int main(int argc, char *argv[]) {
    srand(time(NULL));

    std::vector<NodeV3> nodes;
    std::vector<bi_int> activeNodes; // Try 1
    std::string data = extract(FILE);
    std::cout << data << "\n";
    
    std::vector<Sample> samples;
    //samples = parseData(data);

    for (int i = 0; i < 35; i++) {
        Sample sample;
        float randomF = (float)random() * 30593 / 29877;
        float randomN = fmod(randomF, 200);
        sample.input.push_back(randomN);
        sample.output.push_back(randomN * 2 + 17);

        samples.push_back(sample);
    }

    // NodeV2 node;
    // bi_int connection;
    // connection.index = INPUT_NODE_INDEX;
    // connection.value = 1;
    // node.connected_from.push_back(connection);
    // node.index = 0;
    // nodes.push_back(node);

    // Nodes initialization
    for (int i = 0; i < 14; i++) {
        NodeV3 node;
        node.index = i;
        node.floor = 25 - random() % 50;
        node.threshold = 10 - random() % 20;
        node.multiplier = 1;

        if (i % 3 == 0) {
            Connect connect;
            connect.index = -1;
            connect.strength = 10 - random() % 20;
            node.connected_from.push_back(connect);
        }

        int k = random() % 5;
        for (int j = 0; j < k; j++) {
            Connect connect;
            connect.index = random() % 14;
            connect.strength = 10 - random() % 20;
            node.connected_from.push_back(connect);
        }

        nodes.push_back(node);
    }

    for (auto sample: samples) {
        // bi_int inputNode;
        // inputNode.index = INPUT_NODE_INDEX;
        // inputNode.value = sample.input.front();
        // activeNodes.push_back(inputNode);

        std::cout << "Here: " << sample.input.front() << "\t" << sample.output.front() <<"\n";
        runModel(nodes, sample);
        //runModel(nodes, activeNodes, sample); // timestep, chain ?
        std::cout << "There\n";
    }

    return 0;
}


/* Run Model Try 1, V2 */
std::vector<std::array<int, 1>> runModel(
    std::vector<NodeV2>& nodes,
    std::vector<bi_int> activeNodes,
    Sample sample
) { // Runs and updates the model

    int timestep = 0;
    std::vector<bi_int> currentNodes;
    std::vector<std::array<int, 1>> output; // generalize

    while (1) {
        if (timestep > 5) break; //  primitive handling of breaking system
        currentNodes.clear();

        for (auto &node: nodes) {
            int accumulator = node.bias; // node.value ?
            for (auto inputtingNode: node.connected_from) {
                auto it = std::find_if( // fix implementation
                    activeNodes.begin(),
                    activeNodes.end(),
                    [inputtingNode](bi_int other) {
                        return other.index == inputtingNode.index;
                    }
                );

                if (it != activeNodes.end()) {
                    accumulator += inputtingNode.value * it->value;
                }
            }

            bool activate = accumulator > node.threshold;
            node.value = activate ? accumulator : 0;

            for (auto& inputtingNode: node.connected_from) { // same for output nodes?
                auto it = std::find_if(
                    nodes.begin(),
                    nodes.end(),
                    [inputtingNode](NodeV2 other) {
                        return other.index == inputtingNode.index;
                    }
                ); // Expectations
                // These multipliers should change based off how wrong the signal was
                inputtingNode.value = (float)inputtingNode.value * (activate ? 1.5 : .75) + (accumulator ? 7: -6);
            }

            if (timestep > 4 && node.index == 0) { // ?
                for (auto& inputtingNode: node.connected_from) {
                    auto it = std::find_if(
                        nodes.begin(),
                        nodes.end(),
                        [inputtingNode](NodeV2 other) {
                            return other.index == inputtingNode.index;
                        }
                    );

                    if (activate) {
                        int error = accumulator - sample.output[0];
                        error = error * error / 256;
                        it->threshold += error;
                        it->bias += error / 2;
                        inputtingNode.value *= (error / 128);
                    } else {
                        it->bias += 100;
                        it->threshold -= 100;
                        inputtingNode.value += 35;
                    }
                }
            }

            if (activate) {
                bi_int current;
                current.index = node.index;
                current.value = accumulator;
                currentNodes.push_back(current);
            }
        }

        // Forming output
        std::array<int, 1> outputSlice;
        outputSlice[0] = nodes[0].value;
        output.push_back(outputSlice);

        activeNodes.clear();
        activeNodes = currentNodes;

        timestep++;
    }

    //std::cout << "End\n";
    for (auto outputSlice: output){
        std::cout << outputSlice[0] << std::endl;
    }

    //printModel(nodes);
    return output;
}


void printModel(const std::vector<NodeV2>& nodes) {
   std::cout << "index\tthreshold\tbias\n";
   for (auto node: nodes) {
       std::cout << node.index << "\t\t" << node.threshold << "\t\t\t" << node.bias << "\t| ";
       for (auto connections: node.connected_from) {
           std::cout << connections.index << " " << connections.value << ", ";
       }
       std::cout << "\n";
   }
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

void updateNodeValues(std::vector<NodeV3>& nodes) {
    for (auto& node: nodes) {
        node.value = node.newValue;
        node.newValue = 0;
    }
}

std::vector<std::array<int, 1>> runModel(
    std::vector<NodeV3>& nodes,
    Sample sample
) { // Runs and updates the model

    int timestep = 0;
    std::vector<std::array<int, 1>> output; // generalize


    while (1) {
        if (timestep > 7) break; //  primitive handling of breaking system

        for (auto &node: nodes) {
            float accumulator = 0, newConnectionStrengthTotal = 0, stone;

            for (auto inputtingNodeBi: node.connected_from) {
                const float value = inputtingNodeBi.index >= 0 ? nodes[inputtingNodeBi.index].value: sample.input[-inputtingNodeBi.index - 1];
                accumulator += inputtingNodeBi.strength * value;
            }

            node.newValue = node.multiplier * (std::max(accumulator, node.threshold) - node.threshold) + node.floor;
            stone = std::max(accumulator, node.threshold);

            for (auto& inputtingNodeBi: node.connected_from) { // Expectations
                // Minimize effect implementation
                if (node.index == 0 && timestep > 4) {
                    stone = sample.output[0];
                }

                const float value = inputtingNodeBi.index >= 0 ? nodes[inputtingNodeBi.index].value: sample.input[-inputtingNodeBi.index - 1];
                const float otherValue = (1 - inputtingNodeBi.strength) * std::max(accumulator - inputtingNodeBi.strength * value, node.threshold);
                const float loss = (otherValue - stone) * (otherValue - stone);
                
                if (otherValue - stone > 0) { // overloss and underloss are the same thing...
                    nodes[inputtingNodeBi.index].overLoss += inputtingNodeBi.strength * loss;
                } else {
                    nodes[inputtingNodeBi.index].underLoss += inputtingNodeBi.strength * loss;
                }

                inputtingNodeBi.strength *= 0.5 + 1/(1 + exp(-loss/4)); // Connection pruning
            }

            for (const auto& inputtingNodeBi: node.connected_from) {
                newConnectionStrengthTotal += inputtingNodeBi.strength;
            }

            for (auto& inputtingNodeBi: node.connected_from) {
                inputtingNodeBi.strength /= newConnectionStrengthTotal;
            }

        }

        for (auto& node: nodes) {
            float netLoss = node.overLoss - node.underLoss;
            float lossTension = node.overLoss + node.underLoss;
            if (node.newValue > node.floor) {
                float factor = 0.25 + 1/(1+exp(-lossTension/node.multiplier));
                float magnitude = (factor - 1) * node.multiplier;
                netLoss +=  magnitude * (node.newValue - node.floor) / node.multiplier; // + node.threshold

                node.multiplier *= factor;
                node.threshold + = std::cbrt(netLoss);
            } else {
                node.floor += std::cbrt(netLoss);
            }

            std::cout << node.index << ": " << node.floor << " | " << node.threshold << " | " << node.multiplier << "\n";
        }

        // Forming output
        std::array<int, 1> outputSlice;
        outputSlice[0] = nodes[0].newValue;
        output.push_back(outputSlice);
        
        updateNodeValues(nodes);
        timestep++;
    }

    //std::cout << "End\n";
    for (auto outputSlice: output){
        std::cout << outputSlice[0] << std::endl;
    }

    //printModel(nodes);
    return output;
}
