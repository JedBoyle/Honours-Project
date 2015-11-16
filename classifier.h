
#ifndef CLASSIFIER_H
#define CLASSIFIER_H

// Shark imports.
#include <shark/Models/Trees/CARTClassifier.h>
#include <shark/Data/Dataset.h>
#include <shark/Data/DataView.h>
// cross validation
#include <shark/Data/CVDatasetTools.h>
// Local class imports.
#include "DzCARTTrainer.h"
// STD imports.
#include <random>
#include <iostream>
#include <algorithm>

#include "AbstractClassifier.h"
#include "WeakClassifiers.h"
#include "BoostClassifier.h"
#include "Optimiser.h"


using namespace shark;
using namespace std;


class AbstractClassifier {
public:

	// Train the classifier with the ClassificationDataset.
	virtual void train (const ClassificationDataset& data) = 0;

	// Return the classifcation of a single input.
	virtual int classify (const RealVector& input) = 0;

	// This function will return the summary classification
	// statistics of the classifier on a ClassificationDataset.
 	void evaluate (ClassificationDataset& data) {
		size_t data_size = data.numberOfElements();
		int count_correct = 0;
		int pion_efficieny = 0;
		int electron_efficiency = 0;
		unsigned int classification;
		int pion_electron_count [2] = {0};

		BOOST_FOREACH (ClassificationDataset::element_reference point, data.elements()) {
			pion_electron_count[point.label]++;
			classification = max(0, classify(point.input));
			//cout << point.label << ", " << classification << endl;
	        if (point.label == classification) {
	            count_correct++;
	        }
			if (point.label == 0 && classification == 1) {
				pion_efficieny++;
			}
			else if (point.label == 1 && classification == 1) {
				electron_efficiency++;
			}
	    }

		// TODO add to_string method which means it will print correctly for all classifiers.
		cout << "Evaluation Results For TreeClassifier: "                              			 << "\n"
		 	 << "Number of Observations: " << data_size                                			 << "\n"
			 << "Number of Correct Classifications: " << count_correct                			 << "\n"
			 << "Pion Efficiency: " << (double) pion_efficieny / pion_electron_count[0]          << "\n"
			 << "Electron Efficiency: " << (double) electron_efficiency / pion_electron_count[1] << "\n"
			 << "Total Error Rate: " << 1. - (double) count_correct / data_size                  << endl;

	}

	//TODO overload << operator

	double getError (ClassificationDataset data) {
		double count_correct = 0;
		BOOST_FOREACH (ClassificationDataset::element_reference point, data.elements()) {
	        if (point.label == max(0, classify(point.input))) {
	            count_correct++;
	        }
	    }
		return 1. - count_correct / data.numberOfElements();

	}


};

// This class just wraps the CARTClassifier into an appropraite interface.
class TreeClassifier: public AbstractClassifier {
public:
	DzCARTTrainer trainer;
	CARTClassifier<RealVector> model;
	size_t tree_size;

	// Constructor
	TreeClassifier (void) {}

	// Constructor
	TreeClassifier (int depth): tree_size(pow(2, depth + 1) - 1) {
		trainer.setTreeSize(tree_size);
	}

	// Train the classifier with the ClassificationDataset.
	void train (const ClassificationDataset& data) {
		trainer.train(model, data);
	}

	// Return the classifcation of a single input.
    int classify (const RealVector& input) {
		RealVector output;
		model.eval(input, output);
    	return (output(0) >= output(1)) ? -1 : 1;

    }

};


template <class WeakClassifier>
class BoostClassifier: public AbstractClassifier {
public:
	typedef vector<WeakClassifier> ClassifierVector;
	ClassifierVector weak_classifiers;
	RealVector alphas;
	size_t n_weak_classifiers;


	BoostClassifier (size_t _n_weak_classifiers, WeakClassifier base_classifer): n_weak_classifiers(_n_weak_classifiers) {
		// This constructor needs a base_classifer which is copied n_weak_classifiers time into the weak_classifiers
		// vector and then later trained.
		alphas.resize(n_weak_classifiers + 1);
		for (int i = 0; i < this->n_weak_classifiers; ++i) {
            WeakClassifier learner(base_classifer);
            weak_classifiers.push_back(learner);
            alphas[i] = 0;
        }
    }

	BoostClassifier (size_t _n_weak_classifiers, ClassifierVector _weak_classifers):
			weak_classifiers(_weak_classifers),
			n_weak_classifiers(_n_weak_classifiers) {
			this->alphas.resize(n_weak_classifiers+1);
    }

	// Return the classifcation of a single input.
    int classify (const RealVector& input) {
		// Sum the weighted classifcation of each weak classifier.
		double classification = 0.0;
		for (int i = 0; i < n_weak_classifiers; ++i) {
			classification += alphas[i] * weak_classifiers[i].classify(input);
		}
		return classification < 0 ? -1 : 1;
    }

	// Train the classifier with the ClassificationDataset.
	void train (const ClassificationDataset& data) {
		// Need to make a copy of the data because it is const.
		ClassificationDataset data_copy(data);
        data_copy.makeIndependent(); // Force a deep copy.
        DataView<ClassificationDataset> data_view(data_copy);

		DataView<ClassificationDataset> data_sample;
		size_t data_size = data.numberOfElements();
		RealVector weights(data_size, 1 / (double) data_size);
		IntVector sample(data_size, 0);
		IntVector predictions(data_size, 0);
        double Z, error;

        for (int i = 0; i < n_weak_classifiers; ++i) {
            // Train weak classifier with respect to the sampled data.
			weightedSample(weights, sample);
            weak_classifiers[i].train(toDataset(subset(data_view, sample)));

            // Calculate weak classifier error.
            error = 0.0;
            for (int j = 0; j < data_size; ++j) {
                if (data_view[j].label != max(0, weak_classifiers[i].classify(data_view[j].input))) {
                    predictions[j] = 1;
                    error += weights[j];
                }
                else {
                    predictions[j] = -1;
                }
            }

			// Calculate weak learner weighting.
			error = max(1e-8, error); // Handle the case that the error is 0.
            alphas[i] = (0.5) * log((1 - error) / error);

			// Update weights and calculate the normalisation factor.
            Z = 0.0;
			for (int j = 0; j < data_size; ++j) {
                weights[j] = (weights[j] * exp(predictions[j] * alphas[i]));
                Z += weights[j];
            }

			// Normalise weights
            for (auto& weight : weights) {
				weight = weight / Z;

			}
        }
	}

	// Train the classifier with the ClassificationDataset.
	void simpleTrain (const ClassificationDataset& data) {
		for(auto& classifier : weak_classifiers) {
			classifier.train(data);
		}
	}

	// Use the weights vector to sample sample.size() ints in range [0, weights.size()).
	void weightedSample (const RealVector& weights, IntVector& sample) {
		mt19937 generator;
        discrete_distribution<size_t> distribution(weights.begin(), weights.end());
        for (auto& s : sample) {
            s = distribution(generator);
        }
    }

	// Return the number of weak classifiers being boosted.
	int size (void) {
		return weak_classifiers.size();
	}

	void setAlphas (RealVector _alphas) {
		copy(_alphas.begin(), _alphas.end(), alphas.begin());
	}

	void printAlphas (void) {
		for(auto alpha : alphas) {
			cout << alpha << ", ";
		}
		cout << endl;
	}

	void prune (void) {
		double threshold = 0.1;
		for(auto& alpha : alphas) {
			alpha <= threshold ? alpha = 0 : alpha = alpha;
		}
	}

};


#endif
