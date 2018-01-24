#pragma once

#include "Mesh.h"

class IRobot : public IMesh {
public:
	static IRobot* Create();
};