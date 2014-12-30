/**
 * Example application to show using OpenCL in combination with CMake
 *
 * Author: Matthias Bach <matthias.bach@kip.uni-heidelberg.de>
 */

#include <iostream>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

using namespace std;

int main( int, char** )
{
	const cl_uint MAX_PLATFORMS = 256;
	cl_int err;
	
	cl_platform_id platforms[ MAX_PLATFORMS ];
	cl_uint nPlatforms;
		
	err = clGetPlatformIDs( MAX_PLATFORMS, platforms, &nPlatforms );
	if( err )
	{
		cerr << "Failed to get platforms" << endl;
		return -1;
	}

	for( int i = 0; i < nPlatforms; ++i )
	{ 
		char chBuffer[ 256 ];
	
		// Get and log the platform info
		err = clGetPlatformInfo( platforms[ i ], CL_PLATFORM_NAME, sizeof(chBuffer), chBuffer, NULL);
		if( err ) 
		{
			cerr << "Failed to get platform version" << endl;
			return -1;
		}
		cout << chBuffer << " - ";

		// Get and log the platform info
		err = clGetPlatformInfo( platforms[ i ], CL_PLATFORM_VERSION, sizeof(chBuffer), chBuffer, NULL);
		if( err ) 
		{
			cerr << "Failed to get platform version" << endl;
			return -1;
		}
		cout << chBuffer << " - ";
	
		err = clGetPlatformInfo( platforms[ i ], CL_PLATFORM_VENDOR, sizeof(chBuffer), chBuffer, NULL);
		if( err ) 
		{
			cerr << "Failed to get platform version" << endl;
			return -1;
		}
		cout << chBuffer << " - ";

		err = clGetPlatformInfo( platforms[ i ], CL_PLATFORM_PROFILE, sizeof(chBuffer), chBuffer, NULL);
		if( err ) 
		{
			cerr << "Failed to get platform profile" << endl;
			return -1;
		}
		cout << chBuffer << endl;
	}

	return 0;
}
