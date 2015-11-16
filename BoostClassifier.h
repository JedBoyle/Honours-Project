
// Jedda Boyle (BYLJED001)
// Honours Computer Science Project.

#ifndef BOOSTCLASSIFIER_H
#define BOOSTCLASSIFIER_H

#include <shark/Data/DataView.h>
#include <shark/Data/Dataset.h>
#include "AbstractClassifier.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include <limits>


template <class WeakClassifier>
class AbstractBoostClassifier: public AbstractClassifier {
public:
    // typdedef shark types 
    typedef shark::DataView<shark::ClassificationDataset> DataView;
    typedef shark::ClassificationDataset ClassificationDataset;
	typedef std::vector<WeakClassifier> ClassifierVector;
    typedef shark::RealVector RealVector;
    typedef shark::IntVector IntVector;

    // Member variables
    RealVector classifier_weights;
    ClassifierVector classifiers;
	size_t n_classifiers;
    std::mt19937 generator;

    // This constructor makes n_weak_classifiers copies of base_classifer and puts them into a ClassifierVector.
	AbstractBoostClassifier (size_t _n_classifiers, WeakClassifier base_classifer): n_classifiers(_n_classifiers) {
		classifier_weights.resize(n_classifiers + 1);
		for (int i = 0; i < n_classifiers; ++i) {
            WeakClassifier learner(base_classifer);
            classifiers.push_back(learner);
            classifier_weights[i] = 0;
        }
    }

    // This constructor uses a predefined list of classifiers.
	AbstractBoostClassifier (size_t _n_classifiers, ClassifierVector _weak_classifers):
			classifiers(_weak_classifers),
			n_classifiers(_n_classifiers) {
		classifier_weights.resize(n_classifiers+1);
    }

	// Train the classifier with the ClassificationDataset.
    // This structure of this method comes from the original
    // Adaboost algorithm.
	void train (const ClassificationDataset& data) {
		// Need to make a copy of the data because it is const
        // and is going to be altered into a dataview.
		ClassificationDataset data_copy(data);
        data_copy.makeIndependent(); // Force a deep copy.
        DataView data_view(data_copy);
		DataView data_sample;
		size_t data_size = data.numberOfElements();
		RealVector weights(data_size, 1 / (double) data_size);
        IntVector predictions(data_size, 0);
        IntVector sample(data_size, 0);
        double error;

        for (int i = 0; i < n_classifiers; ++i) {
            // Train weak classifier with respect to the sampled data.
            weightedSample(sample , weights);
            classifiers[i].train(toDataset(shark::subset(data_view, sample)));
            // Calculate each classifiers error.
            error = std::numeric_limits<double>::denorm_min();
            for (int j = 0; j < data_size; ++j) {
                if (data_view[j].label != std::max(0, classifiers[i].classify(data_view[j].input))) {
                    predictions[j] = 1;
                    error += weights[j];
                }
                else {
                    predictions[j] = -1;
                }
            }
            updateClassifierWeight(classifier_weights[i], error);
            updateDataWeights(weights, i, predictions, data_view);

        }
	}

    // Calulate and update the classifier weight.
    virtual void updateClassifierWeight (double& classifier_weight, double error) = 0;

    // Update weights, calculate the normalisation factor and normalise the weights.
    virtual void updateDataWeights (RealVector& weights, int classifier_pos, const IntVector& predictions, const DataView& data_view) = 0;

	// Use the weights vector to sample sample.size() ints in range [0, weights.size()).
	void weightedSample (IntVector& sample, const RealVector& data_weights) {
        std::discrete_distribution<size_t> distribution(data_weights.begin(), data_weights.end());
        for (auto& s : sample) {
            s = distribution(generator);
        }
    }

    // Classify and single input.
    int classify (const RealVector& input) {
		return partialClassify(input, this->n_classifiers - 1) < 0.0 ? -1 : 1;
    }

    // Get the classification of the first m classifiers.
    int classify (const RealVector& input, int m) {
		return partialClassify(input, m - 1) < 0.0 ? -1 : 1;
    }

    // Count the votes of the first m classifiers.
    double partialClassify (const RealVector& input, int m) {
        double T = 0.0;
		for (int i = 0; i <= m; ++i) {
            classifier_weights[i];
            T += (classifier_weights[i] * this->classifiers[i].classify(input));
		}
		return T;

    }

    // Set the classifiers alpha values to a vector of values
	void setAlphas (RealVector _alphas) {
		copy(_alphas.begin(), _alphas.end()-1, classifier_weights.begin());
	}

    //Remove the classifier with the least weight.
    void pruneClassifier (void) {
        double holder = 1000000;
        double* min_weight = &holder;
        for (auto& weight: classifier_weights) {
            // Find the minimum weight that hasn't already been set to zero.
            if (weight < *min_weight && weight != 0) {
                min_weight = &weight;
            }
        }
        *min_weight = 0;
    }

    void printClassifierWeights (void) {
        for (auto& weight: classifier_weights) {
            std::cout << weight << ", ";
        }
        std::cout << std::endl;
    }


    void partialEvaluate (ClassificationDataset data, int n) {
        size_t data_size = data.numberOfElements();
        int count_correct = 0;
        int pion_efficieny = 0;
        int electron_efficiency = 0;
        unsigned int classification;
        int pion_electron_count [2] = {0};

        BOOST_FOREACH (ClassificationDataset::element_reference point, data.elements()) {
        	pion_electron_count[point.label]++;
            classification = std::max(0, classify(point.input, n));
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

        results.pion_efficiency = (double) pion_efficieny / pion_electron_count[0];
        results.electron_efficiency = (double) electron_efficiency / pion_electron_count[1];
        results.error = 1. - (double) count_correct / data_size;
        results.correct_classification = count_correct;
        results.data_size = data_size;
    }

};


template <class WeakClassifier>
class AdaBoostClassifier: public AbstractBoostClassifier<WeakClassifier> {
public:
    // typdedef shark types
    typedef shark::DataView<shark::ClassificationDataset> DataView;
    typedef std::vector<WeakClassifier> ClassifierVector;
    typedef shark::RealVector RealVector;
    typedef shark::IntVector IntVector;

	AdaBoostClassifier (size_t _n_weak_classifiers, WeakClassifier _base_classifer):
        AbstractBoostClassifier<WeakClassifier> (_n_weak_classifiers, _base_classifer) {
    }

	AdaBoostClassifier (size_t _n_weak_classifiers, ClassifierVector _classifers):
        AbstractBoostClassifier<WeakClassifier> (_n_weak_classifiers, _classifers) {
    }

    // Calulate and update the classifier weight.
    void updateClassifierWeight (double& classifier_weight, double error) {
        classifier_weight =  (0.5) * log((1 - error) / error);
    }

    // Update weights, calculate the normalisation factor and normalise the weights.
    void updateDataWeights (RealVector& data_weights, int classifier_pos, const IntVector& predictions, const DataView& data_view) {
        double Z = 0.0;
        for (int j = 0; j < data_weights.size(); ++j) {
            data_weights[j] = data_weights[j] * exp(predictions[j] * this->classifier_weights[classifier_pos]);
            Z += data_weights[j];
        }
        std::transform (data_weights.begin(), data_weights.end(), data_weights.begin(), [Z](double x) {return x / Z;} );

    }

    std::string toString(void) {
        return "AdaBoostClassifier";
    }

};

template <class WeakClassifier>
class EpsilonBoostClassifier: public AbstractBoostClassifier<WeakClassifier> {
public:
    // typdedef shark types
    typedef shark::DataView<shark::ClassificationDataset> DataView;
    typedef std::vector<WeakClassifier> ClassifierVector;
    typedef shark::RealVector RealVector;
    typedef shark::IntVector IntVector;

    // Member variables
    double epsilon;

	EpsilonBoostClassifier (size_t _n_weak_classifiers, WeakClassifier _base_classifer, double _epsilon = 0.01):
        AbstractBoostClassifier<WeakClassifier> (_n_weak_classifiers, _base_classifer),
        epsilon(_epsilon) {
    }

	EpsilonBoostClassifier (size_t _n_weak_classifiers, ClassifierVector _classifers, double _epsilon = 0.01):
        AbstractBoostClassifier<WeakClassifier> (_n_weak_classifiers, _classifers),
        epsilon(_epsilon){
    }

    // Calculate and update the classifier weight.
    void updateClassifierWeight (double& classifier_weight, double error) {
        classifier_weight = epsilon;
    }

    // Update weights, calculate the normalisation factor and normalise the weights.
    void updateDataWeights (RealVector& data_weights, int classifier_pos, const IntVector& predictions, const DataView& data_view) {
        double Z = 0.0;
        for (int j = 0; j < data_weights.size(); ++j) {
            data_weights[j] = data_weights[j] * exp(2 * epsilon * std::max(0, predictions[j]));
            Z += data_weights[j];
        }
        std::transform (data_weights.begin(), data_weights.end(), data_weights.begin(), [Z](double x) {return x / Z;} );
    }

    std::string toString(void) {
        return "EpsilonBoostClassifier";
    }

};

template <class WeakClassifier>
class ELogitBoostClassifier: public AbstractBoostClassifier<WeakClassifier> {
public:
    // typdedef shark types
    typedef shark::DataView<shark::ClassificationDataset> DataView;
    typedef std::vector<WeakClassifier> ClassifierVector;
    typedef shark::RealVector RealVector;
    typedef shark::IntVector IntVector;

    // Member variables
    double epsilon;

	ELogitBoostClassifier (size_t _n_weak_classifiers, WeakClassifier _base_classifer, double _epsilon = 0.01):
        AbstractBoostClassifier<WeakClassifier> (_n_weak_classifiers, _base_classifer),
        epsilon(_epsilon) {
    }

	ELogitBoostClassifier (size_t _n_weak_classifiers, ClassifierVector _classifers, double _epsilon = 0.01):
        AbstractBoostClassifier<WeakClassifier> (_n_weak_classifiers, _classifers),
        epsilon(_epsilon){
    }

    // Calculate and update the classifier weight.
    void updateClassifierWeight (double& classifier_weight, double error) {
        classifier_weight = epsilon;
    }

    // Update weights.
    void updateDataWeights (RealVector& data_weights, int classifier_pos, const IntVector& predictions, const DataView& data_view) {
        int label, partial_classification;
        for (int i = 0; i < data_weights.size(); ++i) {
            partial_classification = this->partialClassify(data_view[i].input, classifier_pos);
             // Convert 0 label to -1.
            data_view[i].label == 1 ? label = 1 : label = -1;
            data_weights[i] = (exp(-1 * label * partial_classification)) / (1 + exp(-1 * label * partial_classification));
        }
    }

    std::string toString(void) {
        return "ELogitBoostClassifier";
    }

};

template <class WeakClassifier>
class EHingeBoostClassifier: public AbstractBoostClassifier<WeakClassifier> {
public:
    // typdedef shark types
    typedef shark::DataView<shark::ClassificationDataset> DataView;
    typedef std::vector<WeakClassifier> ClassifierVector;
    typedef shark::RealVector RealVector;
    typedef shark::IntVector IntVector;

    // Member variables
    double epsilon;

	EHingeBoostClassifier (size_t _n_weak_classifiers, WeakClassifier _base_classifer, double _epsilon = 0.01):
        AbstractBoostClassifier<WeakClassifier> (_n_weak_classifiers, _base_classifer),
        epsilon(_epsilon) {
    }

	EHingeBoostClassifier (size_t _n_weak_classifiers, ClassifierVector _classifers, double _epsilon = 0.01):
        AbstractBoostClassifier<WeakClassifier> (_n_weak_classifiers, _classifers),
        epsilon(_epsilon){
    }

    // Calculate and update the classifier weight.
    void updateClassifierWeight (double& classifier_weight, double error) {
        classifier_weight = epsilon;
    }

    // Update weights.
    void updateDataWeights (RealVector& data_weights, int classifier_pos, const IntVector& predictions, const DataView& data_view) {
        int label, partial_classification;
        for (int i = 0; i < data_weights.size(); ++i) {
            partial_classification = this->partialClassify(data_view[i].input, classifier_pos);
            // Convert 0 label to -1.
            label = (data_view[i].label == 1 ? 1 : -1);
            data_weights[i] = (label * partial_classification < 1 ? 1 : 0);
        }
    }

    std::string toString(void) {
        return "EHingeBoostClassifier";
    }

};

#endif
