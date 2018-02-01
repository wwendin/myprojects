#include "DisplayOutputHW.h"

DisplayOutputHW::DisplayOutputHW(const char *name) :
	B2BModule(name),
  	m_rotation(ROTATION_NONE),
	m_isReady(false)
{
}

DisplayOutputHW::~DisplayOutputHW()
{
}
