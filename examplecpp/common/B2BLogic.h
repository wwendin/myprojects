#pragma once
//
// Logic abstractions
//

#define B2B_BOOL_TO_STRING(b)  ((b) ? "true" : "false")

namespace B2BLogic {

  /////// Bool3: a 3-state value: false, true, unknown
	typedef enum {
		BOOL3_FALSE = 0,
		BOOL3_TRUE,
		BOOL3_UNKNOWN,
		BOOL3_TOTAL		// size of enum (never used as a valid value)
	} Bool3;

	inline bool Bool3IsTrue(Bool3 val) { return val == BOOL3_TRUE; }
	inline bool Bool3IsFalse(Bool3 val) { return val == BOOL3_FALSE; }

	// Convert a Bool3 to C++ bool
	inline bool Bool3ToBool(Bool3 b) { return Bool3IsTrue(b); }
	// Convert a C++ bool to Bool3
	inline Bool3 BoolToBool3(bool b) { return b ? BOOL3_TRUE : BOOL3_FALSE; }

	const char *Bool3ToString(Bool3 val);

  /////// Level: none, low, medium, high
	typedef enum {
		LEVEL_NONE = 0,
		LEVEL_LOW,
		LEVEL_MEDIUM,
		LEVEL_HIGH,
		LEVEL_TOTAL		// size of enum (never used as a valid value)
	} Level;

	const char *LevelToString(Level val);

	// Returns highest range that value falls into.
	// Here is exact logic, if-else order is critical here:
	//   if value <= thresholds[LOW]: returns TOTAL (no threshold reached)
	//   else if value > thresholds[HIGH]: returns HIGH
	//   else if value > thresholds[MEDIUM]: returns MEDIUM.
	//   else if value > thresholds[LOW]: returns LOW.
	//   else returns TOTAL (no threshold reached) // value <= thresholds[LOW]
	// CORNER CASE: Thresholds are all 0s (for example, this happens when no
	//		thresholds are defined in IOConfig).
	//		value <= 0: returns TOTAL (no threshold reached)
	//		value > 0: returns HIGH (because value > HIGH threshold)
	Level FindLevelFromThresholds(float value, float thresholds[]);
}
