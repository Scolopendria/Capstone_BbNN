// perceptron


public class NeuralNetwork
{
	public readonly Layer[] layers;
	public readonly int[] layerSizes;


	public ICost cost;
	System.Random rng;
	NetworkLearnData[] batchLearnData;


	// Create the neural network
	public NeuralNetwork(params int[] layerSizes)
	{
		this.layerSizes = layerSizes;
		rng = new System.Random();


		layers = new Layer[layerSizes.Length - 1];
		for (int i = 0; i < layers.Length; i++)
		{
			layers[i] = new Layer(layerSizes[i], layerSizes[i + 1], rng);
		}


		cost = new Cost.MeanSquaredError();
	}


	// Run the inputs through the network to predict which class they belong to.
	// Also returns the activations from the output layer.
	public (int predictedClass, double[] outputs) Classify(double[] inputs)
	{
		var outputs = CalculateOutputs(inputs);
		int predictedClass = MaxValueIndex(outputs);
		return (predictedClass, outputs);
	}


	// Run the inputs through the network to calculate the outputs
	public double[] CalculateOutputs(double[] inputs)
	{
		foreach (Layer layer in layers)
		{
			inputs = layer.CalculateOutputs(inputs);
		}
		return inputs;
	}




	public void Learn(DataPoint[] trainingData, double learnRate, double regularization = 0, double momentum = 0)
	{


		if (batchLearnData == null || batchLearnData.Length != trainingData.Length)
		{
			batchLearnData = new NetworkLearnData[trainingData.Length];
			for (int i = 0; i < batchLearnData.Length; i++)
			{
				batchLearnData[i] = new NetworkLearnData(layers);
			}
		}


		System.Threading.Tasks.Parallel.For(0, trainingData.Length, (i) =>
		{
			UpdateGradients(trainingData[i], batchLearnData[i]);
		});




		// Update weights and biases based on the calculated gradients
		for (int i = 0; i < layers.Length; i++)
		{
			layers[i].ApplyGradients(learnRate / trainingData.Length, regularization, momentum);
		}
	}




	void UpdateGradients(DataPoint data, NetworkLearnData learnData)
	{
		// Feed data through the network to calculate outputs.
		// Save all inputs/weightedinputs/activations along the way to use for backpropagation.
		double[] inputsToNextLayer = data.inputs;


		for (int i = 0; i < layers.Length; i++)
		{
			inputsToNextLayer = layers[i].CalculateOutputs(inputsToNextLayer, learnData.layerData[i]);
		}


		// -- Backpropagation --
		int outputLayerIndex = layers.Length - 1;
		Layer outputLayer = layers[outputLayerIndex];
		LayerLearnData outputLearnData = learnData.layerData[outputLayerIndex];


		// Update output layer gradients
		outputLayer.CalculateOutputLayerNodeValues(outputLearnData, data.expectedOutputs, cost);
		outputLayer.UpdateGradients(outputLearnData);


		// Update all hidden layer gradients
		for (int i = outputLayerIndex - 1; i >= 0; i--)
		{
			LayerLearnData layerLearnData = learnData.layerData[i];
			Layer hiddenLayer = layers[i];


			hiddenLayer.CalculateHiddenLayerNodeValues(layerLearnData, layers[i + 1], learnData.layerData[i + 1].nodeValues);
			hiddenLayer.UpdateGradients(layerLearnData);
		}


	}


	public void SetCostFunction(ICost costFunction)
	{
		this.cost = costFunction;
	}


	public void SetActivationFunction(IActivation activation)
	{
		SetActivationFunction(activation, activation);
	}


	public void SetActivationFunction(IActivation activation, IActivation outputLayerActivation)
	{
		for (int i = 0; i < layers.Length - 1; i++)
		{
			layers[i].SetActivationFunction(activation);
		}
		layers[layers.Length - 1].SetActivationFunction(outputLayerActivation);
	}




	public int MaxValueIndex(double[] values)
	{
		double maxValue = double.MinValue;
		int index = 0;
		for (int i = 0; i < values.Length; i++)
		{
			if (values[i] > maxValue)
			{
				maxValue = values[i];
				index = i;
			}
		}


		return index;
	}
}




public class NetworkLearnData
{
	public LayerLearnData[] layerData;


	public NetworkLearnData(Layer[] layers)
	{
		layerData = new LayerLearnData[layers.Length];
		for (int i = 0; i < layers.Length; i++)
		{
			layerData[i] = new LayerLearnData(layers[i]);
		}
	}
}


public class LayerLearnData
{
	public double[] inputs;
	public double[] weightedInputs;
	public double[] activations;
	public double[] nodeValues;


	public LayerLearnData(Layer layer)
	{
		weightedInputs = new double[layer.numNodesOut];
		activations = new double[layer.numNodesOut];
		nodeValues = new double[layer.numNodesOut];
	}
}


using static System.Math;


public class Layer
{
	public readonly int numNodesIn;
	public readonly int numNodesOut;


	public readonly double[] weights;
	public readonly double[] biases;


	// Cost gradient with respect to weights and with respect to biases
	public readonly double[] costGradientW;
	public readonly double[] costGradientB;


	// Used for adding momentum to gradient descent
	public readonly double[] weightVelocities;
	public readonly double[] biasVelocities;


	public IActivation activation;


	// Create the layer
	public Layer(int numNodesIn, int numNodesOut, System.Random rng)
	{
		this.numNodesIn = numNodesIn;
		this.numNodesOut = numNodesOut;
		activation = new Activation.Sigmoid();


		weights = new double[numNodesIn * numNodesOut];
		costGradientW = new double[weights.Length];
		biases = new double[numNodesOut];
		costGradientB = new double[biases.Length];


		weightVelocities = new double[weights.Length];
		biasVelocities = new double[biases.Length];


		InitializeRandomWeights(rng);
	}


	// Calculate layer output activations
	public double[] CalculateOutputs(double[] inputs)
	{
		double[] weightedInputs = new double[numNodesOut];


		for (int nodeOut = 0; nodeOut < numNodesOut; nodeOut++)
		{
			double weightedInput = biases[nodeOut];


			for (int nodeIn = 0; nodeIn < numNodesIn; nodeIn++)
			{
				weightedInput += inputs[nodeIn] * GetWeight(nodeIn, nodeOut);
			}
			weightedInputs[nodeOut] = weightedInput;
		}


		// Apply activation function
		double[] activations = new double[numNodesOut];
		for (int outputNode = 0; outputNode < numNodesOut; outputNode++)
		{
			activations[outputNode] = activation.Activate(weightedInputs, outputNode);
		}


		return activations;
	}


	// Calculate layer output activations and store inputs/weightedInputs/activations in the given learnData object
	public double[] CalculateOutputs(double[] inputs, LayerLearnData learnData)
	{
		learnData.inputs = inputs;


		for (int nodeOut = 0; nodeOut < numNodesOut; nodeOut++)
		{
			double weightedInput = biases[nodeOut];
			for (int nodeIn = 0; nodeIn < numNodesIn; nodeIn++)
			{
				weightedInput += inputs[nodeIn] * GetWeight(nodeIn, nodeOut);
			}
			learnData.weightedInputs[nodeOut] = weightedInput;
		}


		// Apply activation function
		for (int i = 0; i < learnData.activations.Length; i++)
		{
			learnData.activations[i] = activation.Activate(learnData.weightedInputs, i);
		}


		return learnData.activations;
	}


	// Update weights and biases based on previously calculated gradients.
	// Also resets the gradients to zero.
	public void ApplyGradients(double learnRate, double regularization, double momentum)
	{
		double weightDecay = (1 - regularization * learnRate);


		for (int i = 0; i < weights.Length; i++)
		{
			double weight = weights[i];
			double velocity = weightVelocities[i] * momentum - costGradientW[i] * learnRate;
			weightVelocities[i] = velocity;
			weights[i] = weight * weightDecay + velocity;
			costGradientW[i] = 0;
		}




		for (int i = 0; i < biases.Length; i++)
		{
			double velocity = biasVelocities[i] * momentum - costGradientB[i] * learnRate;
			biasVelocities[i] = velocity;
			biases[i] += velocity;
			costGradientB[i] = 0;
		}
	}


	// Calculate the "node values" for the output layer. This is an array containing for each node:
	// the partial derivative of the cost with respect to the weighted input
	public void CalculateOutputLayerNodeValues(LayerLearnData layerLearnData, double[] expectedOutputs, ICost cost)
	{
		for (int i = 0; i < layerLearnData.nodeValues.Length; i++)
		{
			// Evaluate partial derivatives for current node: cost/activation & activation/weightedInput
			double costDerivative = cost.CostDerivative(layerLearnData.activations[i], expectedOutputs[i]);
			double activationDerivative = activation.Derivative(layerLearnData.weightedInputs, i);
			layerLearnData.nodeValues[i] = costDerivative * activationDerivative;
		}
	}


	// Calculate the "node values" for a hidden layer. This is an array containing for each node:
	// the partial derivative of the cost with respect to the weighted input
	public void CalculateHiddenLayerNodeValues(LayerLearnData layerLearnData, Layer oldLayer, double[] oldNodeValues)
	{
		for (int newNodeIndex = 0; newNodeIndex < numNodesOut; newNodeIndex++)
		{
			double newNodeValue = 0;
			for (int oldNodeIndex = 0; oldNodeIndex < oldNodeValues.Length; oldNodeIndex++)
			{
				// Partial derivative of the weighted input with respect to the input
				double weightedInputDerivative = oldLayer.GetWeight(newNodeIndex, oldNodeIndex);
				newNodeValue += weightedInputDerivative * oldNodeValues[oldNodeIndex];
			}
			newNodeValue *= activation.Derivative(layerLearnData.weightedInputs, newNodeIndex);
			layerLearnData.nodeValues[newNodeIndex] = newNodeValue;
		}


	}


	public void UpdateGradients(LayerLearnData layerLearnData)
	{
		// Update cost gradient with respect to weights (lock for multithreading)
		lock (costGradientW)
		{
			for (int nodeOut = 0; nodeOut < numNodesOut; nodeOut++)
			{
				double nodeValue = layerLearnData.nodeValues[nodeOut];
				for (int nodeIn = 0; nodeIn < numNodesIn; nodeIn++)
				{
					// Evaluate the partial derivative: cost / weight of current connection
					double derivativeCostWrtWeight = layerLearnData.inputs[nodeIn] * nodeValue;
					// The costGradientW array stores these partial derivatives for each weight.
					// Note: the derivative is being added to the array here because ultimately we want
					// to calculate the average gradient across all the data in the training batch
					costGradientW[GetFlatWeightIndex(nodeIn, nodeOut)] += derivativeCostWrtWeight;
				}
			}
		}


		// Update cost gradient with respect to biases (lock for multithreading)
		lock (costGradientB)
		{
			for (int nodeOut = 0; nodeOut < numNodesOut; nodeOut++)
			{
				// Evaluate partial derivative: cost / bias
				double derivativeCostWrtBias = 1 * layerLearnData.nodeValues[nodeOut];
				costGradientB[nodeOut] += derivativeCostWrtBias;
			}
		}
	}


	public double GetWeight(int nodeIn, int nodeOut)
	{
		int flatIndex = nodeOut * numNodesIn + nodeIn;
		return weights[flatIndex];
	}


	public int GetFlatWeightIndex(int inputNeuronIndex, int outputNeuronIndex)
	{
		return outputNeuronIndex * numNodesIn + inputNeuronIndex;
	}


	public void SetActivationFunction(IActivation activation)
	{
		this.activation = activation;
	}


	public void InitializeRandomWeights(System.Random rng)
	{
		for (int i = 0; i < weights.Length; i++)
		{
			weights[i] = RandomInNormalDistribution(rng, 0, 1) / Sqrt(numNodesIn);
		}


		double RandomInNormalDistribution(System.Random rng, double mean, double standardDeviation)
		{
			double x1 = 1 - rng.NextDouble();
			double x2 = 1 - rng.NextDouble();


			double y1 = Sqrt(-2.0 * Log(x1)) * Cos(2.0 * PI * x2);
			return y1 * standardDeviation + mean;
		}
	}
}


