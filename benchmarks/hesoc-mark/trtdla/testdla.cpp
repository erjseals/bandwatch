#include "argsParser.h"
#include "buffers.h"
#include "common.h"
#include "logger.h"

#include "NvCaffeParser.h"
#include "NvInfer.h"
#include <cuda_runtime_api.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>

#define MAX_WORKSPACE_SIZE ((size_t)(2l*1024l*1024l*1024l))
#define BATCH_SIZE 1

#define KERNEL_SIZE_CONV 5

#define LAYERS_TYPE_FC 0
#define LAYERS_TYPE_CONVOLUTION 1

const std::string gSampleName = "DLA_measure_interf_test";

//!
//! \brief The TestDLAParams structure groups the additional parameters required by
//!         this MLP test.
//!
struct TestDLAParams : public samplesCommon::SampleParams
{
    int inputH;              //!< The input height
    int inputW;              //!< The input width
    int numLayers;           //!< How many layers
    int useDLACore;          //!< Which DLA core to use
    bool help;               //!< Show help?
    bool profile;            //!< Show profile?
    int iterations;          //!< How many iterations?
    int layersType;
    std::string serModel;
};

//! \brief  The TestDLA class implements the MNIST API sample
//!
//! \details It creates a fully connected MLP network, half-float to be processed by the DLA
//!
class TestDLA
{
    template <typename T>
    using SampleUniquePtr = std::unique_ptr<T, samplesCommon::InferDeleter>;

public:
    TestDLA(const TestDLAParams& params)
        : mParams(params)
        , mEngine(nullptr)
    {
    }

    //!
    //! \brief Function builds the network engine
    //!
    bool build(std::string &fileIn);

    //!
    //! \brief Runs the TensorRT inference engine for this sample
    //!
    bool infer();

    //!
    //! \brief Cleans up any state created in this class
    //!
    bool teardown();

private:
    TestDLAParams mParams; //!< The parameters for the sample.

    int mNumber{0}; //!< to init the network

    std::map<std::string, std::pair<nvinfer1::Dims, nvinfer1::Weights>>
        mWeightMap; //!< The weight name to weight value map

    std::shared_ptr<nvinfer1::ICudaEngine> mEngine; //!< The TensorRT engine used to run the network

    nvinfer1::Weights mConvKernel;

    //!
    //! \brief Uses the API to create the MLP Network
    //!
    bool constructNetwork(SampleUniquePtr<nvinfer1::IBuilder>& builder,
        SampleUniquePtr<nvinfer1::INetworkDefinition>& network, SampleUniquePtr<nvinfer1::IBuilderConfig>& config);

    //!
    //! \brief Reads the input  and stores the result in a managed buffer
    //!
    bool processInput(const samplesCommon::BufferManager& buffers);

    //!
    //! \brief Loads and init weights according to program params 
    //!
    std::map<std::string, std::pair<nvinfer1::Dims, nvinfer1::Weights>> loadWeights();

    //!
    //! \brief Add an MLP layer
    //!
    nvinfer1::ILayer* addMLPLayer(nvinfer1::INetworkDefinition* network, nvinfer1::ITensor& inputTensor,
        int32_t hiddenSize, nvinfer1::Weights wts, nvinfer1::Weights bias, nvinfer1::ActivationType actType, int idx);

    nvinfer1::ILayer* addConvLayer(nvinfer1::INetworkDefinition* network, nvinfer1::ITensor& inputTensor,
        int32_t outSize, nvinfer1::Weights wts, nvinfer1::Weights bias, nvinfer1::ActivationType actType, int idx);
};

//!
//! \brief Creates the network, configures the builder and creates the network engine
//!
//! \details This function creates the MLP network by using the API to create a model and builds
//!          the engine that will be used to run MNIST (mEngine)
//!
//! \return Returns true if the engine was created successfully and false otherwise
//!
bool TestDLA::build(std::string &filein)
{

    if(filein.compare("")==0){
        mWeightMap = loadWeights();
        auto builder = SampleUniquePtr<nvinfer1::IBuilder>(nvinfer1::createInferBuilder(gLogger.getTRTLogger()));
        if (!builder)
        {
            return false;
        }

        auto network = SampleUniquePtr<nvinfer1::INetworkDefinition>(builder->createNetwork());
        if (!network)
        {
            return false;
        }

        auto config = SampleUniquePtr<nvinfer1::IBuilderConfig>(builder->createBuilderConfig());
        if (!config)
        {
            return false;
        }

        auto constructed = constructNetwork(builder, network, config);
        if (!constructed)
        {
            return false;
        }

        assert(network->getNbInputs() == 1);
        auto inputDims = network->getInput(0)->getDimensions();
        assert(inputDims.nbDims == 3);

        assert(network->getNbOutputs() == 1);
        auto outputDims = network->getOutput(0)->getDimensions();
        assert(outputDims.nbDims == 3);

        std::string outfile = std::string("ser_eng_dla" + std::to_string(mParams.useDLACore) + ".trt");
        IHostMemory *serializedModel = mEngine->serialize();
        std::ofstream ofs(outfile.c_str(), std::ios::out | std::ios::binary);
        ofs.write((char*)(serializedModel->data()), serializedModel->size());
        ofs.close();
        serializedModel->destroy();

        std::cout << "Serialized model in " << outfile << std::endl;

        return true;

    } else {

        std::ifstream engineFile(filein, std::ios::binary);
        if (!engineFile)
        {
            std::cout << "Error opening engine file: " << filein << std::endl;
            return false;
        }

        engineFile.seekg(0, engineFile.end);
        long int fsize = engineFile.tellg();
        engineFile.seekg(0, engineFile.beg);

        std::vector<char> engineData(fsize);
        engineFile.read(engineData.data(), fsize);
        if (!engineFile)
        {
            std::cout << "Error loading engine file: " << filein << std::endl;
            return false;
        }

        IRuntime* runtime = createInferRuntime(gLogger);
        runtime->setDLACore(mParams.useDLACore);

        /*
        std::shared_ptr<nvinfer1::ICudaEngine>(
                builder->buildEngineWithConfig(*network, *config), samplesCommon::InferDeleter());
        */

        //ICudaEngine* engine = runtime->deserializeCudaEngine(engineData.data(), fsize, nullptr);
        mEngine = std::shared_ptr<nvinfer1::ICudaEngine>(runtime->deserializeCudaEngine(engineData.data(), fsize, nullptr), 
                                               samplesCommon::InferDeleter());
	return true;
    }

}

//!
//! \brief Uses the API to create the MLP Network
//!
//! \param network Pointer to the network that will be populated with the network
//!
//! \param builder Pointer to the engine builder
//!
bool TestDLA::constructNetwork(SampleUniquePtr<nvinfer1::IBuilder>& builder,
    SampleUniquePtr<nvinfer1::INetworkDefinition>& network, SampleUniquePtr<nvinfer1::IBuilderConfig>& config)
{
    
    nvinfer1::ITensor* input = nullptr;

    // FC layers must still have 3 dimensions, so we create a {C, 1, 1,} matrix.

    if(mParams.layersType==LAYERS_TYPE_FC){
        input = network->addInput(mParams.inputTensorNames[0].c_str(), nvinfer1::DataType::kHALF,
                nvinfer1::Dims3{(mParams.inputH) * (mParams.inputW),1,1});
    }else{
        input = network->addInput(mParams.inputTensorNames[0].c_str(), nvinfer1::DataType::kHALF,
                nvinfer1::Dims3{1,(mParams.inputH),(mParams.inputW)});
    }

    assert(input != nullptr);

    const nvinfer1::Weights nullbias{nvinfer1::DataType::kHALF, nullptr, 0};

    size_t l = 0;
    for (auto const& x : mWeightMap)
    {

        int index = ( l==(mWeightMap.size()-1) ? -1 : l );
        nvinfer1::ILayer* layer = nullptr;

	    /*std::cout << "Adding layer " << index << "/" << l << "/"  << mWeightMap.size()  << std::endl;
	    std::cout << "Layer " << x.first << " " << x.second.second.count << std::endl;*/

        if(mParams.layersType==LAYERS_TYPE_FC){
            layer = addMLPLayer(network.get(),*input,mParams.inputH*mParams.inputW,
                                x.second.second, nullbias, nvinfer1::ActivationType::kRELU, index);
    	}
        else{
            layer = addConvLayer(network.get(),*input,l+1,
                                        x.second.second, nullbias, nvinfer1::ActivationType::kRELU, index);
    	}

	    layer->setPrecision(DataType::kHALF);
	    l++;
        input = layer->getOutput(0);
    }

    network->markOutput(*input);
    input->setName(mParams.outputTensorNames[0].c_str());

    TensorFormat mTensorFormat{TensorFormat::kLINEAR};

    network->getInput(0)->setAllowedFormats(static_cast<TensorFormats>(1 << static_cast<int>(mTensorFormat)));
    network->getOutput(0)->setAllowedFormats(static_cast<TensorFormats>(1 << static_cast<int>(mTensorFormat)));

    // Build engine
    builder->setMaxBatchSize(BATCH_SIZE);
    config->setMaxWorkspaceSize(MAX_WORKSPACE_SIZE);
//    config->setFlag(BuilderFlag::kSTRICT_TYPES);
//    config->setFlag(BuilderFlag::kFP16);
	config->setFlags(1 <<  (static_cast<int> (BuilderFlag::kFP16)) | 1<< (static_cast<int> (BuilderFlag::kSTRICT_TYPES)));
    builder->setFp16Mode(true);
    network->getInput(0)->setType(DataType::kHALF);
    network->getOutput(0)->setType(DataType::kHALF);

    samplesCommon::enableDLA(builder.get(), config.get(), mParams.useDLACore);

    builder->allowGPUFallback(false); //just in case

    mEngine = std::shared_ptr<nvinfer1::ICudaEngine>(
        builder->buildEngineWithConfig(*network, *config), samplesCommon::InferDeleter());
    if (!mEngine)
    {
        return false;
    }
        
    return true;
}

//!
//! \brief Runs the TensorRT inference engine for this sample
//!
//! \details This function is the main execution function of the sample. It allocates the buffer,
//!          sets inputs and executes the engine.
//!
bool TestDLA::infer()
{
    // Create RAII buffer manager object
    samplesCommon::BufferManager buffers(mEngine, mParams.batchSize);

    auto context = SampleUniquePtr<nvinfer1::IExecutionContext>(mEngine->createExecutionContext());
    if (!context)
    {
        return false;
    }

    // Read the input data into the managed buffers
    assert(mParams.inputTensorNames.size() == 1);
    if (!processInput(buffers))
    {
        return false;
    }

    SimpleProfiler profiler("Profiler");
    std::chrono::high_resolution_clock::time_point tStart;
    std::chrono::high_resolution_clock::time_point tEnd;
    double cpuTiming;

    // Memcpy from host input buffers to device input buffers
    buffers.copyInputToDevice();
    bool status;


    std::cout << "Start execution on DLA " << std::to_string(mParams.useDLACore) << std::endl;
    if(mParams.profile){
        context->setProfiler(&profiler);
        tStart = std::chrono::high_resolution_clock::now();
    }
    for(int i=0; i<mParams.iterations; i++){
        status = context->execute(mParams.batchSize, buffers.getDeviceBindings().data());
        if (!status)
            return false;
    }
    if(mParams.profile){
        tEnd = std::chrono::high_resolution_clock::now();
        cpuTiming = std::chrono::duration_cast<std::chrono::duration<double>>(tEnd-tStart).count();
        std::stringstream ss;
        ss << "" << profiler;
        std::string profString(ss.str());
        std::size_t found = profString.find("runtime = ");
        if (found!=std::string::npos)
            std::cout << "Profiler time : " << profString.substr(found) << std::endl;
        std::cout << "CPU timing " << cpuTiming << std::endl;
    } 
    std::cout << "End execution on DLA " <<  std::to_string(mParams.useDLACore) << std::endl;

    // Memcpy from device output buffers to host output buffers
    buffers.copyOutputToHost();

    return true;
}

//!
//! \brief Reads the input and stores the result in a managed buffer
//!
bool TestDLA::processInput(const samplesCommon::BufferManager& buffers)
{
    // Read a random digit file
    //srand(unsigned(time(nullptr)));
  
    char* hostDataBuffer = static_cast<char*>(buffers.getHostBuffer(mParams.inputTensorNames[0]));
    /*for (int i = 0; i < mParams.inputH * mParams.inputW; i++)
    {
        mNumber = rand() % mParams.numLayers;
        hostDataBuffer[i] = 1.0 - float(mNumber) / 255.0f;
    }*/
	memset(hostDataBuffer,121,sizeof(float)/2*mParams.inputH*mParams.inputW);

    return true;
}


//!
//! \brief Cleans up any state created in the sample class
//!
bool TestDLA::teardown()
{
    // Release weights host memory
    for (auto& mem : mWeightMap)
    {
        auto weight = mem.second.second;
        {
            delete[] static_cast<const float*>(weight.values);
        }
    }

    return true;
}

//!
//! \brief Loads and initialize weights
//!
//! \details Our weights are garbage values. Only depends on network topology
//!
std::map<std::string, std::pair<nvinfer1::Dims, nvinfer1::Weights>> TestDLA::loadWeights()
{

     if(mParams.layersType==LAYERS_TYPE_CONVOLUTION){
        size_t datasizek = sizeof(float)/2*KERNEL_SIZE_CONV;
        mConvKernel.values = (void*)malloc(datasizek);
        memset((void *) mConvKernel.values,7,datasizek);
        mConvKernel.type = nvinfer1::DataType::kHALF;
        mConvKernel.count = KERNEL_SIZE_CONV*KERNEL_SIZE_CONV;
     }


    std::map<std::string, std::pair<nvinfer1::Dims, nvinfer1::Weights>> weightMap;
 
    for(int l=0; l<mParams.numLayers; l++){
        std::pair<nvinfer1::Dims, nvinfer1::Weights> wt{};

        nvinfer1::Dims3 d(mParams.inputH, mParams.inputW, 1);

        if(mParams.layersType==LAYERS_TYPE_FC){
            d = Dims3(mParams.inputH * mParams.inputW,1,1);
        }

        nvinfer1::Weights w;

        if(mParams.layersType==LAYERS_TYPE_FC)
            w.count = mParams.inputH * mParams.inputW * mParams.inputH * mParams.inputW;
        else w.count =  (l+1) *  mConvKernel.count;

        w.type = nvinfer1::DataType::kHALF;

        const size_t datasize = sizeof(float)/2*w.count;
        w.values = (void *) malloc(datasize);
        memset((void *) w.values,101,datasize);
	    wt.first = d;
	    wt.second = w;
        weightMap.insert(std::pair<std::string,std::pair<nvinfer1::Dims,nvinfer1::Weights>>(std::string("LAYER"+std::to_string(l)),wt));
    }

    return weightMap;
}


nvinfer1::ILayer* TestDLA::addConvLayer(nvinfer1::INetworkDefinition* network, nvinfer1::ITensor& inputTensor,
    int32_t outSize, nvinfer1::Weights wts, nvinfer1::Weights bias, nvinfer1::ActivationType actType, int idx)
{
    //const nvinfer1::Dims3 kd(KERNEL_SIZE_CONV,KERNEL_SIZE_CONV,1);
    std::string baseName("Conv Layer" + (idx == -1 ? "Output" : std::to_string(idx)));
    auto cl = network->addConvolutionNd(inputTensor, outSize, {2, {KERNEL_SIZE_CONV, KERNEL_SIZE_CONV}, {}}, wts, bias);
    //auto cl = network->addConvolutionNd(inputTensor, outSize, kd, wts, bias);
    assert(cl != nullptr);
    std::string clName = baseName + "Convolution" + std::to_string(KERNEL_SIZE_CONV) + "x" + std::to_string(KERNEL_SIZE_CONV);
    cl->setName(clName.c_str());
    cl->setPrecision(DataType::kHALF);
    auto act = network->addActivation(*cl->getOutput(0), actType);
    assert(act != nullptr);
    std::string actName = baseName + "Activation";
    act->setName(actName.c_str());
    return act;
    /*cl->setStride(DimsHW{1, 1});
    // Add max pooling layer with stride of 2x2 and kernel size of 2x2.
    IPoolingLayer* pool = network->addPoolingNd(*cl->getOutput(0), PoolingType::kMAX, Dims{2, {2, 2}, {}});
    assert(pool != nullptr);
    pool->setStride(DimsHW{2, 2});
    return pool;*/
}

//!
//! \brief Add an MLP layer
//!
//! \details
//!     The addMLPLayer function is a simple helper function that creates the combination required for an
//!     MLP layer. By replacing the implementation of this sequence with various implementations, then
//!     then it can be shown how TensorRT optimizations those layer sequences.
//!
nvinfer1::ILayer* TestDLA::addMLPLayer(nvinfer1::INetworkDefinition* network, nvinfer1::ITensor& inputTensor,
    int32_t hiddenSize, nvinfer1::Weights wts, nvinfer1::Weights bias, nvinfer1::ActivationType actType, int idx)
{
    std::string baseName("MLP Layer" + (idx == -1 ? "Output" : std::to_string(idx)));
    auto fc = network->addFullyConnected(inputTensor, hiddenSize, wts, bias);
    assert(fc != nullptr);
    std::string fcName = baseName + "FullyConnected";
    fc->setName(fcName.c_str());
    fc->setPrecision(DataType::kHALF);
    auto act = network->addActivation(*fc->getOutput(0), actType);
    assert(act != nullptr);
    std::string actName = baseName + "Activation";
    act->setName(actName.c_str());
    return act;
}

//!
//! \brief Initializes members of the params struct using the command line args
//! 
TestDLAParams createParams()
{
    TestDLAParams params;
   
    params.inputTensorNames.push_back("input");
    params.batchSize = BATCH_SIZE;
    params.outputTensorNames.push_back("output");
    params.help = false;
    params.numLayers = -1;
    params.inputH = -1;
    params.inputW = -1;
    params.useDLACore = 0;
    params.iterations = 1;
    params.profile = false;
    params.layersType = LAYERS_TYPE_FC;
    params.serModel = "";

    return params;
}


//!
//! \brief Populates the Args struct with the provided command-line parameters.
//!
//! \return boolean If return value is true, execution can continue, otherwise program should exit
//!
inline bool parseArgs(TestDLAParams& args, int argc, char* argv[])
{
    while (1)
    {
        int arg;
        static struct option long_options[] = {{"help", no_argument, 0, 'h'},{"size", required_argument, 0, 's'},
            {"numlayers", required_argument, 0, 'l'}, {"iterations", required_argument, 0, 'i'},
            {"profile", no_argument, 0, 'p'},
            {"useDLACore", required_argument, 0, 'u'},
            {"filein",required_argument,0,'f'}, 
            {nullptr, 0, nullptr, 0}};
        int option_index = 0;
        arg = getopt_long(argc, argv, "hpl:i:u:s:f:", long_options, &option_index);
        if (arg == -1)
        {
            break;
        }

        switch (arg)
        {
        case 'h': args.help = true; return true;
        case 'p': args.profile = true; break;
        case 'i':
            if (optarg)
            {
                args.iterations = std::stoi(optarg);
            }
            break;
        case 'f':
            if(optarg)
            {
                args.serModel = std::string(optarg);
            }
        break;
        case 'u':
            if (optarg)
            {
                args.useDLACore = std::stoi(optarg);
            }
            break;
        case 's':
            if (optarg)
            {
                args.inputH = std::stoi(optarg);
                args.inputW = args.inputH;
            }
            break;
        case 'l':
            if (optarg)
            {
                args.numLayers = std::stoi(optarg);
            }
            break;
        default: return false;
        }
    }
    return true;
}


//!
//! \brief Prints the help information for running this sample
//!
void printHelpInfo()
{
    std::cout << " " << std::endl;
    std::cout << "Usage for building a network: ./testDLA [-h or --help] [-s or --size=<int>] [-l or --numlayers=<int>] [--useDLACore=<int>]" << std::endl;
    std::cout << "--help          Display help information" << std::endl;
    std::cout << "--size=N  Specify the size of each layer the MLP fully connected network in terms of NxN." << std::endl;
    std::cout << "--numlayers=N  Specify the number of layers of the network." << std::endl;
    std::cout << "--useDLACore=N  Specify a DLA engine for layers that support DLA. Value can range from 0 to n-1, "
                 "where n is the number of DLA engines on the platform." << std::endl;
    std::cout << " " << std::endl;
    std::cout << "Usage for loading and running a network: ./testDLA [-h or --help] [-p or --profile] [-f or --filein=<string>] [-i or --iterations=<int>] [--useDLACore=<int>]" << std::endl;
    std::cout << "--profile Activates the per-layer profile info" << std::endl;
    std::cout << "--iterations=N specify how many iterations have to be performed" << std::endl;
    std::cout << "--useDLACore=N  Specify a DLA engine for layers that support DLA. Value can range from 0 to n-1, "
                 "where n is the number of DLA engines on the platform." << std::endl;
}

//!
//! \brief Prints and check an overview of the test configuration
//!
void ccheckArgs(const TestDLAParams *const params){
    std::cout << "SUMMARY:" << std::endl;

    if(params->serModel.compare("")==0){
        std::cout << "Will build and serialize an engine/network such that : " << std::endl;
        std::cout << "Size of each layer: " << params->inputH << "x" << params->inputW << std::endl;
        std::cout << "Num layers: " << params->numLayers << std::endl;
        std::cout << "Use DLA Core: " << params->useDLACore << std::endl;
        //std::cout << "Will " << ((params->profile) ? "" : "not ") << "activate the layer-by-layer profiler" << std::endl;
        //std::cout << "Will perform " << params->iterations << " iterations." << std::endl;
        if( ((params->inputH*params->inputW)<=0)  ||
        (params->numLayers < 1) || (params->useDLACore < 0) ) {
            std::cout << "Error in argument configuration" << std::endl;
            exit(EXIT_FAILURE);
        }
    } else {
        std::cout << "Will load, deserialize and run an engine/network such that : " << std::endl;
        std::cout << "Is parsed from file " << params->serModel << std::endl;
        std::cout << "Will " << ((params->profile) ? "" : "not ") << "activate the layer-by-layer profiler" << std::endl;
        std::cout << "Will perform " << params->iterations << " iterations." << std::endl;
        if(params->iterations<=0){
            std::cout << "Invalid iterations value" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::ifstream infile(params->serModel);
        if(!infile.good()){
            std::cout << "Invalid input file for the model" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char** argv)
{
    TestDLAParams args = createParams();
    bool argsOK = parseArgs(args, argc, argv);
    if (!argsOK)
    {
        gLogError << "Invalid arguments" << std::endl;
        printHelpInfo();
        return EXIT_FAILURE;
    }
    if (args.help)
    {
        printHelpInfo();
        return EXIT_SUCCESS;
    }

    ccheckArgs(&args);
    auto sampleTest = gLogger.defineTest(gSampleName, argc, argv);

    gLogger.reportTestStart(sampleTest);

    TestDLA sample(args);

    if((args.serModel).compare("")==0)
        gLogInfo << "Building the DLA inference engine for the network" << std::endl;
    else gLogInfo << "Running the DLA inference engine for the " << args.serModel << " network" << std::endl;

    if (!sample.build(args.serModel))
    {
        return gLogger.reportFail(sampleTest);
    }

    if(args.serModel.compare("")==0)
    {
        std::cout << "Exiting..." << std::endl;
        return EXIT_SUCCESS;
    }

    if (!sample.infer())
    {
        return gLogger.reportFail(sampleTest);
    }
    if (!sample.teardown())
    {
        return gLogger.reportFail(sampleTest);
    }

    return gLogger.reportPass(sampleTest);
}

