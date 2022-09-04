#pragma once
#include <string>
#include <vector>
#include "utils.hpp"

struct Vector2 { 
	int x, y; 

	Vector2(int x = 0, int y = 0) { this->x = x;  this->y = y; } 
};

struct Vector3 {
	int x, y, z;

	Vector3(int x = 0, int y = 0, int z = 0) { this->x = x; this->y = y; this->z = z; } 
};

enum VarType {
	ERRTYPE,
	TFLOAT,
	TSTRING,
	TVECTOR2,
	TVECTOR3,
	TUINT,
	TINT = 9
};


struct Var {
	uint8_t byteVal = 0;
	short shortVal = 0;
	uint16_t ushortVal = 0;
	int intVal = 0;
	uint32_t uintVal = 0;
	float floatVal = 0;
	int64_t longVal = 0;
	uint64_t ulongVal = 0;
	double doubleVal = 0;

	Vector2 vector2Val = 0;
	Vector3 vector3Val = 0;

	std::string strVal;

	VarType vType = VarType::ERRTYPE;
	uint8_t index = 0;

	operator uint8_t() const { return byteVal; }
	operator short() const { return shortVal; }
	operator uint16_t() const { return ushortVal; }
	operator int() const { return intVal; }
	operator uint32_t() const { return uintVal; }
	operator int64_t() const { return longVal; }
	operator uint64_t() const { return ulongVal; }
	operator double() const { return doubleVal; }
	operator std::string() const { return strVal; }
};

namespace Variant {
	class VariantList {
	public:
		uint8_t* Serialize();

		VariantList() {}
		VariantList(void* data); // Load variantlist from data!
		~VariantList() { varvec.clear(); }
		const bool IsValid();
		const std::string GetFuncName();
		const void destroy();
		const Var GetFuncArg(short index);
		const short GetCount();

		int executionDelay = -1;
		int netID = -1;


	private:
		std::vector<Var> varvec;
		uint8_t varcount = 0;
	};
}