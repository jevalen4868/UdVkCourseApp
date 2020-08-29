#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <assimp/scene.h>

#include "Mesh.h"
using std::vector;

class MeshModel
{
public:
	MeshModel();
	MeshModel(vector<Mesh> meshes);
	~MeshModel();

	size_t getMeshCount();
	Mesh *getMesh(size_t index);

	glm::mat4 getModel();
	void setModel(glm::mat4 model);

	void destroyMeshModel();

	static vector<string> LoadMaterials(const aiScene *scene);
	static std::vector<Mesh> LoadNode(VkPhysicalDevice physDev, VkDevice device, VkQueue transferQueue,
		VkCommandPool transferCommandPool, aiNode *node, const aiScene *scene, vector<int> matToTex);
	static Mesh LoadMesh(VkPhysicalDevice physDev, VkDevice device, VkQueue transferQueue,
		VkCommandPool transferCommandPool, aiMesh *mesh, const aiScene *scene, vector<int> matToTex);

private:
	vector<Mesh> _meshes;
	glm::mat4 _model;
};

