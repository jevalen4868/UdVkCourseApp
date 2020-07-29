#pragma once

#include <vector>

using std::vector;

const vector<const char *> DEVICE_EXTENSIONS{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

// Indices (locations) of Queue Families (if the exist at all)
struct QueueFamilyIndices {
	int graphicsFamily{ -1 }; // location of graphics queue family.
	int presentationFamily{ -1 }; // Location of pres queue family.
	// Check if queue families are valid.
	bool isValid() {
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};

struct SwapchainDetails {
	VkSurfaceCapabilitiesKHR surfaceCapabilities; // Surface properties.
	vector<VkSurfaceFormatKHR> formats; // Surface image formats.
	vector<VkPresentModeKHR> presentationModes; // How images should be presented to screen.
};

struct SwapchainImage {
	VkImage image;
	VkImageView imageView;
};