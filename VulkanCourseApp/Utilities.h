#pragma once

// Indices (locations) of Queue Families (if the exist at all)
struct QueueFamilyIndices {
	int graphicsFamily{ -1 }; // location of graphics queue family.
	// Check if queue families are valid.
	bool isValid() {
		return graphicsFamily >= 0;
	}
};