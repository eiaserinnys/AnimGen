#pragma once

#include "Mesh.h"

class IRobot : public IMesh {
public:
	virtual void Update_Test(DWORD elapsed) = 0;

	static IRobot* Create();
};