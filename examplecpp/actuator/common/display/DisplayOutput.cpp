#include <stdio.h>
#include <algorithm>

#include "boost/bind.hpp"

#include "common/b2bassert.h"
#include "common/FileUtil.h"
#include "log/B2BLog.h"

#include "drivers/common/display/DisplayOutputHW.h"

#include "DisplayOutput.h"

DisplayOutput::DisplayOutput(DisplayOutputHW *displayHW,
							const char *imageFilePath) :
  ExecuteListInThread("DisplayOutput"),
  Actuator(),
  IMAGE_DISPLAY_TIME_DEF_MS(200),  // default is 200ms display time
  m_imageDisplayTimeMS(0), // INVALID
  m_displayHW(displayHW),
  m_imageFilePath(imageFilePath)
{
	if (displayHW == NULL) {
		// should never happen
		B2BLog::Err(LogFilt::LM_ACTUATOR, "DisplayOutput displayHW is NULL");
	}

	CTORCommon();
}

DisplayOutput::DisplayOutput(const char *imageFilePath) :
    ExecuteListInThread("DisplayOutput"),
    Actuator(),
    IMAGE_DISPLAY_TIME_DEF_MS(200),  // default is 200ms display time
    m_imageDisplayTimeMS(0), // INVALID
    m_displayHW(NULL),
    m_imageFilePath(imageFilePath)
{
    CTORCommon();
}

DisplayOutput::~DisplayOutput()
{
	StopExecuting(ExecuteListType::DONEREASON_STOP, false, true);
	Stop(NULL); // Stop our thread
}

void DisplayOutput::CTORCommon()
{
    SetImageDisplayTime(IMAGE_DISPLAY_TIME_DEF_MS);

    m_BuildExecListInternal =
        boost::bind(&DisplayOutput::BuildExecListInternal, this, _1, _2);
}

bool DisplayOutput::BuildExecListInternal(const ExecList &execList,
							   ExecListInternal *pNewInternalList)
{
	// Display images, and do it asynchronously in thread (Worker)
	std::vector<std::string>::const_iterator iter;
	for (iter = execList.begin(); iter != execList.end(); ++iter) {
		std::string filenameFullPath = m_imageFilePath + "/" + *iter;
    	if (!FileUtil::Exists(filenameFullPath.c_str())) {
			B2BLog::Info(LogFilt::LM_ACTUATOR,
							"DisplayOutputFile cannot open file \"%s\"",
							filenameFullPath.c_str());
			return false; // FAIL: failed to open file
		}
		pNewInternalList->push_back(std::string(filenameFullPath));
	}

	return true; // SUCCESS
}

void DisplayOutput::UseHW(DisplayOutputHW *displayHW){
    m_displayHW=displayHW;
}

bool DisplayOutput::DisplayImageFiles(const std::vector<std::string> &fileList,
				b2b::TimeMS imageDisplayTimeMS, bool loop,
				const ExecuteListType::DoneCallback *pDoneCallback)
{
  	SetImageDisplayTime(imageDisplayTimeMS);

	bool returnVal = ExecuteList("DisplayImageFiles", fileList,
							m_BuildExecListInternal, loop, pDoneCallback);
	if (!returnVal) {
		B2BLog::Err(LogFilt::LM_ACTUATOR, "DisplayImageFiles(%s,%d) failed.",
				fileList[0].c_str(), (int)fileList.size());
	}
	return returnVal;
}

bool DisplayOutput::DisplayImageFiles(const char *dirName,
				b2b::TimeMS imageDisplayTimeMS, bool loop,
				const ExecuteListType::DoneCallback *pDoneCallback)
{
	std::string fullDir = m_imageFilePath + "/" + dirName;
	std::vector<std::string> fileList = FileUtil::ListFilesIn(fullDir.c_str(), dirName);
	std::sort(fileList.begin(), fileList.end()); // sort the files

	return DisplayImageFiles(fileList, imageDisplayTimeMS, loop, pDoneCallback);
}

bool DisplayOutput::WorkerExecute(const std::string &entry)
{
	B2BLog::Debug(LogFilt::LM_ACTUATOR, "Display file %s", entry.c_str());

	// Displaying a file takes time and that is why we are in a
	// separate thread
	if (m_displayHW && m_displayHW->IsReady()) {
		m_displayHW->DisplayImageFile(entry.c_str());
	}

	// Amount of time to display file before moving on
	usleep(m_imageDisplayTimeMS*1000);

	return true;
}

void DisplayOutput::WorkerYield()
{
	usleep(50*1000); // sleep 50ms
}
