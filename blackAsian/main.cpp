/*----------------------------------------------------------------------------
*
* Author:   Liang Ma (liang-ma@polito.it)
*
* Host code defining all the parameters and launching the kernel. 
* The only command line argument is the name of the kernel.
*
* ML_cl.h is the OpenCL library file <CL/cl.hpp>. Currently the version shipped with SDAccel is buggy.
*
* Exception handling is enabled (__CL_ENABLE_EXCEPTIONS) to make host code simpler.
*
* The global and local size are set to 1 since the kernel is written in C/C++ instead of OpenCL.
*
* Several input parameters for the simulation are defined in namespace Paras.
*
* S0:		-s stock price at time 0
* K:		-k strike price
* rate:		-r interest rate
* volatility:	-v volatility of stock
* T:		-t time period of the option
*
*
* callR:	-c reference value for call price
* putR:		-p reference value for put price
*
*----------------------------------------------------------------------------
*/

#define __CL_ENABLE_EXCEPTIONS

// This should be used when cl.hpp from SDAccel works.
#ifdef CL_HEADER_BUG_FIXED
#include <CL/cl.hpp>
#else
#include "../common/ML_cl.h"
#endif

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <unistd.h>
using namespace std;

namespace Paras 
{
	double S0 = 100;		// -s
	double K = 110;			// -k
	double rate = 0.05;   		// -r
	double volatility = 0.2;	// -v
	double T = 1.0;			// -t
}

int main(int argc, char** argv)
{
	int opt;
	double callR=-1, putR=-1;
	bool flaga=false,flags=false,flagk=false,
		flagr=false,flagv=false,flagt=false,
		flagc=false,flagp=false;
	char *fPos=NULL;
	while((opt=getopt(argc,argv,"a:s:k:r:v:t:c:p:"))!=-1){
		switch(opt){
			case 'a':
				fPos=optarg;
				flaga=true;
				break;
			case 's':
				Paras::S0=atof(optarg);
				flags=true;
				break;
			case 'k':
				Paras::K=atof(optarg);
				flagk=true;
				break;
			case 'r':
				Paras::rate=atof(optarg);
				flagr=true;
				break;
			case 'v':
				Paras::volatility=atof(optarg);
				flagv=true;
				break;
			case 't':
				Paras::T=atof(optarg);
				flagt=true;
				break;
			case 'c':
				callR=atof(optarg);
				flagc=true;
				break;
			case 'p':
				putR=atof(optarg);
				flagp=true;
				break;
			default:
				cout<<"Usage"<<argv[0]
					<<" -a *.xclbin"
					<<" -s stockPrice"
					<<" -k strikePrice"
					<<" -r rate"
					<<" -v volitility"
					<<" -t time"
					<<" [-c call price]"
					<<" [-p put price]"
					<<endl;
				return -1;
		}
	}
	if(!(flaga&&flags&&flagk&&flagr&&flagv&&flagt))
	{
		cout<<"Usage"<<argv[0]
			<<" -a *.xclbin"
			<<" -s stockPrice"
			<<" -k strikePrice"
			<<" -r rate"
			<<" -v volitility"
			<<" -t time"
			<<endl;
		return -1;
	}
	ifstream ifstr(fPos); 
	const string programString(istreambuf_iterator<char>(ifstr),
		(istreambuf_iterator<char>()));
	vector<float> h_call(1),h_put(1);

	try
	{
		vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);

		cl::Context context(CL_DEVICE_TYPE_ACCELERATOR);
		vector<cl::Device> devices=context.getInfo<CL_CONTEXT_DEVICES>();

		cl::Program::Binaries binaries(1, make_pair(programString.c_str(), programString.length()));
		cl::Program program(context,devices,binaries);
		try
		{
			program.build(devices);
		}
		catch (cl::Error err)
		{
			if (err.err() == CL_BUILD_PROGRAM_FAILURE)
			{
				string info;
				program.getBuildInfo(devices[0],CL_PROGRAM_BUILD_LOG, &info);
				cout << info << endl;
				return EXIT_FAILURE;
			}
			else throw err;
		}

		cl::CommandQueue commandQueue(context, devices[0]);
		
		typedef cl::make_kernel<cl::Buffer,cl::Buffer,float,float,float,float,float> kernelType;
		kernelType kernelFunctor = kernelType(program, "blackAsian");

		cl::Buffer d_call = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(float));
		cl::Buffer d_put  = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(float));

		cl::EnqueueArgs enqueueArgs(commandQueue,cl::NDRange(1),cl::NDRange(1));
		cl::Event event = kernelFunctor(enqueueArgs,
						d_call,d_put,
						Paras::T,
						Paras::rate,
				 		Paras::volatility,
						Paras::S0,
						Paras::K
						);

		commandQueue.finish();
		event.wait();

		cl::copy(commandQueue, d_call, h_call.begin(), h_call.end());
		cl::copy(commandQueue, d_put, h_put.begin(), h_put.end());
		cout<<"the call price is:"<<h_call[0]<<'\t';
		if(flagc)
			cout<<"the difference with the reference value is "<<fabs(h_call[0]/callR-1)*100<<'%'<<endl;
		cout<<endl;
		cout<<"the put price is:"<<h_put[0]<<'\t';
		if(flagp)
			cout<<"the difference with the reference value is "<<fabs(h_put[0]/putR-1)*100<<'%'<<endl;
		cout<<endl;
	}
	catch (cl::Error err)
	{
		cerr
			<< "Error:\t"
			<< err.what()
			<< endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
