
// Shark imports.
#include <shark/Data/Csv.h>
#include <shark/Data/Dataset.h>
#include <shark/Data/CVDatasetTools.h>

// STD imports.
#include <iostream>
#include <unistd.h>
#include <vector>

// Other classes from this package.
#include "AbstractClassifier.h"
#include "WeakClassifiers.h"
#include "BoostClassifier.h"
#include "Optimiser.h"
#include "CrossValidation.h"


using namespace std;
using namespace shark;


// These are functions that are called to test the various different algorithms.
template<class WeakClassifier>
void testWeakClassifierSize (ClassificationDataset& data, size_t min_size ,size_t max_size = 100, size_t step=5, size_t num_steps=-1) {
    cout << "size,pe,ee,e" << endl;
    Evaluation results;
    for (size_t size_ = min_size; size_ <= max_size; size_ = size_ + step) {
        WeakClassifier classifier(size_);
        WeakCrossValidate<WeakClassifier>(results, &classifier, data, 5);
        cout << size_ << ", " << classifier.results.pion_efficiency << ", " << classifier.results.electron_efficiency << ", " << classifier.results.error << endl;
    }
}

template<class BoostClassifier>
void testBoostSize(BoostClassifier& boost, ClassificationDataset& data, size_t step_size = 50, size_t n_weak_classifiers = 1000) {
    vector<Evaluation> evaluations;
    vector<size_t> steps;
    for(size_t i = step_size; i <= n_weak_classifiers; i = i + step_size){
        steps.push_back(i);
    }
    BoostCrossValidate<BoostClassifier>(evaluations, steps, &boost, data, 5);
    cout << "size, pe, ee, e" << endl;
    for(int i = 0; i < evaluations.size(); ++i) {
        cout << steps[i] << ", "<< evaluations[i].pion_efficiency << ", " <<evaluations[i].electron_efficiency <<", "<< evaluations[i].error << endl;
    }
}


template <class BoostClassifier>
void OptimiserCrossValidate (std::vector<Evaluation>& optimised_results, Evaluation& trained_results, BoostClassifier* classifier, shark::ClassificationDataset& data, int n_folds) {
    data.shuffle(); // Shuffle the data to make sure every fold has enough electrons.
    shark::CVFolds<shark::ClassificationDataset> folds = shark::createCVSameSize(data, n_folds);
    double trained_error = 0.;
    double trained_electron_efficiency = 0.;
    double trained_pion_efficiency = 0;
    GeneticOptimiser<BoostClassifier> optimiser;

    optimised_results.clear();
    for (int i = 0; i < classifier->n_classifiers; ++i) {
        Evaluation result;
        optimised_results.push_back(result);
    }


    for (int i = 0; i < n_folds; ++i) {
        classifier->train(folds.training(i));
        classifier->evaluate2(folds.validation(i));
        trained_error += classifier->results.error;
        trained_electron_efficiency += classifier->results.electron_efficiency;
        trained_pion_efficiency += classifier->results.pion_efficiency;
        classifier->printClassifierWeights();
        optimiser.optimise(classifier, folds.training(i));
        classifier->printClassifierWeights();

        for(int j = 0; j < classifier->n_classifiers; ++j) {
            classifier->evaluate2(folds.validation(i));
            optimised_results[j].error += classifier->results.error;
            optimised_results[j].electron_efficiency += classifier->results.electron_efficiency;
            optimised_results[j].pion_efficiency += classifier->results.pion_efficiency;
            classifier->pruneClassifier();

        }

    }
    trained_results.error = trained_error / n_folds;
    trained_results.electron_efficiency = trained_electron_efficiency / n_folds;
    trained_results.pion_efficiency = trained_pion_efficiency / n_folds;

    for(int j = 0; j < classifier->n_classifiers; ++j) {
        optimised_results[j].error = optimised_results[j].error / n_folds;
        optimised_results[j].electron_efficiency = optimised_results[j].electron_efficiency / n_folds;
        optimised_results[j].pion_efficiency = optimised_results[j].pion_efficiency / n_folds;

    }
}

template<class BoostClassifier>
void testOptimiser(BoostClassifier& boost, ClassificationDataset& data) {
    Evaluation trained_results;
    std::vector<Evaluation> optimised_results;
    OptimiserCrossValidate<BoostClassifier>(optimised_results, trained_results, &boost, data, 5);
    cout << trained_results.pion_efficiency << ", " << trained_results.electron_efficiency << ", " << trained_results.error << endl;
    for(int i = 0; i < optimised_results.size(); ++i) {
        cout << optimised_results.size() - i << ", "<< optimised_results[i].pion_efficiency << ", " << optimised_results[i].electron_efficiency << ", " << optimised_results[i].error << endl;

    }
}



int main (void) {
    // Read in the data
    typedef NetworkClassifier<LogisticNeuron, LinearNeuron> Classifier;
    //typedef TreeClassifier Classifier;

    typedef AdaBoostClassifier<Classifier> BoostClassifier;
    //typedef EpsilonBoostClassifier<Classifier> BoostClassifier;


    ClassificationDataset data;
    importCSV(data, "8bitSummation.csv", LAST_COLUMN, ' ');

    // generate classifier
    Classifier classifier(10);
    classifier.train(data);
    classifier.printResults(data);

    // Cross validate the classifier
    Evaluation results;
    WeakCrossValidate<Classifier>(results, &classifier, data, 2);
    cout << results.error << endl;


    // Create a boost classifer.
    BoostClassifier boost(10, classifier);

    // Train boost classifier and print results.
    boost.train(data);
    boost.printResults(data);

    // Cross validate boost classifier.
    vector<Evaluation> eval;
    eval.push_back(results);
    vector<size_t> steps;
    steps.push_back(2);


    BoostCrossValidate<BoostClassifier>(eval, steps , &boost, data, 2);
    cout << eval[0].pion_efficiency << endl;






    return 1;
}
