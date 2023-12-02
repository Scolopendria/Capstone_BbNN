// BrNN experiment

#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

#define FILE "data.txt"
#define INPUT_NODE_INDEX -1
class Sample {
  public:
  std::vector<int> input;
  std::vector<int> output;
};


class bi_int {
  public:
  int index;
  int value;
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
   int threshold = 0;
   int bias = 0;
   // int weight = 1;
   // int value = 0; // ?
   int index = 0;
   std::vector<bi_int> connected_from;
};


std::string extract(std::string filename);
std::vector<std::array<int, 1>> runModel(
  std::vector<NodeV2>& nodes,
  std::vector<bi_int> activeNodes,
  Sample sample
);
std::vector<Sample> parseData(const std::string& data);
void printModel(const std::vector<NodeV2>& nodes);


int main(int argc, char *argv[]) {
  std::vector<NodeV2> nodes;
  std::vector<bi_int> activeNodes; // Try 1
   std::string data = extract(FILE);
  std::cout << data << "\n";


   auto samples = parseData(data);
  
   NodeV2 node;
   bi_int connection;
   connection.index = INPUT_NODE_INDEX;
   connection.value = 1;
   node.connected_from.push_back(connection);
   node.index = 0;
   nodes.push_back(node);
    for (auto sample: samples) {
      // For 1 var input and 1 var output
      bi_int inputNode;
      inputNode.index = INPUT_NODE_INDEX;
      inputNode.value = sample.input.front();
      activeNodes.push_back(inputNode);

       // int cost = 0; // ?
       std::cout << "Here: " << sample.input.front() << "\n";
       runModel(nodes, activeNodes, sample); // timestep, chain ?
   }

   return 0;
}


/* Run Model Try 1, V2 */
std::vector<std::array<int, 1>> runModel(
  std::vector<NodeV2>& nodes,
  std::vector<bi_int>activeNodes,
  Sample sample
) {
   int timestep = 0;
   std::vector<bi_int> currentNodes;
   std::vector<std::array<int, 1>> output; // generalize


   while (1) {
       if (timestep > 20) break; //  primitive handling of breaking system
       currentNodes.clear();
      
       for (auto node: nodes) {
           int accumulator = node.bias; // node.value ?
           for (auto inputtingNodeIndex: node.connected_from) {
               auto it = std::find_if(
                   activeNodes.begin(),
                   activeNodes.end(),
                   [inputtingNodeIndex](bi_int other) {
                       return other.index == inputtingNodeIndex.index;
                   }
               );


               if (it != activeNodes.end()) {
                   accumulator += inputtingNodeIndex.value * it->value;
               }
           }
          
           if (accumulator > node.threshold) {
               bi_int current;
               current.index = node.index;
               current.value = accumulator;
               currentNodes.push_back(current);
           }
       }




       activeNodes.clear();
       activeNodes = currentNodes;
  
       timestep++;
   }

   //std::cout << "Here" << std::endl;
   printModel(nodes);
   return output;
}


void printModel(const std::vector<NodeV2>& nodes) {
   std::cout << "index\tthreshold\tbias\n";
   for (auto node: nodes) {
       std::cout << node.index << "\t" << node.threshold << "\t\t" << node.bias << " | ";
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