#pragma once

#include <vector>
#include <fstream>

using std::vector;
using std::string;
using std::ifstream;
using std::ios;

const int MAX_FRAME_DRAWS = 2;

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

static vector<char> readFile(const string &filename) {
	// Open stream from given file.
	// ios::binary tells stream to read as binary, ios::ate tells stream to start reading from the end.
	ifstream file(filename, ios::binary | ios::ate);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file=" + filename);
	}

	// Get current read position and use to resize file buffer.
	size_t fileSize = (size_t)file.tellg();
	vector<char> fileBuffer(fileSize);
	file.seekg(0); // seek to the start of the file.

	// Read the file data into the buffer (stream "fileSize" in total).
	file.read(fileBuffer.data(), fileSize);

	file.close();

	return fileBuffer;
}