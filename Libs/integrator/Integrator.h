/*
 * Integrator.h
 *
 *  Created on: 16 нояб. 2019 г.
 *      Author: macbook
 */

#ifndef INTEGRATOR_H_
#define INTEGRATOR_H_
#include "math.h"
#include "../../Libs/ringbuff/RingBuf.h"
template <typename T, size_t max>
class Integrator {
private:
	int _cint = 0;
	int _cintM = 0;
	int _cintP = 0;
	int _cintF = 0;
	T _freqInOld = 0;
	uint16_t _pCounter = 0;
	uint16_t _pCounterOld = 0;
	uint8_t _p = 0;
	// int _size;
	T _buffout;
	T *_Buff;
	T *_BuffM;
	T *_BuffP;
	T *_BuffF;
	T _sum = 0;
	T _sumM = 0;
	T _sumP = 0;
	T _sumStd = 0;
	T _DInvOld = 0;
	T _cPerOld = 0;
	T _corrMean = 0;
	T _meanOld = 0;
	T _stdOld = 0;
#define FIRorder 10U
#define Nper 150U

public:
	Integrator() {
	  _Buff = new T[max];
	  _BuffM = new T[max];
	  _BuffP = new T[max];
	  _BuffF = new T[max];

	};


	RingBuf<T,max> buff1;

	RingBuf<T,max> buff2;
	RingBuf<T,max> buff3;
	RingBuf<T,FIRorder> buff4;



	virtual ~Integrator(){};

	T IntegStep(T in, T calib, T coeff, uint16_t cPer){
		T meanVal = calib;
		++_cint;
		T pcorr = 0;
		for (int i = 0;i < cPer-1;i++){
			 _Buff[i] = _Buff[i+1];
		}
		_Buff[cPer-1] = in;
		_sum +=in;
		if (_cint > cPer){
			 pcorr = cPer - _cPerOld;
		    _sum -= _Buff[0];
			meanVal = _sum/(cPer+1+pcorr);
			_cint = max+2;
			_cPerOld = cPer;
		}
		_buffout+=(in-meanVal);
		return coeff*_buffout;
	};

	T IntegStepD(T in, T calib, T coeff, uint16_t cPer){
		T meanVal = calib;
		++_cint;
		for (int i = 0;i < cPer-1;i++){
			 _Buff[i] = _Buff[i+1];
		}
		_Buff[cPer-1] = in;
		_buffout+=(in-meanVal);
		return coeff*_buffout;
	};

	T IntegStepC(T in, T calib, T coeff, uint16_t cPer){
		T meanVal = calib;
		++_cint;
		T corrValue = 0;
		T stdVal = 0;;
		for (int i = 0;i < cPer-1;i++){
			 _Buff[i] = _Buff[i+1];
		}
		_Buff[cPer-1] = in;
		for (int i = 0;i < cPer;i++){
			 _sum += _Buff[i];
			// _sumStd +=_Buff[i]*_Buff[i];
		}
		if (_cint > cPer) {
			meanVal = _sum/cPer;
		//	stdVal = _sumStd/cPer;
			_cint = max;
			_sum = 0;
			_sumStd = 0;

         if (meanVal - _meanOld > 20 || meanVal>3000 )
        	 _corrMean +=10;
         if (meanVal - _meanOld < -20 || meanVal<300)
        	 _corrMean +=-10;
         if (meanVal == _meanOld) {
        	 /*
        	 if (_corrMean > 0 )
        		 _corrMean = _corrMean - 1;
        	 if (_corrMean < 0 )
				 _corrMean = _corrMean + 1;
        	 if (_corrMean == 0 )
        		 */
        		 _corrMean = 0;
         }
		}
        /*
         if (in > _DInvOld && (in - _DInvOld)>100) {
        	 _corrMean = _corrMean - 10;
         }
         if (in < _DInvOld && (_DInvOld - in)>100) {
        	 _corrMean = _corrMean + 10;
         }
         _DInvOld = in;
         */

         _meanOld = meanVal;
         //_stdOld = stdVal;

		_buffout+=(in-meanVal+_corrMean);
		return coeff*_buffout;
	};

	T IntegStepQ(T in, T calib, T coeff){
			T meanVal = calib;
			T esum = 0;
			_sum +=in;
			if(!buff1.add(in,true)){
			    esum = *buff1.peek(0);
			    _sum -=esum;
				meanVal = _sum/(max-1);
			}
			_buffout+=(in-meanVal);
			return coeff*_buffout;
		};

	T MeanStepDel (T in, uint16_t cPer){
		T meanVal = 0;
		++_cintM;
		for (int i = 0;i < cPer-1;i++){
			 _BuffM[i] = _BuffM[i+1];
		}
		_BuffM[cPer-1] = in;
		_sumM +=in;
		if (_cintM > max){
			_sumM -=_BuffM[0];
			_cintM = max+2;
			meanVal = _sumM/(cPer+1);
		}
		return 	in - meanVal;
	}

	T MeanStepDelQ (T in){
		T meanVal = 0;
		T esum = 0;
		_sumM+= in;
		if(!buff2.add(in,true)){
			esum = *buff2.peek(0);
			_sumM -=esum;
			meanVal = _sumM/(max);
		}
		return 	in - meanVal;
	}



	uint16_t PeriodStepCalc (T In, T meanVal) {
		    uint16_t periodOut = 0;
			T pIn, pInOld;
			T delta;
			//T meanVal;
			int pCounter = 0;
			int pN = 0;
			int p = 0;
			T esum;
			_sumP+=In;
			if(!buff3.add(In,true)){
				esum = *buff3.peek(0);
				_sumP -=esum;
				meanVal = _sumP/Nper;
				//meanVal = 0;
				for (int i=0;i<Nper-1;i++) {
					pIn = *buff3.peek(i);
					pInOld = *buff3.peek(i+1);
					pIn = pIn - meanVal;
					pInOld = pInOld - meanVal;
					++pCounter;
					delta = abs((pIn-pInOld)/(pIn+pInOld));
					if (delta>=1) {
						if (p < 2){
							if (p < 1){
								pN = pCounter;
							}
							++p;
						}
						else {
							p = 0;
							periodOut = pCounter-pN;
							pN = 0;
							return periodOut;
					 }
				}
				}
			}
			return periodOut;
		}

	uint16_t PeriodStepCalcQ (T In, T meanVal) {
		    uint16_t periodOut = 0;
			T pIn, pInOld;
			float delta;
			int pCounter = 0;
			int pN = 0;
			int p = 0;
			T esum;
			_sumP+=In;
			if(!buff3.add(In,true)){
				esum = *buff3.peek(0);
				_sumP -=esum;
				meanVal = _sumP/(max);
				//meanVal = 0;
				for (int i=0;i<max-1;i++) {
					pIn = *buff3.peek(i);
					pInOld = *buff3.peek(i+1);
					pIn = pIn - meanVal;
					pInOld = pInOld - meanVal;
					++pCounter;
					if (pIn != pInOld && abs(pInOld) != abs(pIn) ){
					delta = abs((pIn-pInOld)/(pIn+pInOld));
					}
					else delta = 0.5;
					if (delta>=1) {
						if (p < 2){
							if (p < 1){
								pN = pCounter;
							}
							++p;
						}
						else {
							p = 0;
							periodOut = pCounter-pN;
							pN = 0;
							return periodOut;
					 }
				}
				}
			}
			return periodOut;
		}

	uint16_t PeriodStepCalcA (T In, T meanVal) {
			    uint16_t periodOut = 0;
				T pIn, pIn1,pIn2,pIn3;
				T delta;
				T delta1;
				int pCounter = 0;
                int p = 0;
				int pN = 0;
				for (int i = 0;i < Nper-1;i++){
						 _BuffP[i] = _BuffP[i+1];
					}
				_BuffP[Nper-1] = In;
				_sumP +=In;
                ++_cintP;
				if(_cintP>Nper){
					_sumP -=_BuffP[0];
					meanVal = _sumP/Nper;
					//meanVal = 0;
					for (int i = 0; i < Nper-1; i++){
						pIn= _BuffP[i];
						pIn1 = _BuffP[i+1];

						pIn = pIn - meanVal;
						pIn1 = pIn1 - meanVal;
						++pCounter;
						delta = fabs((pIn1-pIn)/(pIn1+pIn));

						if (delta>=1) {
							if (p < 2){
								if (p < 1){
									pN = pCounter;
								}
								++p;
							}
							else {
								p = 0;
								periodOut = pCounter-pN;
								pCounter = 0;
								pN = 0;
								return periodOut;
						 }
					   }
					}
				}
				return periodOut;
			}

	T FIRsmooth (T In) {
		T fIn;
		++_cintF;
		T sumF;
		T Out;
		for (int i = 0;i < FIRorder-1;i++){
			 _BuffF[i] = _BuffF[i+1];
		}
		_BuffF[FIRorder-1] = In;
		if (_cintF>=FIRorder) {
			_cintF = FIRorder+1;
			for (int i = 0;i < FIRorder; i++)
				sumF += _BuffF[i];
			Out = sumF/FIRorder;
			return Out;
		}
		return In;
	}

	T DeltaInverse (T In, T Thr, T Shift){
		T Out;
		if (_DInvOld > 0) {
			if (In - _DInvOld > Thr) {
				return In+Shift;
			}
			else {
				return In-Shift;
			}
		}
		_DInvOld = In;
		return In;
	}

};

#endif /* INTEGRATOR_H_ */
