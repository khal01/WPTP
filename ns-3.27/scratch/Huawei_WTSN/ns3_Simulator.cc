//============================================================================
// Name        : Huawei_WTSN.cpp
// Author      : Dennis Krummacker
// Version     :
// Copyright   : 
// Description : C++, Ansi-style
// Description : ns-3 Simulator. Wireless TSN.
// Description : ns-3 Simulation Hub. The Central for the several scripts. It calls, switches, multiplexes.
//============================================================================




#define NO__NS3_SIMULATOR_CC__FUNCTIONS


//////////////////////////////////////////////////////////////////////////////////////////////////////
//==================================================================================================//
//--------------------------------------------------------------------------------------------------//
//--------  Preamble, Inclusions  ------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------//
//==================================================================================================//
//==================================================================================================//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//==================================================================================================//
//---  The very first intercepted mixed 'Plain C' & 'C++' Part  ------------------------------------//
//==================================================================================================//
//////////////////////////////////////////////////////////////////////////////////////////////////////
// At first include this ----------------------------------------------------------
//---------------------------------------------------------------------------------
extern "C"{
	#include "./global/global_settings.h"
	#include "./global/global_variables.h"
}
// Need this for e.g. some really fundamental Program Basics (if any)  ------------
//---------------------------------------------------------------------------------
#include "Huawei_WTSN_base.hpp"
// Then better start with this  ---------------------------------------------------
//---------------------------------------------------------------------------------
#include "Huawei_WTSN.hpp"
//==================================================================================================//
//==================================================================================================//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------//
//==================================================================================================//
//////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" {////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//==================================================================================================//
	//---  The 'Plain C' Part  -------------------------------------------------------------------------//
	//==================================================================================================//
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//--------------------------------------------------------------------------------------------------//
	//Just to nicely keep order:  -----------------------------------------------------
	//   First include the System / Third-Party Headers  ------------------------------
	//   Format: Use <NAME> for them  -------------------------------------------------
	//---------------------------------------------------------------------------------
	//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	// . . . .  System . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	#include <limits.h>
//	#include <stdbool.h>
//	#include <errno.h>
//	#include <stdio.h>
//	#include <stdlib.h>
//	#include <ctype.h>
//	#include <string.h>
//	#include <linux/types.h>
//	#include <sys/types.h>
//	#include <stdint.h>
//	#include <unistd.h>
	//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	// . . . .  Third Party  . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		//- . - . - Do not use the ns-3 Headers here, they are C++ Headers - . - . -//
		//#include "ns3/core-module.h"
		//#include "ns3/network-module.h"
		//#include "ns3/internet-module.h"
		//#include "ns3/point-to-point-module.h"
		//#include "ns3/applications-module.h"
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//#include "nl80211.h"
	//==================================================================================================//
	//==================================================================================================//
	//==================================================================================================//
	//Then include own Headers  -------------------------------------------------------
	//   Format: Use "NAME" for them  -------------------------------------------------
	//---------------------------------------------------------------------------------
	//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	// . . . .  DenKr_essentials . . . . . . . . . . . . . . . . . . . . . . . . . . .
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	#ifndef NO__DENKR_ESSENTIALS // See "global/global_settings.h"
		#include "DenKr_essentials/DenKr_errno.h"
		#include "DenKr_essentials/PreC/DenKr_PreC.h"
		#include "DenKr_essentials/auxiliary.h"
		#ifdef DENKR_ESSENTIALS_AUXILIARY_H
			//#include "DenKr_essentials/Program_Files/control_cfg_file.h"
			//#include "DenKr_essentials/multi_threading.h"
			//#include "DenKr_essentials/plugins/DL_Libs.h"
		#endif
	#endif
	//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	// . . . .  Own Program proprietary  . . . . . . . . . . . . . . . . . . . . . . .
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	#include "Huawei_WTSN_.h"
	//==================================================================================================//
	//==================================================================================================//
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//--------------------------------------------------------------------------------------------------//
	//==================================================================================================//
	//////////////////////////////////////////////////////////////////////////////////////////////////////
}///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//==================================================================================================//
//---  The 'C++' Part  -----------------------------------------------------------------------------//
//==================================================================================================//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------//
//Just to nicely keep order:  -----------------------------------------------------
//   First include the System / Third-Party Headers  ------------------------------
//   Format: Use <NAME> for them  -------------------------------------------------
//---------------------------------------------------------------------------------
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
// . . . .  System . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include <iostream>
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
// . . . .  Third Party  . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include "ns3/core-module.h"
//#include "ns3/network-module.h"
//#include "ns3/internet-module.h"
//#include "ns3/point-to-point-module.h"
//#include "ns3/applications-module.h"
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//#include <>
//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//Then include own Headers  -------------------------------------------------------
//   Format: Use "NAME" for them  -------------------------------------------------
//---------------------------------------------------------------------------------
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
// . . . .  DenKr_essentials . . . . . . . . . . . . . . . . . . . . . . . . . . .
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifndef NO__DENKR_ESSENTIALS // See "global/global_settings.h"
	//#include "DenKr_essentials/auxiliary.hpp"
#endif
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
// . . . .  Own Program proprietary  . . . . . . . . . . . . . . . . . . . . . . .
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include "ns3_Simulator_modules.hpp"
//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
// Namespaces  --------------------------------------------------------------------
//---------------------------------------------------------------------------------
using namespace std;
using namespace ns3;
//==================================================================================================//
//==================================================================================================//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------//
//==================================================================================================//
//////////////////////////////////////////////////////////////////////////////////////////////////////





//#######################################################################################################################################
//##########========================================================================================================================#####
//##########  Notices, Informations, Further Reading  -------------------------------------------------------------------------=====#####
//##########========================================================================================================================#####
//#######################################################################################################################################
//
//  https://www.nsnam.org/docs/models/html/spectrum.html
//
//##########========================================================================================================================#####
//#######################################################################################################################################
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////







NS_LOG_COMPONENT_DEFINE ("Huawei_WTSN - ns-3 Simulator Hub");

int
ns3_Simulator__main (int argc, char **argv)
{
	int err=0;




    //==================================================================================================//
    //--------------------------------------------------------------------------------------------------//
    //----  ns-3 Setup  --------------------------------------------------------------------------------//
    //--------------------------------------------------------------------------------------------------//
    //==================================================================================================//
//	NS_LOG_UNCOND ("\n<-- Wireless TSN Simulator started. -->");
//	NS_LOG_UNCOND ("<------------------------------------->\n\n");
	std::cout << std::flush;
	printfc(cyan,"\n<-- Wireless TSN Simulator started. -->\n");
	printfc(cyan,"<------------------------------------->\n\n\n");
	std::cout << std::flush;





    //==================================================================================================//
    //--------------------------------------------------------------------------------------------------//
    //----  Start of the Command-Line Multiplexing  ----------------------------------------------------//
    //--------------------------------------------------------------------------------------------------//
    //==================================================================================================//





    //==================================================================================================//
    //--------------------------------------------------------------------------------------------------//
    //----  Config-Files  ------------------------------------------------------------------------------//
    //--------------------------------------------------------------------------------------------------//
    //==================================================================================================//







//=======================================================================================================================================
//=======================================================================================================================================
//=======================================================================================================================================
//----------  Temporary Testing Section  --------------------------------------------------------------------------------------==========
//=======================================================================================================================================
//=======================================================================================================================================
#ifdef DEBUG //----------------------------------------------------------------------------------------------------------------==========
#ifndef RELEASE_VERSION //Just to get sure... ;o) that nothing got forgotten within the final Build  --------------------------==========
//=======================================================================================================================================









//exit(1);

//=======================================================================================================================================
#endif //----------------------------------------------------------------------------------------------------------------------==========
#endif //----------------------------------------------------------------------------------------------------------------------==========
//=======================================================================================================================================
//=======================================================================================================================================
//----------  End  -  Temporary Testing Section  ------------------------------------------------------------------------------==========
//=======================================================================================================================================
//=======================================================================================================================================
//=======================================================================================================================================









    //==================================================================================================//
    //--------------------------------------------------------------------------------------------------//
    //----  Main Operation  ----------------------------------------------------------------------------//
    //--------------------------------------------------------------------------------------------------//
    //==================================================================================================//
//	wifi_sim_1__main(argc, argv);
	wifi_sim_PERR__main(argc, argv);







    //==================================================================================================//
    //--------------------------------------------------------------------------------------------------//
    //----  Program-Termination Sequence  --------------------------------------------------------------//
    //--------------------------------------------------------------------------------------------------//
    //==================================================================================================//
//	NS_LOG_UNCOND ("\n\n>----------------------------------------<");
//	NS_LOG_UNCOND ("--> Wireless TSN Simulator terminated. <--\n");
	std::cout << std::flush;
	printfc(cyan,"\n\n>----------------------------------------<\n");
	printfc(cyan,"--> Wireless TSN Simulator terminated. <--\n\n");
	std::cout << std::flush;

	return err;
}









#undef NO__NS3_SIMULATOR_CC__FUNCTIONS
