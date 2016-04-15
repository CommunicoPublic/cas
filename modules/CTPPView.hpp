/*
 * CTPPView.hpp
 *
 *  Created on: 25.01.2012
 *      Author: e.kurganov
 */

#ifndef __ASCTPPVIEW_HPP_
#define __ASCTPPVIEW_HPP_
#include <CDT.hpp>
#include <CTPP2ErrorCodes.h>
#include <CTPP2VM.hpp>
#include <CTPP2VMDumper.hpp>

#include "ASDebugHelper.hpp"
#include "ASLoadableObject.hpp"
#include "ASLoader.hpp"
#include "ASCTPPLogger.hpp"
#include "ASLogger.hpp"
#include "ASTemplate.hpp"
#include "ASUtil.hpp"
#include "ASView.hpp"



using namespace CAS;

namespace TEST_NS {
/**
 @struct FunctionConfig
 @brief Function configuration
 */
struct FunctionConfig {
	/** Function name          */
	STLW::string name;
	/** Library name           */
	STLW::string library;
	/** Function configuration */
	CTPP::CDT config;
	/** User-defined function  */
	CTPP::SyscallHandler * udf;

	/**
	 @brief Constructor
	 */
	FunctionConfig();
};

/**
 @class CTPPView
 @brief Application server object
 */
class CTPPView: public ASView {
public:
	/**
	 @brief A virtual destructor
	 */
	~CTPPView() throw ();

	/**
	 @brief Constructor
	 */
	CTPPView();

private:
	/** Argument stack size    */
	UINT_32 iArgStackSize;
	/** Code stack size        */
	UINT_32 iCodeStackSize;
	/** Execution steps limit  */
	UINT_32 iStepsLimit;
	/** Function factory       */
	CTPP::SyscallFactory * pSyscallFactory;
	/** CTPP Virtual machine   */
	CTPP::VM * pVM;
	/** Dynamic loader         */
	ASLoader<CTPP::SyscallHandler> * pLoader;
	/** Functions              */
	STLW::vector<FunctionConfig> vFunctions;
	/** Include directories    */
	STLW::vector<STLW::string> vIncludeDirs;
	/** List of templates      */
	STLW::map<STLW::string, ASTemplate *> mTemplates;
	/**
	 @brief Get handler name
	 */
	CCHAR_P GetObjectName() const;

	/**
	 @brief Initialize module
	 @param oConfiguration - module configuration
	 @param oModulesPool - pool of modules
	 @param oObjectPool - pool of objects
	 @param oLogger - logger object
	 @return 0 - if success, -1 - otherwise
	 */
	INT_32 InitModule(CTPP::CDT & oConfiguration, ASPool & oModulesPool,
			ASPool & oObjectPool, ASLogger & oLogger);
	/**
	 @brief Parse template
	 @param sTemplateName - template name
	 @param oLogger - application server logger
	 @return VIEW_OK - if success, VIEW_ERROR - if any error occured
	 */
	ASTemplate * ParseTemplate(const STLW::string & sTemplateName,
			ASLogger & oLogger);
protected:

	/**
	 @brief Write HTTP response
	 @param oData - data model
	 @param oResponse - response object
	 @param pResponseWriter - response writer object
	 @param oGlobalPool - global pool of objects
	 @param oVhostPool - virtual host pool
	 @param oRequestPool - requeset data pool
	 @param pContextData - context data
	 @param oIMC - Inter-Module communication object
	 @param oLogger - application server logger
	 @return VIEW_OK - if success, VIEW_ERROR - if any error occured
	 */
	INT_32 WriteResponse(CTPP::CDT & oData, ASResponse & oResponse,
			ASResponseWriter & pResponseWriter, ASPool & oGlobalPool,
			ASPool & oVhostPool, ASPool & oRequestPool,
			CTPP::CDT & oConfiguration, ASObject * pContextData,
			CTPP::CDT & oIMC, ASLogger & oLogger);

};

}

#endif /* CTPPVIEW_HPP_ */
