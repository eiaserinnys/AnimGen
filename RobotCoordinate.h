#pragma once

class Robot;
struct GeneralizedCoordinate;
struct SolutionCoordinate;

struct RobotCoordinate
{
	GeneralizedCoordinate ToGeneralizedCoordinate(Robot* robot) const;
	bool SetTransform(Robot* robot, const GeneralizedCoordinate& coord, bool validate) const;

	SolutionCoordinate ToSolutionCoordinate(Robot* robot) const;
	void SetTransform(Robot* robot, const SolutionCoordinate& solution, bool validate) const;
};