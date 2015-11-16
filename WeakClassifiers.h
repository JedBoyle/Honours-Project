
// Jedda Boyle (BYLJED001)
// Honours Computer Science Project.

#ifndef WEAKCLASSIFIER_H
#define WEAKCLASSIFIER_H

#include <shark/Models/Trees/CARTClassifier.h>
#include <shark/Data/Dataset.h>
#include "AbstractClassifier.h"
#include "DzCARTTrainer.h"

// ANN
#include <shark/Models/FFNet.h>
#include <shark/Algorithms/GradientDescent/Rprop.h>
#include <shark/ObjectiveFunctions/ErrorFunction.h>
#include <shark/ObjectiveFunctions/Loss/CrossEntropy.h>



class TreeClassifier: public AbstractClassifier {
public:
    typedef shark::RealVector RealVector;
    shark::CARTClassifier<RealVector> model;
    shark::DzCARTTrainer trainer;
	std::size_t tree_size;

	// Constructor
	TreeClassifier (void): tree_size(0) {}

	// Constructor that sets the trees maximum depth.
	TreeClassifier (int depth): tree_size(pow(2, depth + 1) - 1) {
		trainer.setTreeSize(tree_size);
        //trainer.setNodeSize(1500);
	}

	// Train the classifier with the ClassificationDataset.
	void train (const shark::ClassificationDataset& data) {
		trainer.train(model, data);
	}

	// Return the classifcation of a single input.
    int classify (const RealVector& input) {
		RealVector output;
		model.eval(input, output);
    	return (output(0) >= output(1)) ? -1 : 1;
    }

    // Return string representation of the TreeClassifier.
    std::string toString (void) {
        if (tree_size == 0){
                return "TreeClassifier without maximum Depth.";
        }
        return "TreeClassifier of size: " + std::to_string(tree_size);

    }



};

template<class HiddenNeuron, class OutputNeuron>
class NetworkClassifier: public AbstractClassifier {
public:
    typedef shark::RealVector RealVector;
    typedef shark::FFNet<HiddenNeuron, OutputNeuron> FFNetwork;
    FFNetwork model;
    shark::IRpropPlus optimizer;

	std::size_t input_size;
    std::size_t output_size;
    std::size_t hidden_size;
    std::size_t second_hidden_size;
    std::size_t num_steps;

    shark::CrossEntropy loss;

	// Constructor
	NetworkClassifier (std::size_t _hidden_size, std::size_t _num_steps = 200):
                input_size(6),
                hidden_size(_hidden_size),
                second_hidden_size(-1),
                output_size(1),
                num_steps(_num_steps) {
		model.setStructure(input_size, hidden_size, output_size, shark::FFNetStructures::Normal, true);
        initRandomUniform(model, -0.1, 0.1);
	}

    // Constructor
	NetworkClassifier (std::size_t _hidden_size, std::size_t _second_hidden_size, std::size_t _num_steps):
                input_size(6),
                hidden_size(_hidden_size),
                second_hidden_size(_second_hidden_size),
                output_size(1),
                num_steps(_num_steps) {
		model.setStructure(input_size, hidden_size, second_hidden_size, output_size, shark::FFNetStructures::Normal, true);
        initRandomUniform(model, -0.1, 0.1);
	}

	// Train the classifier with the ClassificationDataset.
	void train (const shark::ClassificationDataset& data) {
        shark::ErrorFunction error(data, &model, &loss);
        optimizer.init(error);
        for (int i = 0; i < num_steps; ++i) {
            optimizer.step(error);
        }
	}

	// Return the classifcation of a single input.
    int classify (const RealVector& input) {
		RealVector output;
		model.eval(input, output);
    	return (output[0] > 0) ? 1 : -1; // If output is greater than 0 classification is 1. From ZeroOneLoss class.
    }

    // Return string representation of the TreeClassifier.
    std::string toString (void) {
        return "NetworkClassifier ";
    }

};



#endif
