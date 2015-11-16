
# makefile for yosemite

OPTIONS = -O3 #-Wall
CPP_VERSION = -std=c++11
SHARK_MAKE = -I/usr/local -I/usr/local/include 
#SHARK_MAKE =  -I/Users/dzed/shark/Shark/include -L/Users/dzed/shark/Shark/lib/
BOOST_MAKE = #Nothing needed
LINK = -L/usr/local/lib/ -L/usr/lib -lboost_serialization -lshark
OBJECT_FILES = BoostClassifier.o CrossValidation.o DzCARTTrainer.o  Optimiser.o WeakClassifiers.o AbstractClassifier.o Main.o

#/Users/dzed/shark/Shark/include -L/Users/dzed/shark/Shark/lib/ ${OBJECT_FILES}  -lshark -lboost_serialization -o run

make:
	g++ -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} AbstractClassifier.cpp   -o AbstractClassifier.o
	g++ -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} WeakClassifiers.cpp      -o WeakClassifiers.o
	g++ -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} BoostClassifier.cpp      -o BoostClassifier.o
	g++ -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} CrossValidation.cpp      -o CrossValidation.o
	g++ -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} Optimiser.cpp            -o Optimiser.o
	g++ -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} main.cpp                 -o Main.o
	g++    ${CPP_VERSION} ${SHARK_MAKE} ${OBJECT_FILES} ${BOOST_MAKE} ${LINK}               -o run

AbstractClassifier:
	g++  -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} AbstractClassifier.cpp   -o AbstractClassifier.o
	g++  -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} main.cpp                 -o Main.o
	g++     ${CPP_VERSION} ${SHARK_MAKE} ${OBJECT_FILES} ${BOOST_MAKE} ${LINK}  -o run

WeakClassifiers:
	g++  -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} WeakClassifiers.cpp      -o WeakClassifiers.o
	g++  -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} main.cpp                 -o Main.o
	g++     ${CPP_VERSION} ${SHARK_MAKE} ${OBJECT_FILES} ${BOOST_MAKE} ${LINK}               -o run

BoostClassifier:
	g++  -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} BoostClassifier.cpp     -o BoostClassifier.o
	g++  -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} main.cpp                -o Main.o
	g++     ${CPP_VERSION} ${SHARK_MAKE} ${OBJECT_FILES} ${BOOST_MAKE} ${LINK}              -o run

Optimiser:
	g++  -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} Optimiser.cpp            -o Optimiser.o
	g++  -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} main.cpp                 -o Main.o
	g++     ${CPP_VERSION} ${SHARK_MAKE} ${OBJECT_FILES} ${BOOST_MAKE} ${LINK}               -o run

CrossValidation:
	g++  -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} CrossValidation.cpp      -o CrossValidation.o
	g++  -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} main.cpp                 -o Main.o
	g++     ${CPP_VERSION} ${SHARK_MAKE} ${OBJECT_FILES} ${BOOST_MAKE} ${LINK}               -o run

Main:
	g++  -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE}   ${OPTIONS} main.cpp                 -o Main.o
	g++     ${CPP_VERSION} ${SHARK_MAKE} ${OBJECT_FILES} ${BOOST_MAKE} ${LINK}               -o run

Tree:
	g++  -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE} DzCARTTrainer.cpp   -o DzCARTTrainer.o

clean:
	rm *.o run
	g++  -c ${CPP_VERSION} ${SHARK_MAKE} ${BOOST_MAKE} DzCARTTrainer.cpp   -o DzCARTTrainer.o
