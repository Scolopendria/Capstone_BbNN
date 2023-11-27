// Single node BrNN experiment 
/* Most basic Bronstein Net */

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
	/* This idea is that the node picks the sources of input and tries to produce the most highly valued output. Who grades the output neurons? Going with this idea first. */
	std::vector<int> connected_to;
	/* This idea is that the node tries to decrease the level of chaos by controlling and picking other neurons. End Goal: The model develops a method to control its environment. */
};

std::string extract(std::string filename);
std::vector<std::array<int, 1>> runModel(
    std::vector<Node>& nodes,
    std::vector<bi_int> activeNodes,
    Sample sample
);
std::vector<Sample> parseData(const std::string& data);

int main(int argc, char *argv[]) {
    std::vector<Node> nodes;
	std::vector<bi_int> activeNodes; // Try 1
	
    std::string data = extract(FILE);
    std::cout << data << "\n";

	auto samples = parseData(data);
	
    for (auto sample: samples) {
        // For 1 var input and 1 var output
        bi_int inputNode;
        inputNode.index = INPUT_NODE_INDEX;
        inputNode.value = sample.input.front();
        activeNodes.push_back(inputNode);
        Node node;
        node.connected_from.push_back(INPUT_NODE_INDEX);
        nodes.push_back(node);

        // int cost = 0; // ?
        for (auto sample: samples) {
            std::cout << sample.input.front() << "\n";
            runModel(nodes, activeNodes, sample); // timestep, chain ?
        }
    }
	return 0;
}

/* Run Model */
std::vector<std::array<int, 1>> runModel(
    std::vector<Node>& nodes,
    std::vector<bi_int> activeNodes,
    Sample sample
) {
    int timestep = 0;
    std::vector<bi_int> currentNodes;
    std::vector<std::array<int, 1>> output;
    //   ->|<- This '1' stands for it is worthwhile to continue calculating
	while (1) { // Right now, it is handled by timestep; when timestep reaches 20, it breaks
        currentNodes.clear();
        timestep++;

		for (auto node: nodes) {
			int accumulator = 0;
			for (auto inputtingNodeIndex: node.connected_from) {
				auto it = std::find_if(
					activeNodes.begin(),
					activeNodes.end(),
					[inputtingNodeIndex](bi_int pair) {
						return pair.index == inputtingNodeIndex;
					}
				);
				
				if (it != activeNodes.end()) {
                    accumulator += it->value;
                    currentNodes.push_back(*it);
				}
			}
		}

        if (
            std::find_if(
                activeNodes.begin(),
                activeNodes.end(),
                [](bi_int pair) {
                    return pair.index == 0;
                }
			) == activeNodes.end()
        ) {
            auto it = std::find_if(
                activeNodes.begin(),
                activeNodes.end(),
                [](bi_int pair) {
                    return pair.index == 0;
                }
			);

            // We know 'it' is valid, but check for the grneralized case
            std::array<int, 1> outputSlice = {
                it->value
            };

            output.push_back(outputSlice);
        }

		activeNodes.clear();
		activeNodes = currentNodes;

        if (timestep > 20) break;
    }

    std::cout << "Here!" << std::endl;
    return output;
}

/* Training function */


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



