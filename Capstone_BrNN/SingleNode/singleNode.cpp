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
   int value = 0; // ?
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
       std::cout << "Here: " << sample.input.front() << "\t" << sample.output.front() <<"\n";
       runModel(nodes, activeNodes, sample); // timestep, chain ?
   }

   return 0;
}


/* Run Model Try 1, V2 */
std::vector<std::array<int, 1>> runModel(
  std::vector<NodeV2>& nodes,
  std::vector<bi_int> activeNodes,
  Sample sample
) {
     int timestep = 0;
   std::vector<bi_int> currentNodes;
   std::vector<std::array<int, 1>> output; // generalize


   while (1) {
       if (timestep > 20) break; //  primitive handling of breaking system
       currentNodes.clear();

       // Randomly add nodes and see if it works
       NodeV2 randomNode;
       randomNode.index = nodes.size();
       bi_int randomConnection;
       randomConnection.index = random() % nodes.size();
       if (!(random() % 3)) {
        randomConnection.index = -1;
       }
       randomConnection.value = random() % 50;
       randomNode.connected_from.push_back(randomConnection);
       randomNode.threshold = random() % 100 - 100;
       randomNode.bias = random() % 100 - 100;
       
      
       nodes.push_back(randomNode); // Theoretically, if all works, this should be able to learn...

      bi_int randomConnection2;
       randomConnection2.index = random() % nodes.size();
       randomConnection2.value = random() % 50;
       if (true){
         nodes[0].connected_from .push_back(randomConnection2);
       }
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
            //if (node.index == 0) {
              //std::cout << "test " << activate << " " << node.value << "\n";
            //}
            
            for (auto& inputtingNode: node.connected_from) {
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
            
           if (activate) {
               bi_int current;
               current.index = node.index;
               current.value = accumulator;
               currentNodes.push_back(current);
           }
       }


        std::cout << nodes[0].value << std::endl;
        // Forming output
        std::array<int, 1> outputSlice;
        outputSlice[0] = nodes[0].value;
        output.push_back(outputSlice);

       activeNodes.clear();
       activeNodes = currentNodes;
  
       timestep++;
   }

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