//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "Tendon/StiffTendon.h"
#include "Tendon/ElasticTendon.h"
#include "Tendon/ElasticTendon_BiSec.h"

#include "SetupDataStructure.h"
#include "Activation/ExponentialActivation.h"
#include "ErrorMinimizerAnnealing.h"
#include "HybridWeightings.h"
#include "Curve.h"
#include "ExecutionXmlReader.h"
#include "FileUtils.h"

#include "EMGFromFile.h"
#include "LmtMaFromStorageFile.h"
#include "ExternalTorquesFromStorageFile.h"
#include "ModelEvaluationOnline.h" 

#include "InputQueues.h"

#include "LoggerOnQueues.h"
#include "QueuesToStorageFiles.h"

//#include "ModelEvaluationOffline.h"
//#include "ModelEvaluationHybrid.h"

#include <ctime>

#include <iomanip>

#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <stdlib.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;


template <typename T>
void setupSubject(T& mySubject, string configurationFile) {
    
    SetupDataStructure<T> setupData(configurationFile);
    setupData.createCurves();
    setupData.createMuscles(mySubject);
    setupData.createDoFs(mySubject);
    
}
    

void printHeader() {

    cout << endl;
    cout << "+-+-+-+-+-+-+\n"                            
         << "|C|E|I|N|M|S|\n"
         << "+-+-+-+-+-+-+-+-+-+-+\n"                    
         << "|C|a|l|i|b|r|a|t|e|d|\n"                    
         << "+-+-+-+-+-+-+-+-+-+-+-+-+\n"                
         << "|E|M|G|-|I|n|f|o|r|m|e|d|\n"                
         << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n"
         << "|N|e|u|r|o|m|u|s|c|u|l|o|s|k|e|l|e|t|a|l|\n"
         << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n"
         << "|T|o|o|l|b|o|x|\n"                          
         << "+-+-+-+-+-+-+-+\n\n";                   
        
    }
    
void printAuthors() {
    
    time_t now = std::time(0);
    tm *gmtm = std::gmtime(&now);
    cout << "Copyright (C) " << gmtm->tm_year+1900 << endl;
    cout << "Claudio Pizzolato, Monica Reggiani, David Lloyd, Massimo Sartori\n\n";
    
    cout << "Software developers: Claudio Pizzolato, Monica Reggiani\n";
}
    
    
void setLmtMaFilenames(const string& inputDirectory, const vector< string > dofNames, string& lmtDataFilename, vector< string >& maDataFilenames)
{
  std::string pattern{"_MuscleAnalysis_Length.sto"};
  std::string directory{inputDirectory + "/MuscleAnalysis/"};
  lmtDataFilename = directory + findFile(pattern, directory);
  
  int currentDof = 0; 
  for (auto& it: dofNames)
  {
    std::string pattern = "_MomentArm_" + it + ".sto";
    std::string maDataFilename = directory + findFile(pattern, directory);
    maDataFilenames.push_back(maDataFilename); 
  }
}



void consumeAndStore(CEINMS::Concurrency::Queue< std::vector<double> >& queue, const string& outputFileName, const vector<string>& header) {
  
  queue.subscribe(); 
  CEINMS::InputConnectors::doneWithSubscription.wait();
  
  std::ofstream outputFile (outputFileName);
  if (!outputFile.is_open())
  { 
    std::cout << "Problem opening file" << outputFileName << std::endl;  
    exit(EXIT_FAILURE); 
  }
    
  
  for (auto it: header)
    outputFile << it << ",";
  outputFile << "Time" << ",\n";
  
  for(;;) {
     vector<double> item = queue.pop(); 
     for (auto it : item)
       outputFile << std::setprecision(8)  << it << ","; 
     outputFile << endl; 
  }
}

int main(int argc, char** argv) {
 
    printHeader();
    printAuthors();
    
#ifdef LOG  
  cout << "Check configuration data...\n";
#endif
  
    string subjectFile;
    string executionFile;
    string inputDirectory;
    string outputDirectory;
    string emgGeneratorFile;

    int opt;
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("subject,s", po::value<string>(&subjectFile), "subject xml file")
    ("execution,x", po::value<string>(&executionFile),  "execution xml file")
    ("input-dir,i", po::value<string>(&inputDirectory), "trial directory path")
    ("output-dir,o", po::value<string>(&outputDirectory)->default_value("./Output"), "output directory")
    ("emg-generator,g", po::value<string>(&emgGeneratorFile)->default_value("cfg/xml/emgGenerator.xml"), "EMG mapping");
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help") || argc < 7) {
        cout << desc << "\n";
        return 1;
    }
 

    ExecutionXmlReader executionCfg(executionFile);             
    
    NMSModelCfg::RunMode runMode = executionCfg.getRunMode();
   
//    NMSModelCfg::RunMode runMode = NMSModelCfg::HybridPiecewiseActivationElasticTendonOnline;
    switch(runMode) {
               
        case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOnline: {
          
          
          
          // 1. define the model
          typedef NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online> MyNMSmodel;
          MyNMSmodel mySubject;
          setupSubject(mySubject, subjectFile);
          
          // 2. define the thread connecting with the input sources
          
          string emgFilename(FileUtils::getFile(inputDirectory, "emg.txt"));
          EMGFromFile emgProducer(mySubject, emgFilename, emgGeneratorFile);
          
          vector< string > dofNames; 
          mySubject.getDoFNames(dofNames);
          string lmtFilename;
          vector< string > maFilename;
          setLmtMaFilenames(inputDirectory, dofNames, lmtFilename, maFilename);
          LmtMaFromStorageFile lmtMaProducer(mySubject, lmtFilename, maFilename); 
          
          
          string externalTorqueFilename(FileUtils::getFile(inputDirectory, "inverse_dynamics.sto"));
          ExternalTorquesFromStorageFile externalTorquesProducer(mySubject, externalTorqueFilename); 
   
              
          // 2b. define the thread consuming the output sources
          vector<string> valuesToWrite = {"Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques"};
          QueuesToStorageFiles queuesToStorageFiles(mySubject, valuesToWrite, outputDirectory) ;  
          
          // 3. define the model simulator
          vector<string> valuesToLog = {"Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques"};
          ModelEvaluationOnline<MyNMSmodel, LoggerOnQueues> simulator(mySubject, valuesToLog);
     
          
          CEINMS::InputConnectors::doneWithSubscription.setCount(5);
          CEINMS::OutputConnectors::doneWithExecution.setCount(2);
          
          // 4. start the threads
          std::thread emgProdThread(std::ref(emgProducer)); 
          std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
          std::thread lmtMaProdThread(std::ref(lmtMaProducer));
          std::thread simulatorThread(std::ref(simulator));    
          std::thread queuesToStorageFilesThread(std::ref(queuesToStorageFiles)); 
          
//           vector < string > muscleNames; 
//           mySubject.getMuscleNames(muscleNames);
//           std::thread lmtConsThread(std::ref(consumeAndStore), std::ref(CEINMS::InputConnectors::queueLmt),outputDirectory + "/lmt.csv", std::ref(muscleNames));
//      
    
          emgProdThread.join();
          lmtMaProdThread.join();
          externalTorquesProdThread.join();
          simulatorThread.join();
          queuesToStorageFilesThread.join();

          break;
        }
       
//         case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOffline: {
//           // 1. define the model 
//           typedef NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Offline> MyNMSmodel;
//           MyNMSmodel mySubject;
//           setupSubject(mySubject, subjectFile);
//           
//           // 2. define the producer. They have a reference to the subject and can ask information about it
//           LmtMaFromFile lmtMaProducer(mySubject, inputDirectory);
//           ExternalTorqueFromFile externalTorqueProducer(mySubject, inputDirectory);
//           EMGFromFile emgProducer(mySubject, inputDirectory);
//           
//           ModelEvaluationOffline<MyNMSmodel> consumer(mySubject, outputDirectory);
//           runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
//           break;
//         }
        
//         case NMSModelCfg::OpenLoopExponentialActivationElasticTendonOnline: {
//             typedef NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Online>, CurveMode::Online> MyNMSmodel;
//             MyNMSmodel mySubject;
//             setupSubject(mySubject, subjectFile);
//             ModelEvaluationOnline<MyNMSmodel> consumer(mySubject);
//             runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
//             break;
//         }
//         
//         case NMSModelCfg::OpenLoopExponentialActivationElasticTendonOffline: {
//             typedef NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Offline> MyNMSmodel;
//             MyNMSmodel mySubject;
//             setupSubject(mySubject, subjectFile);
//             ModelEvaluationOffline<MyNMSmodel> consumer(mySubject);
//             runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
//             break;
//         }
        
//         case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOnline: {
//             typedef NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> MyNMSmodel;
//             MyNMSmodel mySubject;
//             setupSubject(mySubject, subjectFile);
//             ModelEvaluationOnline<MyNMSmodel> consumer(mySubject, outputDirectory);
//             runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
//             break;
//         }
//         
//         case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOffline: {
//             typedef NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline> MyNMSmodel;
//             MyNMSmodel mySubject;
//             setupSubject(mySubject, subjectFile);
//             ModelEvaluationOffline<MyNMSmodel> consumer(mySubject, outputDirectory);
//             runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
//             break;
//         }
//         
//         
//         case NMSModelCfg::HybridExponentialActivationStiffTendonOnline: { 
//             typedef NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online> MyNMSmodel;
//             typedef Hybrid::ErrorMinimizerAnnealing<MyNMSmodel> MyErrorMinimizer;
//             SetupDataStructure<MyNMSmodel> setupData(subjectFile);
//             MyNMSmodel mySubject;
//             setupData.createCurves();
//             setupData.createMuscles(mySubject);
//             setupData.createDoFs(mySubject);
//             MyErrorMinimizer errorMinimizer(mySubject);
//             HybridWeightings weightings;
//             executionCfg.getHybridWeightings(weightings.alpha, weightings.beta, weightings.gamma);
//             errorMinimizer.setWeightings(weightings);
//             vector<string> toPredict, toTrack;
//             executionCfg.getMusclesToPredict(toPredict);
//             executionCfg.getMusclesToTrack(toTrack);
//             errorMinimizer.setMusclesNamesWithEmgToPredict(toPredict);
//             errorMinimizer.setMusclesNamesWithEmgToTrack(toTrack);
//             double rt, t, epsilon;
//             unsigned noEpsilon, ns, nt, maxNoEval;
//             executionCfg.getAnnealingParameters(nt, ns, rt, t, maxNoEval, epsilon, noEpsilon);
//             errorMinimizer.setAnnealingParameters(nt, ns, rt, t, maxNoEval, epsilon, noEpsilon);
//             ModelEvaluationHybrid<MyNMSmodel, MyErrorMinimizer> consumer(mySubject, errorMinimizer, outputDirectory);
//             runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
//             break;
//         } 
//         
//           case NMSModelCfg::HybridExponentialActivationElasticTendonBiSecOnline: { 
//             typedef NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> MyNMSmodel;
//             typedef Hybrid::ErrorMinimizerAnnealing<MyNMSmodel> MyErrorMinimizer;
//             SetupDataStructure<MyNMSmodel> setupData(subjectFile);
//             MyNMSmodel mySubject;
//             setupData.createCurves();
//             setupData.createMuscles(mySubject);
//             setupData.createDoFs(mySubject);
//             MyErrorMinimizer errorMinimizer(mySubject);
//             HybridWeightings weightings;
//             executionCfg.getHybridWeightings(weightings.alpha, weightings.beta, weightings.gamma);
//             errorMinimizer.setWeightings(weightings);
//             vector<string> toPredict, toTrack;
//             executionCfg.getMusclesToPredict(toPredict);
//             executionCfg.getMusclesToTrack(toTrack);
//             errorMinimizer.setMusclesNamesWithEmgToPredict(toPredict);
//             errorMinimizer.setMusclesNamesWithEmgToTrack(toTrack);
//             double rt, t, epsilon;
//             unsigned noEpsilon, ns, nt, maxNoEval;
//             executionCfg.getAnnealingParameters(nt, ns, rt, t, maxNoEval, epsilon, noEpsilon);
//             errorMinimizer.setAnnealingParameters(nt, ns, rt, t, maxNoEval, epsilon, noEpsilon);
//             ModelEvaluationHybrid<MyNMSmodel, MyErrorMinimizer> consumer(mySubject, errorMinimizer, outputDirectory);
//             runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
//             break;
//         } 


        default:
            std::cout << "Implementation not available yet. Verify you XML configuration file\n";
            break;
            
                
            
            
    }
    
  
  return 0;
}
