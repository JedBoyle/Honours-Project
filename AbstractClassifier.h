
// Jedda Boyle (BYLJED001)
// Honours Computer Science Project.

#ifndef ABSTRACTCLASSIFIER_H
#define ABSTRACTCLASSIFIER_H

#include <shark/Data/Dataset.h>
#include <iostream>
#include <string>

// Structure used to record the statistics of a classifcation.
struct Evaluation {
	double pion_efficiency = 0;
	double electron_efficiency = 0;
	double error = 0;
	int correct_classification = 0;
	int data_size = 0;
};

class AbstractClassifier {
public:
	Evaluation results;

	// Train the classifier with the ClassificationDataset.
	virtual void train (const shark::ClassificationDataset& data) = 0;

	// Return the classifcation of a single input.
	virtual int classify (const shark::RealVector& input) = 0;

	// This function will return the summary classification
	// statistics of the classifier on a ClassificationDataset.
 	void evaluate (shark::ClassificationDataset& data);

	// This function will return the summary classification
	// statistics of the classifier on a ClassificationDataset.
 	void evaluate2 (shark::ClassificationDataset data);

	// Calulate and print classification results ona ClassificationDataset.
	void printResults (shark::ClassificationDataset& data);

	// Return the error on a ClassificationDataset.
	double getError (shark::ClassificationDataset data);

	virtual std::string toString (void) = 0;


};

#endif
