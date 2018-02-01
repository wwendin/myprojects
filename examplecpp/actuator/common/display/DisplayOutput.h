#pragma once
//
// DisplayOutput: generic class for display output
//		WARNING: Client must call Init() and Start() to enable this class.
//
#include <stdint.h>
#include <vector>

#include "boost/function.hpp"

#include "common/b2btypes.h"

#include "apps/common/ExecuteListInThread.h"
#include "actuator/common/Actuator.h"

class DisplayOutputHW;

class DisplayOutput : public ExecuteListInThread<std::string, std::string>, public Actuator
{
  public:
	// displayHW: defines the destination display HW
	// filePath: defines directory to search for image files
    DisplayOutput(DisplayOutputHW *displayHW,
				const char *imageFilePath);
	//DisplayOutputHW will be set by UseHW().
    DisplayOutput(const char *imageFilePath);
    virtual ~DisplayOutput();

	// Set DisplayOutputHW.  You can call this any time.
    void UseHW(DisplayOutputHW *displayHW);

	// START: required virtuals from ExecuteListInThread; see that class for doc
	bool Init() { return true; } // nothing special to do here
	// END: required virtuals from ExecuteListInThread

	// DisplayImageFile(filename, ....: Display one image file.
	// 		filename: name of file
	// DisplayImageFiles(fileList, ...): Display image files from a list.
	// 		fileList: list of file names to display
	//		Displayed in order in fileList.
	// DisplayImageFiles(dirName, ...):
	//		Display image files from a directory, dirName.
	// 		dirName: directory of file names to display.   
	//		Files will be displayed in sorted order (by std::sort).  
	//		WARNING: Gunnar says he wants a way to blend between multiple files.
	//		   I think This should be handled by the DisplayOutputHW class.
	//
	// All files or directories are relative to m_imageFilePath directory.
	// PAY ATTENTION: instance of DisplayOutputHW determines what file formats
	//	are supported.  DisplayOutput class does not care what file names or
	//  file formats are used.  DisplayOutput simply passes file names to
	//	DisplayOutputHW and it does all the display work.
	//
	// imageDisplayTimeMS: the amount of time, in milliseconds, to display each
	//		file.  This is implemented via a sleep after each file is written
	//		out to display.  If we only display one file in the methods below,
	//		the sleep is done, thus delaying any future display.
	// loop: If true, loop displaying until StopExecuting is called.
	//		 If false, display the file, fileList, dirName's file only once.
	// pDoneCallback: Contains method to call to report we are done.
	//		See DoneReason for all the reasons we can call this callback.
	//		Only called once per DisplayImageFile/DisplayImageFiles() call.
	//		Set to NULL if no reporting is desired.
	//		The callback is called after the imageDisplayTimeMS sleep.
	//
	// Returns: true on success, false otherwise
	//		Will fail and return false if already in process of displaying
	//		a file or file list
	//
	// Will fail if any filename does not exist.
	bool DisplayImageFile(const char *filename, b2b::TimeMS imageDisplayTimeMS,
				bool loop=false,
				const ExecuteListType::DoneCallback *pDoneCallback=NULL)
	{
		std::vector<std::string> fileList;
		fileList.push_back(filename);
	  	return DisplayImageFiles(fileList, loop, pDoneCallback);
	}
	bool DisplayImageFile(const char *filename, bool loop=false,
					const ExecuteListType::DoneCallback *pDoneCallback=NULL)
	{
		return DisplayImageFile(filename, IMAGE_DISPLAY_TIME_DEF_MS, loop,
						  		pDoneCallback);
	}
	bool DisplayImageFiles(const std::vector<std::string> &fileList,
				b2b::TimeMS imageDisplayTimeMS, bool loop=false,
				const ExecuteListType::DoneCallback *pDoneCallback=NULL);
	bool DisplayImageFiles(const std::vector<std::string> &fileList, bool loop=false,
				const ExecuteListType::DoneCallback *pDoneCallback=NULL)
	{
		return DisplayImageFiles(fileList, IMAGE_DISPLAY_TIME_DEF_MS, loop,
						   		 pDoneCallback);
	}
	bool DisplayImageFiles(const char *dirName,
				b2b::TimeMS imageDisplayTimeMS, bool loop=false,
				const ExecuteListType::DoneCallback *pDoneCallback=NULL);
	bool DisplayImageFiles(const char *dirName, bool loop=false,
				const ExecuteListType::DoneCallback *pDoneCallback=NULL)
	{
		return DisplayImageFiles(dirName, IMAGE_DISPLAY_TIME_DEF_MS, loop,
						   		 pDoneCallback);
	}

	// RETURNS: true if currently "in the act of displaying" an image or image
	// list, false otherwise.
	// "in the act of displaying" means that we are writing out the image (or
	// list) to display and are not done yet.
	bool IsDisplaying() { return IsExecuting(); }

	// Set and Get image display time
	// See DisplayImageFile(s) for description of image display time
	void SetImageDisplayTime(b2b::TimeMS imageDisplayTimeMS)
		{ m_imageDisplayTimeMS = imageDisplayTimeMS; }
	b2b::TimeMS GetImageDisplayTime() const { return m_imageDisplayTimeMS; }

  protected:
	// START: required virtuals from ExecuteListInThread
	bool WorkerExecute(const std::string &entry);
	void WorkerYield();
	// END: required virtuals from ExecuteListInThread

  private:
	void CTORCommon(); // common code used by all ctor

	// This is used with ExecuteListInThread::ExecuteList
	BuildExecListInternal m_BuildExecListInternal;
	bool BuildExecListInternal(const ExecList &execList,
							   ExecListInternal *pNewInternalList);

	// Amount of time to display each file.   This is done as a usleep
	// after displaying every file (even if there is only one file in list).
	const b2b::TimeMS IMAGE_DISPLAY_TIME_DEF_MS;

	b2b::TimeMS m_imageDisplayTimeMS; // set by SetImageDisplayTime()

    DisplayOutputHW *m_displayHW;	// from ctor
	std::string m_imageFilePath;	// from ctor

	// tests need access to privates
	friend class CreatureMenu;
};
