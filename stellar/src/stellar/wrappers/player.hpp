#pragma once

#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "model_instance.hpp"
#include "humanoid.hpp"
#include "part.hpp"
#include "int_value.hpp"
#include "bool_value.hpp"

struct Bones final {
	Part Head;

	Part UpperTorso;
	Part LowerTorso;

	Part LeftUpperArm;
	Part LeftLowerArm;
	Part LeftHand;

	Part RightUpperArm;
	Part RightLowerArm;
	Part RightHand;

	Part LeftUpperLeg;
	Part LeftLowerLeg;
	Part LeftFoot;

	Part RightUpperLeg;
	Part RightLowerLeg;
	Part RightFoot;

	Part& operator[](const std::string& part_name) {
		if (part_name == "Head")
			return Head;
		else if (part_name == "UpperTorso")
			return UpperTorso;
		else if (part_name == "LowerTorso")
			return LowerTorso;
		else if (part_name == "LeftUpperArm")
			return LeftUpperArm;
		else if (part_name == "LeftLowerArm")
			return LeftLowerArm;
		else if (part_name == "LeftHand")
			return LeftHand;
		else if (part_name == "RightUpperArm")
			return RightUpperArm;
		else if (part_name == "RightLowerArm")
			return RightLowerArm;
		else if (part_name == "RightHand")
			return RightHand;
		else if (part_name == "LeftUpperLeg")
			return LeftUpperLeg;
		else if (part_name == "LeftLowerLeg")
			return LeftLowerLeg;
		else if (part_name == "LeftFoot")
			return LeftFoot;
		else if (part_name == "RightUpperLeg")
			return RightUpperLeg;
		else if (part_name == "RightLowerLeg")
			return RightLowerLeg;
		else if (part_name == "RightFoot")
			return RightFoot;
	}
};

class Player : public Instance {
public:
	Instance static_instance;
	ModelInstance static_character;
	Humanoid static_humanoid;
	Part static_root_part;
	Part static_head;
	Bones bones;

	const char* crew;
	BoolValue ko_path;
	IntValue armor_path;

	float health;

	ModelInstance character() const {
		if (!self)
			return ModelInstance();

		return read<ModelInstance>(self + stellar::offsets::model_instance);
	}

	bool operator==(const Player& plr) const {
		return plr.static_instance == this->static_instance;
	}
};

#endif