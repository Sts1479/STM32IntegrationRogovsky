/*
 * SimGen.hpp
 *
 *  Created on: Feb 18, 2019
 *      Author: root
 */

#ifndef EMUL_SIMGEN_HPP_
#define EMUL_SIMGEN_HPP_

#include <math.h>
template<typename T>
class SimGen {
 private:
  int _nHarms = 3;
  T _ampHarms[3] = {1, 1, 1};
  int _perCount = 0;
  float _SampleFreq;
 public:
 SimGen (float sf = 3200): _SampleFreq(sf){};
  float MultiHarmGenStep(float PhaseShift, float StartFreq) {
    float buffVal = 0;
    float hSum;
    for (int i = 0; i < _nHarms; i++) hSum += _ampHarms[i];
    for (int ha = 1; ha <= _nHarms; ha++) {
      if (ha == 1) {
        buffVal += _ampHarms[ha - 1] *
                   sin(2 * M_PI * (_perCount * ha * StartFreq / _SampleFreq +
                                   PhaseShift / 180));
      } else {
        buffVal += _ampHarms[ha - 1] *
                   sin(2 * M_PI * _perCount * ha * StartFreq / _SampleFreq);
      }
    }
    if (++_perCount >= _SampleFreq / StartFreq) _perCount = 0;
    return (buffVal / hSum);
  }
  /*
          float MultiGenStep (float _PhaseShift, float _StartFreq) {
                          float buffVal = 0;
                          buffVal = sin(2 * M_PI * (_perCount * _StartFreq /
     _SampleFreq +  _PhaseShift / 180 ) );
                          if (++_perCount >= _SampleFreq /_StartFreq) _perCount
     = 0;
                          return (buffVal);
                  }
  */
  T MultiGenStep(float PhaseShift, float StartFreq, short nCounter) {
    return (sinf(2 * M_PI *
                 (nCounter * StartFreq / _SampleFreq + PhaseShift / 180)));
  }
};

#endif /* EMUL_SIMGEN_HPP_ */
