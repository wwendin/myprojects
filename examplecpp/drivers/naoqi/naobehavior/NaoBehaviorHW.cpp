//
// NAO behavior hardware driver
//
#include <string>

#include "log/B2BLog.h"

#include <alerror/alerror.h>
#include <alcommon/albroker.h>
#include <alproxies/albehaviormanagerproxy.h>

#include "NaoBehaviorHW.h"

bool NaoBehaviorHW::RunBehavior(const char *behaviorName, bool force,
  								 boost::shared_ptr<AL::ALBroker> broker)
{
  if (behaviorName == NULL)
	return false; // Bad name

  	AL::ALBehaviorManagerProxy behMgr(broker);

  	if (behMgr.isBehaviorInstalled(behaviorName)) {
	  
      std::vector<std::string> runList = behMgr.getRunningBehaviors();
      std::vector<std::string>::const_iterator iter;
	  for (iter = runList.begin(); iter != runList.end(); ++iter) {
		std::string runningBehaviorName = *iter;
		B2BLog::Info(LogFilt::LM_DRIVERS, "NAO Behavior \"%s\" is already running.  Stopping it and then we will run \"%s\".", 
							runningBehaviorName.c_str(), behaviorName);
    	behMgr.stopBehavior(runningBehaviorName.c_str());
    	usleep(1 * 1000000); // wait 1 sec for stop (I got 1sec from NAO example code)
      	if (behMgr.isBehaviorRunning(runningBehaviorName.c_str())) {
		  B2BLog::Info(LogFilt::LM_DRIVERS, "NAO Behavior \"%s\" is still running after trying to stop it.%s", 
		  			runningBehaviorName.c_str(), force ? "Ignoring and running requested behavior." : "Giving up.");
		  if (!force)
		  	return false; // FAIL
		}
	  }

      // Launch behaviorName.  This is a BLOCKING call, use post if you do not
      // want to wait for the behavior to finish.
  	  try {
      	behMgr.runBehavior(behaviorName);
  	  } catch (const AL::ALError& e) {
		B2BLog::Err(LogFilt::LM_DRIVERS, 
					"ALBehaviorManagerProxy.runBehavior(%s) call failed %s.", 
										behaviorName, e.what());
		return false; // FAIL
  	  }

    } else {
	  B2BLog::Err(LogFilt::LM_DRIVERS, 
      			"NAO Behavior \"%s\" does not exist.", behaviorName);
	  return false; // FAIL
	}

  return true; // SUCCESS
}

bool NaoBehaviorHW::StopAllBehaviors(boost::shared_ptr<AL::ALBroker> broker)
{
  	AL::ALBehaviorManagerProxy behMgr(broker);

	try {
    	behMgr.stopAllBehaviors();
    	usleep(1 * 1000000); // wait 1 sec for stop (I got 1sec from NAO example code)
  	} catch (const AL::ALError& e) {
		B2BLog::Err(LogFilt::LM_DRIVERS, 
					"ALBehaviorManagerProxy.stopAllBehaviors call failed %s.", 
					e.what());

		return false; // FAIL
  	}

  return true; // SUCCESS
}
