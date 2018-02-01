//
// Audio hardware driver for the following OS: NAOqi
//
#include <string>

#include "log/B2BLog.h"

#include <alerror/alerror.h>
#include <alcommon/albroker.h>
#include <alproxies/alledsproxy.h>

#include "drivers/naoqi/BoardIO.h"

#include "LedOutputHW.h"

LedOutputHW::LedOutputHW(boost::shared_ptr<AL::ALBroker> broker) :
	B2BModule("LedOutputHW"),
	m_broker(broker)
		{

		}


bool LedOutputHW::FadeRGB(const std::string& groupName,const std::string& colorName,
								 const float duration)
{
  AL::ALLedsProxy leds(m_broker);

  leds.fadeRGB(groupName, colorName, duration);

	return false;
}
