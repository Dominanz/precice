#ifndef PRECICE_RECEIVER_H_
#define PRECICE_RECEIVER_H_ 

//
// ASCoDT - Advanced Scientific Computing Development Toolkit
//
// This file was generated by ASCoDT's simplified SIDL compiler.
//
// Authors: Tobias Weinzierl, Atanas Atanasov   
//

#include <iostream>
#include <string>



namespace precice { 

     class Receiver;
}

class precice::Receiver {
  public:
    virtual ~Receiver(){}
     virtual void receive(const double data,const int index,const int rank,int& tag)=0;
     virtual void receiveParallel(const double data,const int index,const int rank,int& tag)=0;


};

#endif