#pragma once

class Robot;
struct GeneralCoordinate;
struct SolutionCoordinate;

struct RobotCoordinate
{
	GeneralCoordinate ToGeneralCoordinate(Robot* robot) const;
	bool SetTransform(Robot* robot, const GeneralCoordinate& coord, bool validate) const;

	SolutionCoordinate ToSolutionCoordinate(Robot* robot) const;
	void SetTransform(Robot* robot, const SolutionCoordinate& solution, bool validate) const;
};