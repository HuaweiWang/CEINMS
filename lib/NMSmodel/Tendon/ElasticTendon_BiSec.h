#ifndef ElasticTendon_BiSec_h
#define ElasticTendon_BiSec_h
#include <string>
#include "Curve.h"

//template <typename CurveMode::Mode mode>
class ElasticTendon_BiSec {

public:
    typedef Curve<CurveMode::Offline> CurveOffline;
    ElasticTendon_BiSec();
    ElasticTendon_BiSec(std::string id);
    ElasticTendon_BiSec( double optimalFibreLength,
                   double pennationAngle,
                   double tendonSlackLength,
                   double percentageChange,
                   double damping,
                   double maxIsometricForce, 
                   double strengthCoefficient,
                   const CurveOffline& activeForceLengthCurve,
                   const CurveOffline& passiveForceLengthCurve, 
                   const CurveOffline& forceVelocityCurve
                 );
    virtual ~ElasticTendon_BiSec() {}
    ElasticTendon_BiSec(const ElasticTendon_BiSec& orig);
    ElasticTendon_BiSec& operator=(const ElasticTendon_BiSec& orig);
    
    void setParametersToComputeForces(double optimalFibreLength,
                                      double pennationAngle,
                                      double tendonSlackLength,
                                      double percentageChange,
                                      double damping, 
                                      double maxIsometricForce, 
                                      double strengthCoefficient); 

    void setTime(const double& time) {} 
    void setMuscleTendonLength(double muscleTendonLength);
    void setActivation(double activation);
    void updateFibreLength();

    double getFibreLength() { return fibreLength_;}
    void setStrengthCoefficient(double strengthCoefficient);
    void setTendonSlackLength(double tendonSlackLength);
    void setCurves(const CurveOffline& activeForceLengthCurve, 
                   const CurveOffline& passiveForceLengthCurve, 
                   const CurveOffline& forceVelocityCurve);
                   
    void pushState();
    void resetState();
    
    
    /*
    
    void setMuscleTendonLength(double muscleTendonLength, double activation, double time);
    void updateFiberLengthUsingNewActivation(double activation, double time);
    double getFiberLength() { return fibreLength_;}
    void setStrengthCoefficient(double strengthCoefficient);
    void setTendonSlackLength(double tendonSlackLength);
    void setCurves(const CurveOffline& activeForceLengthCurve, 
                   const CurveOffline& passiveForceLengthCurve, 
                   const CurveOffline& forceVelocityCurve);
                   
    void resetState();
    */
private:
    double estimateFiberLengthBiSec(double tol, unsigned maxIterations);
    double evaluateForceError(double fibreLength);
    double computeMuscleForce(double fibreLength);
    double computeTendonForce(double fibreLength);
    
    double optimalFibreLength_;
    double pennationAngle_;
    double tendonSlackLength_;
    double percentageChange_;
    double damping_;
    double maxIsometricForce_; 
    double strengthCoefficient_;
    CurveOffline activeForceLengthCurve_;
    CurveOffline passiveForceLengthCurve_;
    CurveOffline forceVelocityCurve_;
    CurveOffline tendonForceStrainCurve_;
    
    double fibreLength_;
    double fibreLengthT1_;          //valore della fiberLength al passo precedente
    double muscleTendonLength_;
    double activation_;
    
    std::string id_;
    
};

#endif