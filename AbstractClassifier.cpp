
// Jedda Boyle (BYLJED001)
// Honours Computer Science Project.

#include "AbstractClassifier.h"

using namespace std;
using namespace shark;

// This function will return the summary classification
// statistics of the classifier on a ClassificationDataset.
void AbstractClassifier::evaluate (ClassificationDataset& data) {
    size_t data_size = data.numberOfElements();
    int count_correct = 0;
    int pion_efficieny = 0;
    int electron_efficiency = 0;
    unsigned int classification;
    int pion_electron_count [2] = {0};

    // Iterate over every element and record if it is correctly classified.
    BOOST_FOREACH (ClassificationDataset::element_reference point, data.elements()) {
    	pion_electron_count[point.label]++;
    	classification = max(0, classify(point.input));
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

    // return the results
    results.pion_efficiency = (double) pion_efficieny / pion_electron_count[0];
    results.electron_efficiency = (double) electron_efficiency / pion_electron_count[1];
    results.error = 1. - (double) count_correct / data_size;
    results.correct_classification = count_correct;
    results.data_size = data_size;
}

// This function will return the summary classification
// statistics of the classifier on a ClassificationDataset.
// This function doesn't take the data as a reference. This
// is less efficient but it is required because the Shark cross
// cross validation object can't be passed as a reference.
void AbstractClassifier::evaluate2 (ClassificationDataset data) {
    size_t data_size = data.numberOfElements();
    int count_correct = 0;
    int pion_efficieny = 0;
    int electron_efficiency = 0;
    unsigned int classification;
    int pion_electron_count [2] = {0};

    // Iterate over every element and record if it is correctly classified.
    BOOST_FOREACH (ClassificationDataset::element_reference point, data.elements()) {
    	pion_electron_count[point.label]++;
    	classification = max(0, classify(point.input));
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

    // return the results
    results.pion_efficiency = (double) pion_efficieny / pion_electron_count[0];
    results.electron_efficiency = (double) electron_efficiency / pion_electron_count[1];
    results.error = 1. - (double) count_correct / data_size;
    results.correct_classification = count_correct;
    results.data_size = data_size;
}

// Calulate and print classification results ona ClassificationDataset.
void AbstractClassifier::printResults (shark::ClassificationDataset& data) {
    evaluate(data);
    cout << toString() << ": "                                                          << "\n"
     	 << "Number of Observations: " << results.data_size                             << "\n"
    	 << "Number of Correct Classifications: " << results.correct_classification     << "\n"
    	 << "Pion Efficiency: " << results.pion_efficiency                              << "\n"
    	 << "Electron Efficiency: " << results.electron_efficiency                      << "\n"
    	 << "Total Error Rate: " << results.error                                       << endl;
}

// Return the error on a ClassificationDataset.
double AbstractClassifier::getError (ClassificationDataset data) {
	evaluate(data);
	return results.error;

}
