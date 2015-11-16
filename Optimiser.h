
// Jedda Boyle (BYLJED001)
// Honours Computer Science Project.

#ifndef OPTIMISER_H
#define OPTIMISER_H

#include <shark/Data/Dataset.h>
#include "BoostClassifier.h"
#include "CrossValidation.h"
#include <iostream>
#include <algorithm>

template<class BoostClassifier>
class GeneticOptimiser {
public:
    typedef shark::RealVector RealVector;
	typedef std::vector<RealVector> GenomeVector;
    typedef shark::ClassificationDataset ClassificationDataset;
	GenomeVector population;
	const double max_alpha;
	const size_t generations;
	const size_t population_size;
	const double mutation_probability;
	const double cross_over_probability;
    const double required_pion_efficiency = 0.1;
    const double required_electron_efficieny = 0.9;
	size_t n_weak_classifiers;
	std::default_random_engine generator;


	GeneticOptimiser (double _max_alpha = 0.75,
                      size_t _generations = 200,
                      size_t _population_size = 500,
                      double _mutation_probability = 0.01,
                      double _cross_over_probabilty = 0.5,
                      double _required_pion_efficiency = 0.05,
                      double _required_electron_efficieny = 0.95):
    	max_alpha(_max_alpha),
    	generations(_generations),
    	population_size(_population_size),
    	mutation_probability(_mutation_probability),
    	cross_over_probability(_cross_over_probabilty),
        required_pion_efficiency(_required_pion_efficiency),
        required_electron_efficieny(_required_electron_efficieny) {
		// Initialise random number generators used in other functions.
		srand(time(NULL));
		generator.seed(time(NULL));
	}

	// Search the weightspace with a genetic algorithm.
	void optimise (BoostClassifier* classifier, ClassificationDataset data ) {
		// Setup.
		n_weak_classifiers = classifier->n_classifiers;
		double population_fitness, max_fitness;
		GenomeVector new_population;
		int max_position;

		// Initialise population.
		for(int i = 0; i < population_size; ++i) {
			RealVector genome;
			for (int j = 0; j < n_weak_classifiers; ++j ) {
				// Generate and add random number between 0 and max_alpha.
				genome.push_back((double) (rand() % 10001) / 10000 * max_alpha);
			}
			// Last position of the genome is used to record its fitness (-1 placeholder).
			genome.push_back(-1);
			population.push_back(genome);
		}

		// Evolutionary loop.
		for (int i = 0; i < generations; ++i) {
			// Reset variables for new generation.
			population_fitness = 0.0;
			max_fitness = 0;
			max_position = -1;

			// Calulate the fitness for each genome.
			for (int j = 0; j < population_size; ++j) {
                population[j][n_weak_classifiers];
                population[j][n_weak_classifiers] = this->fitness(population[j], classifier, data);
                //population[j][n_weak_classifiers] = this->fitness2(population[j], cv);
                population_fitness += population[j][n_weak_classifiers];
				// Record position and value of the maximum fitness.
				if (max_fitness <= population[j][n_weak_classifiers]) {
					max_position = j;
					max_fitness = population[j][n_weak_classifiers];
				}
			}
			//std::cout << "\rPopulation Fitness: "   << population_fitness << std::flush;
			genrateNewPopulation(new_population, population_fitness);
			// Replace the population with new population.
			for(int j = 0; j < population_size; ++j) {
				copy(new_population[j].begin(), new_population[j].end(), population[j].begin());
			}
		}

		// Set the weights of the classifier to the genome with max fitness.
		classifier->setAlphas(population[max_position]);

	}

	// Calculate the fitness of a single genome.
	double fitness (RealVector& genome, BoostClassifier* classifier, ClassificationDataset& data) { // TODO update this to include CV.
		classifier->setAlphas(genome);
        classifier->evaluate(data);
        //double pion_efficieny_penalty = std::max(0.0, classifier->results.pion_efficieny - required_pion_efficiency);
        //double electron_efficiency_penalty = std::max(0.0, required_electron_efficieny - classifier->results.electron_efficiency);
        //return classifier->results.electron_efficiency - classifier->results.pion_efficiency;
        return 1. - classifier->results.error; //- (1 * pion_efficieny_penalty) - (2 * electron_efficiency_penalty);

	}


	// Crossover two parent genomes using uniform crossover with probability cross_over_probability.
	void crossOver (RealVector& child, const  RealVector& l_parent, const RealVector& r_parent) {
		child.clear(); // Make sure child is empty.
		for(int i = 0; i < n_weak_classifiers; ++i) {
			if (cross_over_probability <= (double) (rand() % 10001) / 1000 ) {
				child.push_back(l_parent[i]);
			}
			else {
				child.push_back(r_parent[i]);
			}
		}
	}

	// Mutate a single genome using a normal distribution.
	void mutation (RealVector& genome) {
		std::normal_distribution<double> distribution (1, 0.05);
		for (int i = 0; i < n_weak_classifiers; ++i) {
			if ((double) (rand() % 101) / 100 <= mutation_probability) {
				genome[i] *= std::max(0.0 , distribution(generator));
			}
		}
	}

	void genrateNewPopulation (GenomeVector& new_population, double population_fitness ) {
		new_population.clear();
        int keep = population_size * 0.05 + 1;

        // Sort population so that the first 'keep' genomes are the genomes with the
        // highest fitness. These genomes are kept for the next generation.
        std::nth_element(population.begin(), population.begin() + keep, population.end(),
            [&](const RealVector& a, const RealVector& b) -> bool{
                return a[n_weak_classifiers] > b[n_weak_classifiers];
        });
        for(int i = 0; i < keep; ++i) {
            new_population.push_back(population[i]);
        }

        // Fill the rest of the population with child genomes where the parents
        // are selected using fitness proportionate.
        RealVector l_parent(n_weak_classifiers + 1);
		RealVector r_parent(n_weak_classifiers + 1);
		for (int i = 0; i < population_size - keep; ++i) {
			RealVector child;
			selectParent(l_parent, population_fitness);
			selectParent(r_parent, population_fitness);
			crossOver(child, l_parent, r_parent);
			mutation(child);
			new_population.push_back(child);
		}

	}

	// Select parent using fitness Proportionate selection.
	void selectParent (RealVector& parent, double population_fitness) {
		double spin = ((double) (rand() % 10001) / 10000) * population_fitness;
		double count = 0.0;
		for (auto& i: population) {
			if (count <= spin && spin <= count + i[n_weak_classifiers]) {
				copy(i.begin(), i.end(), parent.begin());
				return;
			}
			count += i[n_weak_classifiers];
		}
        

	}

};

#endif
