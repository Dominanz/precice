// Gateway MexFunction object
#include "mex.hpp"
#include "mexAdapter.hpp"
#include <iostream>
#include <sstream>
#include "precice/SolverInterface.hpp"
#include "precice/Constants.hpp"

using namespace matlab::data;
using matlab::mex::ArgumentList;
using namespace precice;
using namespace precice::constants;

class MexFunction: public matlab::mex::Function {
private:
    SolverInterface* interface;
    ArrayFactory factory;
    bool constructed;
    
    void myMexPrint(const std::string text) {
        std::cout << "MEX gateway: " << text << std::endl;
    }
    
public:
    MexFunction(): constructed{false}, factory{}, interface{NULL} {}

    void operator()(ArgumentList outputs, ArgumentList inputs) {
        bool valid=false;
        // define the functionID
        const TypedArray<uint8_t> functionIDArray = inputs[0];
        int functionID = functionIDArray[0];
        std::cout << "Gateway function " << (int)functionID << " was called." << std::endl;
        
        // Abort if constructor was not called before, or if constructor 
        // was called on an existing solverInterface
        if (functionID==0 && constructed) {
            myMexPrint("Constructor was called but interface is alread construced.");
            return;
        }
        if (!constructed && functionID!=0) {
            myMexPrint("Interface was not construced before.");
            return;
        }
        
        switch (functionID) {
            // 0-9: Construction and Configuration
            // 10-19: Steering
            // 20-29: Status Queries
            // 30-39: Action Methods
            // 40-59: Mesh Access
            // 60-79: Data Access
            case 0: //constructor
            {
                const StringArray solverName = inputs[1];
                interface = new SolverInterface(solverName[0],0,1);
                constructed = true;
                break;
            }
            case 1: //destructor
            {
                delete interface;
                constructed = false;
                break;
            }
            case 2: //configure
            {
                const StringArray configFileName = inputs[1];
                interface->configure(configFileName[0]);
                break;
            }
            
            case 10: //initialize
            {
                double dt = interface->initialize();
                outputs[0] = factory.createArray<double>({1,1}, {dt});
                break;
            }
            case 11: //initializeData
            {
                interface->initializeData();
                break;
            }
            case 12: //advance
            {
                const TypedArray<double> dt_old = inputs[1];
                double dt = interface->advance(dt_old[0]);
                outputs[0] = factory.createArray<double>({1,1}, {dt});
                break;
            }
            case 13: //finalization
            {
                interface->finalize();
                break;
            }
            
            case 20: //getDimensions
            {
                int dims = interface->getDimensions();
                outputs[0] = factory.createArray<uint8_t>({1,1}, {(uint8_t) dims});
                break;
            }
            case 21: //isCouplingOngoing
            {
                bool result = interface->isCouplingOngoing();
                outputs[0] = factory.createArray<bool>({1,1}, {result});
                break;
            }
            case 22: //isReadDataAvailable
            {
                bool result = interface->isReadDataAvailable();
                outputs[0] = factory.createArray<bool>({1,1}, {result});
                break;
            }
            case 23: //isWriteDataRequired
            {
                const TypedArray<double> dt_old = inputs[1];
                bool result = interface->isWriteDataRequired(dt_old[0]);
                outputs[0] = factory.createArray<bool>({1,1}, {result});
                break;
            }
            case 24: //isTimestepComplete
            {
                bool result = interface->isTimestepComplete();
                outputs[0] = factory.createArray<bool>({1,1}, {result});
                break;
            }
            case 25: //hasToEvaluateSurrogateModel
            {
                bool result = interface->hasToEvaluateSurrogateModel();
                outputs[0] = factory.createArray<bool>({1,1}, {result});
                break;
            }
            case 26: //hasToEvaluateFineModel
            {
                bool result = interface->hasToEvaluateFineModel();
                outputs[0] = factory.createArray<bool>({1,1}, {result});
                break;
            }
            
            case 30: //isActionRequired
            {
                const StringArray action = inputs[1];
                bool result = interface->isActionRequired(action[0]);
                outputs[0] = factory.createArray<bool>({1,1}, {result});
                break;
            }
            case 31: //fulfilledAction
            {
                const StringArray action = inputs[1];
                interface->fulfilledAction(action[0]);
                break;
            }
            
            case 40: //has Mesh
            {
                const StringArray meshName = inputs[1];
                bool output = interface->getMeshID(meshName[0]);
                outputs[0] = factory.createScalar<bool>(output);
                break;
            }
            case 41: //getMeshID
            {
                const StringArray meshName = inputs[1];
                int id = interface->getMeshID(meshName[0]);
                outputs[0] = factory.createScalar<int32_t>(id);
                break;
            }
            case 42: //getMeshIDs
            {
                const std::set<int> ids = interface->getMeshIDs();
                outputs[0] = factory.createArray<int32_t>({1,ids.size()}, &*ids.begin(), &*ids.end());
                break;
            }
            
            //getMeshHandle: Not implemented yet
            
            case 44: //setMeshVertex
            {
                const TypedArray<int32_t> meshID = inputs[1];
                const TypedArray<double> position = inputs[2];
                int id = interface->setMeshVertex(meshID[0],&*position.begin());
                outputs[0] = factory.createScalar<int32_t>(id);
                break;
            }
            case 45: //getMeshVertexSize
            {
                const TypedArray<int32_t> meshID = inputs[1];
                int size = interface->setMeshVertex(meshID[0]);
                outputs[0] = factory.createScalar<int32_t>(size);
                break;
            }
            case 46: //setMeshVertices
            {
                const TypedArray<int32_t> meshID = inputs[1];
                const TypedArray<uint64_t> size = inputs[2];
                const TypedArray<double> positions = inputs[3];
                buffer_ptr_t<int32_t> ids_ptr = factory.createBuffer<int32_t>(size[0]);
                int32_t* ids = ids_ptr.get();
                interface->setMeshVertices(meshID[0],size[0],&*positions.begin(),ids);
                outputs[0] = factory.createArrayFromBuffer<int32_t>({1,size[0]}, std::move(ids_ptr));
                break;
            }
            case 47: //getMeshVertices
            {
                const TypedArray<int32_t> meshID = inputs[1];
                const TypedArray<uint64_t> size = inputs[2];
                const TypedArray<int32_t> ids = inputs[3];
                buffer_ptr_t<double> positions_ptr = factory.createBuffer<double>(size[0]);
                double* positions = positions_ptr.get();
                interface->getMeshVertices(meshID[0],size[0],&*ids.begin(),positions);
                outputs[0] = factory.createArrayFromBuffer<int32_t>({1,size[0]}, std::move(positions_ptr));
                break;
            }
            case 48: //getMeshVertexIDsFromPositions
            {
                const TypedArray<int32_t> meshID = inputs[1];
                const TypedArray<uint64_t> size = inputs[2];
                const TypedArray<double> positions = inputs[3];
                buffer_ptr_t<int32_t> ids_ptr = factory.createBuffer<int32_t>(size[0]);
                int32_t* ids = ids_ptr.get();
                interface->getMeshVertexIDsFromPositions(meshID[0],size[0],&*positions.begin(),ids);
                outputs[0] = factory.createArrayFromBuffer<int32_t>({1,size[0]}, std::move(ids_ptr));
                break;
            }
            case 61: //getDataID
            {
                const StringArray dataName = inputs[1];
                const TypedArray<int32_t> meshID = inputs[2];
                int id = interface->getDataID(dataName[0],meshID[0]);
                outputs[0] = factory.createScalar<int32_t>(id);
                break;
            }
            case 66: //writeBlockScalarData
            {
                const TypedArray<int32_t> dataID = inputs[1];
                const TypedArray<uint64_t> size = inputs[2];
                const TypedArray<int32_t> vertexIDs = inputs[3];
                const TypedArray<double> values = inputs[4];
                interface->writeBlockScalarData(dataID[0],size[0],&*vertexIDs.begin(),&*values.begin());
                break;
            }
            case 70: //readBlockScalarData
            {
                const TypedArray<int32_t> dataID = std::move(inputs[1]);
                const TypedArray<uint64_t> size = std::move(inputs[2]);
                const TypedArray<int32_t> valueIndices = std::move(inputs[3]);
                buffer_ptr_t<double> values_ptr = factory.createBuffer<double>(size[0]);
                double* values = values_ptr.get();
                interface->readBlockScalarData(dataID[0],size[0],&*valueIndices.begin(),values);
                outputs[0] = factory.createArrayFromBuffer<double>({1,size[0]}, std::move(values_ptr));
                break;
            }
            default:
                myMexPrint("An unknown ID was passed.");
                return;
        }
        // Do error handling
        // myMexPrint("A problem occurred while executing the function.");
    }
};