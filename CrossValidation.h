
// Jedda Boyle (BYLJED001)
// Honours Computer Science Project.

#ifndef CROSSVALIDATION_H
#define CROSSVALIDATION_H
#include <shark/Data/CVDatasetTools.h>
#include <shark/Data/Dataset.h>
#include "AbstractClassifier.h"
#include "Optimiser.h"
#include "WeakClassifiers.h"
#include "BoostClassifier.h"
#include <vector>


template<class WeakClassifier>
void WeakCrossValidate (Evaluation& results, WeakClassifier* classifier, shark::ClassificationDataset& data, int n_folds = 10) {
    data.shuffle(); // Shuffle the data to make sure every fold has enough electrons.
    shark::CVFolds<shark::ClassificationDataset> folds = shark::createCVSameSize(data, n_folds);
    double error = 0.;
    double electron_efficiency = 0.;
    double pion_efficiency = 0;
    for (int i = 0; i < n_folds; ++i) {
        classifier->train(folds.training(i));
        classifier->evaluate2(folds.validation(i));
        error += classifier->results.error;
        electron_efficiency += classifier->results.electron_efficiency;
        pion_efficiency += classifier->results.pion_efficiency;
    }
    results.error = error / n_folds;
    results.electron_efficiency = electron_efficiency / n_folds;
    results.pion_efficiency = pion_efficiency / n_folds;
}

template <class BoostClassifier>
void BoostCrossValidate (std::vector<Evaluation>& evaluations, std::vector<size_t> steps, BoostClassifier* classifier, shark::ClassificationDataset& data, int n_folds) {
    // Create an evaltion structure for each of the steps which the model is going to be tested on.
    evaluations.clear();
    for(int i = 0; i < steps.size(); ++i) {
        Evaluation evaluation;
        evaluations.push_back(evaluation);
    }
    data.shuffle(); // Shuffle the data to make sure every fold has enough electrons.
    shark::CVFolds<shark::ClassificationDataset> folds = shark::createCVSameSize(data, n_folds);

    for (int i = 0; i < n_folds; ++i) {
        classifier->train(folds.training(i));
        // Test model on different step sizes.
        for (int j = 0; j < steps.size(); ++j) {
				//std::cout << "fold: " << i << ", step: " << j << "\n";
                classifier->partialEvaluate(folds.validation(i), steps[j]);
                evaluations[j].error += classifier->results.error;
                evaluations[j].electron_efficiency += classifier->results.electron_efficiency;
                evaluations[j].pion_efficiency += classifier->results.pion_efficiency;
        }
    }
    for (auto& evaluation: evaluations)  {
        evaluation.error = evaluation.error / n_folds;
        evaluation.electron_efficiency = evaluation.electron_efficiency / n_folds;
        evaluation.pion_efficiency = evaluation.pion_efficiency / n_folds;
    }
}



#endif
