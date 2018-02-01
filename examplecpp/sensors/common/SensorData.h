#pragma once

#include "common/b2btypes.h"
#include "neural/NeuralData.h"

class SensorData : public NeuralData{
public:
	SensorData(const char *source) : NeuralData(source) {}
	SensorData() : NeuralData() {}
	virtual ~SensorData();
};
